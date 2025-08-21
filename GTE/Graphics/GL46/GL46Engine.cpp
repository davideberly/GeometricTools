// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.08.20

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/FontArialW400H18.h>
#include <Graphics/GL46/GL46Engine.h>
#include <Graphics/GL46/GL46AtomicCounterBuffer.h>
#include <Graphics/GL46/GL46BlendState.h>
#include <Graphics/GL46/GL46ConstantBuffer.h>
#include <Graphics/GL46/GL46DepthStencilState.h>
#include <Graphics/GL46/GL46DrawTarget.h>
#include <Graphics/GL46/GL46IndexBuffer.h>
#include <Graphics/GL46/GL46RasterizerState.h>
#include <Graphics/GL46/GL46SamplerState.h>
#include <Graphics/GL46/GL46StructuredBuffer.h>
#include <Graphics/GL46/GL46Texture1.h>
#include <Graphics/GL46/GL46Texture1Array.h>
#include <Graphics/GL46/GL46Texture2.h>
#include <Graphics/GL46/GL46Texture2Array.h>
#include <Graphics/GL46/GL46Texture3.h>
#include <Graphics/GL46/GL46TextureCube.h>
#include <Graphics/GL46/GL46TextureCubeArray.h>
#include <Graphics/GL46/GL46VertexBuffer.h>
#include <Graphics/GL46/GLSLProgramFactory.h>
#include <Graphics/GL46/GLSLComputeProgram.h>
#include <Graphics/GL46/GLSLVisualProgram.h>
using namespace gte;

GL46Engine::GL46Engine()
    :
    mMajor(0),
    mMinor(0),
    mMeetsRequirements(false)
{
    // Initialization of GraphicsEngine members that depend on GL46.
    mILMap = std::make_unique<GL46InputLayoutManager>();

    mCreateGEObject =
    {
        nullptr, // GT_GRAPHICS_OBJECT (abstract base)
        nullptr, // GT_RESOURCE (abstract base)
        nullptr, // GT_BUFFER (abstract base)
        &GL46ConstantBuffer::Create,
        nullptr, // &DX11TextureBuffer::Create,
        &GL46VertexBuffer::Create,
        &GL46IndexBuffer::Create,
        &GL46StructuredBuffer::Create,
        nullptr, // TODO:  Implement TypedBuffer
        nullptr, // &DX11RawBuffer::Create,
        nullptr, // &DX11IndirectArgumentsBuffer::Create,
        nullptr, // GT_TEXTURE (abstract base)
        nullptr, // GT_TEXTURE_SINGLE (abstract base)
        &GL46Texture1::Create,
        &GL46Texture2::Create,
        &GL46TextureRT::Create,
        &GL46TextureDS::Create,
        &GL46Texture3::Create,
        nullptr, // GT_TEXTURE_ARRAY (abstract base)
        &GL46Texture1Array::Create,
        &GL46Texture2Array::Create,
        &GL46TextureCube::Create,
        &GL46TextureCubeArray::Create,
        nullptr, // GT_SHADER (abstract base)
        nullptr, // &DX11VertexShader::Create,
        nullptr, // &DX11GeometryShader::Create,
        nullptr, // &DX11PixelShader::Create,
        nullptr, // &DX11ComputeShader::Create,
        nullptr, // GT_DRAWING_STATE (abstract base)
        &GL46SamplerState::Create,
        &GL46BlendState::Create,
        &GL46DepthStencilState::Create,
        &GL46RasterizerState::Create
    };

    mCreateGEDrawTarget = &GL46DrawTarget::Create;
}

void GL46Engine::CreateDefaultFont()
{
    std::shared_ptr<GLSLProgramFactory> factory = std::make_shared<GLSLProgramFactory>();
    mDefaultFont = std::make_shared<FontArialW400H18>(factory, 256);
    SetDefaultFont();
}

void GL46Engine::DestroyDefaultFont()
{
    if (mDefaultFont)
    {
        mDefaultFont = nullptr;
        mActiveFont = nullptr;
    }
}

bool GL46Engine::Initialize(int32_t requiredMajor, int32_t requiredMinor, bool, bool saveDriverInfo)
{
    if (saveDriverInfo)
    {
        InitializeOpenGL(mMajor, mMinor, "OpenGLDriverInfo.txt");
    }
    else
    {
        InitializeOpenGL(mMajor, mMinor, nullptr);
    }

    mMeetsRequirements = (mMajor > requiredMajor ||
        (mMajor == requiredMajor && mMinor >= requiredMinor));

    if (mMeetsRequirements)
    {
        SetViewport(0, 0, mXSize, mYSize);
        SetDepthRange(0.0f, 1.0f);
        CreateDefaultGlobalState();
        CreateDefaultFont();
        return mMeetsRequirements;
    }
    else
    {
        std::string message = "OpenGL " + std::to_string(requiredMajor) + "."
            + std::to_string(requiredMinor) + " is required.";
        LogError(message);
    }
}

void GL46Engine::Terminate()
{
    // The render state objects (and fonts) are destroyed first so that the
    // render state objects are removed from the bridges before they are
    // cleared later in the destructor.
    DestroyDefaultFont();
    DestroyDefaultGlobalState();

    // Need to remove all the RawBuffer objects used to manage atomic
    // counter buffers.
    mAtomicCounterRawBuffers.clear();

    GraphicsObject::UnsubscribeForDestruction(mGOListener);
    mGOListener = nullptr;

    DrawTarget::UnsubscribeForDestruction(mDTListener);
    mDTListener = nullptr;

    mGOMapMutex.lock();
    if (mGOMap.size() > 0)
    {
        // Bridge map is nonempty on destruction.
        // TODO: In GTL, handle differently. The condition should not occur.
        mGOMap.clear();
    }
    mGOMapMutex.unlock();

    mDTMapMutex.lock();
    if (mDTMap.size() > 0)
    {
        // Draw target map nonempty on destruction.
        // TODO: In GTL, handle differently. The condition should not occur.
        mDTMap.clear();
    }
    mDTMapMutex.unlock();

    if (mILMap->HasElements())
    {
        // Input layout map nonempty on destruction.
        // TODO: In GTL, handle differently. The condition should not occur.
        mILMap->UnbindAll();
    }
    mILMap = nullptr;
}

uint64_t GL46Engine::DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer)
{
    uint32_t numActiveVertices = vbuffer->GetNumActiveElements();
    uint32_t vertexOffset = vbuffer->GetOffset();

    uint32_t numActiveIndices = ibuffer->GetNumActiveIndices();
    uint32_t indexSize = ibuffer->GetElementSize();
    GLenum indexType = (indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT);

    GLenum topology = 0;
    uint32_t type = ibuffer->GetPrimitiveType();
    switch (type)
    {
    case IP_POLYPOINT:
        topology = GL_POINTS;
        break;
    case IP_POLYSEGMENT_DISJOINT:
        topology = GL_LINES;
        break;
    case IP_POLYSEGMENT_CONTIGUOUS:
        topology = GL_LINE_STRIP;
        break;
    case IP_TRIMESH:
        topology = GL_TRIANGLES;
        break;
    case IP_TRISTRIP:
        topology = GL_TRIANGLE_STRIP;
        break;
    case IP_POLYSEGMENT_DISJOINT_ADJ:
        topology = GL_LINES_ADJACENCY;
        break;
    case IP_POLYSEGMENT_CONTIGUOUS_ADJ:
        topology = GL_LINE_STRIP_ADJACENCY;
        break;
    case IP_TRIMESH_ADJ:
        topology = GL_TRIANGLES_ADJACENCY;
        break;
    case IP_TRISTRIP_ADJ:
        topology = GL_TRIANGLE_STRIP_ADJACENCY;
        break;
    default:
        LogError("Unknown primitive topology = " + std::to_string(type));
    }

    GLuint occlusionQuery = 0;
    uint64_t numPixelsDrawn = 0;
    if (mAllowOcclusionQuery)
    {
        occlusionQuery = BeginOcclusionQuery();
    }

    if (ibuffer->IsIndexed())
    {
        uint32_t offset = ibuffer->GetOffset();
        void const* data = reinterpret_cast<void const*>(static_cast<size_t>(indexSize) * static_cast<size_t>(offset));
        glDrawRangeElements(topology, 0, numActiveVertices - 1,
            static_cast<GLsizei>(numActiveIndices), indexType, data);
    }
    else
    {
        // From the OpenGL documentation on gl_VertexID vertex shader
        // variable:  "gl_VertexID is a vertex language input variable that
        // holds an integer index for the vertex. The index is implicitly
        // generated by glDrawArrays and other commands that do not reference
        // the content of the GL_ELEMENT_ARRAY_BUFFER, or explicitly generated
        // from the content of the GL_ELEMENT_ARRAY_BUFFER by commands such as
        // glDrawElements."
        glDrawArrays(topology, static_cast<GLint>(vertexOffset),
            static_cast<GLint>(numActiveVertices));
    }

    if (mAllowOcclusionQuery)
    {
        numPixelsDrawn = EndOcclusionQuery(occlusionQuery);
    }

    return numPixelsDrawn;
}

GLuint GL46Engine::BeginOcclusionQuery()
{
    GLuint occlusionQuery = 0;
    glGenQueries(1, &occlusionQuery);
    if (occlusionQuery > 0)
    {
        glBeginQuery(GL_SAMPLES_PASSED, occlusionQuery);
        return occlusionQuery;
    }

    LogError("glGenQueries failed.");
}

uint64_t GL46Engine::EndOcclusionQuery(GLuint occlusionQuery)
{
    if (occlusionQuery > 0)
    {
        glEndQuery(GL_SAMPLES_PASSED);

        GLint resultAvailable = GL_FALSE;
        while (!resultAvailable)
        {
            glGetQueryObjectiv(occlusionQuery, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);
        }
        GLint samplesPassed = 0;
        glGetQueryObjectiv(occlusionQuery, GL_QUERY_RESULT, &samplesPassed);
        glDeleteQueries(1, &occlusionQuery);
        return static_cast<uint64_t>(samplesPassed);
    }

    LogError("No query provided.");
}

bool GL46Engine::EnableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program)
{
    Shader* vshader = effect->GetVertexShader().get();
    if (!vshader)
    {
        LogError("Effect does not have a vertex shader.");
    }

    Shader* pshader = effect->GetPixelShader().get();
    if (!pshader)
    {
        LogError("Effect does not have a pixel shader.");
    }

    Shader* gshader = effect->GetGeometryShader().get();

    // Enable the shader resources.
    Enable(vshader, program);
    Enable(pshader, program);
    if (gshader)
    {
        Enable(gshader, program);
    }

    return true;
}

void GL46Engine::DisableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program)
{
    Shader* vshader = effect->GetVertexShader().get();
    Shader* pshader = effect->GetPixelShader().get();
    Shader* gshader = effect->GetGeometryShader().get();

    if (gshader)
    {
        Disable(gshader, program);
    }
    Disable(pshader, program);
    Disable(vshader, program);
}

void GL46Engine::Enable(Shader const* shader, GLuint program)
{
    EnableCBuffers(shader, program);
    EnableTBuffers(shader, program);
    EnableSBuffers(shader, program);
    EnableRBuffers(shader, program);
    EnableTextures(shader, program);
    EnableTextureArrays(shader, program);
    EnableSamplers(shader, program);
}

void GL46Engine::Disable(Shader const* shader, GLuint program)
{
    DisableSamplers(shader, program);
    DisableTextureArrays(shader, program);
    DisableTextures(shader, program);
    DisableRBuffers(shader, program);
    DisableSBuffers(shader, program);
    DisableTBuffers(shader, program);
    DisableCBuffers(shader, program);
}

void GL46Engine::EnableCBuffers(Shader const* shader, GLuint program)
{
    int32_t const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        if (cb.object)
        {
            auto gl4CB = static_cast<GL46ConstantBuffer*>(Bind(cb.object));
            if (gl4CB)
            {
                auto const blockIndex = cb.bindPoint;
                if (GL_INVALID_INDEX != static_cast<uint32_t>(blockIndex))
                {
                    auto const unit = mUniformUnitMap.AcquireUnit(program, blockIndex);
                    glUniformBlockBinding(program, blockIndex, unit);
                    gl4CB->AttachToUnit(unit);
                }
            }
            else
            {
                LogError("Failed to bind constant buffer.");
            }
        }
        else
        {
            LogError(cb.name + " is null constant buffer.");
        }
    }
}

void GL46Engine::DisableCBuffers(Shader const* shader, GLuint program)
{
    int32_t const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        auto const blockIndex = cb.bindPoint;
        if (GL_INVALID_INDEX != static_cast<uint32_t>(blockIndex))
        {
            auto const unit = mUniformUnitMap.GetUnit(program, blockIndex);
            glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0);
            mUniformUnitMap.ReleaseUnit(unit);
        }
    }
}

void GL46Engine::EnableTBuffers(Shader const*, GLuint)
{
    // TODO: This function is not yet implemented.
}

void GL46Engine::DisableTBuffers(Shader const*, GLuint)
{
    // TODO: This function is not yet implemented.
}

void GL46Engine::EnableSBuffers(Shader const* shader, GLuint program)
{
    // Configure atomic counter buffer objects used by the shader.
    auto const& atomicCounters = shader->GetData(Shader::AtomicCounterShaderDataLookup);
    auto const& atomicCounterBuffers = shader->GetData(Shader::AtomicCounterBufferShaderDataLookup);
    for (uint32_t acbIndex = 0; acbIndex < atomicCounterBuffers.size(); ++acbIndex)
    {
        auto const& acb = atomicCounterBuffers[acbIndex];

        // Allocate a new raw buffer?
        if (acbIndex >= mAtomicCounterRawBuffers.size())
        {
            mAtomicCounterRawBuffers.push_back(nullptr);
        }

        // Look at the current raw buffer defined at this index.  Could be
        // nullptr if a new location was just inserted.
        auto& rawBuffer = mAtomicCounterRawBuffers[acbIndex];

        // If the raw buffer is not large enough, then unbind old one and
        // ready to create new one.
        if (rawBuffer && (acb.numBytes > static_cast<int32_t>(rawBuffer->GetNumBytes())))
        {
            Unbind(rawBuffer.get());
            rawBuffer = nullptr;
        }

        // Find the currently mapped GL4AtomicCounterBuffer.
        GL46AtomicCounterBuffer* gl4ACB = nullptr;
        if (rawBuffer)
        {
            gl4ACB = static_cast<GL46AtomicCounterBuffer*>(Get(rawBuffer));
        }
        else
        {
            // By definition, RawBuffer contains 4-byte elements.  We do not
            // need CPU-side storage, but we must be able to copy values
            // between buffers.
            rawBuffer = std::make_shared<RawBuffer>((acb.numBytes + 3) / 4, false);
            rawBuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);

            // Do a manual Bind operation because this is a special mapping
            // from RawBuffer to GL4AtomicCounterBuffer.
            auto temp = GL46AtomicCounterBuffer::Create(mGEObjectCreator, rawBuffer.get());
            mGOMapMutex.lock();
            mGOMap.insert(std::make_pair(rawBuffer.get(), temp));
            mGOMapMutex.unlock();
            gl4ACB = static_cast<GL46AtomicCounterBuffer*>(temp.get());
        }

        // TODO: ShaderStorage blocks have a glShaderStorageBlockBinding()
        // call.  Uniform blocks have a glUniforBlockBinding() call.  Is there
        // something equivalent for atomic counters buffers?

        // Bind this atomic counter buffer
        gl4ACB->AttachToUnit(acb.bindPoint);
    }

    int32_t const indexSB = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(indexSB))
    {
        if (sb.object)
        {
            auto gl4SB = static_cast<GL46StructuredBuffer*>(Bind(sb.object));
            if (gl4SB)
            {
                auto const blockIndex = sb.bindPoint;
                if (GL_INVALID_INDEX != static_cast<uint32_t>(blockIndex))
                {
                    auto const unit = mShaderStorageUnitMap.AcquireUnit(program, blockIndex);
                    glShaderStorageBlockBinding(program, blockIndex, unit);

                    // Do not use glBindBufferBase here.  Use AttachToUnit
                    // method in GL4StructuredBuffer.
                    gl4SB->AttachToUnit(unit);

                    // The sb.isGpuWritable flag is used to indicate whether
                    // or not there is atomic counter associated with this
                    // structured buffer.
                    if (sb.isGpuWritable)
                    {
                        // Does the structured buffer counter need to be
                        // reset?
                        gl4SB->SetNumActiveElements();

                        // This structured buffer has index to associated
                        // atomic counter table entry.
                        auto const acIndex = sb.extra;

                        // Where does the associated counter exist in the
                        // shader?
                        auto const acbIndex = atomicCounters[acIndex].bindPoint;
                        auto const acbOffset = atomicCounters[acIndex].extra;

                        // Retrieve the GL4 atomic counter buffer object.
                        auto gl4ACB = static_cast<GL46AtomicCounterBuffer*>(Get(mAtomicCounterRawBuffers[acbIndex]));

                        // Copy the counter value from the structured buffer
                        // object to the appropriate place in the atomic
                        // counter buffer.
                        gl4SB->CopyCounterValueToBuffer(gl4ACB, acbOffset);
                    }
                }
            }
            else
            {
                LogError("Failed to bind structured buffer.");
            }
        }
        else
        {
            LogError(sb.name + " is null structured buffer.");
        }
    }
}

void GL46Engine::DisableSBuffers(Shader const* shader, GLuint program)
{
    // Unbind any atomic counter buffers.
    auto const& atomicCounters = shader->GetData(Shader::AtomicCounterShaderDataLookup);
    auto const& atomicCounterBuffers = shader->GetData(Shader::AtomicCounterBufferShaderDataLookup);
    for (uint32_t acbIndex = 0; acbIndex < atomicCounterBuffers.size(); ++acbIndex)
    {
        auto const& acb = atomicCounterBuffers[acbIndex];
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, acb.bindPoint, 0);
    }

    int32_t const index = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(index))
    {
        if (sb.object)
        {
            auto gl4SB = static_cast<GL46StructuredBuffer*>(Get(sb.object));

            if (gl4SB)
            {
                auto const blockIndex = sb.bindPoint;
                if (GL_INVALID_INDEX != static_cast<uint32_t>(blockIndex))
                {
                    auto const unit = mShaderStorageUnitMap.GetUnit(program, blockIndex);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, 0);
                    mShaderStorageUnitMap.ReleaseUnit(unit);

                    // The sb.isGpuWritable flag is used to indicate whether
                    // or not there is atomic counter associated with this
                    // structured buffer.
                    if (sb.isGpuWritable)
                    {
                        // This structured buffer has index to associated
                        // atomic counter table entry.
                        auto const acIndex = sb.extra;

                        // Where does the associated counter exist in the
                        // shader?
                        auto const acbIndex = atomicCounters[acIndex].bindPoint;
                        auto const acbOffset = atomicCounters[acIndex].extra;

                        // Retrieve the GL4 atomic counter buffer object.
                        auto gl4ACB = static_cast<GL46AtomicCounterBuffer*>(Get(mAtomicCounterRawBuffers[acbIndex]));

                        // Copy the counter value from the appropriate place
                        // in the atomic counter buffer to the structured
                        // buffer object.
                        gl4SB->CopyCounterValueFromBuffer(gl4ACB, acbOffset);
                    }
                }
            }
        }
    }
}

void GL46Engine::EnableRBuffers(Shader const*, GLuint)
{
    // TODO: This function is not yet implemented.
}

void GL46Engine::DisableRBuffers(Shader const*, GLuint)
{
    // TODO: This function is not yet implemented.
}

void GL46Engine::EnableTextures(Shader const* shader, GLuint program)
{
    int32_t const index = TextureSingle::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (!ts.object)
        {
            LogError(ts.name + " is null texture.");
        }

        auto texture = static_cast<GL46TextureSingle*>(Bind(ts.object));
        if (!texture)
        {
            LogError("Failed to bind texture.");
        }

        // By convension, ts.isGpuWritable is true for "image*" and false
        // for "sampler*".
        GLuint handle = texture->GetGLHandle();
        if (ts.isGpuWritable)
        {
            // For "image*" objects in the shader, use "readonly" or
            // "writeonly" attributes in the layout to control R/W/RW access
            // using shader compiler and then connect as GL_READ_WRITE here.
            // Always bind level 0 and all layers.
            GLint unit = mTextureImageUnitMap.AcquireUnit(program, ts.bindPoint);
            glUniform1i(ts.bindPoint, unit);
            uint32_t format = texture->GetTexture()->GetFormat();
            GLuint internalFormat = texture->GetInternalFormat(format);
            glBindImageTexture(unit, handle, 0, GL_TRUE, 0, GL_READ_WRITE, internalFormat);
        }
        else
        {
            GLint unit = mTextureSamplerUnitMap.AcquireUnit(program, ts.bindPoint);
            glUniform1i(ts.bindPoint, unit);
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(texture->GetTarget(), handle);
        }
    }
}

void GL46Engine::DisableTextures(Shader const* shader, GLuint program)
{
    int32_t const index = TextureSingle::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (!ts.object)
        {
            LogError(ts.name + " is null texture.");
        }

        auto texture = static_cast<GL46TextureSingle*>(Get(ts.object));
        if (!texture)
        {
            LogError("Failed to get texture.");
        }

        // By convension, ts.isGpuWritable is true for "image*" and false
        // for "sampler*".
        if (ts.isGpuWritable)
        {
            // For "image*" objects in the shader, use "readonly" or
            // "writeonly" attributes in the layout to control R/W/RW access
            // using shader compiler and then connect as GL_READ_WRITE here.
            // Always bind level 0 and all layers.  TODO: Decide whether
            // unbinding the texture from the image unit is necessary.
            // glBindImageTexture(unit, 0, 0, 0, 0, 0, 0);
            GLint unit = mTextureImageUnitMap.GetUnit(program, ts.bindPoint);
            mTextureImageUnitMap.ReleaseUnit(unit);
        }
        else
        {
            GLint unit = mTextureSamplerUnitMap.GetUnit(program, ts.bindPoint);
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(texture->GetTarget(), 0);
            mTextureSamplerUnitMap.ReleaseUnit(unit);
        }
    }
}

void GL46Engine::EnableTextureArrays(Shader const* shader, GLuint program)
{
    int32_t const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (!ta.object)
        {
            LogError(ta.name + " is null texture array.");
        }

        auto texture = static_cast<GL46TextureArray*>(Bind(ta.object));
        if (!texture)
        {
            LogError("Failed to bind texture array.");
        }

        // By convension, ta.isGpuWritable is true for "image*" and false
        // for "sampler*".
        GLuint handle = texture->GetGLHandle();
        if (ta.isGpuWritable)
        {
            // For "image*" objects in the shader, use "readonly" or
            // "writeonly" attributes in the layout to control R/W/RW access
            // using shader compiler and then connect as GL_READ_WRITE here.
            // Always bind level 0 and all layers.
            GLint unit = mTextureImageUnitMap.AcquireUnit(program, ta.bindPoint);
            glUniform1i(ta.bindPoint, unit);
            uint32_t format = texture->GetTexture()->GetFormat();
            GLuint internalFormat = texture->GetInternalFormat(format);
            glBindImageTexture(unit, handle, 0, GL_TRUE, 0, GL_READ_WRITE, internalFormat);
        }
        else
        {
            GLint unit = mTextureSamplerUnitMap.AcquireUnit(program, ta.bindPoint);
            glUniform1i(ta.bindPoint, unit);
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(texture->GetTarget(), handle);
        }
    }
}

void GL46Engine::DisableTextureArrays(Shader const* shader, GLuint program)
{
    int32_t const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (!ta.object)
        {
            LogError(ta.name + " is null texture array.");
            continue;
        }

        auto texture = static_cast<GL46TextureArray*>(Get(ta.object));
        if (!texture)
        {
            LogError("Failed to get texture array.");
            continue;
        }

        // By convension, ta.isGpuWritable is true for "image*" and false
        // for "sampler*".
        if (ta.isGpuWritable)
        {
            // For "image*" objects in the shader, use "readonly" or
            // "writeonly" attributes in the layout to control R/W/RW access
            // using shader compiler and then connect as GL_READ_WRITE here.
            // Always bind level 0 and all layers.  TODO: Decide whether
            // unbinding the texture from the image unit is necessary.
            // glBindImageTexture(unit, 0, 0, 0, 0, 0, 0);
            GLint unit = mTextureImageUnitMap.GetUnit(program, ta.bindPoint);
            mTextureImageUnitMap.ReleaseUnit(unit);
        }
        else
        {
            GLint unit = mTextureSamplerUnitMap.GetUnit(program, ta.bindPoint);
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(texture->GetTarget(), 0);
            mTextureSamplerUnitMap.ReleaseUnit(unit);
        }
    }
}

void GL46Engine::EnableSamplers(Shader const* shader, GLuint program)
{
    int32_t const index = SamplerState::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Sampler = static_cast<GL46SamplerState*>(Bind(ts.object));
            if (gl4Sampler)
            {
                auto const location = ts.bindPoint;
                auto const unit = mTextureSamplerUnitMap.AcquireUnit(program, location);
                glBindSampler(unit, gl4Sampler->GetGLHandle());
            }
            else
            {
                LogError("Failed to bind sampler.");
            }
        }
        else
        {
            LogError(ts.name + " is null sampler.");
        }
    }
}

void GL46Engine::DisableSamplers(Shader const* shader, GLuint program)
{
    int32_t const index = SamplerState::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Sampler = static_cast<GL46SamplerState*>(Get(ts.object));

            if (gl4Sampler)
            {
                auto const location = ts.bindPoint;
                auto const unit = mTextureSamplerUnitMap.GetUnit(program, location);
                glBindSampler(unit, 0);
                mTextureSamplerUnitMap.ReleaseUnit(unit);
            }
            else
            {
                LogError("Failed to get sampler.");
            }
        }
        else
        {
            LogError(ts.name + " is null sampler.");
        }
    }
}

int32_t GL46Engine::ProgramIndexUnitMap::AcquireUnit(GLint program, GLint index)
{
    int32_t availUnit = -1;
    for (int32_t unit = 0; unit < static_cast<int32_t>(mLinkMap.size()); ++unit)
    {
        auto& item = mLinkMap[unit];

        // Increment link count if already assigned and in use?
        if (program == item.program && index == item.index)
        {
            ++item.linkCount;
            return unit;
        }

        // Found a unit that was previously used but is now available.
        if (0 == item.linkCount)
        {
            if (-1 == availUnit)
            {
                availUnit = unit;
            }
        }
    }

    // New unit number not previously used?
    if (-1 == availUnit)
    {
        // TODO: Consider querying the max number of units and check that
        // this number is not exceeded.
        availUnit = static_cast<int32_t>(mLinkMap.size());
        mLinkMap.push_back({ 0, 0, 0 });
    }

    auto& item = mLinkMap[availUnit];
    item.linkCount = 1;
    item.program = program;
    item.index = index;
    return availUnit;
}

int32_t GL46Engine::ProgramIndexUnitMap::GetUnit(GLint program, GLint index) const
{
    for (int32_t unit = 0; unit < static_cast<int32_t>(mLinkMap.size()); ++unit)
    {
        auto& item = mLinkMap[unit];
        if (program == item.program && index == item.index)
        {
            return unit;
        }
    }
    return -1;
}

void GL46Engine::ProgramIndexUnitMap::ReleaseUnit(uint32_t index)
{
    if (index < mLinkMap.size())
    {
        auto& item = mLinkMap[index];
        if (item.linkCount > 0)
        {
            --item.linkCount;
        }
    }
}

uint32_t GL46Engine::ProgramIndexUnitMap::GetUnitLinkCount(uint32_t unit) const
{
    if (unit < mLinkMap.size())
    {
        return mLinkMap[unit].linkCount;
    }
    return 0;
}

bool GL46Engine::ProgramIndexUnitMap::GetUnitProgramIndex(uint32_t unit, GLint &program, GLint &index) const
{
    if (unit < mLinkMap.size())
    {
        auto& item = mLinkMap[index];
        if (item.linkCount > 0)
        {
            program = item.program;
            index = item.index;
            return true;
        }
    }
    return false;
}

void GL46Engine::SetViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
    glViewport(x, y, w, h);
}

void GL46Engine::GetViewport(int32_t& x, int32_t& y, int32_t& w, int32_t& h) const
{
    int32_t param[4];
    glGetIntegerv(GL_VIEWPORT, param);
    x = param[0];
    y = param[1];
    w = param[2];
    h = param[3];
}

void GL46Engine::SetDepthRange(float zmin, float zmax)
{
    glDepthRange(static_cast<GLdouble>(zmin), static_cast<GLdouble>(zmax));
}

void GL46Engine::GetDepthRange(float& zmin, float& zmax) const
{
    GLdouble param[2];
    glGetDoublev(GL_DEPTH_RANGE, param);
    zmin = static_cast<float>(param[0]);
    zmax = static_cast<float>(param[1]);
}

bool GL46Engine::Resize(uint32_t w, uint32_t h)
{
    mXSize = w;
    mYSize = h;
    int32_t param[4];
    glGetIntegerv(GL_VIEWPORT, param);
    glViewport(param[0], param[1], static_cast<GLint>(w), static_cast<GLint>(h));
    return true;
}

void GL46Engine::ClearColorBuffer()
{
    glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GL46Engine::ClearDepthBuffer()
{
    glClearDepth(mClearDepth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL46Engine::ClearStencilBuffer()
{
    glClearStencil(static_cast<GLint>(mClearStencil));
    glClear(GL_STENCIL_BUFFER_BIT);
}

void GL46Engine::ClearBuffers()
{
    glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
    glClearDepth(mClearDepth);
    glClearStencil(static_cast<GLint>(mClearStencil));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GL46Engine::SetBlendState(std::shared_ptr<BlendState> const& state)
{
    if (state)
    {
        if (state != mActiveBlendState)
        {
            GL46BlendState* gl4State = static_cast<GL46BlendState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable();
                mActiveBlendState = state;
            }
            else
            {
                LogError("Failed to bind blend state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void GL46Engine::SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state)
{
    if (state)
    {
        if (state != mActiveDepthStencilState)
        {
            GL46DepthStencilState* gl4State = static_cast<GL46DepthStencilState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable();
                mActiveDepthStencilState = state;
            }
            else
            {
                LogError("Failed to bind depth-stencil state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void GL46Engine::SetRasterizerState(std::shared_ptr<RasterizerState> const& state)
{
    if (state)
    {
        if (state != mActiveRasterizerState)
        {
            GL46RasterizerState* gl4State = static_cast<GL46RasterizerState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable();
                mActiveRasterizerState = state;
            }
            else
            {
                LogError("Failed to bind rasterizer state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void GL46Engine::Enable(std::shared_ptr<DrawTarget> const& target)
{
    auto gl4Target = static_cast<GL46DrawTarget*>(Bind(target));
    gl4Target->Enable();
}

void GL46Engine::Disable(std::shared_ptr<DrawTarget> const& target)
{
    auto gl4Target = static_cast<GL46DrawTarget*>(Get(target));
    if (gl4Target)
    {
        gl4Target->Disable();
    }
}

bool GL46Engine::Update(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL46Buffer*>(Bind(buffer));
    return glBuffer->Update();
}

bool GL46Engine::Update(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->Update();
}

bool GL46Engine::Update(std::shared_ptr<TextureSingle> const& texture, uint32_t level)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->Update(level);
}

bool GL46Engine::Update(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->Update();
}

bool GL46Engine::Update(std::shared_ptr<TextureArray> const& textureArray, uint32_t item, uint32_t level)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->Update(item, level);
}

bool GL46Engine::CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL46Buffer*>(Bind(buffer));
    return glBuffer->CopyCpuToGpu();
}

bool GL46Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->CopyCpuToGpu();
}

bool GL46Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, uint32_t level)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->CopyCpuToGpu(level);
}

bool GL46Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyCpuToGpu();
}

bool GL46Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, uint32_t item, uint32_t level)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyCpuToGpu(item, level);
}

bool GL46Engine::CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL46Buffer*>(Bind(buffer));
    return glBuffer->CopyGpuToCpu();
}

bool GL46Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->CopyGpuToCpu();
}

bool GL46Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, uint32_t level)
{
    if (!texture->GetData())
    {
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL46TextureSingle*>(Bind(texture));
    return glTexture->CopyGpuToCpu(level);
}

bool GL46Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyGpuToCpu();
}

bool GL46Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, uint32_t item, uint32_t level)
{
    if (!textureArray->GetData())
    {
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL46TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyGpuToCpu(item, level);
}

void GL46Engine::CopyGpuToGpu(
    std::shared_ptr<Buffer> const& buffer0,
    std::shared_ptr<Buffer> const& buffer1)
{
    (void)buffer0;
    (void)buffer1;
    LogError("This function is not yet implemented.");
}

void GL46Engine::CopyGpuToGpu(
    std::shared_ptr<TextureSingle> const& texture0,
    std::shared_ptr<TextureSingle> const& texture1)
{
    (void)texture0;
    (void)texture1;
    LogError("This function is not yet implemented.");
}

void GL46Engine::CopyGpuToGpu(
    std::shared_ptr<TextureSingle> const& texture0,
    std::shared_ptr<TextureSingle> const& texture1,
    uint32_t level)
{
    (void)texture0;
    (void)texture1;
    (void)level;
    LogError("This function is not yet implemented.");
}

void GL46Engine::CopyGpuToGpu(
    std::shared_ptr<TextureArray> const& textureArray0,
    std::shared_ptr<TextureArray> const& textureArray1)
{
    (void)textureArray0;
    (void)textureArray1;
    LogError("This function is not yet implemented.");
}

void GL46Engine::CopyGpuToGpu(
    std::shared_ptr<TextureArray> const& textureArray0,
    std::shared_ptr<TextureArray> const& textureArray1,
    uint32_t item, uint32_t level)
{
    (void)textureArray0;
    (void)textureArray1;
    (void)item;
    (void)level;
    LogError("This function is not yet implemented.");
}

bool GL46Engine::GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer)
{
    auto gl4Object = Get(buffer);
    if (gl4Object)
    {
        auto gl4SBuffer = static_cast<GL46StructuredBuffer*>(gl4Object);
        return gl4SBuffer->GetNumActiveElements();
    }
    return false;
}

bool GL46Engine::BindProgram(std::shared_ptr<ComputeProgram> const&)
{
    // TODO: Why are we not adding the compute shader to the mGOMap?
    return true;
}

void GL46Engine::Execute(std::shared_ptr<ComputeProgram> const& program,
    uint32_t numXGroups, uint32_t numYGroups, uint32_t numZGroups)
{
    auto glslProgram = std::dynamic_pointer_cast<GLSLComputeProgram>(program);
    if (glslProgram && numXGroups > 0 && numYGroups > 0 && numZGroups > 0)
    {
        auto const& cshader = glslProgram->GetComputeShader();
        auto programHandle = glslProgram->GetProgramHandle();
        if (cshader && programHandle > 0)
        {
            glUseProgram(programHandle);
            Enable(cshader.get(), programHandle);
            glDispatchCompute(numXGroups, numYGroups, numZGroups);
            Disable(cshader.get(), programHandle);
            glUseProgram(0);
        }
    }
    else
    {
        LogError("Invalid input parameter.");
    }
}

void GL46Engine::WaitForFinish()
{
    // TODO. Determine whether OpenGL can wait for a compute program to
    // finish. Is this simply a call to glFinish()?  If so, how does that
    // affect graphics-related work that is queued up on the GPU?
    LogError("This function is not yet implemented.");
}

void GL46Engine::Flush()
{
    glFlush();
}

uint64_t GL46Engine::DrawPrimitive(std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer, std::shared_ptr<VisualEffect> const& effect)
{
    GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
    if (!gl4program)
    {
        LogError("A visual program must exist.");
    }

    uint64_t numPixelsDrawn = 0;
    auto programHandle = gl4program->GetProgramHandle();
    glUseProgram(programHandle);

    if (EnableShaders(effect, programHandle))
    {
        // Enable the vertex buffer and input layout.
        GL46VertexBuffer* gl4VBuffer = nullptr;
        GL46InputLayout* gl4Layout = nullptr;
        if (vbuffer->StandardUsage())
        {
            gl4VBuffer = static_cast<GL46VertexBuffer*>(Bind(vbuffer));
            GL46InputLayoutManager* manager = static_cast<GL46InputLayoutManager*>(mILMap.get());
            gl4Layout = manager->Bind(programHandle, gl4VBuffer->GetGLHandle(), vbuffer.get());
            gl4Layout->Enable();
        }

        // Enable the index buffer.
        GL46IndexBuffer* gl4IBuffer = nullptr;
        if (ibuffer->IsIndexed())
        {
            gl4IBuffer = static_cast<GL46IndexBuffer*>(Bind(ibuffer));
            gl4IBuffer->Enable();
        }

        numPixelsDrawn = DrawPrimitive(vbuffer.get(), ibuffer.get());

        // Disable the vertex buffer and input layout.
        if (vbuffer->StandardUsage())
        {
            gl4Layout->Disable();
        }

        // Disable the index buffer.
        if (gl4IBuffer)
        {
            gl4IBuffer->Disable();
        }

        DisableShaders(effect, programHandle);
    }

    glUseProgram(0);

    return numPixelsDrawn;
}

