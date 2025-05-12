// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GL46.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// Support for versioning.
int constexpr OPENGL_VERSION_1_2 = 12;
int constexpr OPENGL_VERSION_1_3 = 13;
int constexpr OPENGL_VERSION_1_4 = 14;
int constexpr OPENGL_VERSION_1_5 = 15;
int constexpr OPENGL_VERSION_2_0 = 20;
int constexpr OPENGL_VERSION_2_1 = 21;
int constexpr OPENGL_VERSION_3_0 = 30;
int constexpr OPENGL_VERSION_3_1 = 31;
int constexpr OPENGL_VERSION_3_2 = 32;
int constexpr OPENGL_VERSION_3_3 = 33;
int constexpr OPENGL_VERSION_4_0 = 40;
int constexpr OPENGL_VERSION_4_1 = 41;
int constexpr OPENGL_VERSION_4_2 = 42;
int constexpr OPENGL_VERSION_4_3 = 43;
int constexpr OPENGL_VERSION_4_4 = 44;
int constexpr OPENGL_VERSION_4_5 = 45;
int constexpr OPENGL_VERSION_4_6 = 46;

// OpenGL version information.
static int GetOpenGLVersion()
{
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    return 10 * major + minor;
}

// Support for querying the OpenGL function pointers. Each platform must
// provide its own GetOpenGLFunctionPointer.
template <typename PGLFunction>
static void GetOpenGLFunction(char const* name, PGLFunction& function)
{
    extern void* GetOpenGLFunctionPointer(char const*);
    function = (PGLFunction)GetOpenGLFunctionPointer(name);
}

// Listen for glError warnings or errors.
#define GTE_GL46_THROW_ON_REPORT_LISTENER_WARNING
static void OpenGLReportListener(char const* glFunction, GLenum code)
{
    if (!glFunction)
    {
        throw std::runtime_error(std::string(__FILE__) + "(" +
            std::string(__FUNCTION__) + "," + std::to_string(__LINE__) +
            "): OpenGL function pointer is null.\n");
    }

    std::string strFunction(glFunction);
    if (code != GL_ZERO)
    {
        std::string strCode;
        switch (code)
        {
        case GL_INVALID_ENUM:
            strCode = "GL_INVALID_ENUM";
            return;
        case GL_INVALID_VALUE:
            strCode = "GL_INVALID_VALUE";
            return;
        case GL_INVALID_OPERATION:
            strCode = "GL_INVALID_OPERATION";
            return;
        case GL_STACK_OVERFLOW:
            strCode = "GL_STACK_OVERFLOW";
            return;
        case GL_STACK_UNDERFLOW:
            strCode = "GL_STACK_UNDERFLOW";
            return;
        case GL_OUT_OF_MEMORY:
            strCode = "GL_OUT_OF_MEMORY";
            return;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            strCode = "GL_INVALID_FRAMEBUFFER_OPERATION";
            return;
        case GL_CONTEXT_LOST:
            strCode = "GL_CONTEXT_LOST";
            return;
        default:
#if defined(GTE_GL46_THROW_ON_REPORT_LISTENER_WARNING)
        {
            std::string message = "GL error <" + strCode + "> in " + strFunction;
            throw std::runtime_error(std::string(__FILE__) + "(" +
                std::string(__FUNCTION__) + "," + std::to_string(__LINE__) +
                "): " + message + "\n");
        }
#endif
        }
    }
}

static void ReportGLError(const char* glFunction)
{
    // code:
    //   0x0500 GL_INVALID_ENUM
    //   0x0501 GL_INVALID_VALUE
    //   0x0502 GL_INVALID_OPERATION
    //   0x0503 GL_STACK_OVERFLOW
    //   0x0504 GL_STACK_UNDERFLOW
    //   0x0505 GL_OUT_OF_MEMORY
    //   0x0506 GL_INVALID_FRAMEBUFFER_OPERATION
    //   0x0507 GL_CONTEXT_LOST

    GLenum code = glGetError();
    while (code != 0)
    {
        OpenGLReportListener(glFunction, code);
        code = glGetError();
    }
}

static void ReportGLNullFunction(const char* glFunction)
{
    OpenGLReportListener(glFunction, GL_ZERO);
}

// Generate OpenGL function call traces. The results are stored in a text file
// GLTrace.txt in the working folder of the project or the directory of the
// executable. You can clear the trace at any time during program execution.
// This allows you to obtain a trace for a specific block of code. You can
// also insert messages into the trace to identify which application functions
// were called. This allows you to determine the OpenGL functions called by
// the application functions.
//#define GTE_ENABLE_GLTRACE
#if defined(GTE_ENABLE_GLTRACE)
#include <array>
#include <cstdint>
#include <map>

class GLTrace
{
public:
    GLTrace()
        :
        mTraceFileName("GLTrace.txt"),
        mTraceFile{}
    {
    }

    ~GLTrace()
    {
    }

    void Clear()
    {
        mTraceFile.open(mTraceFileName);
        mTraceFile.close();
    }

    void Message(std::string const& message)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << "// " << message << std::endl;
        mTraceFile.close();
    }

    void Call(std::string const& name, std::string const& result)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9, typename T10>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9, T10 const& arg10)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9 << ", "
            << arg10
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9, typename T10, typename T11>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9, T10 const& arg10, T11 const& arg11)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9 << ", "
            << arg10 << ", "
            << arg11
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9, typename T10, typename T11,
        typename T12>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9, T10 const& arg10, T11 const& arg11,
        T12 const& arg12)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9 << ", "
            << arg10 << ", "
            << arg11 << ", "
            << arg12
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9, typename T10, typename T11,
        typename T12, typename T13>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9, T10 const& arg10, T11 const& arg11,
        T12 const& arg12, T13 const& arg13)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9 << ", "
            << arg10 << ", "
            << arg11 << ", "
            << arg12 << ", "
            << arg13
            << ")" << std::endl;
        mTraceFile.close();
    }

    template <typename T0, typename T1, typename T2, typename T3,
        typename T4, typename T5, typename T6, typename T7,
        typename T8, typename T9, typename T10, typename T11,
        typename T12, typename T13, typename T14>
    void Call(std::string const& name, std::string const& result,
        T0 const& arg0, T1 const& arg1, T2 const& arg2, T3 const& arg3,
        T4 const& arg4, T5 const& arg5, T6 const& arg6, T7 const& arg7,
        T8 const& arg8, T9 const& arg9, T10 const& arg10, T11 const& arg11,
        T12 const& arg12, T13 const& arg13, T14 const& arg14)
    {
        mTraceFile.open(mTraceFileName, std::ios::app);
        mTraceFile << name;
        if (result != "")
        {
            mTraceFile << "<<" << result << ">>";
        }
        mTraceFile
            << "("
            << arg0 << ", "
            << arg1 << ", "
            << arg2 << ", "
            << arg3 << ", "
            << arg4 << ", "
            << arg5 << ", "
            << arg6 << ", "
            << arg7 << ", "
            << arg8 << ", "
            << arg9 << ", "
            << arg10 << ", "
            << arg11 << ", "
            << arg12 << ", "
            << arg13 << ", "
            << arg14
            << ")" << std::endl;
        mTraceFile.close();
    }

    std::string GetBoolean(std::uint32_t value) const
    {
        return msBoolean[value];
    }

    std::string GetTopology(std::uint32_t value) const
    {
        auto iter = msTopology.find(value);
        if (iter != msTopology.end())
        {
            return iter->second;
        }
        else
        {
            return "UNKNOWN";
        }
    }

    std::string GetName(std::uint32_t value) const
    {
        auto iter = msName.find(value);
        if (iter != msName.end())
        {
            return iter->second;
        }
        else
        {
            return "UNKNOWN";
        }
    }

    template <typename T>
    std::string GetArray(std::size_t numElements, T const* elements)
    {
        std::string result = "{";
        for (std::size_t i = 0; i < numElements; ++i)
        {
            result += std::to_string(elements[i]);
            if (i + 1 < numElements)
            {
                result += ",";
            }
        }
        result += "}";
        return result;
    }

    std::string GetStringArray(std::size_t numElements, const GLchar* const* elements)
    {
        std::string result = "{";
        for (std::size_t i = 0; i < numElements; ++i)
        {
            result += std::string(elements[i]);
            if (i + 1 < numElements)
            {
                result += ",";
            }
        }
        result += "}";
        return result;
    }

    std::string GetString(char const* elements)
    {
        std::string result = "{" + std::string(elements) + "}";
        return result;
    }

    std::string GetString(char* elements)
    {
        std::string result = "{" + std::string(elements) + "}";
        return result;
    }

    std::string GetEnumArray(std::size_t numElements, GLenum const* elements)
    {
        std::string result = "{";
        for (std::size_t i = 0; i < numElements; ++i)
        {
            result += GetName(elements[i]);
            if (i + 1 < numElements)
            {
                result += ",";
            }
        }
        result += "}";
        return result;
    }

private:
    std::string mTraceFileName;
    std::ofstream mTraceFile;
    static std::array<std::string, 2> msBoolean;
    static std::map<std::uint32_t, std::string> msTopology;
    static std::map<std::uint32_t, std::string> msName;
};

static GLTrace gsTrace{};

void GLTraceClear()
{
    gsTrace.Clear();
}

void GLTraceMessage(std::string const& message)
{
    gsTrace.Message(message);
}

#else

void GLTraceClear()
{
}

void GLTraceMessage(std::string const&)
{
}

#endif

#if !defined(GTE_USE_MSWINDOWS)

int constexpr OPENGL_VERSION_1_1 = 11;

// GL_VERSION_1_0

static PFNGLCULLFACEPROC sglCullFace = nullptr;
static PFNGLFRONTFACEPROC sglFrontFace = nullptr;
static PFNGLHINTPROC sglHint = nullptr;
static PFNGLLINEWIDTHPROC sglLineWidth = nullptr;
static PFNGLPOINTSIZEPROC sglPointSize = nullptr;
static PFNGLPOLYGONMODEPROC sglPolygonMode = nullptr;
static PFNGLSCISSORPROC sglScissor = nullptr;
static PFNGLTEXPARAMETERFPROC sglTexParameterf = nullptr;
static PFNGLTEXPARAMETERFVPROC sglTexParameterfv = nullptr;
static PFNGLTEXPARAMETERIPROC sglTexParameteri = nullptr;
static PFNGLTEXPARAMETERIVPROC sglTexParameteriv = nullptr;
static PFNGLTEXIMAGE1DPROC sglTexImage1D = nullptr;
static PFNGLTEXIMAGE2DPROC sglTexImage2D = nullptr;
static PFNGLDRAWBUFFERPROC sglDrawBuffer = nullptr;
static PFNGLCLEARPROC sglClear = nullptr;
static PFNGLCLEARCOLORPROC sglClearColor = nullptr;
static PFNGLCLEARSTENCILPROC sglClearStencil = nullptr;
static PFNGLCLEARDEPTHPROC sglClearDepth = nullptr;
static PFNGLSTENCILMASKPROC sglStencilMask = nullptr;
static PFNGLCOLORMASKPROC sglColorMask = nullptr;
static PFNGLDEPTHMASKPROC sglDepthMask = nullptr;
static PFNGLDISABLEPROC sglDisable = nullptr;
static PFNGLENABLEPROC sglEnable = nullptr;
static PFNGLFINISHPROC sglFinish = nullptr;
static PFNGLFLUSHPROC sglFlush = nullptr;
static PFNGLBLENDFUNCPROC sglBlendFunc = nullptr;
static PFNGLLOGICOPPROC sglLogicOp = nullptr;
static PFNGLSTENCILFUNCPROC sglStencilFunc = nullptr;
static PFNGLSTENCILOPPROC sglStencilOp = nullptr;
static PFNGLDEPTHFUNCPROC sglDepthFunc = nullptr;
static PFNGLPIXELSTOREFPROC sglPixelStoref = nullptr;
static PFNGLPIXELSTOREIPROC sglPixelStorei = nullptr;
static PFNGLREADBUFFERPROC sglReadBuffer = nullptr;
static PFNGLREADPIXELSPROC sglReadPixels = nullptr;
static PFNGLGETBOOLEANVPROC sglGetBooleanv = nullptr;
static PFNGLGETDOUBLEVPROC sglGetDoublev = nullptr;
static PFNGLGETERRORPROC sglGetError = nullptr;
static PFNGLGETFLOATVPROC sglGetFloatv = nullptr;
static PFNGLGETINTEGERVPROC sglGetIntegerv = nullptr;
static PFNGLGETSTRINGPROC sglGetString = nullptr;
static PFNGLGETTEXIMAGEPROC sglGetTexImage = nullptr;
static PFNGLGETTEXPARAMETERFVPROC sglGetTexParameterfv = nullptr;
static PFNGLGETTEXPARAMETERIVPROC sglGetTexParameteriv = nullptr;
static PFNGLGETTEXLEVELPARAMETERFVPROC sglGetTexLevelParameterfv = nullptr;
static PFNGLGETTEXLEVELPARAMETERIVPROC sglGetTexLevelParameteriv = nullptr;
static PFNGLISENABLEDPROC sglIsEnabled = nullptr;
static PFNGLDEPTHRANGEPROC sglDepthRange = nullptr;
static PFNGLVIEWPORTPROC sglViewport = nullptr;

void APIENTRY glCullFace(GLenum mode)
{
    if (sglCullFace)
    {
        sglCullFace(mode);
        ReportGLError("glCullFace");
    }
    else
    {
        ReportGLNullFunction("glCullFace");
    }
}

void APIENTRY glFrontFace(GLenum mode)
{
    if (sglFrontFace)
    {
        sglFrontFace(mode);
        ReportGLError("glFrontFace");
    }
    else
    {
        ReportGLNullFunction("glFrontFace");
    }
}

void APIENTRY glHint(GLenum target, GLenum mode)
{
    if (sglHint)
    {
        sglHint(target, mode);
        ReportGLError("glHint");
    }
    else
    {
        ReportGLNullFunction("glHint");
    }
}

void APIENTRY glLineWidth(GLfloat width)
{
    if (sglLineWidth)
    {
        sglLineWidth(width);
        ReportGLError("glLineWidth");
    }
    else
    {
        ReportGLNullFunction("glLineWidth");
    }
}

void APIENTRY glPointSize(GLfloat size)
{
    if (sglPointSize)
    {
        sglPointSize(size);
        ReportGLError("glPointSize");
    }
    else
    {
        ReportGLNullFunction("glPointSize");
    }
}

void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
    if (sglPolygonMode)
    {
        sglPolygonMode(face, mode);
        ReportGLError("glPolygonMode");
    }
    else
    {
        ReportGLNullFunction("glPolygonMode");
    }
}

void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglScissor)
    {
        sglScissor(x, y, width, height);
        ReportGLError("glScissor");
    }
    else
    {
        ReportGLNullFunction("glScissor");
    }
}

void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    if (sglTexParameterf)
    {
        sglTexParameterf(target, pname, param);
        ReportGLError("glTexParameterf");
    }
    else
    {
        ReportGLNullFunction("glTexParameterf");
    }
}

void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    if (sglTexParameterfv)
    {
        sglTexParameterfv(target, pname, params);
        ReportGLError("glTexParameterfv");
    }
    else
    {
        ReportGLNullFunction("glTexParameterfv");
    }
}

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    if (sglTexParameteri)
    {
        sglTexParameteri(target, pname, param);
        ReportGLError("glTexParameteri");
    }
    else
    {
        ReportGLNullFunction("glTexParameteri");
    }
}

void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    if (sglTexParameteriv)
    {
        sglTexParameteriv(target, pname, params);
        ReportGLError("glTexParameteriv");
    }
    else
    {
        ReportGLNullFunction("glTexParameteriv");
    }
}

void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexImage1D)
    {
        sglTexImage1D(target, level, internalformat, width, border, format, type, pixels);
        ReportGLError("glTexImage1D");
    }
    else
    {
        ReportGLNullFunction("glTexImage1D");
    }
}

void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexImage2D)
    {
        sglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        ReportGLError("glTexImage2D");
    }
    else
    {
        ReportGLNullFunction("glTexImage2D");
    }
}

void APIENTRY glDrawBuffer(GLenum buf)
{
    if (sglDrawBuffer)
    {
        sglDrawBuffer(buf);
        ReportGLError("glDrawBuffer");
    }
    else
    {
        ReportGLNullFunction("glDrawBuffer");
    }
}

void APIENTRY glClear(GLbitfield mask)
{
    if (sglClear)
    {
        sglClear(mask);
        ReportGLError("glClear");
    }
    else
    {
        ReportGLNullFunction("glClear");
    }
}

void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    if (sglClearColor)
    {
        sglClearColor(red, green, blue, alpha);
        ReportGLError("glClearColor");
    }
    else
    {
        ReportGLNullFunction("glClearColor");
    }
}

void APIENTRY glClearStencil(GLint s)
{
    if (sglClearStencil)
    {
        sglClearStencil(s);
        ReportGLError("glClearStencil");
    }
    else
    {
        ReportGLNullFunction("glClearStencil");
    }
}

void APIENTRY glClearDepth(GLdouble depth)
{
    if (sglClearDepth)
    {
        sglClearDepth(depth);
        ReportGLError("glClearDepth");
    }
    else
    {
        ReportGLNullFunction("glClearDepth");
    }
}

void APIENTRY glStencilMask(GLuint mask)
{
    if (sglStencilMask)
    {
        sglStencilMask(mask);
        ReportGLError("glStencilMask");
    }
    else
    {
        ReportGLNullFunction("glStencilMask");
    }
}

void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    if (sglColorMask)
    {
        sglColorMask(red, green, blue, alpha);
        ReportGLError("glColorMask");
    }
    else
    {
        ReportGLNullFunction("glColorMask");
    }
}

void APIENTRY glDepthMask(GLboolean flag)
{
    if (sglDepthMask)
    {
        sglDepthMask(flag);
        ReportGLError("glDepthMask");
    }
    else
    {
        ReportGLNullFunction("glDepthMask");
    }
}

void APIENTRY glDisable(GLenum cap)
{
    if (sglDisable)
    {
        sglDisable(cap);
        ReportGLError("glDisable");
    }
    else
    {
        ReportGLNullFunction("glDisable");
    }
}

void APIENTRY glEnable(GLenum cap)
{
    if (sglEnable)
    {
        sglEnable(cap);
        ReportGLError("glEnable");
    }
    else
    {
        ReportGLNullFunction("glEnable");
    }
}

void APIENTRY glFinish()
{
    if (sglFinish)
    {
        sglFinish();
        ReportGLError("glFinish");
    }
    else
    {
        ReportGLNullFunction("glFinish");
    }
}

void APIENTRY glFlush()
{
    if (sglFlush)
    {
        sglFlush();
        ReportGLError("glFlush");
    }
    else
    {
        ReportGLNullFunction("glFlush");
    }
}

void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    if (sglBlendFunc)
    {
        sglBlendFunc(sfactor, dfactor);
        ReportGLError("glBlendFunc");
    }
    else
    {
        ReportGLNullFunction("glBlendFunc");
    }
}

void APIENTRY glLogicOp(GLenum opcode)
{
    if (sglLogicOp)
    {
        sglLogicOp(opcode);
        ReportGLError("glLogicOp");
    }
    else
    {
        ReportGLNullFunction("glLogicOp");
    }
}

void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    if (sglStencilFunc)
    {
        sglStencilFunc(func, ref, mask);
        ReportGLError("glStencilFunc");
    }
    else
    {
        ReportGLNullFunction("glStencilFunc");
    }
}

void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    if (sglStencilOp)
    {
        sglStencilOp(fail, zfail, zpass);
        ReportGLError("glStencilOp");
    }
    else
    {
        ReportGLNullFunction("glStencilOp");
    }
}

void APIENTRY glDepthFunc(GLenum func)
{
    if (sglDepthFunc)
    {
        sglDepthFunc(func);
        ReportGLError("glDepthFunc");
    }
    else
    {
        ReportGLNullFunction("glDepthFunc");
    }
}

void APIENTRY glPixelStoref(GLenum pname, GLfloat param)
{
    if (sglPixelStoref)
    {
        sglPixelStoref(pname, param);
        ReportGLError("glPixelStoref");
    }
    else
    {
        ReportGLNullFunction("glPixelStoref");
    }
}

void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    if (sglPixelStorei)
    {
        sglPixelStorei(pname, param);
        ReportGLError("glPixelStorei");
    }
    else
    {
        ReportGLNullFunction("glPixelStorei");
    }
}

void APIENTRY glReadBuffer(GLenum src)
{
    if (sglReadBuffer)
    {
        sglReadBuffer(src);
        ReportGLError("glReadBuffer");
    }
    else
    {
        ReportGLNullFunction("glReadBuffer");
    }
}

void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
    if (sglReadPixels)
    {
        sglReadPixels(x, y, width, height, format, type, pixels);
        ReportGLError("glReadPixels");
    }
    else
    {
        ReportGLNullFunction("glReadPixels");
    }
}

void APIENTRY glGetBooleanv(GLenum pname, GLboolean* data)
{
    if (sglGetBooleanv)
    {
        sglGetBooleanv(pname, data);
        ReportGLError("glGetBooleanv");
    }
    else
    {
        ReportGLNullFunction("glGetBooleanv");
    }
}

void APIENTRY glGetDoublev(GLenum pname, GLdouble* data)
{
    if (sglGetDoublev)
    {
        sglGetDoublev(pname, data);
        ReportGLError("glGetDoublev");
    }
    else
    {
        ReportGLNullFunction("glGetDoublev");
    }
}

GLenum APIENTRY glGetError()
{
    GLenum result;
    if (sglGetError)
    {
        result = sglGetError();
    }
    else
    {
        ReportGLNullFunction("glGetError");
        result = 0;
    }
    return result;
}

void APIENTRY glGetFloatv(GLenum pname, GLfloat* data)
{
    if (sglGetFloatv)
    {
        sglGetFloatv(pname, data);
        ReportGLError("glGetFloatv");
    }
    else
    {
        ReportGLNullFunction("glGetFloatv");
    }
}

void APIENTRY glGetIntegerv(GLenum pname, GLint* data)
{
    if (sglGetIntegerv)
    {
        sglGetIntegerv(pname, data);
        ReportGLError("glGetIntegerv");
    }
    else
    {
        ReportGLNullFunction("glGetIntegerv");
    }
}

const GLubyte* APIENTRY glGetString(GLenum name)
{
    const GLubyte* result;
    if (sglGetString)
    {
        result = sglGetString(name);
        ReportGLError("glGetString");
    }
    else
    {
        ReportGLNullFunction("glGetString");
        result = 0;
    }
    return result;
}

void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
    if (sglGetTexImage)
    {
        sglGetTexImage(target, level, format, type, pixels);
        ReportGLError("glGetTexImage");
    }
    else
    {
        ReportGLNullFunction("glGetTexImage");
    }
}

void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    if (sglGetTexParameterfv)
    {
        sglGetTexParameterfv(target, pname, params);
        ReportGLError("glGetTexParameterfv");
    }
    else
    {
        ReportGLNullFunction("glGetTexParameterfv");
    }
}

void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetTexParameteriv)
    {
        sglGetTexParameteriv(target, pname, params);
        ReportGLError("glGetTexParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetTexParameteriv");
    }
}

void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
    if (sglGetTexLevelParameterfv)
    {
        sglGetTexLevelParameterfv(target, level, pname, params);
        ReportGLError("glGetTexLevelParameterfv");
    }
    else
    {
        ReportGLNullFunction("glGetTexLevelParameterfv");
    }
}

void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
    if (sglGetTexLevelParameteriv)
    {
        sglGetTexLevelParameteriv(target, level, pname, params);
        ReportGLError("glGetTexLevelParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetTexLevelParameteriv");
    }
}

GLboolean APIENTRY glIsEnabled(GLenum cap)
{
    GLboolean result;
    if (sglIsEnabled)
    {
        result = sglIsEnabled(cap);
        ReportGLError("glIsEnabled");
    }
    else
    {
        ReportGLNullFunction("glIsEnabled");
        result = 0;
    }
    return result;
}

void APIENTRY glDepthRange(GLdouble n, GLdouble f)
{
    if (sglDepthRange)
    {
        sglDepthRange(n, f);
        ReportGLError("glDepthRange");
    }
    else
    {
        ReportGLNullFunction("glDepthRange");
    }
}

void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglViewport)
    {
        sglViewport(x, y, width, height);
        ReportGLError("glViewport");
    }
    else
    {
        ReportGLNullFunction("glViewport");
    }
}

static void Initialize_OPENGL_VERSION_1_0()
{
    GetOpenGLFunction("glCullFace", sglCullFace);
    GetOpenGLFunction("glFrontFace", sglFrontFace);
    GetOpenGLFunction("glHint", sglHint);
    GetOpenGLFunction("glLineWidth", sglLineWidth);
    GetOpenGLFunction("glPointSize", sglPointSize);
    GetOpenGLFunction("glPolygonMode", sglPolygonMode);
    GetOpenGLFunction("glScissor", sglScissor);
    GetOpenGLFunction("glTexParameterf", sglTexParameterf);
    GetOpenGLFunction("glTexParameterfv", sglTexParameterfv);
    GetOpenGLFunction("glTexParameteri", sglTexParameteri);
    GetOpenGLFunction("glTexParameteriv", sglTexParameteriv);
    GetOpenGLFunction("glTexImage1D", sglTexImage1D);
    GetOpenGLFunction("glTexImage2D", sglTexImage2D);
    GetOpenGLFunction("glDrawBuffer", sglDrawBuffer);
    GetOpenGLFunction("glClear", sglClear);
    GetOpenGLFunction("glClearColor", sglClearColor);
    GetOpenGLFunction("glClearStencil", sglClearStencil);
    GetOpenGLFunction("glClearDepth", sglClearDepth);
    GetOpenGLFunction("glStencilMask", sglStencilMask);
    GetOpenGLFunction("glColorMask", sglColorMask);
    GetOpenGLFunction("glDepthMask", sglDepthMask);
    GetOpenGLFunction("glDisable", sglDisable);
    GetOpenGLFunction("glEnable", sglEnable);
    GetOpenGLFunction("glFinish", sglFinish);
    GetOpenGLFunction("glFlush", sglFlush);
    GetOpenGLFunction("glBlendFunc", sglBlendFunc);
    GetOpenGLFunction("glLogicOp", sglLogicOp);
    GetOpenGLFunction("glStencilFunc", sglStencilFunc);
    GetOpenGLFunction("glStencilOp", sglStencilOp);
    GetOpenGLFunction("glDepthFunc", sglDepthFunc);
    GetOpenGLFunction("glPixelStoref", sglPixelStoref);
    GetOpenGLFunction("glPixelStorei", sglPixelStorei);
    GetOpenGLFunction("glReadBuffer", sglReadBuffer);
    GetOpenGLFunction("glReadPixels", sglReadPixels);
    GetOpenGLFunction("glGetBooleanv", sglGetBooleanv);
    GetOpenGLFunction("glGetDoublev", sglGetDoublev);
    GetOpenGLFunction("glGetError", sglGetError);
    GetOpenGLFunction("glGetFloatv", sglGetFloatv);
    GetOpenGLFunction("glGetIntegerv", sglGetIntegerv);
    GetOpenGLFunction("glGetString", sglGetString);
    GetOpenGLFunction("glGetTexImage", sglGetTexImage);
    GetOpenGLFunction("glGetTexParameterfv", sglGetTexParameterfv);
    GetOpenGLFunction("glGetTexParameteriv", sglGetTexParameteriv);
    GetOpenGLFunction("glGetTexLevelParameterfv", sglGetTexLevelParameterfv);
    GetOpenGLFunction("glGetTexLevelParameteriv", sglGetTexLevelParameteriv);
    GetOpenGLFunction("glIsEnabled", sglIsEnabled);
    GetOpenGLFunction("glDepthRange", sglDepthRange);
    GetOpenGLFunction("glViewport", sglViewport);
}

// GL_VERSION_1_1

static PFNGLDRAWARRAYSPROC sglDrawArrays = nullptr;
static PFNGLDRAWELEMENTSPROC sglDrawElements = nullptr;
static PFNGLGETPOINTERVPROC sglGetPointerv = nullptr;
static PFNGLPOLYGONOFFSETPROC sglPolygonOffset = nullptr;
static PFNGLCOPYTEXIMAGE1DPROC sglCopyTexImage1D = nullptr;
static PFNGLCOPYTEXIMAGE2DPROC sglCopyTexImage2D = nullptr;
static PFNGLCOPYTEXSUBIMAGE1DPROC sglCopyTexSubImage1D = nullptr;
static PFNGLCOPYTEXSUBIMAGE2DPROC sglCopyTexSubImage2D = nullptr;
static PFNGLTEXSUBIMAGE1DPROC sglTexSubImage1D = nullptr;
static PFNGLTEXSUBIMAGE2DPROC sglTexSubImage2D = nullptr;
static PFNGLBINDTEXTUREPROC sglBindTexture = nullptr;
static PFNGLDELETETEXTURESPROC sglDeleteTextures = nullptr;
static PFNGLGENTEXTURESPROC sglGenTextures = nullptr;
static PFNGLISTEXTUREPROC sglIsTexture = nullptr;

void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    if (sglDrawArrays)
    {
        sglDrawArrays(mode, first, count);
        ReportGLError("glDrawArrays");
    }
    else
    {
        ReportGLNullFunction("glDrawArrays");
    }
}

void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    if (sglDrawElements)
    {
        sglDrawElements(mode, count, type, indices);
        ReportGLError("glDrawElements");
    }
    else
    {
        ReportGLNullFunction("glDrawElements");
    }
}

void APIENTRY glGetPointerv(GLenum pname, void** params)
{
    if (sglGetPointerv)
    {
        sglGetPointerv(pname, params);
        ReportGLError("glGetPointerv");
    }
    else
    {
        ReportGLNullFunction("glGetPointerv");
    }
}

void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    if (sglPolygonOffset)
    {
        sglPolygonOffset(factor, units);
        ReportGLError("glPolygonOffset");
    }
    else
    {
        ReportGLNullFunction("glPolygonOffset");
    }
}

void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    if (sglCopyTexImage1D)
    {
        sglCopyTexImage1D(target, level, internalformat, x, y, width, border);
        ReportGLError("glCopyTexImage1D");
    }
    else
    {
        ReportGLNullFunction("glCopyTexImage1D");
    }
}

void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    if (sglCopyTexImage2D)
    {
        sglCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
        ReportGLError("glCopyTexImage2D");
    }
    else
    {
        ReportGLNullFunction("glCopyTexImage2D");
    }
}

void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    if (sglCopyTexSubImage1D)
    {
        sglCopyTexSubImage1D(target, level, xoffset, x, y, width);
        ReportGLError("glCopyTexSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glCopyTexSubImage1D");
    }
}

void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglCopyTexSubImage2D)
    {
        sglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
        ReportGLError("glCopyTexSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glCopyTexSubImage2D");
    }
}

void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexSubImage1D)
    {
        sglTexSubImage1D(target, level, xoffset, width, format, type, pixels);
        ReportGLError("glTexSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glTexSubImage1D");
    }
}

void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexSubImage2D)
    {
        sglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        ReportGLError("glTexSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glTexSubImage2D");
    }
}

void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    if (sglBindTexture)
    {
        sglBindTexture(target, texture);
        ReportGLError("glBindTexture");
    }
    else
    {
        ReportGLNullFunction("glBindTexture");
    }
}

void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures)
{
    if (sglDeleteTextures)
    {
        sglDeleteTextures(n, textures);
        ReportGLError("glDeleteTextures");
    }
    else
    {
        ReportGLNullFunction("glDeleteTextures");
    }
}

void APIENTRY glGenTextures(GLsizei n, GLuint* textures)
{
    if (sglGenTextures)
    {
        sglGenTextures(n, textures);
        ReportGLError("glGenTextures");
    }
    else
    {
        ReportGLNullFunction("glGenTextures");
    }
}

GLboolean APIENTRY glIsTexture(GLuint texture)
{
    GLboolean result;
    if (sglIsTexture)
    {
        result = sglIsTexture(texture);
        ReportGLError("glIsTexture");
    }
    else
    {
        ReportGLNullFunction("glIsTexture");
        result = 0;
    }
    return result;
}

static void Initialize_OPENGL_VERSION_1_1()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_1_1)
    {
        GetOpenGLFunction("glDrawArrays", sglDrawArrays);
        GetOpenGLFunction("glDrawElements", sglDrawElements);
        GetOpenGLFunction("glGetPointerv", sglGetPointerv);
        GetOpenGLFunction("glPolygonOffset", sglPolygonOffset);
        GetOpenGLFunction("glCopyTexImage1D", sglCopyTexImage1D);
        GetOpenGLFunction("glCopyTexImage2D", sglCopyTexImage2D);
        GetOpenGLFunction("glCopyTexSubImage1D", sglCopyTexSubImage1D);
        GetOpenGLFunction("glCopyTexSubImage2D", sglCopyTexSubImage2D);
        GetOpenGLFunction("glTexSubImage1D", sglTexSubImage1D);
        GetOpenGLFunction("glTexSubImage2D", sglTexSubImage2D);
        GetOpenGLFunction("glBindTexture", sglBindTexture);
        GetOpenGLFunction("glDeleteTextures", sglDeleteTextures);
        GetOpenGLFunction("glGenTextures", sglGenTextures);
        GetOpenGLFunction("glIsTexture", sglIsTexture);
    }
}

#endif

// GL_VERSION_1_2

static PFNGLDRAWRANGEELEMENTSPROC sglDrawRangeElements = nullptr;
static PFNGLTEXIMAGE3DPROC sglTexImage3D = nullptr;
static PFNGLTEXSUBIMAGE3DPROC sglTexSubImage3D = nullptr;
static PFNGLCOPYTEXSUBIMAGE3DPROC sglCopyTexSubImage3D = nullptr;

void APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
    if (sglDrawRangeElements)
    {
        sglDrawRangeElements(mode, start, end, count, type, indices);
        ReportGLError("glDrawRangeElements");
    }
    else
    {
        ReportGLNullFunction("glDrawRangeElements");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawRangeElements", "",
        gsTrace.GetTopology(mode), start, end, count, gsTrace.GetName(type), "indices");
#endif
}

void APIENTRY glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexImage3D)
    {
        sglTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
        ReportGLError("glTexImage3D");
    }
    else
    {
        ReportGLNullFunction("glTexImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexImage3D", "",
        gsTrace.GetName(target), level, internalformat, width, height, depth,
        border, gsTrace.GetName(format), gsTrace.GetName(type), "pixels");
#endif
}

void APIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
    if (sglTexSubImage3D)
    {
        sglTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
        ReportGLError("glTexSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glTexSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexSubImage3D", "",
        gsTrace.GetName(target), level, xoffset, yoffset, zoffset,
        width, height, depth, gsTrace.GetName(format), gsTrace.GetName(type),
        "pixels");
#endif
}

void APIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglCopyTexSubImage3D)
    {
        sglCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
        ReportGLError("glCopyTexSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glCopyTexSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyTexSubImage3D", "",
        gsTrace.GetName(target), level, xoffset, yoffset, zoffset,
        x, y, width, height);
#endif
}

static void Initialize_OPENGL_VERSION_1_2()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_1_2)
    {
        GetOpenGLFunction("glDrawRangeElements", sglDrawRangeElements);
        GetOpenGLFunction("glTexImage3D", sglTexImage3D);
        GetOpenGLFunction("glTexSubImage3D", sglTexSubImage3D);
        GetOpenGLFunction("glCopyTexSubImage3D", sglCopyTexSubImage3D);
    }
}

// GL_VERSION_1_3

static PFNGLACTIVETEXTUREPROC sglActiveTexture = nullptr;
static PFNGLSAMPLECOVERAGEPROC sglSampleCoverage = nullptr;
static PFNGLCOMPRESSEDTEXIMAGE3DPROC sglCompressedTexImage3D = nullptr;
static PFNGLCOMPRESSEDTEXIMAGE2DPROC sglCompressedTexImage2D = nullptr;
static PFNGLCOMPRESSEDTEXIMAGE1DPROC sglCompressedTexImage1D = nullptr;
static PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC sglCompressedTexSubImage3D = nullptr;
static PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC sglCompressedTexSubImage2D = nullptr;
static PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC sglCompressedTexSubImage1D = nullptr;
static PFNGLGETCOMPRESSEDTEXIMAGEPROC sglGetCompressedTexImage = nullptr;

void APIENTRY glActiveTexture(GLenum texture)
{
    if (sglActiveTexture)
    {
        sglActiveTexture(texture);
        ReportGLError("glActiveTexture");
    }
    else
    {
        ReportGLNullFunction("glActiveTexture");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glActiveTexture", "",
        gsTrace.GetName(texture));
#endif
}

void APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
    if (sglSampleCoverage)
    {
        sglSampleCoverage(value, invert);
        ReportGLError("glSampleCoverage");
    }
    else
    {
        ReportGLNullFunction("glSampleCoverage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSampleCoverage", "",
        value, gsTrace.GetBoolean(invert));
#endif
}

void APIENTRY glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexImage3D)
    {
        sglCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
        ReportGLError("glCompressedTexImage3D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexImage3D", "",
        gsTrace.GetName(target), level, gsTrace.GetName(internalformat),
        width, height, depth, border, imageSize, "data");
#endif
}

void APIENTRY glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexImage2D)
    {
        sglCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
        ReportGLError("glCompressedTexImage2D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexImage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexImage2D", "",
        gsTrace.GetName(target), level, gsTrace.GetName(internalformat),
        width, height, border, imageSize, "data");
#endif
}

void APIENTRY glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexImage1D)
    {
        sglCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
        ReportGLError("glCompressedTexImage1D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexImage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexImage1D", "",
        gsTrace.GetName(target), level, gsTrace.GetName(internalformat),
        width, border, imageSize, "data");
#endif
}

void APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexSubImage3D)
    {
        sglCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
        ReportGLError("glCompressedTexSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexSubImage3D", "",
        gsTrace.GetName(target), level, xoffset, yoffset, zoffset, width,
        height, depth, gsTrace.GetName(format), imageSize, "data");
#endif
}

void APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexSubImage2D)
    {
        sglCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
        ReportGLError("glCompressedTexSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexSubImage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexSubImage2D", "",
        gsTrace.GetName(target), level, xoffset, yoffset, width, height,
        gsTrace.GetName(format), imageSize, "data");
#endif
}

void APIENTRY glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTexSubImage1D)
    {
        sglCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
        ReportGLError("glCompressedTexSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTexSubImage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTexSubImage1D", "",
        gsTrace.GetName(target), level, xoffset, width,
        gsTrace.GetName(format), imageSize, "data");
#endif
}

void APIENTRY glGetCompressedTexImage(GLenum target, GLint level, void* img)
{
    if (sglGetCompressedTexImage)
    {
        sglGetCompressedTexImage(target, level, img);
        ReportGLError("glGetCompressedTexImage");
    }
    else
    {
        ReportGLNullFunction("glGetCompressedTexImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetCompressedTexImage", "",
        gsTrace.GetName(target), level, "img");
#endif
}

static void Initialize_OPENGL_VERSION_1_3()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_1_3)
    {
        GetOpenGLFunction("glActiveTexture", sglActiveTexture);
        GetOpenGLFunction("glSampleCoverage", sglSampleCoverage);
        GetOpenGLFunction("glCompressedTexImage3D", sglCompressedTexImage3D);
        GetOpenGLFunction("glCompressedTexImage2D", sglCompressedTexImage2D);
        GetOpenGLFunction("glCompressedTexImage1D", sglCompressedTexImage1D);
        GetOpenGLFunction("glCompressedTexSubImage3D", sglCompressedTexSubImage3D);
        GetOpenGLFunction("glCompressedTexSubImage2D", sglCompressedTexSubImage2D);
        GetOpenGLFunction("glCompressedTexSubImage1D", sglCompressedTexSubImage1D);
        GetOpenGLFunction("glGetCompressedTexImage", sglGetCompressedTexImage);
    }
}

// GL_VERSION_1_4

static PFNGLBLENDFUNCSEPARATEPROC sglBlendFuncSeparate = nullptr;
static PFNGLMULTIDRAWARRAYSPROC sglMultiDrawArrays = nullptr;
static PFNGLMULTIDRAWELEMENTSPROC sglMultiDrawElements = nullptr;
static PFNGLPOINTPARAMETERFPROC sglPointParameterf = nullptr;
static PFNGLPOINTPARAMETERFVPROC sglPointParameterfv = nullptr;
static PFNGLPOINTPARAMETERIPROC sglPointParameteri = nullptr;
static PFNGLPOINTPARAMETERIVPROC sglPointParameteriv = nullptr;
static PFNGLBLENDCOLORPROC sglBlendColor = nullptr;
static PFNGLBLENDEQUATIONPROC sglBlendEquation = nullptr;

void APIENTRY glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    if (sglBlendFuncSeparate)
    {
        sglBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
        ReportGLError("glBlendFuncSeparate");
    }
    else
    {
        ReportGLNullFunction("glBlendFuncSeparate");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendFuncSeparate", "",
        gsTrace.GetName(sfactorRGB), gsTrace.GetName(dfactorRGB),
        gsTrace.GetName(sfactorAlpha), gsTrace.GetName(dfactorAlpha));
#endif
}

void APIENTRY glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount)
{
    if (sglMultiDrawArrays)
    {
        sglMultiDrawArrays(mode, first, count, drawcount);
        ReportGLError("glMultiDrawArrays");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawArrays");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawArrays", "",
        gsTrace.GetName(mode), "first", "count", drawcount);
#endif
}

void APIENTRY glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount)
{
    if (sglMultiDrawElements)
    {
        sglMultiDrawElements(mode, count, type, indices, drawcount);
        ReportGLError("glMultiDrawElements");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawElements");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawElements", "",
        gsTrace.GetName(mode), "count", type, "indices", drawcount);
#endif
}

void APIENTRY glPointParameterf(GLenum pname, GLfloat param)
{
    if (sglPointParameterf)
    {
        sglPointParameterf(pname, param);
        ReportGLError("glPointParameterf");
    }
    else
    {
        ReportGLNullFunction("glPointParameterf");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPointParameterf", "",
        gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glPointParameterfv(GLenum pname, const GLfloat* params)
{
    if (sglPointParameterfv)
    {
        sglPointParameterfv(pname, params);
        ReportGLError("glPointParameterfv");
    }
    else
    {
        ReportGLNullFunction("glPointParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPointParameterfv", "",
        gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glPointParameteri(GLenum pname, GLint param)
{
    if (sglPointParameteri)
    {
        sglPointParameteri(pname, param);
        ReportGLError("glPointParameteri");
    }
    else
    {
        ReportGLNullFunction("glPointParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPointParameteri", "",
        gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glPointParameteriv(GLenum pname, const GLint* params)
{
    if (sglPointParameteriv)
    {
        sglPointParameteriv(pname, params);
        ReportGLError("glPointParameteriv");
    }
    else
    {
        ReportGLNullFunction("glPointParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPointParameteriv", "",
        gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    if (sglBlendColor)
    {
        sglBlendColor(red, green, blue, alpha);
        ReportGLError("glBlendColor");
    }
    else
    {
        ReportGLNullFunction("glBlendColor");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendColor", "",
        red, green, blue, alpha);
#endif
}

void APIENTRY glBlendEquation(GLenum mode)
{
    if (sglBlendEquation)
    {
        sglBlendEquation(mode);
        ReportGLError("glBlendEquation");
    }
    else
    {
        ReportGLNullFunction("glBlendEquation");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendEquation", "",
        gsTrace.GetName(mode));
#endif
}

static void Initialize_OPENGL_VERSION_1_4()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_1_4)
    {
        GetOpenGLFunction("glBlendFuncSeparate", sglBlendFuncSeparate);
        GetOpenGLFunction("glMultiDrawArrays", sglMultiDrawArrays);
        GetOpenGLFunction("glMultiDrawElements", sglMultiDrawElements);
        GetOpenGLFunction("glPointParameterf", sglPointParameterf);
        GetOpenGLFunction("glPointParameterfv", sglPointParameterfv);
        GetOpenGLFunction("glPointParameteri", sglPointParameteri);
        GetOpenGLFunction("glPointParameteriv", sglPointParameteriv);
        GetOpenGLFunction("glBlendColor", sglBlendColor);
        GetOpenGLFunction("glBlendEquation", sglBlendEquation);
    }
}

// GL_VERSION_1_5

static PFNGLGENQUERIESPROC sglGenQueries = nullptr;
static PFNGLDELETEQUERIESPROC sglDeleteQueries = nullptr;
static PFNGLISQUERYPROC sglIsQuery = nullptr;
static PFNGLBEGINQUERYPROC sglBeginQuery = nullptr;
static PFNGLENDQUERYPROC sglEndQuery = nullptr;
static PFNGLGETQUERYIVPROC sglGetQueryiv = nullptr;
static PFNGLGETQUERYOBJECTIVPROC sglGetQueryObjectiv = nullptr;
static PFNGLGETQUERYOBJECTUIVPROC sglGetQueryObjectuiv = nullptr;
static PFNGLBINDBUFFERPROC sglBindBuffer = nullptr;
static PFNGLDELETEBUFFERSPROC sglDeleteBuffers = nullptr;
static PFNGLGENBUFFERSPROC sglGenBuffers = nullptr;
static PFNGLISBUFFERPROC sglIsBuffer = nullptr;
static PFNGLBUFFERDATAPROC sglBufferData = nullptr;
static PFNGLBUFFERSUBDATAPROC sglBufferSubData = nullptr;
static PFNGLGETBUFFERSUBDATAPROC sglGetBufferSubData = nullptr;
static PFNGLMAPBUFFERPROC sglMapBuffer = nullptr;
static PFNGLUNMAPBUFFERPROC sglUnmapBuffer = nullptr;
static PFNGLGETBUFFERPARAMETERIVPROC sglGetBufferParameteriv = nullptr;
static PFNGLGETBUFFERPOINTERVPROC sglGetBufferPointerv = nullptr;

void APIENTRY glGenQueries(GLsizei n, GLuint* ids)
{
    if (sglGenQueries)
    {
        sglGenQueries(n, ids);
        ReportGLError("glGenQueries");
    }
    else
    {
        ReportGLNullFunction("glGenQueries");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenQueries", "",
        n, gsTrace.GetArray(n, ids));
#endif
}

void APIENTRY glDeleteQueries(GLsizei n, const GLuint* ids)
{
    if (sglDeleteQueries)
    {
        sglDeleteQueries(n, ids);
        ReportGLError("glDeleteQueries");
    }
    else
    {
        ReportGLNullFunction("glDeleteQueries");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteQueries", "",
        n, gsTrace.GetArray(n, ids));
#endif
}

GLboolean APIENTRY glIsQuery(GLuint id)
{
    GLboolean result;
    if (sglIsQuery)
    {
        result = sglIsQuery(id);
        ReportGLError("glIsQuery");
    }
    else
    {
        ReportGLNullFunction("glIsQuery");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsQuery", gsTrace.GetBoolean(result),
        id);
#endif
    return result;
}

void APIENTRY glBeginQuery(GLenum target, GLuint id)
{
    if (sglBeginQuery)
    {
        sglBeginQuery(target, id);
        ReportGLError("glBeginQuery");
    }
    else
    {
        ReportGLNullFunction("glBeginQuery");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBeginQuery", "",
        gsTrace.GetName(target), id);
#endif
}

void APIENTRY glEndQuery(GLenum target)
{
    if (sglEndQuery)
    {
        sglEndQuery(target);
        ReportGLError("glEndQuery");
    }
    else
    {
        ReportGLNullFunction("glEndQuery");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEndQuery", "",
        gsTrace.GetName(target));
#endif
}

void APIENTRY glGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetQueryiv)
    {
        sglGetQueryiv(target, pname, params);
        ReportGLError("glGetQueryiv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryiv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params)
{
    if (sglGetQueryObjectiv)
    {
        sglGetQueryObjectiv(id, pname, params);
        ReportGLError("glGetQueryObjectiv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryObjectiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryObjectiv", "",
        id, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
    if (sglGetQueryObjectuiv)
    {
        sglGetQueryObjectuiv(id, pname, params);
        ReportGLError("glGetQueryObjectuiv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryObjectuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryObjectuiv", "",
        id, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    if (sglBindBuffer)
    {
        sglBindBuffer(target, buffer);
        ReportGLError("glBindBuffer");
    }
    else
    {
        ReportGLNullFunction("glBindBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindBuffer", "",
        gsTrace.GetName(target), buffer);
#endif
}

void APIENTRY glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    if (sglDeleteBuffers)
    {
        sglDeleteBuffers(n, buffers);
        ReportGLError("glDeleteBuffers");
    }
    else
    {
        ReportGLNullFunction("glDeleteBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteBuffers", "",
        n, gsTrace.GetArray(n, buffers));
#endif
}

void APIENTRY glGenBuffers(GLsizei n, GLuint* buffers)
{
    if (sglGenBuffers)
    {
        sglGenBuffers(n, buffers);
        ReportGLError("glGenBuffers");
    }
    else
    {
        ReportGLNullFunction("glGenBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenBuffers", "",
        n, gsTrace.GetArray(n, buffers));
#endif
}

GLboolean APIENTRY glIsBuffer(GLuint buffer)
{
    GLboolean result;
    if (sglIsBuffer)
    {
        result = sglIsBuffer(buffer);
        ReportGLError("glIsBuffer");
    }
    else
    {
        ReportGLNullFunction("glIsBuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsBuffer", gsTrace.GetBoolean(result),
        buffer);
#endif
    return result;
}

void APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    if (sglBufferData)
    {
        sglBufferData(target, size, data, usage);
        ReportGLError("glBufferData");
    }
    else
    {
        ReportGLNullFunction("glBufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBufferData", "",
        gsTrace.GetName(target), size, "data", gsTrace.GetName(usage));
#endif
}

void APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    if (sglBufferSubData)
    {
        sglBufferSubData(target, offset, size, data);
        ReportGLError("glBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBufferSubData", "",
        gsTrace.GetName(target), offset, size, "data");
#endif
}

void APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
    if (sglGetBufferSubData)
    {
        sglGetBufferSubData(target, offset, size, data);
        ReportGLError("glGetBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glGetBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetBufferSubData", "",
        gsTrace.GetName(target), offset, size, "data");
#endif
}

void* APIENTRY glMapBuffer(GLenum target, GLenum access)
{
    void* result;
    if (sglMapBuffer)
    {
        result = sglMapBuffer(target, access);
        ReportGLError("glMapBuffer");
    }
    else
    {
        ReportGLNullFunction("glMapBuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMapBuffer", "pointer",
        gsTrace.GetName(target), gsTrace.GetName(access));
#endif
    return result;
}

GLboolean APIENTRY glUnmapBuffer(GLenum target)
{
    GLboolean result;
    if (sglUnmapBuffer)
    {
        result = sglUnmapBuffer(target);
        ReportGLError("glUnmapBuffer");
    }
    else
    {
        ReportGLNullFunction("glUnmapBuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUnmapBuffer", gsTrace.GetBoolean(result),
        gsTrace.GetName(target));
#endif
    return result;
}

void APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetBufferParameteriv)
    {
        sglGetBufferParameteriv(target, pname, params);
        ReportGLError("glGetBufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetBufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetBufferParameteriv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetBufferPointerv(GLenum target, GLenum pname, void** params)
{
    if (sglGetBufferPointerv)
    {
        sglGetBufferPointerv(target, pname, params);
        ReportGLError("glGetBufferPointerv");
    }
    else
    {
        ReportGLNullFunction("glGetBufferPointerv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetBufferPointerv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

static void Initialize_OPENGL_VERSION_1_5()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_1_5)
    {
        GetOpenGLFunction("glGenQueries", sglGenQueries);
        GetOpenGLFunction("glDeleteQueries", sglDeleteQueries);
        GetOpenGLFunction("glIsQuery", sglIsQuery);
        GetOpenGLFunction("glBeginQuery", sglBeginQuery);
        GetOpenGLFunction("glEndQuery", sglEndQuery);
        GetOpenGLFunction("glGetQueryiv", sglGetQueryiv);
        GetOpenGLFunction("glGetQueryObjectiv", sglGetQueryObjectiv);
        GetOpenGLFunction("glGetQueryObjectuiv", sglGetQueryObjectuiv);
        GetOpenGLFunction("glBindBuffer", sglBindBuffer);
        GetOpenGLFunction("glDeleteBuffers", sglDeleteBuffers);
        GetOpenGLFunction("glGenBuffers", sglGenBuffers);
        GetOpenGLFunction("glIsBuffer", sglIsBuffer);
        GetOpenGLFunction("glBufferData", sglBufferData);
        GetOpenGLFunction("glBufferSubData", sglBufferSubData);
        GetOpenGLFunction("glGetBufferSubData", sglGetBufferSubData);
        GetOpenGLFunction("glMapBuffer", sglMapBuffer);
        GetOpenGLFunction("glUnmapBuffer", sglUnmapBuffer);
        GetOpenGLFunction("glGetBufferParameteriv", sglGetBufferParameteriv);
        GetOpenGLFunction("glGetBufferPointerv", sglGetBufferPointerv);
    }
}

// GL_VERSION_2_0

static PFNGLBLENDEQUATIONSEPARATEPROC sglBlendEquationSeparate = nullptr;
static PFNGLDRAWBUFFERSPROC sglDrawBuffers = nullptr;
static PFNGLSTENCILOPSEPARATEPROC sglStencilOpSeparate = nullptr;
static PFNGLSTENCILFUNCSEPARATEPROC sglStencilFuncSeparate = nullptr;
static PFNGLSTENCILMASKSEPARATEPROC sglStencilMaskSeparate = nullptr;
static PFNGLATTACHSHADERPROC sglAttachShader = nullptr;
static PFNGLBINDATTRIBLOCATIONPROC sglBindAttribLocation = nullptr;
static PFNGLCOMPILESHADERPROC sglCompileShader = nullptr;
static PFNGLCREATEPROGRAMPROC sglCreateProgram = nullptr;
static PFNGLCREATESHADERPROC sglCreateShader = nullptr;
static PFNGLDELETEPROGRAMPROC sglDeleteProgram = nullptr;
static PFNGLDELETESHADERPROC sglDeleteShader = nullptr;
static PFNGLDETACHSHADERPROC sglDetachShader = nullptr;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC sglDisableVertexAttribArray = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC sglEnableVertexAttribArray = nullptr;
static PFNGLGETACTIVEATTRIBPROC sglGetActiveAttrib = nullptr;
static PFNGLGETACTIVEUNIFORMPROC sglGetActiveUniform = nullptr;
static PFNGLGETATTACHEDSHADERSPROC sglGetAttachedShaders = nullptr;
static PFNGLGETATTRIBLOCATIONPROC sglGetAttribLocation = nullptr;
static PFNGLGETPROGRAMIVPROC sglGetProgramiv = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC sglGetProgramInfoLog = nullptr;
static PFNGLGETSHADERIVPROC sglGetShaderiv = nullptr;
static PFNGLGETSHADERINFOLOGPROC sglGetShaderInfoLog = nullptr;
static PFNGLGETSHADERSOURCEPROC sglGetShaderSource = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC sglGetUniformLocation = nullptr;
static PFNGLGETUNIFORMFVPROC sglGetUniformfv = nullptr;
static PFNGLGETUNIFORMIVPROC sglGetUniformiv = nullptr;
static PFNGLGETVERTEXATTRIBDVPROC sglGetVertexAttribdv = nullptr;
static PFNGLGETVERTEXATTRIBFVPROC sglGetVertexAttribfv = nullptr;
static PFNGLGETVERTEXATTRIBIVPROC sglGetVertexAttribiv = nullptr;
static PFNGLGETVERTEXATTRIBPOINTERVPROC sglGetVertexAttribPointerv = nullptr;
static PFNGLISPROGRAMPROC sglIsProgram = nullptr;
static PFNGLISSHADERPROC sglIsShader = nullptr;
static PFNGLLINKPROGRAMPROC sglLinkProgram = nullptr;
static PFNGLSHADERSOURCEPROC sglShaderSource = nullptr;
static PFNGLUSEPROGRAMPROC sglUseProgram = nullptr;
static PFNGLUNIFORM1FPROC sglUniform1f = nullptr;
static PFNGLUNIFORM2FPROC sglUniform2f = nullptr;
static PFNGLUNIFORM3FPROC sglUniform3f = nullptr;
static PFNGLUNIFORM4FPROC sglUniform4f = nullptr;
static PFNGLUNIFORM1IPROC sglUniform1i = nullptr;
static PFNGLUNIFORM2IPROC sglUniform2i = nullptr;
static PFNGLUNIFORM3IPROC sglUniform3i = nullptr;
static PFNGLUNIFORM4IPROC sglUniform4i = nullptr;
static PFNGLUNIFORM1FVPROC sglUniform1fv = nullptr;
static PFNGLUNIFORM2FVPROC sglUniform2fv = nullptr;
static PFNGLUNIFORM3FVPROC sglUniform3fv = nullptr;
static PFNGLUNIFORM4FVPROC sglUniform4fv = nullptr;
static PFNGLUNIFORM1IVPROC sglUniform1iv = nullptr;
static PFNGLUNIFORM2IVPROC sglUniform2iv = nullptr;
static PFNGLUNIFORM3IVPROC sglUniform3iv = nullptr;
static PFNGLUNIFORM4IVPROC sglUniform4iv = nullptr;
static PFNGLUNIFORMMATRIX2FVPROC sglUniformMatrix2fv = nullptr;
static PFNGLUNIFORMMATRIX3FVPROC sglUniformMatrix3fv = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC sglUniformMatrix4fv = nullptr;
static PFNGLVALIDATEPROGRAMPROC sglValidateProgram = nullptr;
static PFNGLVERTEXATTRIB1DPROC sglVertexAttrib1d = nullptr;
static PFNGLVERTEXATTRIB1DVPROC sglVertexAttrib1dv = nullptr;
static PFNGLVERTEXATTRIB1FPROC sglVertexAttrib1f = nullptr;
static PFNGLVERTEXATTRIB1FVPROC sglVertexAttrib1fv = nullptr;
static PFNGLVERTEXATTRIB1SPROC sglVertexAttrib1s = nullptr;
static PFNGLVERTEXATTRIB1SVPROC sglVertexAttrib1sv = nullptr;
static PFNGLVERTEXATTRIB2DPROC sglVertexAttrib2d = nullptr;
static PFNGLVERTEXATTRIB2DVPROC sglVertexAttrib2dv = nullptr;
static PFNGLVERTEXATTRIB2FPROC sglVertexAttrib2f = nullptr;
static PFNGLVERTEXATTRIB2FVPROC sglVertexAttrib2fv = nullptr;
static PFNGLVERTEXATTRIB2SPROC sglVertexAttrib2s = nullptr;
static PFNGLVERTEXATTRIB2SVPROC sglVertexAttrib2sv = nullptr;
static PFNGLVERTEXATTRIB3DPROC sglVertexAttrib3d = nullptr;
static PFNGLVERTEXATTRIB3DVPROC sglVertexAttrib3dv = nullptr;
static PFNGLVERTEXATTRIB3FPROC sglVertexAttrib3f = nullptr;
static PFNGLVERTEXATTRIB3FVPROC sglVertexAttrib3fv = nullptr;
static PFNGLVERTEXATTRIB3SPROC sglVertexAttrib3s = nullptr;
static PFNGLVERTEXATTRIB3SVPROC sglVertexAttrib3sv = nullptr;
static PFNGLVERTEXATTRIB4NBVPROC sglVertexAttrib4Nbv = nullptr;
static PFNGLVERTEXATTRIB4NIVPROC sglVertexAttrib4Niv = nullptr;
static PFNGLVERTEXATTRIB4NSVPROC sglVertexAttrib4Nsv = nullptr;
static PFNGLVERTEXATTRIB4NUBPROC sglVertexAttrib4Nub = nullptr;
static PFNGLVERTEXATTRIB4NUBVPROC sglVertexAttrib4Nubv = nullptr;
static PFNGLVERTEXATTRIB4NUIVPROC sglVertexAttrib4Nuiv = nullptr;
static PFNGLVERTEXATTRIB4NUSVPROC sglVertexAttrib4Nusv = nullptr;
static PFNGLVERTEXATTRIB4BVPROC sglVertexAttrib4bv = nullptr;
static PFNGLVERTEXATTRIB4DPROC sglVertexAttrib4d = nullptr;
static PFNGLVERTEXATTRIB4DVPROC sglVertexAttrib4dv = nullptr;
static PFNGLVERTEXATTRIB4FPROC sglVertexAttrib4f = nullptr;
static PFNGLVERTEXATTRIB4FVPROC sglVertexAttrib4fv = nullptr;
static PFNGLVERTEXATTRIB4IVPROC sglVertexAttrib4iv = nullptr;
static PFNGLVERTEXATTRIB4SPROC sglVertexAttrib4s = nullptr;
static PFNGLVERTEXATTRIB4SVPROC sglVertexAttrib4sv = nullptr;
static PFNGLVERTEXATTRIB4UBVPROC sglVertexAttrib4ubv = nullptr;
static PFNGLVERTEXATTRIB4UIVPROC sglVertexAttrib4uiv = nullptr;
static PFNGLVERTEXATTRIB4USVPROC sglVertexAttrib4usv = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC sglVertexAttribPointer = nullptr;

void APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    if (sglBlendEquationSeparate)
    {
        sglBlendEquationSeparate(modeRGB, modeAlpha);
        ReportGLError("glBlendEquationSeparate");
    }
    else
    {
        ReportGLNullFunction("glBlendEquationSeparate");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendEquationSeparate", "",
        gsTrace.GetName(modeRGB), gsTrace.GetName(modeAlpha));
#endif
}

void APIENTRY glDrawBuffers(GLsizei n, const GLenum* bufs)
{
    if (sglDrawBuffers)
    {
        sglDrawBuffers(n, bufs);
        ReportGLError("glDrawBuffers");
    }
    else
    {
        ReportGLNullFunction("glDrawBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawBuffers", "",
        n, gsTrace.GetEnumArray(n, bufs));
#endif
}

void APIENTRY glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    if (sglStencilOpSeparate)
    {
        sglStencilOpSeparate(face, sfail, dpfail, dppass);
        ReportGLError("glStencilOpSeparate");
    }
    else
    {
        ReportGLNullFunction("glStencilOpSeparate");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glStencilOpSeparate", "",
        gsTrace.GetName(face), gsTrace.GetName(sfail),
        gsTrace.GetName(dpfail), gsTrace.GetName(dppass));
#endif
}

void APIENTRY glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    if (sglStencilFuncSeparate)
    {
        sglStencilFuncSeparate(face, func, ref, mask);
        ReportGLError("glStencilFuncSeparate");
    }
    else
    {
        ReportGLNullFunction("glStencilFuncSeparate");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glStencilFuncSeparate", "",
        gsTrace.GetName(face), gsTrace.GetName(func), ref, mask);
#endif
}

void APIENTRY glStencilMaskSeparate(GLenum face, GLuint mask)
{
    if (sglStencilMaskSeparate)
    {
        sglStencilMaskSeparate(face, mask);
        ReportGLError("glStencilMaskSeparate");
    }
    else
    {
        ReportGLNullFunction("glStencilMaskSeparate");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glStencilMaskSeparate", "",
        gsTrace.GetName(face), mask);
#endif
}

void APIENTRY glAttachShader(GLuint program, GLuint shader)
{
    if (sglAttachShader)
    {
        sglAttachShader(program, shader);
        ReportGLError("glAttachShader");
    }
    else
    {
        ReportGLNullFunction("glAttachShader");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glAttachShader", "",
        program, shader);
#endif
}

void APIENTRY glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    if (sglBindAttribLocation)
    {
        sglBindAttribLocation(program, index, name);
        ReportGLError("glBindAttribLocation");
    }
    else
    {
        ReportGLNullFunction("glBindAttribLocation");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindAttribLocation", "",
        program, index, name);
#endif
}

void APIENTRY glCompileShader(GLuint shader)
{
    if (sglCompileShader)
    {
        sglCompileShader(shader);
        ReportGLError("glCompileShader");
    }
    else
    {
        ReportGLNullFunction("glCompileShader");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompileShader", "",
        shader);
#endif
}

GLuint APIENTRY glCreateProgram()
{
    GLuint result;
    if (sglCreateProgram)
    {
        result = sglCreateProgram();
        ReportGLError("glCreateProgram");
    }
    else
    {
        ReportGLNullFunction("glCreateProgram");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateProgram", std::to_string(result));
#endif
    return result;
}

GLuint APIENTRY glCreateShader(GLenum type)
{
    GLuint result;
    if (sglCreateShader)
    {
        result = sglCreateShader(type);
        ReportGLError("glCreateShader");
    }
    else
    {
        ReportGLNullFunction("glCreateShader");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateShader", std::to_string(result),
        gsTrace.GetName(type));
#endif
    return result;
}

void APIENTRY glDeleteProgram(GLuint program)
{
    if (sglDeleteProgram)
    {
        sglDeleteProgram(program);
        ReportGLError("glDeleteProgram");
    }
    else
    {
        ReportGLNullFunction("glDeleteProgram");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteProgram", "",
        program);
#endif
}

void APIENTRY glDeleteShader(GLuint shader)
{
    if (sglDeleteShader)
    {
        sglDeleteShader(shader);
        ReportGLError("glDeleteShader");
    }
    else
    {
        ReportGLNullFunction("glDeleteShader");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteShader", "",
        shader);
#endif
}

void APIENTRY glDetachShader(GLuint program, GLuint shader)
{
    if (sglDetachShader)
    {
        sglDetachShader(program, shader);
        ReportGLError("glDetachShader");
    }
    else
    {
        ReportGLNullFunction("glDetachShader");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDetachShader", "",
        program, shader);
#endif
}

void APIENTRY glDisableVertexAttribArray(GLuint index)
{
    if (sglDisableVertexAttribArray)
    {
        sglDisableVertexAttribArray(index);
        ReportGLError("glDisableVertexAttribArray");
    }
    else
    {
        ReportGLNullFunction("glDisableVertexAttribArray");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDisableVertexAttribArray", "",
        index);
#endif
}

void APIENTRY glEnableVertexAttribArray(GLuint index)
{
    if (sglEnableVertexAttribArray)
    {
        sglEnableVertexAttribArray(index);
        ReportGLError("glEnableVertexAttribArray");
    }
    else
    {
        ReportGLNullFunction("glEnableVertexAttribArray");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEnableVertexAttribArray", "",
        index);
#endif
}

void APIENTRY glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    if (sglGetActiveAttrib)
    {
        sglGetActiveAttrib(program, index, bufSize, length, size, type, name);
        ReportGLError("glGetActiveAttrib");
    }
    else
    {
        ReportGLNullFunction("glGetActiveAttrib");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveAttrib", "",
        program, index, bufSize, *length, *size, gsTrace.GetName(*type), name);
#endif
}

void APIENTRY glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    if (sglGetActiveUniform)
    {
        sglGetActiveUniform(program, index, bufSize, length, size, type, name);
        ReportGLError("glGetActiveUniform");
    }
    else
    {
        ReportGLNullFunction("glGetActiveUniform");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveUniform", "",
        program, index, bufSize, *length, *size, gsTrace.GetName(*type), name);
#endif
}

void APIENTRY glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
    if (sglGetAttachedShaders)
    {
        sglGetAttachedShaders(program, maxCount, count, shaders);
        ReportGLError("glGetAttachedShaders");
    }
    else
    {
        ReportGLNullFunction("glGetAttachedShaders");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetAttachedShaders", "",
        program, maxCount, *count, gsTrace.GetArray(*count, shaders));
#endif
}

GLint APIENTRY glGetAttribLocation(GLuint program, const GLchar* name)
{
    GLint result;
    if (sglGetAttribLocation)
    {
        result = sglGetAttribLocation(program, name);
        ReportGLError("glGetAttribLocation");
    }
    else
    {
        ReportGLNullFunction("glGetAttribLocation");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetAttribLocation", std::to_string(result),
        program, name);
#endif
    return result;
}

void APIENTRY glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    if (sglGetProgramiv)
    {
        sglGetProgramiv(program, pname, params);
        ReportGLError("glGetProgramiv");
    }
    else
    {
        ReportGLNullFunction("glGetProgramiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramiv", "",
        program, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (sglGetProgramInfoLog)
    {
        sglGetProgramInfoLog(program, bufSize, length, infoLog);
        ReportGLError("glGetProgramInfoLog");
    }
    else
    {
        ReportGLNullFunction("glGetProgramInfoLog");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramInfoLog", "",
        program, bufSize, *length, infoLog);
#endif
}

void APIENTRY glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    if (sglGetShaderiv)
    {
        sglGetShaderiv(shader, pname, params);
        ReportGLError("glGetShaderiv");
    }
    else
    {
        ReportGLNullFunction("glGetShaderiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetShaderiv", "",
        shader, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (sglGetShaderInfoLog)
    {
        sglGetShaderInfoLog(shader, bufSize, length, infoLog);
        ReportGLError("glGetShaderInfoLog");
    }
    else
    {
        ReportGLNullFunction("glGetShaderInfoLog");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetShaderInfoLog", "",
        shader, bufSize, *length, infoLog);
#endif
}

void APIENTRY glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
    if (sglGetShaderSource)
    {
        sglGetShaderSource(shader, bufSize, length, source);
        ReportGLError("glGetShaderSource");
    }
    else
    {
        ReportGLNullFunction("glGetShaderSource");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetShaderSource", "",
        shader, bufSize, *length, "source");
#endif
}

GLint APIENTRY glGetUniformLocation(GLuint program, const GLchar* name)
{
    GLint result;
    if (sglGetUniformLocation)
    {
        result = sglGetUniformLocation(program, name);
        ReportGLError("glGetUniformLocation");
    }
    else
    {
        ReportGLNullFunction("glGetUniformLocation");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformLocation", std::to_string(result),
        program, name);
#endif
    return result;
}

void APIENTRY glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    if (sglGetUniformfv)
    {
        sglGetUniformfv(program, location, params);
        ReportGLError("glGetUniformfv");
    }
    else
    {
        ReportGLNullFunction("glGetUniformfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformfv","",
        program, location, *params);
#endif
}

void APIENTRY glGetUniformiv(GLuint program, GLint location, GLint* params)
{
    if (sglGetUniformiv)
    {
        sglGetUniformiv(program, location, params);
        ReportGLError("glGetUniformiv");
    }
    else
    {
        ReportGLNullFunction("glGetUniformiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformiv", "",
        program, location, *params);
#endif
}

void APIENTRY glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params)
{
    if (sglGetVertexAttribdv)
    {
        sglGetVertexAttribdv(index, pname, params);
        ReportGLError("glGetVertexAttribdv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribdv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribdv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    if (sglGetVertexAttribfv)
    {
        sglGetVertexAttribfv(index, pname, params);
        ReportGLError("glGetVertexAttribfv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribfv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    if (sglGetVertexAttribiv)
    {
        sglGetVertexAttribiv(index, pname, params);
        ReportGLError("glGetVertexAttribiv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribiv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
    if (sglGetVertexAttribPointerv)
    {
        sglGetVertexAttribPointerv(index, pname, pointer);
        ReportGLError("glGetVertexAttribPointerv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribPointerv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribPointerv", "",
        index, gsTrace.GetName(pname), "pointer");
#endif
}

GLboolean APIENTRY glIsProgram(GLuint program)
{
    GLboolean result;
    if (sglIsProgram)
    {
        result = sglIsProgram(program);
        ReportGLError("glIsProgram");
    }
    else
    {
        ReportGLNullFunction("glIsProgram");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsProgram", gsTrace.GetBoolean(result),
        program);
#endif
    return result;
}

GLboolean APIENTRY glIsShader(GLuint shader)
{
    GLboolean result;
    if (sglIsShader)
    {
        result = sglIsShader(shader);
        ReportGLError("glIsShader");
    }
    else
    {
        ReportGLNullFunction("glIsShader");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsShader", gsTrace.GetBoolean(result),
        shader);
#endif
    return result;
}

void APIENTRY glLinkProgram(GLuint program)
{
    if (sglLinkProgram)
    {
        sglLinkProgram(program);
        ReportGLError("glLinkProgram");
    }
    else
    {
        ReportGLNullFunction("glLinkProgram");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glLinkProgram", "",
        program);
#endif
}

void APIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    if (sglShaderSource)
    {
        sglShaderSource(shader, count, string, length);
        ReportGLError("glShaderSource");
    }
    else
    {
        ReportGLNullFunction("glShaderSource");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glShaderSource", "",
        shader, count, "strings", "lengths");
#endif
}

void APIENTRY glUseProgram(GLuint program)
{
    if (sglUseProgram)
    {
        sglUseProgram(program);
        ReportGLError("glUseProgram");
    }
    else
    {
        ReportGLNullFunction("glUseProgram");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUseProgram", "",
        program);
#endif
}

void APIENTRY glUniform1f(GLint location, GLfloat v0)
{
    if (sglUniform1f)
    {
        sglUniform1f(location, v0);
        ReportGLError("glUniform1f");
    }
    else
    {
        ReportGLNullFunction("glUniform1f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1f", "",
        location, v0);
#endif
}

void APIENTRY glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    if (sglUniform2f)
    {
        sglUniform2f(location, v0, v1);
        ReportGLError("glUniform2f");
    }
    else
    {
        ReportGLNullFunction("glUniform2f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2f", "",
        location, v0, v1);
#endif
}

void APIENTRY glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if (sglUniform3f)
    {
        sglUniform3f(location, v0, v1, v2);
        ReportGLError("glUniform3f");
    }
    else
    {
        ReportGLNullFunction("glUniform3f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3f", "",
        location, v0, v1, v2);
#endif
}

void APIENTRY glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if (sglUniform4f)
    {
        sglUniform4f(location, v0, v1, v2, v3);
        ReportGLError("glUniform4f");
    }
    else
    {
        ReportGLNullFunction("glUniform4f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4f", "",
        location, v0, v1, v2, v3);
#endif
}

void APIENTRY glUniform1i(GLint location, GLint v0)
{
    if (sglUniform1i)
    {
        sglUniform1i(location, v0);
        ReportGLError("glUniform1i");
    }
    else
    {
        ReportGLNullFunction("glUniform1i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1i", "",
        location, v0);
#endif
}

void APIENTRY glUniform2i(GLint location, GLint v0, GLint v1)
{
    if (sglUniform2i)
    {
        sglUniform2i(location, v0, v1);
        ReportGLError("glUniform2i");
    }
    else
    {
        ReportGLNullFunction("glUniform2i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2i", "",
        location, v0, v1);
#endif
}

void APIENTRY glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    if (sglUniform3i)
    {
        sglUniform3i(location, v0, v1, v2);
        ReportGLError("glUniform3i");
    }
    else
    {
        ReportGLNullFunction("glUniform3i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3i", "",
        location, v0, v1, v2);
#endif
}

void APIENTRY glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if (sglUniform4i)
    {
        sglUniform4i(location, v0, v1, v2, v3);
        ReportGLError("glUniform4i");
    }
    else
    {
        ReportGLNullFunction("glUniform4i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4i", "",
        location, v0, v1, v2, v3);
#endif
}

void APIENTRY glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
    if (sglUniform1fv)
    {
        sglUniform1fv(location, count, value);
        ReportGLError("glUniform1fv");
    }
    else
    {
        ReportGLNullFunction("glUniform1fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1fv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
    if (sglUniform2fv)
    {
        sglUniform2fv(location, count, value);
        ReportGLError("glUniform2fv");
    }
    else
    {
        ReportGLNullFunction("glUniform2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2fv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    if (sglUniform3fv)
    {
        sglUniform3fv(location, count, value);
        ReportGLError("glUniform3fv");
    }
    else
    {
        ReportGLNullFunction("glUniform3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3fv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
    if (sglUniform4fv)
    {
        sglUniform4fv(location, count, value);
        ReportGLError("glUniform4fv");
    }
    else
    {
        ReportGLNullFunction("glUniform4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4fv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform1iv(GLint location, GLsizei count, const GLint* value)
{
    if (sglUniform1iv)
    {
        sglUniform1iv(location, count, value);
        ReportGLError("glUniform1iv");
    }
    else
    {
        ReportGLNullFunction("glUniform1iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1iv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform2iv(GLint location, GLsizei count, const GLint* value)
{
    if (sglUniform2iv)
    {
        sglUniform2iv(location, count, value);
        ReportGLError("glUniform2iv");
    }
    else
    {
        ReportGLNullFunction("glUniform2iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2iv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform3iv(GLint location, GLsizei count, const GLint* value)
{
    if (sglUniform3iv)
    {
        sglUniform3iv(location, count, value);
        ReportGLError("glUniform3iv");
    }
    else
    {
        ReportGLNullFunction("glUniform3iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3iv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform4iv(GLint location, GLsizei count, const GLint* value)
{
    if (sglUniform4iv)
    {
        sglUniform4iv(location, count, value);
        ReportGLError("glUniform4iv");
    }
    else
    {
        ReportGLNullFunction("glUniform4iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4iv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix2fv)
    {
        sglUniformMatrix2fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(4ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix3fv)
    {
        sglUniformMatrix3fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(9ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix4fv)
    {
        sglUniformMatrix4fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(16ull * count, value));
#endif
}

void APIENTRY glValidateProgram(GLuint program)
{
    if (sglValidateProgram)
    {
        sglValidateProgram(program);
        ReportGLError("glValidateProgram");
    }
    else
    {
        ReportGLNullFunction("glValidateProgram");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glValidateProgram", "",
        program);
#endif
}

void APIENTRY glVertexAttrib1d(GLuint index, GLdouble x)
{
    if (sglVertexAttrib1d)
    {
        sglVertexAttrib1d(index, x);
        ReportGLError("glVertexAttrib1d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1d", "",
        index, x);
#endif
}

void APIENTRY glVertexAttrib1dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttrib1dv)
    {
        sglVertexAttrib1dv(index, v);
        ReportGLError("glVertexAttrib1dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1dv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttrib1f(GLuint index, GLfloat x)
{
    if (sglVertexAttrib1f)
    {
        sglVertexAttrib1f(index, x);
        ReportGLError("glVertexAttrib1f");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1f", "",
        index, x);
#endif
}

void APIENTRY glVertexAttrib1fv(GLuint index, const GLfloat* v)
{
    if (sglVertexAttrib1fv)
    {
        sglVertexAttrib1fv(index, v);
        ReportGLError("glVertexAttrib1fv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1fv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttrib1s(GLuint index, GLshort x)
{
    if (sglVertexAttrib1s)
    {
        sglVertexAttrib1s(index, x);
        ReportGLError("glVertexAttrib1s");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1s");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1s", "",
        index, x);
#endif
}

void APIENTRY glVertexAttrib1sv(GLuint index, const GLshort* v)
{
    if (sglVertexAttrib1sv)
    {
        sglVertexAttrib1sv(index, v);
        ReportGLError("glVertexAttrib1sv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib1sv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib1sv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    if (sglVertexAttrib2d)
    {
        sglVertexAttrib2d(index, x, y);
        ReportGLError("glVertexAttrib2d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2d", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttrib2dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttrib2dv)
    {
        sglVertexAttrib2dv(index, v);
        ReportGLError("glVertexAttrib2dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2dv", "",
        index, gsTrace.GetArray(2, v));
#endif
}

void APIENTRY glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    if (sglVertexAttrib2f)
    {
        sglVertexAttrib2f(index, x, y);
        ReportGLError("glVertexAttrib2f");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2f", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttrib2fv(GLuint index, const GLfloat* v)
{
    if (sglVertexAttrib2fv)
    {
        sglVertexAttrib2fv(index, v);
        ReportGLError("glVertexAttrib2fv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2fv", "",
        index, gsTrace.GetArray(2, v));
#endif
}

void APIENTRY glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    if (sglVertexAttrib2s)
    {
        sglVertexAttrib2s(index, x, y);
        ReportGLError("glVertexAttrib2s");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2s");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2s", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttrib2sv(GLuint index, const GLshort* v)
{
    if (sglVertexAttrib2sv)
    {
        sglVertexAttrib2sv(index, v);
        ReportGLError("glVertexAttrib2sv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib2sv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib2sv", "",
        index, gsTrace.GetArray(2, v));
#endif
}

void APIENTRY glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    if (sglVertexAttrib3d)
    {
        sglVertexAttrib3d(index, x, y, z);
        ReportGLError("glVertexAttrib3d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3d", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttrib3dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttrib3dv)
    {
        sglVertexAttrib3dv(index, v);
        ReportGLError("glVertexAttrib3dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3dv", "",
        index, gsTrace.GetArray(3, v));
#endif
}

void APIENTRY glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    if (sglVertexAttrib3f)
    {
        sglVertexAttrib3f(index, x, y, z);
        ReportGLError("glVertexAttrib3f");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3f", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttrib3fv(GLuint index, const GLfloat* v)
{
    if (sglVertexAttrib3fv)
    {
        sglVertexAttrib3fv(index, v);
        ReportGLError("glVertexAttrib3fv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3fv", "",
        index, gsTrace.GetArray(3, v));
#endif
}

void APIENTRY glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    if (sglVertexAttrib3s)
    {
        sglVertexAttrib3s(index, x, y, z);
        ReportGLError("glVertexAttrib3s");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3s");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3s", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttrib3sv(GLuint index, const GLshort* v)
{
    if (sglVertexAttrib3sv)
    {
        sglVertexAttrib3sv(index, v);
        ReportGLError("glVertexAttrib3sv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib3sv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib3sv", "",
        index, gsTrace.GetArray(3, v));
#endif
}

void APIENTRY glVertexAttrib4Nbv(GLuint index, const GLbyte* v)
{
    if (sglVertexAttrib4Nbv)
    {
        sglVertexAttrib4Nbv(index, v);
        ReportGLError("glVertexAttrib4Nbv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nbv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nbv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4Niv(GLuint index, const GLint* v)
{
    if (sglVertexAttrib4Niv)
    {
        sglVertexAttrib4Niv(index, v);
        ReportGLError("glVertexAttrib4Niv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Niv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Niv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4Nsv(GLuint index, const GLshort* v)
{
    if (sglVertexAttrib4Nsv)
    {
        sglVertexAttrib4Nsv(index, v);
        ReportGLError("glVertexAttrib4Nsv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nsv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nsv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    if (sglVertexAttrib4Nub)
    {
        sglVertexAttrib4Nub(index, x, y, z, w);
        ReportGLError("glVertexAttrib4Nub");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nub");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nub", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttrib4Nubv(GLuint index, const GLubyte* v)
{
    if (sglVertexAttrib4Nubv)
    {
        sglVertexAttrib4Nubv(index, v);
        ReportGLError("glVertexAttrib4Nubv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nubv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nubv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4Nuiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttrib4Nuiv)
    {
        sglVertexAttrib4Nuiv(index, v);
        ReportGLError("glVertexAttrib4Nuiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nuiv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4Nusv(GLuint index, const GLushort* v)
{
    if (sglVertexAttrib4Nusv)
    {
        sglVertexAttrib4Nusv(index, v);
        ReportGLError("glVertexAttrib4Nusv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4Nusv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nusv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4bv(GLuint index, const GLbyte* v)
{
    if (sglVertexAttrib4bv)
    {
        sglVertexAttrib4bv(index, v);
        ReportGLError("glVertexAttrib4bv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4bv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4Nbv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if (sglVertexAttrib4d)
    {
        sglVertexAttrib4d(index, x, y, z, w);
        ReportGLError("glVertexAttrib4d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4d", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttrib4dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttrib4dv)
    {
        sglVertexAttrib4dv(index, v);
        ReportGLError("glVertexAttrib4dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4dv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    if (sglVertexAttrib4f)
    {
        sglVertexAttrib4f(index, x, y, z, w);
        ReportGLError("glVertexAttrib4f");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4f", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttrib4fv(GLuint index, const GLfloat* v)
{
    if (sglVertexAttrib4fv)
    {
        sglVertexAttrib4fv(index, v);
        ReportGLError("glVertexAttrib4fv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4fv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4iv(GLuint index, const GLint* v)
{
    if (sglVertexAttrib4iv)
    {
        sglVertexAttrib4iv(index, v);
        ReportGLError("glVertexAttrib4iv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4iv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    if (sglVertexAttrib4s)
    {
        sglVertexAttrib4s(index, x, y, z, w);
        ReportGLError("glVertexAttrib4s");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4s");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4s", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttrib4sv(GLuint index, const GLshort* v)
{
    if (sglVertexAttrib4sv)
    {
        sglVertexAttrib4sv(index, v);
        ReportGLError("glVertexAttrib4sv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4sv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4sv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4ubv(GLuint index, const GLubyte* v)
{
    if (sglVertexAttrib4ubv)
    {
        sglVertexAttrib4ubv(index, v);
        ReportGLError("glVertexAttrib4ubv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4ubv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4ubv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4uiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttrib4uiv)
    {
        sglVertexAttrib4uiv(index, v);
        ReportGLError("glVertexAttrib4uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4uiv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttrib4usv(GLuint index, const GLushort* v)
{
    if (sglVertexAttrib4usv)
    {
        sglVertexAttrib4usv(index, v);
        ReportGLError("glVertexAttrib4usv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttrib4usv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttrib4usv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    if (sglVertexAttribPointer)
    {
        sglVertexAttribPointer(index, size, type, normalized, stride, pointer);
        ReportGLError("glVertexAttribPointer");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribPointer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribPointer", "",
        index, size, gsTrace.GetName(type), gsTrace.GetBoolean(normalized),
        stride, gsTrace.GetArray(size, static_cast<std::uint64_t const*>(pointer)));
#endif
}

static void Initialize_OPENGL_VERSION_2_0()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_2_0)
    {
        GetOpenGLFunction("glBlendEquationSeparate", sglBlendEquationSeparate);
        GetOpenGLFunction("glDrawBuffers", sglDrawBuffers);
        GetOpenGLFunction("glStencilOpSeparate", sglStencilOpSeparate);
        GetOpenGLFunction("glStencilFuncSeparate", sglStencilFuncSeparate);
        GetOpenGLFunction("glStencilMaskSeparate", sglStencilMaskSeparate);
        GetOpenGLFunction("glAttachShader", sglAttachShader);
        GetOpenGLFunction("glBindAttribLocation", sglBindAttribLocation);
        GetOpenGLFunction("glCompileShader", sglCompileShader);
        GetOpenGLFunction("glCreateProgram", sglCreateProgram);
        GetOpenGLFunction("glCreateShader", sglCreateShader);
        GetOpenGLFunction("glDeleteProgram", sglDeleteProgram);
        GetOpenGLFunction("glDeleteShader", sglDeleteShader);
        GetOpenGLFunction("glDetachShader", sglDetachShader);
        GetOpenGLFunction("glDisableVertexAttribArray", sglDisableVertexAttribArray);
        GetOpenGLFunction("glEnableVertexAttribArray", sglEnableVertexAttribArray);
        GetOpenGLFunction("glGetActiveAttrib", sglGetActiveAttrib);
        GetOpenGLFunction("glGetActiveUniform", sglGetActiveUniform);
        GetOpenGLFunction("glGetAttachedShaders", sglGetAttachedShaders);
        GetOpenGLFunction("glGetAttribLocation", sglGetAttribLocation);
        GetOpenGLFunction("glGetProgramiv", sglGetProgramiv);
        GetOpenGLFunction("glGetProgramInfoLog", sglGetProgramInfoLog);
        GetOpenGLFunction("glGetShaderiv", sglGetShaderiv);
        GetOpenGLFunction("glGetShaderInfoLog", sglGetShaderInfoLog);
        GetOpenGLFunction("glGetShaderSource", sglGetShaderSource);
        GetOpenGLFunction("glGetUniformLocation", sglGetUniformLocation);
        GetOpenGLFunction("glGetUniformfv", sglGetUniformfv);
        GetOpenGLFunction("glGetUniformiv", sglGetUniformiv);
        GetOpenGLFunction("glGetVertexAttribdv", sglGetVertexAttribdv);
        GetOpenGLFunction("glGetVertexAttribfv", sglGetVertexAttribfv);
        GetOpenGLFunction("glGetVertexAttribiv", sglGetVertexAttribiv);
        GetOpenGLFunction("glGetVertexAttribPointerv", sglGetVertexAttribPointerv);
        GetOpenGLFunction("glIsProgram", sglIsProgram);
        GetOpenGLFunction("glIsShader", sglIsShader);
        GetOpenGLFunction("glLinkProgram", sglLinkProgram);
        GetOpenGLFunction("glShaderSource", sglShaderSource);
        GetOpenGLFunction("glUseProgram", sglUseProgram);
        GetOpenGLFunction("glUniform1f", sglUniform1f);
        GetOpenGLFunction("glUniform2f", sglUniform2f);
        GetOpenGLFunction("glUniform3f", sglUniform3f);
        GetOpenGLFunction("glUniform4f", sglUniform4f);
        GetOpenGLFunction("glUniform1i", sglUniform1i);
        GetOpenGLFunction("glUniform2i", sglUniform2i);
        GetOpenGLFunction("glUniform3i", sglUniform3i);
        GetOpenGLFunction("glUniform4i", sglUniform4i);
        GetOpenGLFunction("glUniform1fv", sglUniform1fv);
        GetOpenGLFunction("glUniform2fv", sglUniform2fv);
        GetOpenGLFunction("glUniform3fv", sglUniform3fv);
        GetOpenGLFunction("glUniform4fv", sglUniform4fv);
        GetOpenGLFunction("glUniform1iv", sglUniform1iv);
        GetOpenGLFunction("glUniform2iv", sglUniform2iv);
        GetOpenGLFunction("glUniform3iv", sglUniform3iv);
        GetOpenGLFunction("glUniform4iv", sglUniform4iv);
        GetOpenGLFunction("glUniformMatrix2fv", sglUniformMatrix2fv);
        GetOpenGLFunction("glUniformMatrix3fv", sglUniformMatrix3fv);
        GetOpenGLFunction("glUniformMatrix4fv", sglUniformMatrix4fv);
        GetOpenGLFunction("glValidateProgram", sglValidateProgram);
        GetOpenGLFunction("glVertexAttrib1d", sglVertexAttrib1d);
        GetOpenGLFunction("glVertexAttrib1dv", sglVertexAttrib1dv);
        GetOpenGLFunction("glVertexAttrib1f", sglVertexAttrib1f);
        GetOpenGLFunction("glVertexAttrib1fv", sglVertexAttrib1fv);
        GetOpenGLFunction("glVertexAttrib1s", sglVertexAttrib1s);
        GetOpenGLFunction("glVertexAttrib1sv", sglVertexAttrib1sv);
        GetOpenGLFunction("glVertexAttrib2d", sglVertexAttrib2d);
        GetOpenGLFunction("glVertexAttrib2dv", sglVertexAttrib2dv);
        GetOpenGLFunction("glVertexAttrib2f", sglVertexAttrib2f);
        GetOpenGLFunction("glVertexAttrib2fv", sglVertexAttrib2fv);
        GetOpenGLFunction("glVertexAttrib2s", sglVertexAttrib2s);
        GetOpenGLFunction("glVertexAttrib2sv", sglVertexAttrib2sv);
        GetOpenGLFunction("glVertexAttrib3d", sglVertexAttrib3d);
        GetOpenGLFunction("glVertexAttrib3dv", sglVertexAttrib3dv);
        GetOpenGLFunction("glVertexAttrib3f", sglVertexAttrib3f);
        GetOpenGLFunction("glVertexAttrib3fv", sglVertexAttrib3fv);
        GetOpenGLFunction("glVertexAttrib3s", sglVertexAttrib3s);
        GetOpenGLFunction("glVertexAttrib3sv", sglVertexAttrib3sv);
        GetOpenGLFunction("glVertexAttrib4Nbv", sglVertexAttrib4Nbv);
        GetOpenGLFunction("glVertexAttrib4Niv", sglVertexAttrib4Niv);
        GetOpenGLFunction("glVertexAttrib4Nsv", sglVertexAttrib4Nsv);
        GetOpenGLFunction("glVertexAttrib4Nub", sglVertexAttrib4Nub);
        GetOpenGLFunction("glVertexAttrib4Nubv", sglVertexAttrib4Nubv);
        GetOpenGLFunction("glVertexAttrib4Nuiv", sglVertexAttrib4Nuiv);
        GetOpenGLFunction("glVertexAttrib4Nusv", sglVertexAttrib4Nusv);
        GetOpenGLFunction("glVertexAttrib4bv", sglVertexAttrib4bv);
        GetOpenGLFunction("glVertexAttrib4d", sglVertexAttrib4d);
        GetOpenGLFunction("glVertexAttrib4dv", sglVertexAttrib4dv);
        GetOpenGLFunction("glVertexAttrib4f", sglVertexAttrib4f);
        GetOpenGLFunction("glVertexAttrib4fv", sglVertexAttrib4fv);
        GetOpenGLFunction("glVertexAttrib4iv", sglVertexAttrib4iv);
        GetOpenGLFunction("glVertexAttrib4s", sglVertexAttrib4s);
        GetOpenGLFunction("glVertexAttrib4sv", sglVertexAttrib4sv);
        GetOpenGLFunction("glVertexAttrib4ubv", sglVertexAttrib4ubv);
        GetOpenGLFunction("glVertexAttrib4uiv", sglVertexAttrib4uiv);
        GetOpenGLFunction("glVertexAttrib4usv", sglVertexAttrib4usv);
        GetOpenGLFunction("glVertexAttribPointer", sglVertexAttribPointer);
    }
}

// GL_VERSION_2_1

static PFNGLUNIFORMMATRIX2X3FVPROC sglUniformMatrix2x3fv = nullptr;
static PFNGLUNIFORMMATRIX3X2FVPROC sglUniformMatrix3x2fv = nullptr;
static PFNGLUNIFORMMATRIX2X4FVPROC sglUniformMatrix2x4fv = nullptr;
static PFNGLUNIFORMMATRIX4X2FVPROC sglUniformMatrix4x2fv = nullptr;
static PFNGLUNIFORMMATRIX3X4FVPROC sglUniformMatrix3x4fv = nullptr;
static PFNGLUNIFORMMATRIX4X3FVPROC sglUniformMatrix4x3fv = nullptr;

void APIENTRY glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix2x3fv)
    {
        sglUniformMatrix2x3fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2x3fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2x3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2x3fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix3x2fv)
    {
        sglUniformMatrix3x2fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3x2fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3x2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3x2fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix2x4fv)
    {
        sglUniformMatrix2x4fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2x4fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2x4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2x4fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix4x2fv)
    {
        sglUniformMatrix4x2fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4x2fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4x2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4x2fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix3x4fv)
    {
        sglUniformMatrix3x4fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3x4fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3x4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3x4fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglUniformMatrix4x3fv)
    {
        sglUniformMatrix4x3fv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4x3fv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4x3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4x3fv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

static void Initialize_OPENGL_VERSION_2_1()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_2_1)
    {
        GetOpenGLFunction("glUniformMatrix2x3fv", sglUniformMatrix2x3fv);
        GetOpenGLFunction("glUniformMatrix3x2fv", sglUniformMatrix3x2fv);
        GetOpenGLFunction("glUniformMatrix2x4fv", sglUniformMatrix2x4fv);
        GetOpenGLFunction("glUniformMatrix4x2fv", sglUniformMatrix4x2fv);
        GetOpenGLFunction("glUniformMatrix3x4fv", sglUniformMatrix3x4fv);
        GetOpenGLFunction("glUniformMatrix4x3fv", sglUniformMatrix4x3fv);
    }
}

// GL_VERSION_3_0

static PFNGLCOLORMASKIPROC sglColorMaski = nullptr;
static PFNGLGETBOOLEANI_VPROC sglGetBooleani_v = nullptr;
static PFNGLGETINTEGERI_VPROC sglGetIntegeri_v = nullptr;
static PFNGLENABLEIPROC sglEnablei = nullptr;
static PFNGLDISABLEIPROC sglDisablei = nullptr;
static PFNGLISENABLEDIPROC sglIsEnabledi = nullptr;
static PFNGLBEGINTRANSFORMFEEDBACKPROC sglBeginTransformFeedback = nullptr;
static PFNGLENDTRANSFORMFEEDBACKPROC sglEndTransformFeedback = nullptr;
static PFNGLBINDBUFFERRANGEPROC sglBindBufferRange = nullptr;
static PFNGLBINDBUFFERBASEPROC sglBindBufferBase = nullptr;
static PFNGLTRANSFORMFEEDBACKVARYINGSPROC sglTransformFeedbackVaryings = nullptr;
static PFNGLGETTRANSFORMFEEDBACKVARYINGPROC sglGetTransformFeedbackVarying = nullptr;
static PFNGLCLAMPCOLORPROC sglClampColor = nullptr;
static PFNGLBEGINCONDITIONALRENDERPROC sglBeginConditionalRender = nullptr;
static PFNGLENDCONDITIONALRENDERPROC sglEndConditionalRender = nullptr;
static PFNGLVERTEXATTRIBIPOINTERPROC sglVertexAttribIPointer = nullptr;
static PFNGLGETVERTEXATTRIBIIVPROC sglGetVertexAttribIiv = nullptr;
static PFNGLGETVERTEXATTRIBIUIVPROC sglGetVertexAttribIuiv = nullptr;
static PFNGLVERTEXATTRIBI1IPROC sglVertexAttribI1i = nullptr;
static PFNGLVERTEXATTRIBI2IPROC sglVertexAttribI2i = nullptr;
static PFNGLVERTEXATTRIBI3IPROC sglVertexAttribI3i = nullptr;
static PFNGLVERTEXATTRIBI4IPROC sglVertexAttribI4i = nullptr;
static PFNGLVERTEXATTRIBI1UIPROC sglVertexAttribI1ui = nullptr;
static PFNGLVERTEXATTRIBI2UIPROC sglVertexAttribI2ui = nullptr;
static PFNGLVERTEXATTRIBI3UIPROC sglVertexAttribI3ui = nullptr;
static PFNGLVERTEXATTRIBI4UIPROC sglVertexAttribI4ui = nullptr;
static PFNGLVERTEXATTRIBI1IVPROC sglVertexAttribI1iv = nullptr;
static PFNGLVERTEXATTRIBI2IVPROC sglVertexAttribI2iv = nullptr;
static PFNGLVERTEXATTRIBI3IVPROC sglVertexAttribI3iv = nullptr;
static PFNGLVERTEXATTRIBI4IVPROC sglVertexAttribI4iv = nullptr;
static PFNGLVERTEXATTRIBI1UIVPROC sglVertexAttribI1uiv = nullptr;
static PFNGLVERTEXATTRIBI2UIVPROC sglVertexAttribI2uiv = nullptr;
static PFNGLVERTEXATTRIBI3UIVPROC sglVertexAttribI3uiv = nullptr;
static PFNGLVERTEXATTRIBI4UIVPROC sglVertexAttribI4uiv = nullptr;
static PFNGLVERTEXATTRIBI4BVPROC sglVertexAttribI4bv = nullptr;
static PFNGLVERTEXATTRIBI4SVPROC sglVertexAttribI4sv = nullptr;
static PFNGLVERTEXATTRIBI4UBVPROC sglVertexAttribI4ubv = nullptr;
static PFNGLVERTEXATTRIBI4USVPROC sglVertexAttribI4usv = nullptr;
static PFNGLGETUNIFORMUIVPROC sglGetUniformuiv = nullptr;
static PFNGLBINDFRAGDATALOCATIONPROC sglBindFragDataLocation = nullptr;
static PFNGLGETFRAGDATALOCATIONPROC sglGetFragDataLocation = nullptr;
static PFNGLUNIFORM1UIPROC sglUniform1ui = nullptr;
static PFNGLUNIFORM2UIPROC sglUniform2ui = nullptr;
static PFNGLUNIFORM3UIPROC sglUniform3ui = nullptr;
static PFNGLUNIFORM4UIPROC sglUniform4ui = nullptr;
static PFNGLUNIFORM1UIVPROC sglUniform1uiv = nullptr;
static PFNGLUNIFORM2UIVPROC sglUniform2uiv = nullptr;
static PFNGLUNIFORM3UIVPROC sglUniform3uiv = nullptr;
static PFNGLUNIFORM4UIVPROC sglUniform4uiv = nullptr;
static PFNGLTEXPARAMETERIIVPROC sglTexParameterIiv = nullptr;
static PFNGLTEXPARAMETERIUIVPROC sglTexParameterIuiv = nullptr;
static PFNGLGETTEXPARAMETERIIVPROC sglGetTexParameterIiv = nullptr;
static PFNGLGETTEXPARAMETERIUIVPROC sglGetTexParameterIuiv = nullptr;
static PFNGLCLEARBUFFERIVPROC sglClearBufferiv = nullptr;
static PFNGLCLEARBUFFERUIVPROC sglClearBufferuiv = nullptr;
static PFNGLCLEARBUFFERFVPROC sglClearBufferfv = nullptr;
static PFNGLCLEARBUFFERFIPROC sglClearBufferfi = nullptr;
static PFNGLGETSTRINGIPROC sglGetStringi = nullptr;
static PFNGLISRENDERBUFFERPROC sglIsRenderbuffer = nullptr;
static PFNGLBINDRENDERBUFFERPROC sglBindRenderbuffer = nullptr;
static PFNGLDELETERENDERBUFFERSPROC sglDeleteRenderbuffers = nullptr;
static PFNGLGENRENDERBUFFERSPROC sglGenRenderbuffers = nullptr;
static PFNGLRENDERBUFFERSTORAGEPROC sglRenderbufferStorage = nullptr;
static PFNGLGETRENDERBUFFERPARAMETERIVPROC sglGetRenderbufferParameteriv = nullptr;
static PFNGLISFRAMEBUFFERPROC sglIsFramebuffer = nullptr;
static PFNGLBINDFRAMEBUFFERPROC sglBindFramebuffer = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC sglDeleteFramebuffers = nullptr;
static PFNGLGENFRAMEBUFFERSPROC sglGenFramebuffers = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC sglCheckFramebufferStatus = nullptr;
static PFNGLFRAMEBUFFERTEXTURE1DPROC sglFramebufferTexture1D = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC sglFramebufferTexture2D = nullptr;
static PFNGLFRAMEBUFFERTEXTURE3DPROC sglFramebufferTexture3D = nullptr;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC sglFramebufferRenderbuffer = nullptr;
static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC sglGetFramebufferAttachmentParameteriv = nullptr;
static PFNGLGENERATEMIPMAPPROC sglGenerateMipmap = nullptr;
static PFNGLBLITFRAMEBUFFERPROC sglBlitFramebuffer = nullptr;
static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC sglRenderbufferStorageMultisample = nullptr;
static PFNGLFRAMEBUFFERTEXTURELAYERPROC sglFramebufferTextureLayer = nullptr;
static PFNGLMAPBUFFERRANGEPROC sglMapBufferRange = nullptr;
static PFNGLFLUSHMAPPEDBUFFERRANGEPROC sglFlushMappedBufferRange = nullptr;
static PFNGLBINDVERTEXARRAYPROC sglBindVertexArray = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC sglDeleteVertexArrays = nullptr;
static PFNGLGENVERTEXARRAYSPROC sglGenVertexArrays = nullptr;
static PFNGLISVERTEXARRAYPROC sglIsVertexArray = nullptr;

void APIENTRY glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    if (sglColorMaski)
    {
        sglColorMaski(index, r, g, b, a);
        ReportGLError("glColorMaski");
    }
    else
    {
        ReportGLNullFunction("glColorMaski");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glColorMaski", "",
        index, gsTrace.GetBoolean(r), gsTrace.GetBoolean(g), gsTrace.GetBoolean(b), gsTrace.GetBoolean(a));
#endif
}

void APIENTRY glGetBooleani_v(GLenum target, GLuint index, GLboolean* data)
{
    if (sglGetBooleani_v)
    {
        sglGetBooleani_v(target, index, data);
        ReportGLError("glGetBooleani_v");
    }
    else
    {
        ReportGLNullFunction("glGetBooleani_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetBooleani_v", "",
        gsTrace.GetBoolean(target), index, gsTrace.GetBoolean(*data));
#endif
}

void APIENTRY glGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
    if (sglGetIntegeri_v)
    {
        sglGetIntegeri_v(target, index, data);
        ReportGLError("glGetIntegeri_v");
    }
    else
    {
        ReportGLNullFunction("glGetIntegeri_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetIntegeri_v", "",
        gsTrace.GetBoolean(target), index, *data);
#endif
}

void APIENTRY glEnablei(GLenum target, GLuint index)
{
    if (sglEnablei)
    {
        sglEnablei(target, index);
        ReportGLError("glEnablei");
    }
    else
    {
        ReportGLNullFunction("glEnablei");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEnablei", "",
        gsTrace.GetBoolean(target), index);
#endif
}

void APIENTRY glDisablei(GLenum target, GLuint index)
{
    if (sglDisablei)
    {
        sglDisablei(target, index);
        ReportGLError("glDisablei");
    }
    else
    {
        ReportGLNullFunction("glDisablei");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDisablei", "",
        gsTrace.GetBoolean(target), index);
#endif
}

GLboolean APIENTRY glIsEnabledi(GLenum target, GLuint index)
{
    GLboolean result;
    if (sglIsEnabledi)
    {
        result = sglIsEnabledi(target, index);
        ReportGLError("glIsEnabledi");
    }
    else
    {
        ReportGLNullFunction("glIsEnabledi");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsEnabledi", gsTrace.GetBoolean(result),
        gsTrace.GetBoolean(target), index);
#endif
    return result;
}

void APIENTRY glBeginTransformFeedback(GLenum primitiveMode)
{
    if (sglBeginTransformFeedback)
    {
        sglBeginTransformFeedback(primitiveMode);
        ReportGLError("glBeginTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glBeginTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBeginTransformFeedback", "",
        gsTrace.GetBoolean(primitiveMode));
#endif
}

void APIENTRY glEndTransformFeedback()
{
    if (sglEndTransformFeedback)
    {
        sglEndTransformFeedback();
        ReportGLError("glEndTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glEndTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEndTransformFeedback", "");
#endif
}

void APIENTRY glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if (sglBindBufferRange)
    {
        sglBindBufferRange(target, index, buffer, offset, size);
        ReportGLError("glBindBufferRange");
    }
    else
    {
        ReportGLNullFunction("glBindBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindBufferRange", "",
        gsTrace.GetBoolean(target), index, buffer, offset, size);
#endif
}

void APIENTRY glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    if (sglBindBufferBase)
    {
        sglBindBufferBase(target, index, buffer);
        ReportGLError("glBindBufferBase");
    }
    else
    {
        ReportGLNullFunction("glBindBufferBase");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindBufferBase", "",
        gsTrace.GetName(target), index, buffer);
#endif
}

void APIENTRY glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    if (sglTransformFeedbackVaryings)
    {
        sglTransformFeedbackVaryings(program, count, varyings, bufferMode);
        ReportGLError("glTransformFeedbackVaryings");
    }
    else
    {
        ReportGLNullFunction("glTransformFeedbackVaryings");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTransformFeedbackVaryings", "",
        program, count, gsTrace.GetStringArray(count, varyings), gsTrace.GetName(bufferMode));
#endif
}

void APIENTRY glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    if (sglGetTransformFeedbackVarying)
    {
        sglGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
        ReportGLError("glGetTransformFeedbackVarying");
    }
    else
    {
        ReportGLNullFunction("glGetTransformFeedbackVarying");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTransformFeedbackVaryings", "",
        program, index, bufSize, *length, *size, gsTrace.GetName(*type), name);
#endif
}

void APIENTRY glClampColor(GLenum target, GLenum clamp)
{
    if (sglClampColor)
    {
        sglClampColor(target, clamp);
        ReportGLError("glClampColor");
    }
    else
    {
        ReportGLNullFunction("glClampColor");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClampColor", "",
        gsTrace.GetName(target), gsTrace.GetName(clamp));
#endif
}

void APIENTRY glBeginConditionalRender(GLuint id, GLenum mode)
{
    if (sglBeginConditionalRender)
    {
        sglBeginConditionalRender(id, mode);
        ReportGLError("glBeginConditionalRender");
    }
    else
    {
        ReportGLNullFunction("glBeginConditionalRender");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBeginConditionalRender", "",
        id, gsTrace.GetName(mode));
#endif
}

void APIENTRY glEndConditionalRender()
{
    if (sglEndConditionalRender)
    {
        sglEndConditionalRender();
        ReportGLError("glEndConditionalRender");
    }
    else
    {
        ReportGLNullFunction("glEndConditionalRender");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEndConditionalRender", "");
#endif
}

void APIENTRY glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    if (sglVertexAttribIPointer)
    {
        sglVertexAttribIPointer(index, size, type, stride, pointer);
        ReportGLError("glVertexAttribIPointer");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribIPointer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribIPointer", "",
        index, size, gsTrace.GetName(type), stride, "pointer");
#endif
}

void APIENTRY glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
    if (sglGetVertexAttribIiv)
    {
        sglGetVertexAttribIiv(index, pname, params);
        ReportGLError("glGetVertexAttribIiv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribIiv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
    if (sglGetVertexAttribIuiv)
    {
        sglGetVertexAttribIuiv(index, pname, params);
        ReportGLError("glGetVertexAttribIuiv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribIuiv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glVertexAttribI1i(GLuint index, GLint x)
{
    if (sglVertexAttribI1i)
    {
        sglVertexAttribI1i(index, x);
        ReportGLError("glVertexAttribI1i");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI1i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI1i", "",
        index, x);
#endif
}

void APIENTRY glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
    if (sglVertexAttribI2i)
    {
        sglVertexAttribI2i(index, x, y);
        ReportGLError("glVertexAttribI2i");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI2i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI2i", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
    if (sglVertexAttribI3i)
    {
        sglVertexAttribI3i(index, x, y, z);
        ReportGLError("glVertexAttribI3i");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI3i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI3i", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    if (sglVertexAttribI4i)
    {
        sglVertexAttribI4i(index, x, y, z, w);
        ReportGLError("glVertexAttribI4i");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4i", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttribI1ui(GLuint index, GLuint x)
{
    if (sglVertexAttribI1ui)
    {
        sglVertexAttribI1ui(index, x);
        ReportGLError("glVertexAttribI1ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI1ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI1ui", "",
        index, x);
#endif
}

void APIENTRY glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
    if (sglVertexAttribI2ui)
    {
        sglVertexAttribI2ui(index, x, y);
        ReportGLError("glVertexAttribI2ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI2ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI2ui", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
    if (sglVertexAttribI3ui)
    {
        sglVertexAttribI3ui(index, x, y, z);
        ReportGLError("glVertexAttribI3ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI3ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI3ui", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    if (sglVertexAttribI4ui)
    {
        sglVertexAttribI4ui(index, x, y, z, w);
        ReportGLError("glVertexAttribI4ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4ui", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttribI1iv(GLuint index, const GLint* v)
{
    if (sglVertexAttribI1iv)
    {
        sglVertexAttribI1iv(index, v);
        ReportGLError("glVertexAttribI1iv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI1iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI1iv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI2iv(GLuint index, const GLint* v)
{
    if (sglVertexAttribI2iv)
    {
        sglVertexAttribI2iv(index, v);
        ReportGLError("glVertexAttribI2iv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI2iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI2iv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI3iv(GLuint index, const GLint* v)
{
    if (sglVertexAttribI3iv)
    {
        sglVertexAttribI3iv(index, v);
        ReportGLError("glVertexAttribI3iv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI3iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI3iv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4iv(GLuint index, const GLint* v)
{
    if (sglVertexAttribI4iv)
    {
        sglVertexAttribI4iv(index, v);
        ReportGLError("glVertexAttribI4iv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4iv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI1uiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttribI1uiv)
    {
        sglVertexAttribI1uiv(index, v);
        ReportGLError("glVertexAttribI1uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI1uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI1uiv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI2uiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttribI2uiv)
    {
        sglVertexAttribI2uiv(index, v);
        ReportGLError("glVertexAttribI2uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI2uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI2uiv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI3uiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttribI3uiv)
    {
        sglVertexAttribI3uiv(index, v);
        ReportGLError("glVertexAttribI3uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI3uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI3uiv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4uiv(GLuint index, const GLuint* v)
{
    if (sglVertexAttribI4uiv)
    {
        sglVertexAttribI4uiv(index, v);
        ReportGLError("glVertexAttribI4uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4uiv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4bv(GLuint index, const GLbyte* v)
{
    if (sglVertexAttribI4bv)
    {
        sglVertexAttribI4bv(index, v);
        ReportGLError("glVertexAttribI4bv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4bv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4bv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4sv(GLuint index, const GLshort* v)
{
    if (sglVertexAttribI4sv)
    {
        sglVertexAttribI4sv(index, v);
        ReportGLError("glVertexAttribI4sv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4sv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4sv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4ubv(GLuint index, const GLubyte* v)
{
    if (sglVertexAttribI4ubv)
    {
        sglVertexAttribI4ubv(index, v);
        ReportGLError("glVertexAttribI4ubv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4ubv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4ubv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribI4usv(GLuint index, const GLushort* v)
{
    if (sglVertexAttribI4usv)
    {
        sglVertexAttribI4usv(index, v);
        ReportGLError("glVertexAttribI4usv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribI4usv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribI4usv", "",
        index, *v);
#endif
}

void APIENTRY glGetUniformuiv(GLuint program, GLint location, GLuint* params)
{
    if (sglGetUniformuiv)
    {
        sglGetUniformuiv(program, location, params);
        ReportGLError("glGetUniformuiv");
    }
    else
    {
        ReportGLNullFunction("glGetUniformuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformuiv", "",
        program, location, *params);
#endif
}

void APIENTRY glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name)
{
    if (sglBindFragDataLocation)
    {
        sglBindFragDataLocation(program, color, name);
        ReportGLError("glBindFragDataLocation");
    }
    else
    {
        ReportGLNullFunction("glBindFragDataLocation");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindFragDataLocation", "",
        program, color, name);
#endif
}

GLint APIENTRY glGetFragDataLocation(GLuint program, const GLchar* name)
{
    GLint result;
    if (sglGetFragDataLocation)
    {
        result = sglGetFragDataLocation(program, name);
        ReportGLError("glGetFragDataLocation");
    }
    else
    {
        ReportGLNullFunction("glGetFragDataLocation");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetFragDataLocation", std::to_string(result),
        program, name);
#endif
    return result;
}

void APIENTRY glUniform1ui(GLint location, GLuint v0)
{
    if (sglUniform1ui)
    {
        sglUniform1ui(location, v0);
        ReportGLError("glUniform1ui");
    }
    else
    {
        ReportGLNullFunction("glUniform1ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1ui", "",
        location, v0);
#endif
}

void APIENTRY glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    if (sglUniform2ui)
    {
        sglUniform2ui(location, v0, v1);
        ReportGLError("glUniform2ui");
    }
    else
    {
        ReportGLNullFunction("glUniform2ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2ui", "",
        location, v0, v1);
#endif
}

void APIENTRY glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if (sglUniform3ui)
    {
        sglUniform3ui(location, v0, v1, v2);
        ReportGLError("glUniform3ui");
    }
    else
    {
        ReportGLNullFunction("glUniform3ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3ui", "",
        location, v0, v1, v2);
#endif
}

void APIENTRY glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if (sglUniform4ui)
    {
        sglUniform4ui(location, v0, v1, v2, v3);
        ReportGLError("glUniform4ui");
    }
    else
    {
        ReportGLNullFunction("glUniform4ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4ui", "",
        location, v0, v1, v2, v3);
#endif
}

void APIENTRY glUniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
    if (sglUniform1uiv)
    {
        sglUniform1uiv(location, count, value);
        ReportGLError("glUniform1uiv");
    }
    else
    {
        ReportGLNullFunction("glUniform1uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1uiv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
    if (sglUniform2uiv)
    {
        sglUniform2uiv(location, count, value);
        ReportGLError("glUniform2uiv");
    }
    else
    {
        ReportGLNullFunction("glUniform2uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2uiv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
    if (sglUniform3uiv)
    {
        sglUniform3uiv(location, count, value);
        ReportGLError("glUniform3uiv");
    }
    else
    {
        ReportGLNullFunction("glUniform3uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3uiv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
    if (sglUniform4uiv)
    {
        sglUniform4uiv(location, count, value);
        ReportGLError("glUniform4uiv");
    }
    else
    {
        ReportGLNullFunction("glUniform4uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4uiv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glTexParameterIiv(GLenum target, GLenum pname, const GLint* params)
{
    if (sglTexParameterIiv)
    {
        sglTexParameterIiv(target, pname, params);
        ReportGLError("glTexParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glTexParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexParameterIiv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glTexParameterIuiv(GLenum target, GLenum pname, const GLuint* params)
{
    if (sglTexParameterIuiv)
    {
        sglTexParameterIuiv(target, pname, params);
        ReportGLError("glTexParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glTexParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexParameterIuiv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetTexParameterIiv)
    {
        sglGetTexParameterIiv(target, pname, params);
        ReportGLError("glGetTexParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glGetTexParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTexParameterIiv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params)
{
    if (sglGetTexParameterIuiv)
    {
        sglGetTexParameterIuiv(target, pname, params);
        ReportGLError("glGetTexParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glGetTexParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTexParameterIuiv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    if (sglClearBufferiv)
    {
        sglClearBufferiv(buffer, drawbuffer, value);
        ReportGLError("glClearBufferiv");
    }
    else
    {
        ReportGLNullFunction("glClearBufferiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferiv", "",
        gsTrace.GetName(buffer), drawbuffer, *value);
#endif
}

void APIENTRY glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    if (sglClearBufferuiv)
    {
        sglClearBufferuiv(buffer, drawbuffer, value);
        ReportGLError("glClearBufferuiv");
    }
    else
    {
        ReportGLNullFunction("glClearBufferuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferuiv", "",
        gsTrace.GetName(buffer), drawbuffer, *value);
#endif
}

void APIENTRY glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    if (sglClearBufferfv)
    {
        sglClearBufferfv(buffer, drawbuffer, value);
        ReportGLError("glClearBufferfv");
    }
    else
    {
        ReportGLNullFunction("glClearBufferfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferfv", "",
        gsTrace.GetName(buffer), drawbuffer, *value);
#endif
}

void APIENTRY glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    if (sglClearBufferfi)
    {
        sglClearBufferfi(buffer, drawbuffer, depth, stencil);
        ReportGLError("glClearBufferfi");
    }
    else
    {
        ReportGLNullFunction("glClearBufferfi");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferfi", "",
        gsTrace.GetName(buffer), drawbuffer, depth, stencil);
#endif
}

const GLubyte* APIENTRY glGetStringi(GLenum name, GLuint index)
{
    const GLubyte* result;
    if (sglGetStringi)
    {
        result = sglGetStringi(name, index);
        ReportGLError("glGetStringi");
    }
    else
    {
        ReportGLNullFunction("glGetStringi");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetStringi", std::string(reinterpret_cast<char const*>(result)),
        gsTrace.GetName(name), index);
#endif
    return result;
}

GLboolean APIENTRY glIsRenderbuffer(GLuint renderbuffer)
{
    GLboolean result;
    if (sglIsRenderbuffer)
    {
        result = sglIsRenderbuffer(renderbuffer);
        ReportGLError("glIsRenderbuffer");
    }
    else
    {
        ReportGLNullFunction("glIsRenderbuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsRenderbuffer", gsTrace.GetBoolean(result),
        renderbuffer);
#endif
    return result;
}

void APIENTRY glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    if (sglBindRenderbuffer)
    {
        sglBindRenderbuffer(target, renderbuffer);
        ReportGLError("glBindRenderbuffer");
    }
    else
    {
        ReportGLNullFunction("glBindRenderbuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindRenderbuffer", "",
        gsTrace.GetName(target), renderbuffer);
#endif
}

void APIENTRY glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    if (sglDeleteRenderbuffers)
    {
        sglDeleteRenderbuffers(n, renderbuffers);
        ReportGLError("glDeleteRenderbuffers");
    }
    else
    {
        ReportGLNullFunction("glDeleteRenderbuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteRenderbuffers", "",
        n, gsTrace.GetArray(n, renderbuffers));
#endif
}

void APIENTRY glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    if (sglGenRenderbuffers)
    {
        sglGenRenderbuffers(n, renderbuffers);
        ReportGLError("glGenRenderbuffers");
    }
    else
    {
        ReportGLNullFunction("glGenRenderbuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenRenderbuffers", "",
        n, gsTrace.GetArray(n, renderbuffers));
#endif
}

void APIENTRY glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglRenderbufferStorage)
    {
        sglRenderbufferStorage(target, internalformat, width, height);
        ReportGLError("glRenderbufferStorage");
    }
    else
    {
        ReportGLNullFunction("glRenderbufferStorage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glRenderbufferStorage", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat), width, height);
#endif
}

void APIENTRY glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetRenderbufferParameteriv)
    {
        sglGetRenderbufferParameteriv(target, pname, params);
        ReportGLError("glGetRenderbufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetRenderbufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetRenderbufferParameteriv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

GLboolean APIENTRY glIsFramebuffer(GLuint framebuffer)
{
    GLboolean result;
    if (sglIsFramebuffer)
    {
        result = sglIsFramebuffer(framebuffer);
        ReportGLError("glIsFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glIsFramebuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsFramebuffer", gsTrace.GetBoolean(result),
        framebuffer);
#endif
    return result;
}

void APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    if (sglBindFramebuffer)
    {
        sglBindFramebuffer(target, framebuffer);
        ReportGLError("glBindFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glBindFramebuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindFramebuffer", "",
        gsTrace.GetName(target), framebuffer);
#endif
}

void APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    if (sglDeleteFramebuffers)
    {
        sglDeleteFramebuffers(n, framebuffers);
        ReportGLError("glDeleteFramebuffers");
    }
    else
    {
        ReportGLNullFunction("glDeleteFramebuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteFramebuffers", "",
        n, gsTrace.GetArray(n, framebuffers));
#endif
}

void APIENTRY glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    if (sglGenFramebuffers)
    {
        sglGenFramebuffers(n, framebuffers);
        ReportGLError("glGenFramebuffers");
    }
    else
    {
        ReportGLNullFunction("glGenFramebuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenFramebuffers", "",
        n, gsTrace.GetArray(n, framebuffers));
#endif
}

GLenum APIENTRY glCheckFramebufferStatus(GLenum target)
{
    GLenum result;
    if (sglCheckFramebufferStatus)
    {
        result = sglCheckFramebufferStatus(target);
        ReportGLError("glCheckFramebufferStatus");
    }
    else
    {
        ReportGLNullFunction("glCheckFramebufferStatus");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCheckFramebufferStatus", gsTrace.GetName(result),
        gsTrace.GetName(target));
#endif
    return result;
}

void APIENTRY glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    if (sglFramebufferTexture1D)
    {
        sglFramebufferTexture1D(target, attachment, textarget, texture, level);
        ReportGLError("glFramebufferTexture1D");
    }
    else
    {
        ReportGLNullFunction("glFramebufferTexture1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferTexture1D", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        gsTrace.GetName(textarget), texture, level);
#endif
}

void APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    if (sglFramebufferTexture2D)
    {
        sglFramebufferTexture2D(target, attachment, textarget, texture, level);
        ReportGLError("glFramebufferTexture2D");
    }
    else
    {
        ReportGLNullFunction("glFramebufferTexture2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferTexture2D", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        gsTrace.GetName(textarget), texture, level);
#endif
}

void APIENTRY glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    if (sglFramebufferTexture3D)
    {
        sglFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
        ReportGLError("glFramebufferTexture3D");
    }
    else
    {
        ReportGLNullFunction("glFramebufferTexture3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferTexture3D", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        gsTrace.GetName(textarget), texture, level, zoffset);
#endif
}

void APIENTRY glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    if (sglFramebufferRenderbuffer)
    {
        sglFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
        ReportGLError("glFramebufferRenderbuffer");
    }
    else
    {
        ReportGLNullFunction("glFramebufferRenderbuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferRenderbuffer", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        gsTrace.GetName(renderbuffertarget), renderbuffer);
#endif
}

void APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    if (sglGetFramebufferAttachmentParameteriv)
    {
        sglGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
        ReportGLError("glGetFramebufferAttachmentParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetFramebufferAttachmentParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetFramebufferAttachmentParameteriv", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGenerateMipmap(GLenum target)
{
    if (sglGenerateMipmap)
    {
        sglGenerateMipmap(target);
        ReportGLError("glGenerateMipmap");
    }
    else
    {
        ReportGLNullFunction("glGenerateMipmap");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenerateMipmap", "",
        gsTrace.GetName(target));
#endif
}

void APIENTRY glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    if (sglBlitFramebuffer)
    {
        sglBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        ReportGLError("glBlitFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glBlitFramebuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlitFramebuffer", "",
        srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
        mask, gsTrace.GetName(filter));
#endif
}

void APIENTRY glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglRenderbufferStorageMultisample)
    {
        sglRenderbufferStorageMultisample(target, samples, internalformat, width, height);
        ReportGLError("glRenderbufferStorageMultisample");
    }
    else
    {
        ReportGLNullFunction("glRenderbufferStorageMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glRenderbufferStorageMultisample", "",
        gsTrace.GetName(target), samples, gsTrace.GetName(internalformat),
        width, height);
#endif
}

void APIENTRY glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    if (sglFramebufferTextureLayer)
    {
        sglFramebufferTextureLayer(target, attachment, texture, level, layer);
        ReportGLError("glFramebufferTextureLayer");
    }
    else
    {
        ReportGLNullFunction("glFramebufferTextureLayer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferTextureLayer", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment),
        texture, level, layer);
#endif
}

void* APIENTRY glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    void* result;
    if (sglMapBufferRange)
    {
        result = sglMapBufferRange(target, offset, length, access);
        ReportGLError("glMapBufferRange");
    }
    else
    {
        ReportGLNullFunction("glMapBufferRange");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMapBufferRange", "pointer",
        gsTrace.GetName(target), offset, length, access);
#endif
    return result;
}

void APIENTRY glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    if (sglFlushMappedBufferRange)
    {
        sglFlushMappedBufferRange(target, offset, length);
        ReportGLError("glFlushMappedBufferRange");
    }
    else
    {
        ReportGLNullFunction("glFlushMappedBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFlushMappedBufferRange", "",
        gsTrace.GetName(target), offset, length);
#endif
}

void APIENTRY glBindVertexArray(GLuint array)
{
    if (sglBindVertexArray)
    {
        sglBindVertexArray(array);
        ReportGLError("glBindVertexArray");
    }
    else
    {
        ReportGLNullFunction("glBindVertexArray");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindVertexArray", "",
        array);
#endif
}

void APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    if (sglDeleteVertexArrays)
    {
        sglDeleteVertexArrays(n, arrays);
        ReportGLError("glDeleteVertexArrays");
    }
    else
    {
        ReportGLNullFunction("glDeleteVertexArrays");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteVertexArrays", "",
        n, gsTrace.GetArray(n, arrays));
#endif
}

void APIENTRY glGenVertexArrays(GLsizei n, GLuint* arrays)
{
    if (sglGenVertexArrays)
    {
        sglGenVertexArrays(n, arrays);
        ReportGLError("glGenVertexArrays");
    }
    else
    {
        ReportGLNullFunction("glGenVertexArrays");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenVertexArrays", "",
        n, gsTrace.GetArray(n, arrays));
#endif
}

GLboolean APIENTRY glIsVertexArray(GLuint array)
{
    GLboolean result;
    if (sglIsVertexArray)
    {
        result = sglIsVertexArray(array);
        ReportGLError("glIsVertexArray");
    }
    else
    {
        ReportGLNullFunction("glIsVertexArray");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsVertexArray", gsTrace.GetBoolean(result),
        array);
#endif
    return result;
}

static void Initialize_OPENGL_VERSION_3_0()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_3_0)
    {
        GetOpenGLFunction("glColorMaski", sglColorMaski);
        GetOpenGLFunction("glGetBooleani_v", sglGetBooleani_v);
        GetOpenGLFunction("glGetIntegeri_v", sglGetIntegeri_v);
        GetOpenGLFunction("glEnablei", sglEnablei);
        GetOpenGLFunction("glDisablei", sglDisablei);
        GetOpenGLFunction("glIsEnabledi", sglIsEnabledi);
        GetOpenGLFunction("glBeginTransformFeedback", sglBeginTransformFeedback);
        GetOpenGLFunction("glEndTransformFeedback", sglEndTransformFeedback);
        GetOpenGLFunction("glBindBufferRange", sglBindBufferRange);
        GetOpenGLFunction("glBindBufferBase", sglBindBufferBase);
        GetOpenGLFunction("glTransformFeedbackVaryings", sglTransformFeedbackVaryings);
        GetOpenGLFunction("glGetTransformFeedbackVarying", sglGetTransformFeedbackVarying);
        GetOpenGLFunction("glClampColor", sglClampColor);
        GetOpenGLFunction("glBeginConditionalRender", sglBeginConditionalRender);
        GetOpenGLFunction("glEndConditionalRender", sglEndConditionalRender);
        GetOpenGLFunction("glVertexAttribIPointer", sglVertexAttribIPointer);
        GetOpenGLFunction("glGetVertexAttribIiv", sglGetVertexAttribIiv);
        GetOpenGLFunction("glGetVertexAttribIuiv", sglGetVertexAttribIuiv);
        GetOpenGLFunction("glVertexAttribI1i", sglVertexAttribI1i);
        GetOpenGLFunction("glVertexAttribI2i", sglVertexAttribI2i);
        GetOpenGLFunction("glVertexAttribI3i", sglVertexAttribI3i);
        GetOpenGLFunction("glVertexAttribI4i", sglVertexAttribI4i);
        GetOpenGLFunction("glVertexAttribI1ui", sglVertexAttribI1ui);
        GetOpenGLFunction("glVertexAttribI2ui", sglVertexAttribI2ui);
        GetOpenGLFunction("glVertexAttribI3ui", sglVertexAttribI3ui);
        GetOpenGLFunction("glVertexAttribI4ui", sglVertexAttribI4ui);
        GetOpenGLFunction("glVertexAttribI1iv", sglVertexAttribI1iv);
        GetOpenGLFunction("glVertexAttribI2iv", sglVertexAttribI2iv);
        GetOpenGLFunction("glVertexAttribI3iv", sglVertexAttribI3iv);
        GetOpenGLFunction("glVertexAttribI4iv", sglVertexAttribI4iv);
        GetOpenGLFunction("glVertexAttribI1uiv", sglVertexAttribI1uiv);
        GetOpenGLFunction("glVertexAttribI2uiv", sglVertexAttribI2uiv);
        GetOpenGLFunction("glVertexAttribI3uiv", sglVertexAttribI3uiv);
        GetOpenGLFunction("glVertexAttribI4uiv", sglVertexAttribI4uiv);
        GetOpenGLFunction("glVertexAttribI4bv", sglVertexAttribI4bv);
        GetOpenGLFunction("glVertexAttribI4sv", sglVertexAttribI4sv);
        GetOpenGLFunction("glVertexAttribI4ubv", sglVertexAttribI4ubv);
        GetOpenGLFunction("glVertexAttribI4usv", sglVertexAttribI4usv);
        GetOpenGLFunction("glGetUniformuiv", sglGetUniformuiv);
        GetOpenGLFunction("glBindFragDataLocation", sglBindFragDataLocation);
        GetOpenGLFunction("glGetFragDataLocation", sglGetFragDataLocation);
        GetOpenGLFunction("glUniform1ui", sglUniform1ui);
        GetOpenGLFunction("glUniform2ui", sglUniform2ui);
        GetOpenGLFunction("glUniform3ui", sglUniform3ui);
        GetOpenGLFunction("glUniform4ui", sglUniform4ui);
        GetOpenGLFunction("glUniform1uiv", sglUniform1uiv);
        GetOpenGLFunction("glUniform2uiv", sglUniform2uiv);
        GetOpenGLFunction("glUniform3uiv", sglUniform3uiv);
        GetOpenGLFunction("glUniform4uiv", sglUniform4uiv);
        GetOpenGLFunction("glTexParameterIiv", sglTexParameterIiv);
        GetOpenGLFunction("glTexParameterIuiv", sglTexParameterIuiv);
        GetOpenGLFunction("glGetTexParameterIiv", sglGetTexParameterIiv);
        GetOpenGLFunction("glGetTexParameterIuiv", sglGetTexParameterIuiv);
        GetOpenGLFunction("glClearBufferiv", sglClearBufferiv);
        GetOpenGLFunction("glClearBufferuiv", sglClearBufferuiv);
        GetOpenGLFunction("glClearBufferfv", sglClearBufferfv);
        GetOpenGLFunction("glClearBufferfi", sglClearBufferfi);
        GetOpenGLFunction("glGetStringi", sglGetStringi);
        GetOpenGLFunction("glIsRenderbuffer", sglIsRenderbuffer);
        GetOpenGLFunction("glBindRenderbuffer", sglBindRenderbuffer);
        GetOpenGLFunction("glDeleteRenderbuffers", sglDeleteRenderbuffers);
        GetOpenGLFunction("glGenRenderbuffers", sglGenRenderbuffers);
        GetOpenGLFunction("glRenderbufferStorage", sglRenderbufferStorage);
        GetOpenGLFunction("glGetRenderbufferParameteriv", sglGetRenderbufferParameteriv);
        GetOpenGLFunction("glIsFramebuffer", sglIsFramebuffer);
        GetOpenGLFunction("glBindFramebuffer", sglBindFramebuffer);
        GetOpenGLFunction("glDeleteFramebuffers", sglDeleteFramebuffers);
        GetOpenGLFunction("glGenFramebuffers", sglGenFramebuffers);
        GetOpenGLFunction("glCheckFramebufferStatus", sglCheckFramebufferStatus);
        GetOpenGLFunction("glFramebufferTexture1D", sglFramebufferTexture1D);
        GetOpenGLFunction("glFramebufferTexture2D", sglFramebufferTexture2D);
        GetOpenGLFunction("glFramebufferTexture3D", sglFramebufferTexture3D);
        GetOpenGLFunction("glFramebufferRenderbuffer", sglFramebufferRenderbuffer);
        GetOpenGLFunction("glGetFramebufferAttachmentParameteriv", sglGetFramebufferAttachmentParameteriv);
        GetOpenGLFunction("glGenerateMipmap", sglGenerateMipmap);
        GetOpenGLFunction("glBlitFramebuffer", sglBlitFramebuffer);
        GetOpenGLFunction("glRenderbufferStorageMultisample", sglRenderbufferStorageMultisample);
        GetOpenGLFunction("glFramebufferTextureLayer", sglFramebufferTextureLayer);
        GetOpenGLFunction("glMapBufferRange", sglMapBufferRange);
        GetOpenGLFunction("glFlushMappedBufferRange", sglFlushMappedBufferRange);
        GetOpenGLFunction("glBindVertexArray", sglBindVertexArray);
        GetOpenGLFunction("glDeleteVertexArrays", sglDeleteVertexArrays);
        GetOpenGLFunction("glGenVertexArrays", sglGenVertexArrays);
        GetOpenGLFunction("glIsVertexArray", sglIsVertexArray);
    }
}

// GL_VERSION_3_1

static PFNGLDRAWARRAYSINSTANCEDPROC sglDrawArraysInstanced = nullptr;
static PFNGLDRAWELEMENTSINSTANCEDPROC sglDrawElementsInstanced = nullptr;
static PFNGLTEXBUFFERPROC sglTexBuffer = nullptr;
static PFNGLPRIMITIVERESTARTINDEXPROC sglPrimitiveRestartIndex = nullptr;
static PFNGLCOPYBUFFERSUBDATAPROC sglCopyBufferSubData = nullptr;
static PFNGLGETUNIFORMINDICESPROC sglGetUniformIndices = nullptr;
static PFNGLGETACTIVEUNIFORMSIVPROC sglGetActiveUniformsiv = nullptr;
static PFNGLGETACTIVEUNIFORMNAMEPROC sglGetActiveUniformName = nullptr;
static PFNGLGETUNIFORMBLOCKINDEXPROC sglGetUniformBlockIndex = nullptr;
static PFNGLGETACTIVEUNIFORMBLOCKIVPROC sglGetActiveUniformBlockiv = nullptr;
static PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC sglGetActiveUniformBlockName = nullptr;
static PFNGLUNIFORMBLOCKBINDINGPROC sglUniformBlockBinding = nullptr;

void APIENTRY glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    if (sglDrawArraysInstanced)
    {
        sglDrawArraysInstanced(mode, first, count, instancecount);
        ReportGLError("glDrawArraysInstanced");
    }
    else
    {
        ReportGLNullFunction("glDrawArraysInstanced");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawArraysInstanced", "",
        gsTrace.GetName(mode), first, count, instancecount);
#endif
}

void APIENTRY glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    if (sglDrawElementsInstanced)
    {
        sglDrawElementsInstanced(mode, count, type, indices, instancecount);
        ReportGLError("glDrawElementsInstanced");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsInstanced");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsInstanced", "",
        gsTrace.GetName(mode), count, gsTrace.GetName(type), "indices", instancecount);
#endif
}

void APIENTRY glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    if (sglTexBuffer)
    {
        sglTexBuffer(target, internalformat, buffer);
        ReportGLError("glTexBuffer");
    }
    else
    {
        ReportGLNullFunction("glTexBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexBuffer", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat), buffer);
#endif
}

void APIENTRY glPrimitiveRestartIndex(GLuint index)
{
    if (sglPrimitiveRestartIndex)
    {
        sglPrimitiveRestartIndex(index);
        ReportGLError("glPrimitiveRestartIndex");
    }
    else
    {
        ReportGLNullFunction("glPrimitiveRestartIndex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPrimitiveRestartIndex", "",
        index);
#endif
}

void APIENTRY glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    if (sglCopyBufferSubData)
    {
        sglCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
        ReportGLError("glCopyBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glCopyBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyBufferSubData", "",
       gsTrace.GetName(readTarget), gsTrace.GetName(writeTarget),
        readOffset, writeOffset, size);
#endif
}

void APIENTRY glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    if (sglGetUniformIndices)
    {
        sglGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
        ReportGLError("glGetUniformIndices");
    }
    else
    {
        ReportGLNullFunction("glGetUniformIndices");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformIndices", "",
        program, uniformCount, gsTrace.GetStringArray(uniformCount, uniformNames),
        gsTrace.GetArray(uniformCount, uniformIndices));
#endif
}

void APIENTRY glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    if (sglGetActiveUniformsiv)
    {
        sglGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
        ReportGLError("glGetActiveUniformsiv");
    }
    else
    {
        ReportGLNullFunction("glGetActiveUniformsiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveUniformsiv", "",
        program, uniformCount, gsTrace.GetArray(uniformCount, uniformIndices),
        gsTrace.GetName(pname), gsTrace.GetArray(uniformCount, params));
#endif
}

void APIENTRY glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName)
{
    if (sglGetActiveUniformName)
    {
        sglGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
        ReportGLError("glGetActiveUniformName");
    }
    else
    {
        ReportGLNullFunction("glGetActiveUniformName");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveUniformName", "",
        program, uniformIndex, bufSize, *length, uniformName);
#endif
}

GLuint APIENTRY glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
    GLuint result;
    if (sglGetUniformBlockIndex)
    {
        result = sglGetUniformBlockIndex(program, uniformBlockName);
        ReportGLError("glGetUniformBlockIndex");
    }
    else
    {
        ReportGLNullFunction("glGetUniformBlockIndex");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformBlockIndex", std::to_string(result),
        program, uniformBlockName);
#endif
    return result;
}

void APIENTRY glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    if (sglGetActiveUniformBlockiv)
    {
        sglGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
        ReportGLError("glGetActiveUniformBlockiv");
    }
    else
    {
        ReportGLNullFunction("glGetActiveUniformBlockiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveUniformBlockiv", "",
        program, uniformBlockIndex, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    if (sglGetActiveUniformBlockName)
    {
        sglGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
        ReportGLError("glGetActiveUniformBlockName");
    }
    else
    {
        ReportGLNullFunction("glGetActiveUniformBlockName");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveUniformBlockName", "",
        program, uniformBlockIndex, bufSize, *length, uniformBlockName);
#endif
}

void APIENTRY glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    if (sglUniformBlockBinding)
    {
        sglUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
        ReportGLError("glUniformBlockBinding");
    }
    else
    {
        ReportGLNullFunction("glUniformBlockBinding");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformBlockBinding", "",
        program, uniformBlockIndex, uniformBlockBinding);
#endif
}

static void Initialize_OPENGL_VERSION_3_1()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_3_1)
    {
        GetOpenGLFunction("glDrawArraysInstanced", sglDrawArraysInstanced);
        GetOpenGLFunction("glDrawElementsInstanced", sglDrawElementsInstanced);
        GetOpenGLFunction("glTexBuffer", sglTexBuffer);
        GetOpenGLFunction("glPrimitiveRestartIndex", sglPrimitiveRestartIndex);
        GetOpenGLFunction("glCopyBufferSubData", sglCopyBufferSubData);
        GetOpenGLFunction("glGetUniformIndices", sglGetUniformIndices);
        GetOpenGLFunction("glGetActiveUniformsiv", sglGetActiveUniformsiv);
        GetOpenGLFunction("glGetActiveUniformName", sglGetActiveUniformName);
        GetOpenGLFunction("glGetUniformBlockIndex", sglGetUniformBlockIndex);
        GetOpenGLFunction("glGetActiveUniformBlockiv", sglGetActiveUniformBlockiv);
        GetOpenGLFunction("glGetActiveUniformBlockName", sglGetActiveUniformBlockName);
        GetOpenGLFunction("glUniformBlockBinding", sglUniformBlockBinding);
    }
}

// GL_VERSION_3_2

static PFNGLDRAWELEMENTSBASEVERTEXPROC sglDrawElementsBaseVertex = nullptr;
static PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC sglDrawRangeElementsBaseVertex = nullptr;
static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC sglDrawElementsInstancedBaseVertex = nullptr;
static PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC sglMultiDrawElementsBaseVertex = nullptr;
static PFNGLPROVOKINGVERTEXPROC sglProvokingVertex = nullptr;
static PFNGLFENCESYNCPROC sglFenceSync = nullptr;
static PFNGLISSYNCPROC sglIsSync = nullptr;
static PFNGLDELETESYNCPROC sglDeleteSync = nullptr;
static PFNGLCLIENTWAITSYNCPROC sglClientWaitSync = nullptr;
static PFNGLWAITSYNCPROC sglWaitSync = nullptr;
static PFNGLGETINTEGER64VPROC sglGetInteger64v = nullptr;
static PFNGLGETSYNCIVPROC sglGetSynciv = nullptr;
static PFNGLGETINTEGER64I_VPROC sglGetInteger64i_v = nullptr;
static PFNGLGETBUFFERPARAMETERI64VPROC sglGetBufferParameteri64v = nullptr;
static PFNGLFRAMEBUFFERTEXTUREPROC sglFramebufferTexture = nullptr;
static PFNGLTEXIMAGE2DMULTISAMPLEPROC sglTexImage2DMultisample = nullptr;
static PFNGLTEXIMAGE3DMULTISAMPLEPROC sglTexImage3DMultisample = nullptr;
static PFNGLGETMULTISAMPLEFVPROC sglGetMultisamplefv = nullptr;
static PFNGLSAMPLEMASKIPROC sglSampleMaski = nullptr;

void APIENTRY glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    if (sglDrawElementsBaseVertex)
    {
        sglDrawElementsBaseVertex(mode, count, type, indices, basevertex);
        ReportGLError("glDrawElementsBaseVertex");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsBaseVertex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsBaseVertex", "",
        gsTrace.GetName(mode), count, gsTrace.GetName(type), "indices", basevertex);
#endif
}

void APIENTRY glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    if (sglDrawRangeElementsBaseVertex)
    {
        sglDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
        ReportGLError("glDrawRangeElementsBaseVertex");
    }
    else
    {
        ReportGLNullFunction("glDrawRangeElementsBaseVertex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawRangeElementsBaseVertex", "",
        gsTrace.GetName(mode), start, end, count, gsTrace.GetName(type), "indices", basevertex);
#endif
}

void APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex)
{
    if (sglDrawElementsInstancedBaseVertex)
    {
        sglDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
        ReportGLError("glDrawElementsInstancedBaseVertex");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsInstancedBaseVertex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsInstancedBaseVertex", "",
        gsTrace.GetName(mode), count, gsTrace.GetName(type), "indices",
        instancecount, basevertex);
#endif
}

void APIENTRY glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex)
{
    if (sglMultiDrawElementsBaseVertex)
    {
        sglMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
        ReportGLError("glMultiDrawElementsBaseVertex");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawElementsBaseVertex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawElementsBaseVertex", "",
        gsTrace.GetName(mode), gsTrace.GetArray(drawcount, count),
        gsTrace.GetName(type), "indices",
        drawcount, gsTrace.GetArray(drawcount, basevertex));
#endif
}

void APIENTRY glProvokingVertex(GLenum mode)
{
    if (sglProvokingVertex)
    {
        sglProvokingVertex(mode);
        ReportGLError("glProvokingVertex");
    }
    else
    {
        ReportGLNullFunction("glProvokingVertex");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProvokingVertex", "",
        gsTrace.GetName(mode));
#endif
}

GLsync APIENTRY glFenceSync(GLenum condition, GLbitfield flags)
{
    GLsync result;
    if (sglFenceSync)
    {
        result = sglFenceSync(condition, flags);
        ReportGLError("glFenceSync");
    }
    else
    {
        ReportGLNullFunction("glFenceSync");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFenceSync", "GLSync",
        gsTrace.GetName(condition), flags);
#endif
    return result;
}

GLboolean APIENTRY glIsSync(GLsync sync)
{
    GLboolean result;
    if (sglIsSync)
    {
        result = sglIsSync(sync);
        ReportGLError("glIsSync");
    }
    else
    {
        ReportGLNullFunction("glIsSync");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsSync", gsTrace.GetBoolean(result),
        "GLSync");
#endif
    return result;
}

void APIENTRY glDeleteSync(GLsync sync)
{
    if (sglDeleteSync)
    {
        sglDeleteSync(sync);
        ReportGLError("glDeleteSync");
    }
    else
    {
        ReportGLNullFunction("glDeleteSync");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteSync", "",
        "GLSync");
#endif
}

GLenum APIENTRY glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    GLenum result;
    if (sglClientWaitSync)
    {
        result = sglClientWaitSync(sync, flags, timeout);
        ReportGLError("glClientWaitSync");
    }
    else
    {
        ReportGLNullFunction("glClientWaitSync");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClientWaitSync", gsTrace.GetName(result),
        "GLSync", flags, timeout);
#endif
    return result;
}

void APIENTRY glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    if (sglWaitSync)
    {
        sglWaitSync(sync, flags, timeout);
        ReportGLError("glWaitSync");
    }
    else
    {
        ReportGLNullFunction("glWaitSync");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glWaitSync", "",
        "GLSync", flags, timeout);
#endif
}

void APIENTRY glGetInteger64v(GLenum pname, GLint64* data)
{
    if (sglGetInteger64v)
    {
        sglGetInteger64v(pname, data);
        ReportGLError("glGetInteger64v");
    }
    else
    {
        ReportGLNullFunction("glGetInteger64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetInteger64v", "",
        gsTrace.GetName(pname), *data);
#endif
}

void APIENTRY glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    if (sglGetSynciv)
    {
        sglGetSynciv(sync, pname, bufSize, length, values);
        ReportGLError("glGetSynciv");
    }
    else
    {
        ReportGLNullFunction("glGetSynciv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSynciv", "",
        "GLSync", gsTrace.GetName(pname), bufSize, *length,
        gsTrace.GetArray(*length, values));
#endif
}

void APIENTRY glGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
    if (sglGetInteger64i_v)
    {
        sglGetInteger64i_v(target, index, data);
        ReportGLError("glGetInteger64i_v");
    }
    else
    {
        ReportGLNullFunction("glGetInteger64i_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetInteger64i_v", "",
        gsTrace.GetName(target), index, *data);
#endif
}

void APIENTRY glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
    if (sglGetBufferParameteri64v)
    {
        sglGetBufferParameteri64v(target, pname, params);
        ReportGLError("glGetBufferParameteri64v");
    }
    else
    {
        ReportGLNullFunction("glGetBufferParameteri64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetBufferParameteri64v", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}


void APIENTRY glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    if (sglFramebufferTexture)
    {
        sglFramebufferTexture(target, attachment, texture, level);
        ReportGLError("glFramebufferTexture");
    }
    else
    {
        ReportGLNullFunction("glFramebufferTexture");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferTexture", "",
        gsTrace.GetName(target), gsTrace.GetName(attachment), texture, level);
#endif
}

void APIENTRY glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if (sglTexImage2DMultisample)
    {
        sglTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
        ReportGLError("glTexImage2DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTexImage2DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexImage2DMultisample", "",
        gsTrace.GetName(target), samples, gsTrace.GetName(internalformat), width, height, gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    if (sglTexImage3DMultisample)
    {
        sglTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
        ReportGLError("glTexImage3DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTexImage3DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexImage3DMultisample", "",
        gsTrace.GetName(target), samples, gsTrace.GetName(internalformat), width, height, depth, gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val)
{
    if (sglGetMultisamplefv)
    {
        sglGetMultisamplefv(pname, index, val);
        ReportGLError("glGetMultisamplefv");
    }
    else
    {
        ReportGLNullFunction("glGetMultisamplefv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetMultisamplefv", "",
        gsTrace.GetName(pname), index, *val);
#endif
}

void APIENTRY glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    if (sglSampleMaski)
    {
        sglSampleMaski(maskNumber, mask);
        ReportGLError("glSampleMaski");
    }
    else
    {
        ReportGLNullFunction("glSampleMaski");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSampleMaski", "",
        maskNumber, mask);
#endif
}

static void Initialize_OPENGL_VERSION_3_2()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_3_2)
    {
        GetOpenGLFunction("glDrawElementsBaseVertex", sglDrawElementsBaseVertex);
        GetOpenGLFunction("glDrawRangeElementsBaseVertex", sglDrawRangeElementsBaseVertex);
        GetOpenGLFunction("glDrawElementsInstancedBaseVertex", sglDrawElementsInstancedBaseVertex);
        GetOpenGLFunction("glMultiDrawElementsBaseVertex", sglMultiDrawElementsBaseVertex);
        GetOpenGLFunction("glProvokingVertex", sglProvokingVertex);
        GetOpenGLFunction("glFenceSync", sglFenceSync);
        GetOpenGLFunction("glIsSync", sglIsSync);
        GetOpenGLFunction("glDeleteSync", sglDeleteSync);
        GetOpenGLFunction("glClientWaitSync", sglClientWaitSync);
        GetOpenGLFunction("glWaitSync", sglWaitSync);
        GetOpenGLFunction("glGetInteger64v", sglGetInteger64v);
        GetOpenGLFunction("glGetSynciv", sglGetSynciv);
        GetOpenGLFunction("glGetInteger64i_v", sglGetInteger64i_v);
        GetOpenGLFunction("glGetBufferParameteri64v", sglGetBufferParameteri64v);
        GetOpenGLFunction("glFramebufferTexture", sglFramebufferTexture);
        GetOpenGLFunction("glTexImage2DMultisample", sglTexImage2DMultisample);
        GetOpenGLFunction("glTexImage3DMultisample", sglTexImage3DMultisample);
        GetOpenGLFunction("glGetMultisamplefv", sglGetMultisamplefv);
        GetOpenGLFunction("glSampleMaski", sglSampleMaski);
    }
}

// GL_VERSION_3_3

static PFNGLBINDFRAGDATALOCATIONINDEXEDPROC sglBindFragDataLocationIndexed = nullptr;
static PFNGLGETFRAGDATAINDEXPROC sglGetFragDataIndex = nullptr;
static PFNGLGENSAMPLERSPROC sglGenSamplers = nullptr;
static PFNGLDELETESAMPLERSPROC sglDeleteSamplers = nullptr;
static PFNGLISSAMPLERPROC sglIsSampler = nullptr;
static PFNGLBINDSAMPLERPROC sglBindSampler = nullptr;
static PFNGLSAMPLERPARAMETERIPROC sglSamplerParameteri = nullptr;
static PFNGLSAMPLERPARAMETERIVPROC sglSamplerParameteriv = nullptr;
static PFNGLSAMPLERPARAMETERFPROC sglSamplerParameterf = nullptr;
static PFNGLSAMPLERPARAMETERFVPROC sglSamplerParameterfv = nullptr;
static PFNGLSAMPLERPARAMETERIIVPROC sglSamplerParameterIiv = nullptr;
static PFNGLSAMPLERPARAMETERIUIVPROC sglSamplerParameterIuiv = nullptr;
static PFNGLGETSAMPLERPARAMETERIVPROC sglGetSamplerParameteriv = nullptr;
static PFNGLGETSAMPLERPARAMETERIIVPROC sglGetSamplerParameterIiv = nullptr;
static PFNGLGETSAMPLERPARAMETERFVPROC sglGetSamplerParameterfv = nullptr;
static PFNGLGETSAMPLERPARAMETERIUIVPROC sglGetSamplerParameterIuiv = nullptr;
static PFNGLQUERYCOUNTERPROC sglQueryCounter = nullptr;
static PFNGLGETQUERYOBJECTI64VPROC sglGetQueryObjecti64v = nullptr;
static PFNGLGETQUERYOBJECTUI64VPROC sglGetQueryObjectui64v = nullptr;
static PFNGLVERTEXATTRIBDIVISORPROC sglVertexAttribDivisor = nullptr;
static PFNGLVERTEXATTRIBP1UIPROC sglVertexAttribP1ui = nullptr;
static PFNGLVERTEXATTRIBP1UIVPROC sglVertexAttribP1uiv = nullptr;
static PFNGLVERTEXATTRIBP2UIPROC sglVertexAttribP2ui = nullptr;
static PFNGLVERTEXATTRIBP2UIVPROC sglVertexAttribP2uiv = nullptr;
static PFNGLVERTEXATTRIBP3UIPROC sglVertexAttribP3ui = nullptr;
static PFNGLVERTEXATTRIBP3UIVPROC sglVertexAttribP3uiv = nullptr;
static PFNGLVERTEXATTRIBP4UIPROC sglVertexAttribP4ui = nullptr;
static PFNGLVERTEXATTRIBP4UIVPROC sglVertexAttribP4uiv = nullptr;

void APIENTRY glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name)
{
    if (sglBindFragDataLocationIndexed)
    {
        sglBindFragDataLocationIndexed(program, colorNumber, index, name);
        ReportGLError("glBindFragDataLocationIndexed");
    }
    else
    {
        ReportGLNullFunction("glBindFragDataLocationIndexed");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindFragDataLocationIndexed", "",
        program, colorNumber, index, name);
#endif
}

GLint APIENTRY glGetFragDataIndex(GLuint program, const GLchar* name)
{
    GLint result;
    if (sglGetFragDataIndex)
    {
        result = sglGetFragDataIndex(program, name);
        ReportGLError("glGetFragDataIndex");
    }
    else
    {
        ReportGLNullFunction("glGetFragDataIndex");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetFragDataIndex", "",
        program, name);
#endif
    return result;
}

void APIENTRY glGenSamplers(GLsizei count, GLuint* samplers)
{
    if (sglGenSamplers)
    {
        sglGenSamplers(count, samplers);
        ReportGLError("glGenSamplers");
    }
    else
    {
        ReportGLNullFunction("glGenSamplers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenSamplers", "",
        count, gsTrace.GetArray(count, samplers));
#endif
}

void APIENTRY glDeleteSamplers(GLsizei count, const GLuint* samplers)
{
    if (sglDeleteSamplers)
    {
        sglDeleteSamplers(count, samplers);
        ReportGLError("glDeleteSamplers");
    }
    else
    {
        ReportGLNullFunction("glDeleteSamplers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteSamplers", "",
        count, gsTrace.GetArray(count, samplers));
#endif
}

GLboolean APIENTRY glIsSampler(GLuint sampler)
{
    GLboolean result;
    if (sglIsSampler)
    {
        result = sglIsSampler(sampler);
        ReportGLError("glIsSampler");
    }
    else
    {
        ReportGLNullFunction("glIsSampler");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsSampler", gsTrace.GetBoolean(result),
        sampler);
#endif
    return result;
}

void APIENTRY glBindSampler(GLuint unit, GLuint sampler)
{
    if (sglBindSampler)
    {
        sglBindSampler(unit, sampler);
        ReportGLError("glBindSampler");
    }
    else
    {
        ReportGLNullFunction("glBindSampler");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindSampler", "",
        unit, sampler);
#endif
}

void APIENTRY glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    if (sglSamplerParameteri)
    {
        sglSamplerParameteri(sampler, pname, param);
        ReportGLError("glSamplerParameteri");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameteri", "",
        sampler, gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
    if (sglSamplerParameteriv)
    {
        sglSamplerParameteriv(sampler, pname, param);
        ReportGLError("glSamplerParameteriv");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameteriv", "",
        sampler, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    if (sglSamplerParameterf)
    {
        sglSamplerParameterf(sampler, pname, param);
        ReportGLError("glSamplerParameterf");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameterf");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameterf", "",
        sampler, gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
    if (sglSamplerParameterfv)
    {
        sglSamplerParameterfv(sampler, pname, param);
        ReportGLError("glSamplerParameterfv");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameterfv", "",
        sampler, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param)
{
    if (sglSamplerParameterIiv)
    {
        sglSamplerParameterIiv(sampler, pname, param);
        ReportGLError("glSamplerParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameterIiv", "",
        sampler, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param)
{
    if (sglSamplerParameterIuiv)
    {
        sglSamplerParameterIuiv(sampler, pname, param);
        ReportGLError("glSamplerParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glSamplerParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSamplerParameterIuiv", "",
        sampler, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
    if (sglGetSamplerParameteriv)
    {
        sglGetSamplerParameteriv(sampler, pname, params);
        ReportGLError("glGetSamplerParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetSamplerParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSamplerParameteriv", "",
        sampler, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params)
{
    if (sglGetSamplerParameterIiv)
    {
        sglGetSamplerParameterIiv(sampler, pname, params);
        ReportGLError("glGetSamplerParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glGetSamplerParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSamplerParameterIiv", "",
        sampler, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
    if (sglGetSamplerParameterfv)
    {
        sglGetSamplerParameterfv(sampler, pname, params);
        ReportGLError("glGetSamplerParameterfv");
    }
    else
    {
        ReportGLNullFunction("glGetSamplerParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSamplerParameterfv", "",
        sampler, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params)
{
    if (sglGetSamplerParameterIuiv)
    {
        sglGetSamplerParameterIuiv(sampler, pname, params);
        ReportGLError("glGetSamplerParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glGetSamplerParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSamplerParameterIuiv", "",
        sampler, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glQueryCounter(GLuint id, GLenum target)
{
    if (sglQueryCounter)
    {
        sglQueryCounter(id, target);
        ReportGLError("glQueryCounter");
    }
    else
    {
        ReportGLNullFunction("glQueryCounter");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glQueryCounter", "",
        id, gsTrace.GetName(target));
#endif
}

void APIENTRY glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params)
{
    if (sglGetQueryObjecti64v)
    {
        sglGetQueryObjecti64v(id, pname, params);
        ReportGLError("glGetQueryObjecti64v");
    }
    else
    {
        ReportGLNullFunction("glGetQueryObjecti64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryObjecti64v", "",
        id, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params)
{
    if (sglGetQueryObjectui64v)
    {
        sglGetQueryObjectui64v(id, pname, params);
        ReportGLError("glGetQueryObjectui64v");
    }
    else
    {
        ReportGLNullFunction("glGetQueryObjectui64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryObjectui64v", "",
        id, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    if (sglVertexAttribDivisor)
    {
        sglVertexAttribDivisor(index, divisor);
        ReportGLError("glVertexAttribDivisor");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribDivisor");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribDivisor", "",
        index, divisor);
#endif
}

void APIENTRY glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if (sglVertexAttribP1ui)
    {
        sglVertexAttribP1ui(index, type, normalized, value);
        ReportGLError("glVertexAttribP1ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP1ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP1ui", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), value);
#endif
}

void APIENTRY glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if (sglVertexAttribP1uiv)
    {
        sglVertexAttribP1uiv(index, type, normalized, value);
        ReportGLError("glVertexAttribP1uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP1uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP1uiv", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), *value);
#endif
}

void APIENTRY glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if (sglVertexAttribP2ui)
    {
        sglVertexAttribP2ui(index, type, normalized, value);
        ReportGLError("glVertexAttribP2ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP2ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP2ui", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), value);
#endif
}

void APIENTRY glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if (sglVertexAttribP2uiv)
    {
        sglVertexAttribP2uiv(index, type, normalized, value);
        ReportGLError("glVertexAttribP2uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP2uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP2uiv", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), *value);
#endif
}

void APIENTRY glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if (sglVertexAttribP3ui)
    {
        sglVertexAttribP3ui(index, type, normalized, value);
        ReportGLError("glVertexAttribP3ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP3ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP3ui", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), value);
#endif
}

void APIENTRY glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if (sglVertexAttribP3uiv)
    {
        sglVertexAttribP3uiv(index, type, normalized, value);
        ReportGLError("glVertexAttribP3uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP3uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP3uiv", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), *value);
#endif
}

void APIENTRY glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    if (sglVertexAttribP4ui)
    {
        sglVertexAttribP4ui(index, type, normalized, value);
        ReportGLError("glVertexAttribP4ui");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP4ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP4ui", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), value);
#endif
}

void APIENTRY glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
    if (sglVertexAttribP4uiv)
    {
        sglVertexAttribP4uiv(index, type, normalized, value);
        ReportGLError("glVertexAttribP4uiv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribP4uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribP4uiv", "",
        index, gsTrace.GetName(type), gsTrace.GetBoolean(normalized), *value);
#endif
}

static void Initialize_OPENGL_VERSION_3_3()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_3_3)
    {
        GetOpenGLFunction("glBindFragDataLocationIndexed", sglBindFragDataLocationIndexed);
        GetOpenGLFunction("glGetFragDataIndex", sglGetFragDataIndex);
        GetOpenGLFunction("glGenSamplers", sglGenSamplers);
        GetOpenGLFunction("glDeleteSamplers", sglDeleteSamplers);
        GetOpenGLFunction("glIsSampler", sglIsSampler);
        GetOpenGLFunction("glBindSampler", sglBindSampler);
        GetOpenGLFunction("glSamplerParameteri", sglSamplerParameteri);
        GetOpenGLFunction("glSamplerParameteriv", sglSamplerParameteriv);
        GetOpenGLFunction("glSamplerParameterf", sglSamplerParameterf);
        GetOpenGLFunction("glSamplerParameterfv", sglSamplerParameterfv);
        GetOpenGLFunction("glSamplerParameterIiv", sglSamplerParameterIiv);
        GetOpenGLFunction("glSamplerParameterIuiv", sglSamplerParameterIuiv);
        GetOpenGLFunction("glGetSamplerParameteriv", sglGetSamplerParameteriv);
        GetOpenGLFunction("glGetSamplerParameterIiv", sglGetSamplerParameterIiv);
        GetOpenGLFunction("glGetSamplerParameterfv", sglGetSamplerParameterfv);
        GetOpenGLFunction("glGetSamplerParameterIuiv", sglGetSamplerParameterIuiv);
        GetOpenGLFunction("glQueryCounter", sglQueryCounter);
        GetOpenGLFunction("glGetQueryObjecti64v", sglGetQueryObjecti64v);
        GetOpenGLFunction("glGetQueryObjectui64v", sglGetQueryObjectui64v);
        GetOpenGLFunction("glVertexAttribDivisor", sglVertexAttribDivisor);
        GetOpenGLFunction("glVertexAttribP1ui", sglVertexAttribP1ui);
        GetOpenGLFunction("glVertexAttribP1uiv", sglVertexAttribP1uiv);
        GetOpenGLFunction("glVertexAttribP2ui", sglVertexAttribP2ui);
        GetOpenGLFunction("glVertexAttribP2uiv", sglVertexAttribP2uiv);
        GetOpenGLFunction("glVertexAttribP3ui", sglVertexAttribP3ui);
        GetOpenGLFunction("glVertexAttribP3uiv", sglVertexAttribP3uiv);
        GetOpenGLFunction("glVertexAttribP4ui", sglVertexAttribP4ui);
        GetOpenGLFunction("glVertexAttribP4uiv", sglVertexAttribP4uiv);
    }
}

// GL_VERSION_4_0

static PFNGLMINSAMPLESHADINGPROC sglMinSampleShading = nullptr;
static PFNGLBLENDEQUATIONIPROC sglBlendEquationi = nullptr;
static PFNGLBLENDEQUATIONSEPARATEIPROC sglBlendEquationSeparatei = nullptr;
static PFNGLBLENDFUNCIPROC sglBlendFunci = nullptr;
static PFNGLBLENDFUNCSEPARATEIPROC sglBlendFuncSeparatei = nullptr;
static PFNGLDRAWARRAYSINDIRECTPROC sglDrawArraysIndirect = nullptr;
static PFNGLDRAWELEMENTSINDIRECTPROC sglDrawElementsIndirect = nullptr;
static PFNGLUNIFORM1DPROC sglUniform1d = nullptr;
static PFNGLUNIFORM2DPROC sglUniform2d = nullptr;
static PFNGLUNIFORM3DPROC sglUniform3d = nullptr;
static PFNGLUNIFORM4DPROC sglUniform4d = nullptr;
static PFNGLUNIFORM1DVPROC sglUniform1dv = nullptr;
static PFNGLUNIFORM2DVPROC sglUniform2dv = nullptr;
static PFNGLUNIFORM3DVPROC sglUniform3dv = nullptr;
static PFNGLUNIFORM4DVPROC sglUniform4dv = nullptr;
static PFNGLUNIFORMMATRIX2DVPROC sglUniformMatrix2dv = nullptr;
static PFNGLUNIFORMMATRIX3DVPROC sglUniformMatrix3dv = nullptr;
static PFNGLUNIFORMMATRIX4DVPROC sglUniformMatrix4dv = nullptr;
static PFNGLUNIFORMMATRIX2X3DVPROC sglUniformMatrix2x3dv = nullptr;
static PFNGLUNIFORMMATRIX2X4DVPROC sglUniformMatrix2x4dv = nullptr;
static PFNGLUNIFORMMATRIX3X2DVPROC sglUniformMatrix3x2dv = nullptr;
static PFNGLUNIFORMMATRIX3X4DVPROC sglUniformMatrix3x4dv = nullptr;
static PFNGLUNIFORMMATRIX4X2DVPROC sglUniformMatrix4x2dv = nullptr;
static PFNGLUNIFORMMATRIX4X3DVPROC sglUniformMatrix4x3dv = nullptr;
static PFNGLGETUNIFORMDVPROC sglGetUniformdv = nullptr;
static PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC sglGetSubroutineUniformLocation = nullptr;
static PFNGLGETSUBROUTINEINDEXPROC sglGetSubroutineIndex = nullptr;
static PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC sglGetActiveSubroutineUniformiv = nullptr;
static PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC sglGetActiveSubroutineUniformName = nullptr;
static PFNGLGETACTIVESUBROUTINENAMEPROC sglGetActiveSubroutineName = nullptr;
static PFNGLUNIFORMSUBROUTINESUIVPROC sglUniformSubroutinesuiv = nullptr;
static PFNGLGETUNIFORMSUBROUTINEUIVPROC sglGetUniformSubroutineuiv = nullptr;
static PFNGLGETPROGRAMSTAGEIVPROC sglGetProgramStageiv = nullptr;
static PFNGLPATCHPARAMETERIPROC sglPatchParameteri = nullptr;
static PFNGLPATCHPARAMETERFVPROC sglPatchParameterfv = nullptr;
static PFNGLBINDTRANSFORMFEEDBACKPROC sglBindTransformFeedback = nullptr;
static PFNGLDELETETRANSFORMFEEDBACKSPROC sglDeleteTransformFeedbacks = nullptr;
static PFNGLGENTRANSFORMFEEDBACKSPROC sglGenTransformFeedbacks = nullptr;
static PFNGLISTRANSFORMFEEDBACKPROC sglIsTransformFeedback = nullptr;
static PFNGLPAUSETRANSFORMFEEDBACKPROC sglPauseTransformFeedback = nullptr;
static PFNGLRESUMETRANSFORMFEEDBACKPROC sglResumeTransformFeedback = nullptr;
static PFNGLDRAWTRANSFORMFEEDBACKPROC sglDrawTransformFeedback = nullptr;
static PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC sglDrawTransformFeedbackStream = nullptr;
static PFNGLBEGINQUERYINDEXEDPROC sglBeginQueryIndexed = nullptr;
static PFNGLENDQUERYINDEXEDPROC sglEndQueryIndexed = nullptr;
static PFNGLGETQUERYINDEXEDIVPROC sglGetQueryIndexediv = nullptr;

void APIENTRY glMinSampleShading(GLfloat value)
{
    if (sglMinSampleShading)
    {
        sglMinSampleShading(value);
        ReportGLError("glMinSampleShading");
    }
    else
    {
        ReportGLNullFunction("glMinSampleShading");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMinSampleShading", "",
        value);
#endif
}

void APIENTRY glBlendEquationi(GLuint buf, GLenum mode)
{
    if (sglBlendEquationi)
    {
        sglBlendEquationi(buf, mode);
        ReportGLError("glBlendEquationi");
    }
    else
    {
        ReportGLNullFunction("glBlendEquationi");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendEquationi", "",
        buf, mode);
#endif
}

void APIENTRY glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    if (sglBlendEquationSeparatei)
    {
        sglBlendEquationSeparatei(buf, modeRGB, modeAlpha);
        ReportGLError("glBlendEquationSeparatei");
    }
    else
    {
        ReportGLNullFunction("glBlendEquationSeparatei");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendEquationi", "",
        buf, gsTrace.GetName(modeRGB), gsTrace.GetName(modeAlpha));
#endif
}

void APIENTRY glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
    if (sglBlendFunci)
    {
        sglBlendFunci(buf, src, dst);
        ReportGLError("glBlendFunci");
    }
    else
    {
        ReportGLNullFunction("glBlendFunci");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendFunci", "",
        buf, gsTrace.GetName(src), gsTrace.GetName(dst));
#endif
}

void APIENTRY glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    if (sglBlendFuncSeparatei)
    {
        sglBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
        ReportGLError("glBlendFuncSeparatei");
    }
    else
    {
        ReportGLNullFunction("glBlendFuncSeparatei");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlendFuncSeparatei", "",
        buf, gsTrace.GetName(srcRGB), gsTrace.GetName(dstRGB),
        gsTrace.GetName(srcAlpha), gsTrace.GetName(dstAlpha));
#endif
}

void APIENTRY glDrawArraysIndirect(GLenum mode, const void* indirect)
{
    if (sglDrawArraysIndirect)
    {
        sglDrawArraysIndirect(mode, indirect);
        ReportGLError("glDrawArraysIndirect");
    }
    else
    {
        ReportGLNullFunction("glDrawArraysIndirect");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawArraysIndirect", "",
        gsTrace.GetName(mode), "indirect");
#endif
}

void APIENTRY glDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect)
{
    if (sglDrawElementsIndirect)
    {
        sglDrawElementsIndirect(mode, type, indirect);
        ReportGLError("glDrawElementsIndirect");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsIndirect");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsIndirect", "",
        gsTrace.GetName(mode), gsTrace.GetName(type), "indirect");
#endif
}

void APIENTRY glUniform1d(GLint location, GLdouble x)
{
    if (sglUniform1d)
    {
        sglUniform1d(location, x);
        ReportGLError("glUniform1d");
    }
    else
    {
        ReportGLNullFunction("glUniform1d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1d", "",
        location, x);
#endif
}

void APIENTRY glUniform2d(GLint location, GLdouble x, GLdouble y)
{
    if (sglUniform2d)
    {
        sglUniform2d(location, x, y);
        ReportGLError("glUniform2d");
    }
    else
    {
        ReportGLNullFunction("glUniform2d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2d", "",
        location, x, y);
#endif
}

void APIENTRY glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    if (sglUniform3d)
    {
        sglUniform3d(location, x, y, z);
        ReportGLError("glUniform3d");
    }
    else
    {
        ReportGLNullFunction("glUniform3d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3d", "",
        location, x, y, z);
#endif
}

void APIENTRY glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if (sglUniform4d)
    {
        sglUniform4d(location, x, y, z, w);
        ReportGLError("glUniform4d");
    }
    else
    {
        ReportGLNullFunction("glUniform4d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3d", "",
        location, x, y, z, w);
#endif
}

void APIENTRY glUniform1dv(GLint location, GLsizei count, const GLdouble* value)
{
    if (sglUniform1dv)
    {
        sglUniform1dv(location, count, value);
        ReportGLError("glUniform1dv");
    }
    else
    {
        ReportGLNullFunction("glUniform1dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform1dv", "",
        location, count, *value);
#endif
}

void APIENTRY glUniform2dv(GLint location, GLsizei count, const GLdouble* value)
{
    if (sglUniform2dv)
    {
        sglUniform2dv(location, count, value);
        ReportGLError("glUniform2dv");
    }
    else
    {
        ReportGLNullFunction("glUniform2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform2dv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform3dv(GLint location, GLsizei count, const GLdouble* value)
{
    if (sglUniform3dv)
    {
        sglUniform3dv(location, count, value);
        ReportGLError("glUniform3dv");
    }
    else
    {
        ReportGLNullFunction("glUniform3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform3dv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniform4dv(GLint location, GLsizei count, const GLdouble* value)
{
    if (sglUniform4dv)
    {
        sglUniform4dv(location, count, value);
        ReportGLError("glUniform4dv");
    }
    else
    {
        ReportGLNullFunction("glUniform4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniform4dv", "",
        location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix2dv)
    {
        sglUniformMatrix2dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(4ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix3dv)
    {
        sglUniformMatrix3dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(9ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix4dv)
    {
        sglUniformMatrix4dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(16ull * count, value));
#endif
}

void APIENTRY glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix2x3dv)
    {
        sglUniformMatrix2x3dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2x3dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2x3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2x3dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix2x4dv)
    {
        sglUniformMatrix2x4dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix2x4dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix2x4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix2x4dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix3x2dv)
    {
        sglUniformMatrix3x2dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3x2dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3x2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3x2dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix3x4dv)
    {
        sglUniformMatrix3x4dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix3x4dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix3x4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix3x4dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix4x2dv)
    {
        sglUniformMatrix4x2dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4x2dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4x2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4x2dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglUniformMatrix4x3dv)
    {
        sglUniformMatrix4x3dv(location, count, transpose, value);
        ReportGLError("glUniformMatrix4x3dv");
    }
    else
    {
        ReportGLNullFunction("glUniformMatrix4x3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformMatrix4x3dv", "",
        location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glGetUniformdv(GLuint program, GLint location, GLdouble* params)
{
    if (sglGetUniformdv)
    {
        sglGetUniformdv(program, location, params);
        ReportGLError("glGetUniformdv");
    }
    else
    {
        ReportGLNullFunction("glGetUniformdv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformdv", "",
        program, location, *params);
#endif
}

GLint APIENTRY glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar* name)
{
    GLint result;
    if (sglGetSubroutineUniformLocation)
    {
        result = sglGetSubroutineUniformLocation(program, shadertype, name);
        ReportGLError("glGetSubroutineUniformLocation");
    }
    else
    {
        ReportGLNullFunction("glGetSubroutineUniformLocation");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSubroutineUniformLocation", std::to_string(result),
        program, gsTrace.GetName(shadertype), name);
#endif
    return result;
}

GLuint APIENTRY glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar* name)
{
    GLuint result;
    if (sglGetSubroutineIndex)
    {
        result = sglGetSubroutineIndex(program, shadertype, name);
        ReportGLError("glGetSubroutineIndex");
    }
    else
    {
        ReportGLNullFunction("glGetSubroutineIndex");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetSubroutineIndex", std::to_string(result),
        program, gsTrace.GetName(shadertype), name);
#endif
    return result;
}

void APIENTRY glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values)
{
    if (sglGetActiveSubroutineUniformiv)
    {
        sglGetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
        ReportGLError("glGetActiveSubroutineUniformiv");
    }
    else
    {
        ReportGLNullFunction("glGetActiveSubroutineUniformiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveSubroutineUniformiv", "",
        program, gsTrace.GetName(shadertype), index, gsTrace.GetName(pname), *values);
#endif
}

void APIENTRY glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name)
{
    if (sglGetActiveSubroutineUniformName)
    {
        sglGetActiveSubroutineUniformName(program, shadertype, index, bufsize, length, name);
        ReportGLError("glGetActiveSubroutineUniformName");
    }
    else
    {
        ReportGLNullFunction("glGetActiveSubroutineUniformName");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveSubroutineUniformName", "",
        program, gsTrace.GetName(shadertype), index, bufsize, *length, name);
#endif
}

void APIENTRY glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name)
{
    if (sglGetActiveSubroutineName)
    {
        sglGetActiveSubroutineName(program, shadertype, index, bufsize, length, name);
        ReportGLError("glGetActiveSubroutineName");
    }
    else
    {
        ReportGLNullFunction("glGetActiveSubroutineName");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveSubroutineName", "",
        program, gsTrace.GetName(shadertype), index, bufsize, *length, name);
#endif
}

void APIENTRY glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices)
{
    if (sglUniformSubroutinesuiv)
    {
        sglUniformSubroutinesuiv(shadertype, count, indices);
        ReportGLError("glUniformSubroutinesuiv");
    }
    else
    {
        ReportGLNullFunction("glUniformSubroutinesuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUniformSubroutinesuiv", "",
        gsTrace.GetName(shadertype), count, gsTrace.GetArray(count, indices));
#endif
}

void APIENTRY glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint* params)
{
    if (sglGetUniformSubroutineuiv)
    {
        sglGetUniformSubroutineuiv(shadertype, location, params);
        ReportGLError("glGetUniformSubroutineuiv");
    }
    else
    {
        ReportGLNullFunction("glGetUniformSubroutineuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetUniformSubroutineuiv", "",
        gsTrace.GetName(shadertype), location, *params);
#endif
}

void APIENTRY glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values)
{
    if (sglGetProgramStageiv)
    {
        sglGetProgramStageiv(program, shadertype, pname, values);
        ReportGLError("glGetProgramStageiv");
    }
    else
    {
        ReportGLNullFunction("glGetProgramStageiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramStageiv", "",
        program, gsTrace.GetName(shadertype), gsTrace.GetName(pname), *values);
#endif
}

void APIENTRY glPatchParameteri(GLenum pname, GLint value)
{
    if (sglPatchParameteri)
    {
        sglPatchParameteri(pname, value);
        ReportGLError("glPatchParameteri");
    }
    else
    {
        ReportGLNullFunction("glPatchParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPatchParameteri", "",
        gsTrace.GetName(pname), value);
#endif
}

void APIENTRY glPatchParameterfv(GLenum pname, const GLfloat* values)
{
    if (sglPatchParameterfv)
    {
        sglPatchParameterfv(pname, values);
        ReportGLError("glPatchParameterfv");
    }
    else
    {
        ReportGLNullFunction("glPatchParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPatchParameterfv", "",
        gsTrace.GetName(pname), *values);
#endif
}

void APIENTRY glBindTransformFeedback(GLenum target, GLuint id)
{
    if (sglBindTransformFeedback)
    {
        sglBindTransformFeedback(target, id);
        ReportGLError("glBindTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glBindTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindTransformFeedback", "",
        gsTrace.GetName(target), id);
#endif
}

void APIENTRY glDeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
    if (sglDeleteTransformFeedbacks)
    {
        sglDeleteTransformFeedbacks(n, ids);
        ReportGLError("glDeleteTransformFeedbacks");
    }
    else
    {
        ReportGLNullFunction("glDeleteTransformFeedbacks");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteTransformFeedbacks", "",
        n, gsTrace.GetArray(n, ids));
#endif
}

void APIENTRY glGenTransformFeedbacks(GLsizei n, GLuint* ids)
{
    if (sglGenTransformFeedbacks)
    {
        sglGenTransformFeedbacks(n, ids);
        ReportGLError("glGenTransformFeedbacks");
    }
    else
    {
        ReportGLNullFunction("glGenTransformFeedbacks");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenTransformFeedbacks", "",
        n, gsTrace.GetArray(n, ids));
#endif
}

GLboolean APIENTRY glIsTransformFeedback(GLuint id)
{
    GLboolean result;
    if (sglIsTransformFeedback)
    {
        result = sglIsTransformFeedback(id);
        ReportGLError("glIsTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glIsTransformFeedback");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsTransformFeedback", gsTrace.GetBoolean(result),
        id);
#endif
    return result;
}

void APIENTRY glPauseTransformFeedback()
{
    if (sglPauseTransformFeedback)
    {
        sglPauseTransformFeedback();
        ReportGLError("glPauseTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glPauseTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPauseTransformFeedback", "");
#endif
}

void APIENTRY glResumeTransformFeedback()
{
    if (sglResumeTransformFeedback)
    {
        sglResumeTransformFeedback();
        ReportGLError("glResumeTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glResumeTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glResumeTransformFeedback", "");
#endif
}

void APIENTRY glDrawTransformFeedback(GLenum mode, GLuint id)
{
    if (sglDrawTransformFeedback)
    {
        sglDrawTransformFeedback(mode, id);
        ReportGLError("glDrawTransformFeedback");
    }
    else
    {
        ReportGLNullFunction("glDrawTransformFeedback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawTransformFeedback", "",
        gsTrace.GetName(mode), id);
#endif
}

void APIENTRY glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
    if (sglDrawTransformFeedbackStream)
    {
        sglDrawTransformFeedbackStream(mode, id, stream);
        ReportGLError("glDrawTransformFeedbackStream");
    }
    else
    {
        ReportGLNullFunction("glDrawTransformFeedbackStream");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawTransformFeedbackStream", "",
        gsTrace.GetName(mode), id, stream);
#endif
}

void APIENTRY glBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
    if (sglBeginQueryIndexed)
    {
        sglBeginQueryIndexed(target, index, id);
        ReportGLError("glBeginQueryIndexed");
    }
    else
    {
        ReportGLNullFunction("glBeginQueryIndexed");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBeginQueryIndexed", "",
        gsTrace.GetName(target), index, id);
#endif
}

void APIENTRY glEndQueryIndexed(GLenum target, GLuint index)
{
    if (sglEndQueryIndexed)
    {
        sglEndQueryIndexed(target, index);
        ReportGLError("glEndQueryIndexed");
    }
    else
    {
        ReportGLNullFunction("glEndQueryIndexed");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEndQueryIndexed", "",
        gsTrace.GetName(target), index);
#endif
}

void APIENTRY glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint* params)
{
    if (sglGetQueryIndexediv)
    {
        sglGetQueryIndexediv(target, index, pname, params);
        ReportGLError("glGetQueryIndexediv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryIndexediv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryIndexediv", "",
        gsTrace.GetName(target), index, gsTrace.GetName(pname), *params);
#endif
}

static void Initialize_OPENGL_VERSION_4_0()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_0)
    {
        GetOpenGLFunction("glMinSampleShading", sglMinSampleShading);
        GetOpenGLFunction("glBlendEquationi", sglBlendEquationi);
        GetOpenGLFunction("glBlendEquationSeparatei", sglBlendEquationSeparatei);
        GetOpenGLFunction("glBlendFunci", sglBlendFunci);
        GetOpenGLFunction("glBlendFuncSeparatei", sglBlendFuncSeparatei);
        GetOpenGLFunction("glDrawArraysIndirect", sglDrawArraysIndirect);
        GetOpenGLFunction("glDrawElementsIndirect", sglDrawElementsIndirect);
        GetOpenGLFunction("glUniform1d", sglUniform1d);
        GetOpenGLFunction("glUniform2d", sglUniform2d);
        GetOpenGLFunction("glUniform3d", sglUniform3d);
        GetOpenGLFunction("glUniform4d", sglUniform4d);
        GetOpenGLFunction("glUniform1dv", sglUniform1dv);
        GetOpenGLFunction("glUniform2dv", sglUniform2dv);
        GetOpenGLFunction("glUniform3dv", sglUniform3dv);
        GetOpenGLFunction("glUniform4dv", sglUniform4dv);
        GetOpenGLFunction("glUniformMatrix2dv", sglUniformMatrix2dv);
        GetOpenGLFunction("glUniformMatrix3dv", sglUniformMatrix3dv);
        GetOpenGLFunction("glUniformMatrix4dv", sglUniformMatrix4dv);
        GetOpenGLFunction("glUniformMatrix2x3dv", sglUniformMatrix2x3dv);
        GetOpenGLFunction("glUniformMatrix2x4dv", sglUniformMatrix2x4dv);
        GetOpenGLFunction("glUniformMatrix3x2dv", sglUniformMatrix3x2dv);
        GetOpenGLFunction("glUniformMatrix3x4dv", sglUniformMatrix3x4dv);
        GetOpenGLFunction("glUniformMatrix4x2dv", sglUniformMatrix4x2dv);
        GetOpenGLFunction("glUniformMatrix4x3dv", sglUniformMatrix4x3dv);
        GetOpenGLFunction("glGetUniformdv", sglGetUniformdv);
        GetOpenGLFunction("glGetSubroutineUniformLocation", sglGetSubroutineUniformLocation);
        GetOpenGLFunction("glGetSubroutineIndex", sglGetSubroutineIndex);
        GetOpenGLFunction("glGetActiveSubroutineUniformiv", sglGetActiveSubroutineUniformiv);
        GetOpenGLFunction("glGetActiveSubroutineUniformName", sglGetActiveSubroutineUniformName);
        GetOpenGLFunction("glGetActiveSubroutineName", sglGetActiveSubroutineName);
        GetOpenGLFunction("glUniformSubroutinesuiv", sglUniformSubroutinesuiv);
        GetOpenGLFunction("glGetUniformSubroutineuiv", sglGetUniformSubroutineuiv);
        GetOpenGLFunction("glGetProgramStageiv", sglGetProgramStageiv);
        GetOpenGLFunction("glPatchParameteri", sglPatchParameteri);
        GetOpenGLFunction("glPatchParameterfv", sglPatchParameterfv);
        GetOpenGLFunction("glBindTransformFeedback", sglBindTransformFeedback);
        GetOpenGLFunction("glDeleteTransformFeedbacks", sglDeleteTransformFeedbacks);
        GetOpenGLFunction("glGenTransformFeedbacks", sglGenTransformFeedbacks);
        GetOpenGLFunction("glIsTransformFeedback", sglIsTransformFeedback);
        GetOpenGLFunction("glPauseTransformFeedback", sglPauseTransformFeedback);
        GetOpenGLFunction("glResumeTransformFeedback", sglResumeTransformFeedback);
        GetOpenGLFunction("glDrawTransformFeedback", sglDrawTransformFeedback);
        GetOpenGLFunction("glDrawTransformFeedbackStream", sglDrawTransformFeedbackStream);
        GetOpenGLFunction("glBeginQueryIndexed", sglBeginQueryIndexed);
        GetOpenGLFunction("glEndQueryIndexed", sglEndQueryIndexed);
        GetOpenGLFunction("glGetQueryIndexediv", sglGetQueryIndexediv);
    }
}

// GL_VERSION_4_1

static PFNGLRELEASESHADERCOMPILERPROC sglReleaseShaderCompiler = nullptr;
static PFNGLSHADERBINARYPROC sglShaderBinary = nullptr;
static PFNGLGETSHADERPRECISIONFORMATPROC sglGetShaderPrecisionFormat = nullptr;
static PFNGLDEPTHRANGEFPROC sglDepthRangef = nullptr;
static PFNGLCLEARDEPTHFPROC sglClearDepthf = nullptr;
static PFNGLGETPROGRAMBINARYPROC sglGetProgramBinary = nullptr;
static PFNGLPROGRAMBINARYPROC sglProgramBinary = nullptr;
static PFNGLPROGRAMPARAMETERIPROC sglProgramParameteri = nullptr;
static PFNGLUSEPROGRAMSTAGESPROC sglUseProgramStages = nullptr;
static PFNGLACTIVESHADERPROGRAMPROC sglActiveShaderProgram = nullptr;
static PFNGLCREATESHADERPROGRAMVPROC sglCreateShaderProgramv = nullptr;
static PFNGLBINDPROGRAMPIPELINEPROC sglBindProgramPipeline = nullptr;
static PFNGLDELETEPROGRAMPIPELINESPROC sglDeleteProgramPipelines = nullptr;
static PFNGLGENPROGRAMPIPELINESPROC sglGenProgramPipelines = nullptr;
static PFNGLISPROGRAMPIPELINEPROC sglIsProgramPipeline = nullptr;
static PFNGLGETPROGRAMPIPELINEIVPROC sglGetProgramPipelineiv = nullptr;
static PFNGLPROGRAMUNIFORM1IPROC sglProgramUniform1i = nullptr;
static PFNGLPROGRAMUNIFORM1IVPROC sglProgramUniform1iv = nullptr;
static PFNGLPROGRAMUNIFORM1FPROC sglProgramUniform1f = nullptr;
static PFNGLPROGRAMUNIFORM1FVPROC sglProgramUniform1fv = nullptr;
static PFNGLPROGRAMUNIFORM1DPROC sglProgramUniform1d = nullptr;
static PFNGLPROGRAMUNIFORM1DVPROC sglProgramUniform1dv = nullptr;
static PFNGLPROGRAMUNIFORM1UIPROC sglProgramUniform1ui = nullptr;
static PFNGLPROGRAMUNIFORM1UIVPROC sglProgramUniform1uiv = nullptr;
static PFNGLPROGRAMUNIFORM2IPROC sglProgramUniform2i = nullptr;
static PFNGLPROGRAMUNIFORM2IVPROC sglProgramUniform2iv = nullptr;
static PFNGLPROGRAMUNIFORM2FPROC sglProgramUniform2f = nullptr;
static PFNGLPROGRAMUNIFORM2FVPROC sglProgramUniform2fv = nullptr;
static PFNGLPROGRAMUNIFORM2DPROC sglProgramUniform2d = nullptr;
static PFNGLPROGRAMUNIFORM2DVPROC sglProgramUniform2dv = nullptr;
static PFNGLPROGRAMUNIFORM2UIPROC sglProgramUniform2ui = nullptr;
static PFNGLPROGRAMUNIFORM2UIVPROC sglProgramUniform2uiv = nullptr;
static PFNGLPROGRAMUNIFORM3IPROC sglProgramUniform3i = nullptr;
static PFNGLPROGRAMUNIFORM3IVPROC sglProgramUniform3iv = nullptr;
static PFNGLPROGRAMUNIFORM3FPROC sglProgramUniform3f = nullptr;
static PFNGLPROGRAMUNIFORM3FVPROC sglProgramUniform3fv = nullptr;
static PFNGLPROGRAMUNIFORM3DPROC sglProgramUniform3d = nullptr;
static PFNGLPROGRAMUNIFORM3DVPROC sglProgramUniform3dv = nullptr;
static PFNGLPROGRAMUNIFORM3UIPROC sglProgramUniform3ui = nullptr;
static PFNGLPROGRAMUNIFORM3UIVPROC sglProgramUniform3uiv = nullptr;
static PFNGLPROGRAMUNIFORM4IPROC sglProgramUniform4i = nullptr;
static PFNGLPROGRAMUNIFORM4IVPROC sglProgramUniform4iv = nullptr;
static PFNGLPROGRAMUNIFORM4FPROC sglProgramUniform4f = nullptr;
static PFNGLPROGRAMUNIFORM4FVPROC sglProgramUniform4fv = nullptr;
static PFNGLPROGRAMUNIFORM4DPROC sglProgramUniform4d = nullptr;
static PFNGLPROGRAMUNIFORM4DVPROC sglProgramUniform4dv = nullptr;
static PFNGLPROGRAMUNIFORM4UIPROC sglProgramUniform4ui = nullptr;
static PFNGLPROGRAMUNIFORM4UIVPROC sglProgramUniform4uiv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2FVPROC sglProgramUniformMatrix2fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3FVPROC sglProgramUniformMatrix3fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4FVPROC sglProgramUniformMatrix4fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2DVPROC sglProgramUniformMatrix2dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3DVPROC sglProgramUniformMatrix3dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4DVPROC sglProgramUniformMatrix4dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC sglProgramUniformMatrix2x3fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC sglProgramUniformMatrix3x2fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC sglProgramUniformMatrix2x4fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC sglProgramUniformMatrix4x2fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC sglProgramUniformMatrix3x4fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC sglProgramUniformMatrix4x3fv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC sglProgramUniformMatrix2x3dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC sglProgramUniformMatrix3x2dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC sglProgramUniformMatrix2x4dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC sglProgramUniformMatrix4x2dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC sglProgramUniformMatrix3x4dv = nullptr;
static PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC sglProgramUniformMatrix4x3dv = nullptr;
static PFNGLVALIDATEPROGRAMPIPELINEPROC sglValidateProgramPipeline = nullptr;
static PFNGLGETPROGRAMPIPELINEINFOLOGPROC sglGetProgramPipelineInfoLog = nullptr;
static PFNGLVERTEXATTRIBL1DPROC sglVertexAttribL1d = nullptr;
static PFNGLVERTEXATTRIBL2DPROC sglVertexAttribL2d = nullptr;
static PFNGLVERTEXATTRIBL3DPROC sglVertexAttribL3d = nullptr;
static PFNGLVERTEXATTRIBL4DPROC sglVertexAttribL4d = nullptr;
static PFNGLVERTEXATTRIBL1DVPROC sglVertexAttribL1dv = nullptr;
static PFNGLVERTEXATTRIBL2DVPROC sglVertexAttribL2dv = nullptr;
static PFNGLVERTEXATTRIBL3DVPROC sglVertexAttribL3dv = nullptr;
static PFNGLVERTEXATTRIBL4DVPROC sglVertexAttribL4dv = nullptr;
static PFNGLVERTEXATTRIBLPOINTERPROC sglVertexAttribLPointer = nullptr;
static PFNGLGETVERTEXATTRIBLDVPROC sglGetVertexAttribLdv = nullptr;
static PFNGLVIEWPORTARRAYVPROC sglViewportArrayv = nullptr;
static PFNGLVIEWPORTINDEXEDFPROC sglViewportIndexedf = nullptr;
static PFNGLVIEWPORTINDEXEDFVPROC sglViewportIndexedfv = nullptr;
static PFNGLSCISSORARRAYVPROC sglScissorArrayv = nullptr;
static PFNGLSCISSORINDEXEDPROC sglScissorIndexed = nullptr;
static PFNGLSCISSORINDEXEDVPROC sglScissorIndexedv = nullptr;
static PFNGLDEPTHRANGEARRAYVPROC sglDepthRangeArrayv = nullptr;
static PFNGLDEPTHRANGEINDEXEDPROC sglDepthRangeIndexed = nullptr;
static PFNGLGETFLOATI_VPROC sglGetFloati_v = nullptr;
static PFNGLGETDOUBLEI_VPROC sglGetDoublei_v = nullptr;

void APIENTRY glReleaseShaderCompiler()
{
    if (sglReleaseShaderCompiler)
    {
        sglReleaseShaderCompiler();
        ReportGLError("glReleaseShaderCompiler");
    }
    else
    {
        ReportGLNullFunction("glReleaseShaderCompiler");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glReleaseShaderCompiler", "");
#endif
}

void APIENTRY glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
    if (sglShaderBinary)
    {
        sglShaderBinary(count, shaders, binaryformat, binary, length);
        ReportGLError("glShaderBinary");
    }
    else
    {
        ReportGLNullFunction("glShaderBinary");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glShaderBinary", "",
        count, gsTrace.GetArray(count, shaders), gsTrace.GetName(binaryformat), "binary", length);
#endif
}

void APIENTRY glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    if (sglGetShaderPrecisionFormat)
    {
        sglGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
        ReportGLError("glGetShaderPrecisionFormat");
    }
    else
    {
        ReportGLNullFunction("glGetShaderPrecisionFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetShaderPrecisionFormat", "",
        gsTrace.GetName(shadertype), gsTrace.GetName(precisiontype), *range, *precision);
#endif
}

void APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
    if (sglDepthRangef)
    {
        sglDepthRangef(n, f);
        ReportGLError("glDepthRangef");
    }
    else
    {
        ReportGLNullFunction("glDepthRangef");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDepthRangef", "",
        n, f);
#endif
}

void APIENTRY glClearDepthf(GLfloat d)
{
    if (sglClearDepthf)
    {
        sglClearDepthf(d);
        ReportGLError("glClearDepthf");
    }
    else
    {
        ReportGLNullFunction("glClearDepthf");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearDepthf", "",
        d);
#endif
}

void APIENTRY glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
    if (sglGetProgramBinary)
    {
        sglGetProgramBinary(program, bufSize, length, binaryFormat, binary);
        ReportGLError("glGetProgramBinary");
    }
    else
    {
        ReportGLNullFunction("glGetProgramBinary");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramBinary", "",
        program, bufSize, *length, gsTrace.GetEnumArray(*length, binaryFormat), "binary");
#endif
}

void APIENTRY glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
    if (sglProgramBinary)
    {
        sglProgramBinary(program, binaryFormat, binary, length);
        ReportGLError("glProgramBinary");
    }
    else
    {
        ReportGLNullFunction("glProgramBinary");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramBinary", "",
        program, gsTrace.GetName(binaryFormat), "binary", length);
#endif
}

void APIENTRY glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    if (sglProgramParameteri)
    {
        sglProgramParameteri(program, pname, value);
        ReportGLError("glProgramParameteri");
    }
    else
    {
        ReportGLNullFunction("glProgramParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramParameteri", "",
        program, gsTrace.GetName(pname), value);
#endif
}

void APIENTRY glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
    if (sglUseProgramStages)
    {
        sglUseProgramStages(pipeline, stages, program);
        ReportGLError("glUseProgramStages");
    }
    else
    {
        ReportGLNullFunction("glUseProgramStages");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUseProgramStages", "",
        pipeline, stages, program);
#endif
}

void APIENTRY glActiveShaderProgram(GLuint pipeline, GLuint program)
{
    if (sglActiveShaderProgram)
    {
        sglActiveShaderProgram(pipeline, program);
        ReportGLError("glActiveShaderProgram");
    }
    else
    {
        ReportGLNullFunction("glActiveShaderProgram");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glActiveShaderProgram", "",
        pipeline, program);
#endif
}

GLuint APIENTRY glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar* const* strings)
{
    GLuint result;
    if (sglCreateShaderProgramv)
    {
        result = sglCreateShaderProgramv(type, count, strings);
        ReportGLError("glCreateShaderProgramv");
    }
    else
    {
        ReportGLNullFunction("glCreateShaderProgramv");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateShaderProgramv", std::to_string(result),
        gsTrace.GetName(type), count, gsTrace.GetStringArray(count, strings));
#endif
    return result;
}

void APIENTRY glBindProgramPipeline(GLuint pipeline)
{
    if (sglBindProgramPipeline)
    {
        sglBindProgramPipeline(pipeline);
        ReportGLError("glBindProgramPipeline");
    }
    else
    {
        ReportGLNullFunction("glBindProgramPipeline");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindProgramPipeline", "",
        pipeline);
#endif
}

void APIENTRY glDeleteProgramPipelines(GLsizei n, const GLuint* pipelines)
{
    if (sglDeleteProgramPipelines)
    {
        sglDeleteProgramPipelines(n, pipelines);
        ReportGLError("glDeleteProgramPipelines");
    }
    else
    {
        ReportGLNullFunction("glDeleteProgramPipelines");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDeleteProgramPipelines", "",
        n, gsTrace.GetArray(n, pipelines));
#endif
}

void APIENTRY glGenProgramPipelines(GLsizei n, GLuint* pipelines)
{
    if (sglGenProgramPipelines)
    {
        sglGenProgramPipelines(n, pipelines);
        ReportGLError("glGenProgramPipelines");
    }
    else
    {
        ReportGLNullFunction("glGenProgramPipelines");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenProgramPipelines", "",
        n, gsTrace.GetArray(n, pipelines));
#endif
}

GLboolean APIENTRY glIsProgramPipeline(GLuint pipeline)
{
    GLboolean result;
    if (sglIsProgramPipeline)
    {
        result = sglIsProgramPipeline(pipeline);
        ReportGLError("glIsProgramPipeline");
    }
    else
    {
        ReportGLNullFunction("glIsProgramPipeline");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glIsProgramPipeline", gsTrace.GetBoolean(result),
        pipeline);
#endif
    return result;
}

void APIENTRY glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params)
{
    if (sglGetProgramPipelineiv)
    {
        sglGetProgramPipelineiv(pipeline, pname, params);
        ReportGLError("glGetProgramPipelineiv");
    }
    else
    {
        ReportGLNullFunction("glGetProgramPipelineiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramPipelineiv", "",
        pipeline, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
    if (sglProgramUniform1i)
    {
        sglProgramUniform1i(program, location, v0);
        ReportGLError("glProgramUniform1i");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1i", "",
        program, location, v0);
#endif
}

void APIENTRY glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if (sglProgramUniform1iv)
    {
        sglProgramUniform1iv(program, location, count, value);
        ReportGLError("glProgramUniform1iv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1iv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
    if (sglProgramUniform1f)
    {
        sglProgramUniform1f(program, location, v0);
        ReportGLError("glProgramUniform1f");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1f", "",
        program, location, v0);
#endif
}

void APIENTRY glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if (sglProgramUniform1fv)
    {
        sglProgramUniform1fv(program, location, count, value);
        ReportGLError("glProgramUniform1fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1fv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
    if (sglProgramUniform1d)
    {
        sglProgramUniform1d(program, location, v0);
        ReportGLError("glProgramUniform1d");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1d", "",
        program, location, v0);
#endif
}

void APIENTRY glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if (sglProgramUniform1dv)
    {
        sglProgramUniform1dv(program, location, count, value);
        ReportGLError("glProgramUniform1dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1dv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
    if (sglProgramUniform1ui)
    {
        sglProgramUniform1ui(program, location, v0);
        ReportGLError("glProgramUniform1ui");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1ui", "",
        program, location, v0);
#endif
}

void APIENTRY glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if (sglProgramUniform1uiv)
    {
        sglProgramUniform1uiv(program, location, count, value);
        ReportGLError("glProgramUniform1uiv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform1uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform1uiv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
    if (sglProgramUniform2i)
    {
        sglProgramUniform2i(program, location, v0, v1);
        ReportGLError("glProgramUniform2i");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2i", "",
        program, location, v0, v1);
#endif
}

void APIENTRY glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if (sglProgramUniform2iv)
    {
        sglProgramUniform2iv(program, location, count, value);
        ReportGLError("glProgramUniform2iv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2iv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    if (sglProgramUniform2f)
    {
        sglProgramUniform2f(program, location, v0, v1);
        ReportGLError("glProgramUniform2f");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2f", "",
        program, location, v0, v1);
#endif
}

void APIENTRY glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if (sglProgramUniform2fv)
    {
        sglProgramUniform2fv(program, location, count, value);
        ReportGLError("glProgramUniform2fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2fv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
    if (sglProgramUniform2d)
    {
        sglProgramUniform2d(program, location, v0, v1);
        ReportGLError("glProgramUniform2d");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2d", "",
        program, location, v0, v1);
#endif
}

void APIENTRY glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if (sglProgramUniform2dv)
    {
        sglProgramUniform2dv(program, location, count, value);
        ReportGLError("glProgramUniform2dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2dv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    if (sglProgramUniform2ui)
    {
        sglProgramUniform2ui(program, location, v0, v1);
        ReportGLError("glProgramUniform2ui");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2ui", "",
        program, location, v0, v1);
#endif
}

void APIENTRY glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if (sglProgramUniform2uiv)
    {
        sglProgramUniform2uiv(program, location, count, value);
        ReportGLError("glProgramUniform2uiv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform2uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform2uiv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    if (sglProgramUniform3i)
    {
        sglProgramUniform3i(program, location, v0, v1, v2);
        ReportGLError("glProgramUniform3i");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3i", "",
        program, location, v0, v1, v2);
#endif
}

void APIENTRY glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if (sglProgramUniform3iv)
    {
        sglProgramUniform3iv(program, location, count, value);
        ReportGLError("glProgramUniform3iv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3iv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    if (sglProgramUniform3f)
    {
        sglProgramUniform3f(program, location, v0, v1, v2);
        ReportGLError("glProgramUniform3f");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3f", "",
        program, location, v0, v1, v2);
#endif
}

void APIENTRY glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if (sglProgramUniform3fv)
    {
        sglProgramUniform3fv(program, location, count, value);
        ReportGLError("glProgramUniform3fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3fv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
    if (sglProgramUniform3d)
    {
        sglProgramUniform3d(program, location, v0, v1, v2);
        ReportGLError("glProgramUniform3d");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3d", "",
        program, location, v0, v1, v2);
#endif
}

void APIENTRY glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if (sglProgramUniform3dv)
    {
        sglProgramUniform3dv(program, location, count, value);
        ReportGLError("glProgramUniform3dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3dv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    if (sglProgramUniform3ui)
    {
        sglProgramUniform3ui(program, location, v0, v1, v2);
        ReportGLError("glProgramUniform3ui");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3ui", "",
        program, location, v0, v1, v2);
#endif
}

void APIENTRY glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if (sglProgramUniform3uiv)
    {
        sglProgramUniform3uiv(program, location, count, value);
        ReportGLError("glProgramUniform3uiv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform3uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform3uiv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    if (sglProgramUniform4i)
    {
        sglProgramUniform4i(program, location, v0, v1, v2, v3);
        ReportGLError("glProgramUniform4i");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4i");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4i", "",
        program, location, v0, v1, v2, v3);
#endif
}

void APIENTRY glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
    if (sglProgramUniform4iv)
    {
        sglProgramUniform4iv(program, location, count, value);
        ReportGLError("glProgramUniform4iv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4iv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    if (sglProgramUniform4f)
    {
        sglProgramUniform4f(program, location, v0, v1, v2, v3);
        ReportGLError("glProgramUniform4f");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4f");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4f", "",
        program, location, v0, v1, v2, v3);
#endif
}

void APIENTRY glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
    if (sglProgramUniform4fv)
    {
        sglProgramUniform4fv(program, location, count, value);
        ReportGLError("glProgramUniform4fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4fv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
    if (sglProgramUniform4d)
    {
        sglProgramUniform4d(program, location, v0, v1, v2, v3);
        ReportGLError("glProgramUniform4d");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4d", "",
        program, location, v0, v1, v2, v3);
#endif
}

void APIENTRY glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
    if (sglProgramUniform4dv)
    {
        sglProgramUniform4dv(program, location, count, value);
        ReportGLError("glProgramUniform4dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4dv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    if (sglProgramUniform4ui)
    {
        sglProgramUniform4ui(program, location, v0, v1, v2, v3);
        ReportGLError("glProgramUniform4ui");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4ui");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4ui", "",
        program, location, v0, v1, v2, v3);
#endif
}

void APIENTRY glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
    if (sglProgramUniform4uiv)
    {
        sglProgramUniform4uiv(program, location, count, value);
        ReportGLError("glProgramUniform4uiv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniform4uiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniform4uiv", "",
        program, location, count, gsTrace.GetArray(count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix2fv)
    {
        sglProgramUniformMatrix2fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(4ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix3fv)
    {
        sglProgramUniformMatrix3fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(9ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix4fv)
    {
        sglProgramUniformMatrix4fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(16ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix2dv)
    {
        sglProgramUniformMatrix2dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(4ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix3dv)
    {
        sglProgramUniformMatrix3dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(9ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix4dv)
    {
        sglProgramUniformMatrix4dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(16ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix2x3fv)
    {
        sglProgramUniformMatrix2x3fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2x3fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2x3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2x3fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix3x2fv)
    {
        sglProgramUniformMatrix3x2fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3x2fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3x2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3x2fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix2x4fv)
    {
        sglProgramUniformMatrix2x4fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2x4fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2x4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2x4fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix4x2fv)
    {
        sglProgramUniformMatrix4x2fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4x2fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4x2fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4x2fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix3x4fv)
    {
        sglProgramUniformMatrix3x4fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3x4fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3x4fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3x4fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (sglProgramUniformMatrix4x3fv)
    {
        sglProgramUniformMatrix4x3fv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4x3fv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4x3fv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4x3fv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix2x3dv)
    {
        sglProgramUniformMatrix2x3dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2x3dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2x3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2x3dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix3x2dv)
    {
        sglProgramUniformMatrix3x2dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3x2dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3x2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3x2dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(6ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix2x4dv)
    {
        sglProgramUniformMatrix2x4dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix2x4dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix2x4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix2x4dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix4x2dv)
    {
        sglProgramUniformMatrix4x2dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4x2dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4x2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4x2dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(8ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix3x4dv)
    {
        sglProgramUniformMatrix3x4dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix3x4dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix3x4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix3x4dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
    if (sglProgramUniformMatrix4x3dv)
    {
        sglProgramUniformMatrix4x3dv(program, location, count, transpose, value);
        ReportGLError("glProgramUniformMatrix4x3dv");
    }
    else
    {
        ReportGLNullFunction("glProgramUniformMatrix4x3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glProgramUniformMatrix4x3dv", "",
        program, location, count, gsTrace.GetBoolean(transpose), gsTrace.GetArray(12ull * count, value));
#endif
}

void APIENTRY glValidateProgramPipeline(GLuint pipeline)
{
    if (sglValidateProgramPipeline)
    {
        sglValidateProgramPipeline(pipeline);
        ReportGLError("glValidateProgramPipeline");
    }
    else
    {
        ReportGLNullFunction("glValidateProgramPipeline");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glValidateProgramPipeline", "",
        pipeline);
#endif
}

void APIENTRY glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (sglGetProgramPipelineInfoLog)
    {
        sglGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
        ReportGLError("glGetProgramPipelineInfoLog");
    }
    else
    {
        ReportGLNullFunction("glGetProgramPipelineInfoLog");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramPipelineInfoLog", "",
        pipeline, bufSize, *length, infoLog);
#endif
}

void APIENTRY glVertexAttribL1d(GLuint index, GLdouble x)
{
    if (sglVertexAttribL1d)
    {
        sglVertexAttribL1d(index, x);
        ReportGLError("glVertexAttribL1d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL1d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL1d", "",
        index, x);
#endif
}

void APIENTRY glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
    if (sglVertexAttribL2d)
    {
        sglVertexAttribL2d(index, x, y);
        ReportGLError("glVertexAttribL2d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL2d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL1d", "",
        index, x, y);
#endif
}

void APIENTRY glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    if (sglVertexAttribL3d)
    {
        sglVertexAttribL3d(index, x, y, z);
        ReportGLError("glVertexAttribL3d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL3d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL1d", "",
        index, x, y, z);
#endif
}

void APIENTRY glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    if (sglVertexAttribL4d)
    {
        sglVertexAttribL4d(index, x, y, z, w);
        ReportGLError("glVertexAttribL4d");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL4d");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL1d", "",
        index, x, y, z, w);
#endif
}

void APIENTRY glVertexAttribL1dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttribL1dv)
    {
        sglVertexAttribL1dv(index, v);
        ReportGLError("glVertexAttribL1dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL1dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL1dv", "",
        index, *v);
#endif
}

void APIENTRY glVertexAttribL2dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttribL2dv)
    {
        sglVertexAttribL2dv(index, v);
        ReportGLError("glVertexAttribL2dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL2dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL2dv", "",
        index, gsTrace.GetArray(2, v));
#endif
}

void APIENTRY glVertexAttribL3dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttribL3dv)
    {
        sglVertexAttribL3dv(index, v);
        ReportGLError("glVertexAttribL3dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL3dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL3dv", "",
        index, gsTrace.GetArray(3, v));
#endif
}

void APIENTRY glVertexAttribL4dv(GLuint index, const GLdouble* v)
{
    if (sglVertexAttribL4dv)
    {
        sglVertexAttribL4dv(index, v);
        ReportGLError("glVertexAttribL4dv");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribL4dv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribL4dv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    if (sglVertexAttribLPointer)
    {
        sglVertexAttribLPointer(index, size, type, stride, pointer);
        ReportGLError("glVertexAttribLPointer");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribLPointer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribLPointer", "",
        index, size, gsTrace.GetName(type), stride),
        gsTrace.GetArray(size, reinterpret_cast<std::uint64_t const*>(pointer));
#endif
}

void APIENTRY glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble* params)
{
    if (sglGetVertexAttribLdv)
    {
        sglGetVertexAttribLdv(index, pname, params);
        ReportGLError("glGetVertexAttribLdv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexAttribLdv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexAttribLdv", "",
        index, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glViewportArrayv(GLuint first, GLsizei count, const GLfloat* v)
{
    if (sglViewportArrayv)
    {
        sglViewportArrayv(first, count, v);
        ReportGLError("glViewportArrayv");
    }
    else
    {
        ReportGLNullFunction("glViewportArrayv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glViewportArrayv", "",
        first, count, gsTrace.GetArray(4ull * count, v + 4ull * first));
#endif
}

void APIENTRY glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    if (sglViewportIndexedf)
    {
        sglViewportIndexedf(index, x, y, w, h);
        ReportGLError("glViewportIndexedf");
    }
    else
    {
        ReportGLNullFunction("glViewportIndexedf");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glViewportIndexedf", "",
        index, x, y, w, h);
#endif
}

void APIENTRY glViewportIndexedfv(GLuint index, const GLfloat* v)
{
    if (sglViewportIndexedfv)
    {
        sglViewportIndexedfv(index, v);
        ReportGLError("glViewportIndexedfv");
    }
    else
    {
        ReportGLNullFunction("glViewportIndexedfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glViewportIndexedfv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glScissorArrayv(GLuint first, GLsizei count, const GLint* v)
{
    if (sglScissorArrayv)
    {
        sglScissorArrayv(first, count, v);
        ReportGLError("glScissorArrayv");
    }
    else
    {
        ReportGLNullFunction("glScissorArrayv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glScissorArrayv", "",
        first, count, gsTrace.GetArray(4ull * count, v + 4ull * first));
#endif
}

void APIENTRY glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
    if (sglScissorIndexed)
    {
        sglScissorIndexed(index, left, bottom, width, height);
        ReportGLError("glScissorIndexed");
    }
    else
    {
        ReportGLNullFunction("glScissorIndexed");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glScissorIndexed", "",
        index, left, bottom, width, height);
#endif
}

void APIENTRY glScissorIndexedv(GLuint index, const GLint* v)
{
    if (sglScissorIndexedv)
    {
        sglScissorIndexedv(index, v);
        ReportGLError("glScissorIndexedv");
    }
    else
    {
        ReportGLNullFunction("glScissorIndexedv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glScissorIndexedv", "",
        index, gsTrace.GetArray(4, v));
#endif
}

void APIENTRY glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble* v)
{
    if (sglDepthRangeArrayv)
    {
        sglDepthRangeArrayv(first, count, v);
        ReportGLError("glDepthRangeArrayv");
    }
    else
    {
        ReportGLNullFunction("glDepthRangeArrayv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDepthRangeArrayv", "",
        first, count, gsTrace.GetArray(2ull * count, v + 2ull * first));
#endif
}

void APIENTRY glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f)
{
    if (sglDepthRangeIndexed)
    {
        sglDepthRangeIndexed(index, n, f);
        ReportGLError("glDepthRangeIndexed");
    }
    else
    {
        ReportGLNullFunction("glDepthRangeIndexed");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDepthRangeIndexed", "",
        index, n, f);
#endif
}

void APIENTRY glGetFloati_v(GLenum target, GLuint index, GLfloat* data)
{
    if (sglGetFloati_v)
    {
        sglGetFloati_v(target, index, data);
        ReportGLError("glGetFloati_v");
    }
    else
    {
        ReportGLNullFunction("glGetFloati_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetFloati_v", "",
        gsTrace.GetName(target), index, *data);
#endif
}

void APIENTRY glGetDoublei_v(GLenum target, GLuint index, GLdouble* data)
{
    if (sglGetDoublei_v)
    {
        sglGetDoublei_v(target, index, data);
        ReportGLError("glGetDoublei_v");
    }
    else
    {
        ReportGLNullFunction("glGetDoublei_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetDoublei_v", "",
        gsTrace.GetName(target), index, *data);
#endif
}

static void Initialize_OPENGL_VERSION_4_1()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_1)
    {
        GetOpenGLFunction("glReleaseShaderCompiler", sglReleaseShaderCompiler);
        GetOpenGLFunction("glShaderBinary", sglShaderBinary);
        GetOpenGLFunction("glGetShaderPrecisionFormat", sglGetShaderPrecisionFormat);
        GetOpenGLFunction("glDepthRangef", sglDepthRangef);
        GetOpenGLFunction("glClearDepthf", sglClearDepthf);
        GetOpenGLFunction("glGetProgramBinary", sglGetProgramBinary);
        GetOpenGLFunction("glProgramBinary", sglProgramBinary);
        GetOpenGLFunction("glProgramParameteri", sglProgramParameteri);
        GetOpenGLFunction("glUseProgramStages", sglUseProgramStages);
        GetOpenGLFunction("glActiveShaderProgram", sglActiveShaderProgram);
        GetOpenGLFunction("glCreateShaderProgramv", sglCreateShaderProgramv);
        GetOpenGLFunction("glBindProgramPipeline", sglBindProgramPipeline);
        GetOpenGLFunction("glDeleteProgramPipelines", sglDeleteProgramPipelines);
        GetOpenGLFunction("glGenProgramPipelines", sglGenProgramPipelines);
        GetOpenGLFunction("glIsProgramPipeline", sglIsProgramPipeline);
        GetOpenGLFunction("glGetProgramPipelineiv", sglGetProgramPipelineiv);
        GetOpenGLFunction("glProgramUniform1i", sglProgramUniform1i);
        GetOpenGLFunction("glProgramUniform1iv", sglProgramUniform1iv);
        GetOpenGLFunction("glProgramUniform1f", sglProgramUniform1f);
        GetOpenGLFunction("glProgramUniform1fv", sglProgramUniform1fv);
        GetOpenGLFunction("glProgramUniform1d", sglProgramUniform1d);
        GetOpenGLFunction("glProgramUniform1dv", sglProgramUniform1dv);
        GetOpenGLFunction("glProgramUniform1ui", sglProgramUniform1ui);
        GetOpenGLFunction("glProgramUniform1uiv", sglProgramUniform1uiv);
        GetOpenGLFunction("glProgramUniform2i", sglProgramUniform2i);
        GetOpenGLFunction("glProgramUniform2iv", sglProgramUniform2iv);
        GetOpenGLFunction("glProgramUniform2f", sglProgramUniform2f);
        GetOpenGLFunction("glProgramUniform2fv", sglProgramUniform2fv);
        GetOpenGLFunction("glProgramUniform2d", sglProgramUniform2d);
        GetOpenGLFunction("glProgramUniform2dv", sglProgramUniform2dv);
        GetOpenGLFunction("glProgramUniform2ui", sglProgramUniform2ui);
        GetOpenGLFunction("glProgramUniform2uiv", sglProgramUniform2uiv);
        GetOpenGLFunction("glProgramUniform3i", sglProgramUniform3i);
        GetOpenGLFunction("glProgramUniform3iv", sglProgramUniform3iv);
        GetOpenGLFunction("glProgramUniform3f", sglProgramUniform3f);
        GetOpenGLFunction("glProgramUniform3fv", sglProgramUniform3fv);
        GetOpenGLFunction("glProgramUniform3d", sglProgramUniform3d);
        GetOpenGLFunction("glProgramUniform3dv", sglProgramUniform3dv);
        GetOpenGLFunction("glProgramUniform3ui", sglProgramUniform3ui);
        GetOpenGLFunction("glProgramUniform3uiv", sglProgramUniform3uiv);
        GetOpenGLFunction("glProgramUniform4i", sglProgramUniform4i);
        GetOpenGLFunction("glProgramUniform4iv", sglProgramUniform4iv);
        GetOpenGLFunction("glProgramUniform4f", sglProgramUniform4f);
        GetOpenGLFunction("glProgramUniform4fv", sglProgramUniform4fv);
        GetOpenGLFunction("glProgramUniform4d", sglProgramUniform4d);
        GetOpenGLFunction("glProgramUniform4dv", sglProgramUniform4dv);
        GetOpenGLFunction("glProgramUniform4ui", sglProgramUniform4ui);
        GetOpenGLFunction("glProgramUniform4uiv", sglProgramUniform4uiv);
        GetOpenGLFunction("glProgramUniformMatrix2fv", sglProgramUniformMatrix2fv);
        GetOpenGLFunction("glProgramUniformMatrix3fv", sglProgramUniformMatrix3fv);
        GetOpenGLFunction("glProgramUniformMatrix4fv", sglProgramUniformMatrix4fv);
        GetOpenGLFunction("glProgramUniformMatrix2dv", sglProgramUniformMatrix2dv);
        GetOpenGLFunction("glProgramUniformMatrix3dv", sglProgramUniformMatrix3dv);
        GetOpenGLFunction("glProgramUniformMatrix4dv", sglProgramUniformMatrix4dv);
        GetOpenGLFunction("glProgramUniformMatrix2x3fv", sglProgramUniformMatrix2x3fv);
        GetOpenGLFunction("glProgramUniformMatrix3x2fv", sglProgramUniformMatrix3x2fv);
        GetOpenGLFunction("glProgramUniformMatrix2x4fv", sglProgramUniformMatrix2x4fv);
        GetOpenGLFunction("glProgramUniformMatrix4x2fv", sglProgramUniformMatrix4x2fv);
        GetOpenGLFunction("glProgramUniformMatrix3x4fv", sglProgramUniformMatrix3x4fv);
        GetOpenGLFunction("glProgramUniformMatrix4x3fv", sglProgramUniformMatrix4x3fv);
        GetOpenGLFunction("glProgramUniformMatrix2x3dv", sglProgramUniformMatrix2x3dv);
        GetOpenGLFunction("glProgramUniformMatrix3x2dv", sglProgramUniformMatrix3x2dv);
        GetOpenGLFunction("glProgramUniformMatrix2x4dv", sglProgramUniformMatrix2x4dv);
        GetOpenGLFunction("glProgramUniformMatrix4x2dv", sglProgramUniformMatrix4x2dv);
        GetOpenGLFunction("glProgramUniformMatrix3x4dv", sglProgramUniformMatrix3x4dv);
        GetOpenGLFunction("glProgramUniformMatrix4x3dv", sglProgramUniformMatrix4x3dv);
        GetOpenGLFunction("glValidateProgramPipeline", sglValidateProgramPipeline);
        GetOpenGLFunction("glGetProgramPipelineInfoLog", sglGetProgramPipelineInfoLog);
        GetOpenGLFunction("glVertexAttribL1d", sglVertexAttribL1d);
        GetOpenGLFunction("glVertexAttribL2d", sglVertexAttribL2d);
        GetOpenGLFunction("glVertexAttribL3d", sglVertexAttribL3d);
        GetOpenGLFunction("glVertexAttribL4d", sglVertexAttribL4d);
        GetOpenGLFunction("glVertexAttribL1dv", sglVertexAttribL1dv);
        GetOpenGLFunction("glVertexAttribL2dv", sglVertexAttribL2dv);
        GetOpenGLFunction("glVertexAttribL3dv", sglVertexAttribL3dv);
        GetOpenGLFunction("glVertexAttribL4dv", sglVertexAttribL4dv);
        GetOpenGLFunction("glVertexAttribLPointer", sglVertexAttribLPointer);
        GetOpenGLFunction("glGetVertexAttribLdv", sglGetVertexAttribLdv);
        GetOpenGLFunction("glViewportArrayv", sglViewportArrayv);
        GetOpenGLFunction("glViewportIndexedf", sglViewportIndexedf);
        GetOpenGLFunction("glViewportIndexedfv", sglViewportIndexedfv);
        GetOpenGLFunction("glScissorArrayv", sglScissorArrayv);
        GetOpenGLFunction("glScissorIndexed", sglScissorIndexed);
        GetOpenGLFunction("glScissorIndexedv", sglScissorIndexedv);
        GetOpenGLFunction("glDepthRangeArrayv", sglDepthRangeArrayv);
        GetOpenGLFunction("glDepthRangeIndexed", sglDepthRangeIndexed);
        GetOpenGLFunction("glGetFloati_v", sglGetFloati_v);
        GetOpenGLFunction("glGetDoublei_v", sglGetDoublei_v);
    }
}

// GL_VERSION_4_2

static PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC sglDrawArraysInstancedBaseInstance = nullptr;
static PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC sglDrawElementsInstancedBaseInstance = nullptr;
static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC sglDrawElementsInstancedBaseVertexBaseInstance = nullptr;
static PFNGLGETINTERNALFORMATIVPROC sglGetInternalformativ = nullptr;
static PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC sglGetActiveAtomicCounterBufferiv = nullptr;
static PFNGLBINDIMAGETEXTUREPROC sglBindImageTexture = nullptr;
static PFNGLMEMORYBARRIERPROC sglMemoryBarrier = nullptr;
static PFNGLTEXSTORAGE1DPROC sglTexStorage1D = nullptr;
static PFNGLTEXSTORAGE2DPROC sglTexStorage2D = nullptr;
static PFNGLTEXSTORAGE3DPROC sglTexStorage3D = nullptr;
static PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC sglDrawTransformFeedbackInstanced = nullptr;
static PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC sglDrawTransformFeedbackStreamInstanced = nullptr;

void APIENTRY glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    if (sglDrawArraysInstancedBaseInstance)
    {
        sglDrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
        ReportGLError("glDrawArraysInstancedBaseInstance");
    }
    else
    {
        ReportGLNullFunction("glDrawArraysInstancedBaseInstance");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawArraysInstancedBaseInstance", "",
        gsTrace.GetName(mode), first, count, instancecount, baseinstance);
#endif
}

void APIENTRY glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance)
{
    if (sglDrawElementsInstancedBaseInstance)
    {
        sglDrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
        ReportGLError("glDrawElementsInstancedBaseInstance");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsInstancedBaseInstance");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsInstancedBaseInstance", "",
        gsTrace.GetName(mode), count, gsTrace.GetName(type),
        "indices", instancecount, baseinstance);
#endif
}

void APIENTRY glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    if (sglDrawElementsInstancedBaseVertexBaseInstance)
    {
        sglDrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance);
        ReportGLError("glDrawElementsInstancedBaseVertexBaseInstance");
    }
    else
    {
        ReportGLNullFunction("glDrawElementsInstancedBaseVertexBaseInstance");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawElementsInstancedBaseVertexBaseInstance", "",
        gsTrace.GetName(mode), count, gsTrace.GetName(type),
        "indices", instancecount, basevertex, baseinstance);
#endif
}

void APIENTRY glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    if (sglGetInternalformativ)
    {
        sglGetInternalformativ(target, internalformat, pname, bufSize, params);
        ReportGLError("glGetInternalformativ");
    }
    else
    {
        ReportGLNullFunction("glGetInternalformativ");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetInternalformativ", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat),
        gsTrace.GetName(pname), bufSize, *params);
#endif
}

void APIENTRY glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params)
{
    if (sglGetActiveAtomicCounterBufferiv)
    {
        sglGetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
        ReportGLError("glGetActiveAtomicCounterBufferiv");
    }
    else
    {
        ReportGLNullFunction("glGetActiveAtomicCounterBufferiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetActiveAtomicCounterBufferiv", "",
        program, bufferIndex, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    if (sglBindImageTexture)
    {
        sglBindImageTexture(unit, texture, level, layered, layer, access, format);
        ReportGLError("glBindImageTexture");
    }
    else
    {
        ReportGLNullFunction("glBindImageTexture");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindImageTexture", "",
        unit, texture, level, gsTrace.GetBoolean(layered), layer,
        gsTrace.GetName(access), gsTrace.GetName(format));
#endif
}

void APIENTRY glMemoryBarrier(GLbitfield barriers)
{
    if (sglMemoryBarrier)
    {
        sglMemoryBarrier(barriers);
        ReportGLError("glMemoryBarrier");
    }
    else
    {
        ReportGLNullFunction("glMemoryBarrier");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMemoryBarrier", "",
        barriers);
#endif
}

void APIENTRY glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    if (sglTexStorage1D)
    {
        sglTexStorage1D(target, levels, internalformat, width);
        ReportGLError("glTexStorage1D");
    }
    else
    {
        ReportGLNullFunction("glTexStorage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexStorage1D", "",
        gsTrace.GetName(target), levels, gsTrace.GetName(internalformat),
        width);
#endif
}

void APIENTRY glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglTexStorage2D)
    {
        sglTexStorage2D(target, levels, internalformat, width, height);
        ReportGLError("glTexStorage2D");
    }
    else
    {
        ReportGLNullFunction("glTexStorage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexStorage2D", "",
        gsTrace.GetName(target), levels, gsTrace.GetName(internalformat),
        width, height);
#endif
}

void APIENTRY glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    if (sglTexStorage3D)
    {
        sglTexStorage3D(target, levels, internalformat, width, height, depth);
        ReportGLError("glTexStorage3D");
    }
    else
    {
        ReportGLNullFunction("glTexStorage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexStorage3D", "",
        gsTrace.GetName(target), levels, gsTrace.GetName(internalformat),
        width, height, depth);
#endif
}

void APIENTRY glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount)
{
    if (sglDrawTransformFeedbackInstanced)
    {
        sglDrawTransformFeedbackInstanced(mode, id, instancecount);
        ReportGLError("glDrawTransformFeedbackInstanced");
    }
    else
    {
        ReportGLNullFunction("glDrawTransformFeedbackInstanced");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawTransformFeedbackInstanced", "",
        gsTrace.GetName(mode), id, instancecount);
#endif
}

void APIENTRY glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
    if (sglDrawTransformFeedbackStreamInstanced)
    {
        sglDrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount);
        ReportGLError("glDrawTransformFeedbackStreamInstanced");
    }
    else
    {
        ReportGLNullFunction("glDrawTransformFeedbackStreamInstanced");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDrawTransformFeedbackStreamInstanced", "",
        gsTrace.GetName(mode), id, stream, instancecount);
#endif
}

static void Initialize_OPENGL_VERSION_4_2()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_2)
    {
        GetOpenGLFunction("glDrawArraysInstancedBaseInstance", sglDrawArraysInstancedBaseInstance);
        GetOpenGLFunction("glDrawElementsInstancedBaseInstance", sglDrawElementsInstancedBaseInstance);
        GetOpenGLFunction("glDrawElementsInstancedBaseVertexBaseInstance", sglDrawElementsInstancedBaseVertexBaseInstance);
        GetOpenGLFunction("glGetInternalformativ", sglGetInternalformativ);
        GetOpenGLFunction("glGetActiveAtomicCounterBufferiv", sglGetActiveAtomicCounterBufferiv);
        GetOpenGLFunction("glBindImageTexture", sglBindImageTexture);
        GetOpenGLFunction("glMemoryBarrier", sglMemoryBarrier);
        GetOpenGLFunction("glTexStorage1D", sglTexStorage1D);
        GetOpenGLFunction("glTexStorage2D", sglTexStorage2D);
        GetOpenGLFunction("glTexStorage3D", sglTexStorage3D);
        GetOpenGLFunction("glDrawTransformFeedbackInstanced", sglDrawTransformFeedbackInstanced);
        GetOpenGLFunction("glDrawTransformFeedbackStreamInstanced", sglDrawTransformFeedbackStreamInstanced);
    }
}

// GL_VERSION_4_3

static PFNGLCLEARBUFFERDATAPROC sglClearBufferData = nullptr;
static PFNGLCLEARBUFFERSUBDATAPROC sglClearBufferSubData = nullptr;
static PFNGLDISPATCHCOMPUTEPROC sglDispatchCompute = nullptr;
static PFNGLDISPATCHCOMPUTEINDIRECTPROC sglDispatchComputeIndirect = nullptr;
static PFNGLCOPYIMAGESUBDATAPROC sglCopyImageSubData = nullptr;
static PFNGLFRAMEBUFFERPARAMETERIPROC sglFramebufferParameteri = nullptr;
static PFNGLGETFRAMEBUFFERPARAMETERIVPROC sglGetFramebufferParameteriv = nullptr;
static PFNGLGETINTERNALFORMATI64VPROC sglGetInternalformati64v = nullptr;
static PFNGLINVALIDATETEXSUBIMAGEPROC sglInvalidateTexSubImage = nullptr;
static PFNGLINVALIDATETEXIMAGEPROC sglInvalidateTexImage = nullptr;
static PFNGLINVALIDATEBUFFERSUBDATAPROC sglInvalidateBufferSubData = nullptr;
static PFNGLINVALIDATEBUFFERDATAPROC sglInvalidateBufferData = nullptr;
static PFNGLINVALIDATEFRAMEBUFFERPROC sglInvalidateFramebuffer = nullptr;
static PFNGLINVALIDATESUBFRAMEBUFFERPROC sglInvalidateSubFramebuffer = nullptr;
static PFNGLMULTIDRAWARRAYSINDIRECTPROC sglMultiDrawArraysIndirect = nullptr;
static PFNGLMULTIDRAWELEMENTSINDIRECTPROC sglMultiDrawElementsIndirect = nullptr;
static PFNGLGETPROGRAMINTERFACEIVPROC sglGetProgramInterfaceiv = nullptr;
static PFNGLGETPROGRAMRESOURCEINDEXPROC sglGetProgramResourceIndex = nullptr;
static PFNGLGETPROGRAMRESOURCENAMEPROC sglGetProgramResourceName = nullptr;
static PFNGLGETPROGRAMRESOURCEIVPROC sglGetProgramResourceiv = nullptr;
static PFNGLGETPROGRAMRESOURCELOCATIONPROC sglGetProgramResourceLocation = nullptr;
static PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC sglGetProgramResourceLocationIndex = nullptr;
static PFNGLSHADERSTORAGEBLOCKBINDINGPROC sglShaderStorageBlockBinding = nullptr;
static PFNGLTEXBUFFERRANGEPROC sglTexBufferRange = nullptr;
static PFNGLTEXSTORAGE2DMULTISAMPLEPROC sglTexStorage2DMultisample = nullptr;
static PFNGLTEXSTORAGE3DMULTISAMPLEPROC sglTexStorage3DMultisample = nullptr;
static PFNGLTEXTUREVIEWPROC sglTextureView = nullptr;
static PFNGLBINDVERTEXBUFFERPROC sglBindVertexBuffer = nullptr;
static PFNGLVERTEXATTRIBFORMATPROC sglVertexAttribFormat = nullptr;
static PFNGLVERTEXATTRIBIFORMATPROC sglVertexAttribIFormat = nullptr;
static PFNGLVERTEXATTRIBLFORMATPROC sglVertexAttribLFormat = nullptr;
static PFNGLVERTEXATTRIBBINDINGPROC sglVertexAttribBinding = nullptr;
static PFNGLVERTEXBINDINGDIVISORPROC sglVertexBindingDivisor = nullptr;
static PFNGLDEBUGMESSAGECONTROLPROC sglDebugMessageControl = nullptr;
static PFNGLDEBUGMESSAGEINSERTPROC sglDebugMessageInsert = nullptr;
static PFNGLDEBUGMESSAGECALLBACKPROC sglDebugMessageCallback = nullptr;
static PFNGLGETDEBUGMESSAGELOGPROC sglGetDebugMessageLog = nullptr;
static PFNGLPUSHDEBUGGROUPPROC sglPushDebugGroup = nullptr;
static PFNGLPOPDEBUGGROUPPROC sglPopDebugGroup = nullptr;
static PFNGLOBJECTLABELPROC sglObjectLabel = nullptr;
static PFNGLGETOBJECTLABELPROC sglGetObjectLabel = nullptr;
static PFNGLOBJECTPTRLABELPROC sglObjectPtrLabel = nullptr;
static PFNGLGETOBJECTPTRLABELPROC sglGetObjectPtrLabel = nullptr;

void APIENTRY glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
    if (sglClearBufferData)
    {
        sglClearBufferData(target, internalformat, format, type, data);
        ReportGLError("glClearBufferData");
    }
    else
    {
        ReportGLNullFunction("glClearBufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferData", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat),
        gsTrace.GetName(type), "data");
#endif
}

void APIENTRY glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
    if (sglClearBufferSubData)
    {
        sglClearBufferSubData(target, internalformat, offset, size, format, type, data);
        ReportGLError("glClearBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glClearBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearBufferSubData", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat),
        offset, size, gsTrace.GetName(format), gsTrace.GetName(type), "data");
#endif
}

void APIENTRY glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    if (sglDispatchCompute)
    {
        sglDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
        ReportGLError("glDispatchCompute");
    }
    else
    {
        ReportGLNullFunction("glDispatchCompute");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDispatchCompute", "",
        num_groups_x, num_groups_y, num_groups_z);
#endif
}

void APIENTRY glDispatchComputeIndirect(GLintptr indirect)
{
    if (sglDispatchComputeIndirect)
    {
        sglDispatchComputeIndirect(indirect);
        ReportGLError("glDispatchComputeIndirect");
    }
    else
    {
        ReportGLNullFunction("glDispatchComputeIndirect");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDispatchComputeIndirect", "",
        indirect);
#endif
}

void APIENTRY glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    if (sglCopyImageSubData)
    {
        sglCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
        ReportGLError("glCopyImageSubData");
    }
    else
    {
        ReportGLNullFunction("glCopyImageSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyImageSubData", "",
        gsTrace.GetName(srcName), gsTrace.GetName(srcTarget), srcLevel,
        srcX, srcY, srcZ, gsTrace.GetName(dstName), gsTrace.GetName(dstTarget),
        dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
#endif
}

void APIENTRY glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    if (sglFramebufferParameteri)
    {
        sglFramebufferParameteri(target, pname, param);
        ReportGLError("glFramebufferParameteri");
    }
    else
    {
        ReportGLNullFunction("glFramebufferParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferParameteri", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    if (sglGetFramebufferParameteriv)
    {
        sglGetFramebufferParameteriv(target, pname, params);
        ReportGLError("glGetFramebufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetFramebufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFramebufferParameteriv", "",
        gsTrace.GetName(target), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64* params)
{
    if (sglGetInternalformati64v)
    {
        sglGetInternalformati64v(target, internalformat, pname, bufSize, params);
        ReportGLError("glGetInternalformati64v");
    }
    else
    {
        ReportGLNullFunction("glGetInternalformati64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetInternalformati64v", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat),
        gsTrace.GetName(pname), bufSize, *params);
#endif
}

void APIENTRY glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    if (sglInvalidateTexSubImage)
    {
        sglInvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth);
        ReportGLError("glInvalidateTexSubImage");
    }
    else
    {
        ReportGLNullFunction("glInvalidateTexSubImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateTexSubImage", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth);
#endif
}

void APIENTRY glInvalidateTexImage(GLuint texture, GLint level)
{
    if (sglInvalidateTexImage)
    {
        sglInvalidateTexImage(texture, level);
        ReportGLError("glInvalidateTexImage");
    }
    else
    {
        ReportGLNullFunction("glInvalidateTexImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateTexImage", "",
        texture, level);
#endif
}

void APIENTRY glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    if (sglInvalidateBufferSubData)
    {
        sglInvalidateBufferSubData(buffer, offset, length);
        ReportGLError("glInvalidateBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glInvalidateBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateBufferSubData", "",
        buffer, offset, length);
#endif
}

void APIENTRY glInvalidateBufferData(GLuint buffer)
{
    if (sglInvalidateBufferData)
    {
        sglInvalidateBufferData(buffer);
        ReportGLError("glInvalidateBufferData");
    }
    else
    {
        ReportGLNullFunction("glInvalidateBufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateBufferData", "",
        buffer);
#endif
}

void APIENTRY glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    if (sglInvalidateFramebuffer)
    {
        sglInvalidateFramebuffer(target, numAttachments, attachments);
        ReportGLError("glInvalidateFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glInvalidateFramebuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateFramebuffer", "",
        gsTrace.GetName(target), numAttachments,
        gsTrace.GetEnumArray(numAttachments, attachments));
#endif
}

void APIENTRY glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglInvalidateSubFramebuffer)
    {
        sglInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
        ReportGLError("glInvalidateSubFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glInvalidateSubFramebuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateSubFramebuffer", "",
        gsTrace.GetName(target), numAttachments,
        gsTrace.GetEnumArray(numAttachments, attachments), x, y, width, height);
#endif
}

void APIENTRY glMultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride)
{
    if (sglMultiDrawArraysIndirect)
    {
        sglMultiDrawArraysIndirect(mode, indirect, drawcount, stride);
        ReportGLError("glMultiDrawArraysIndirect");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawArraysIndirect");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawArraysIndirect", "",
        gsTrace.GetName(mode), "indirect", drawcount, stride);
#endif
}

void APIENTRY glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride)
{
    if (sglMultiDrawElementsIndirect)
    {
        sglMultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
        ReportGLError("glMultiDrawElementsIndirect");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawElementsIndirect");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawElementsIndirect", "",
        gsTrace.GetName(mode), gsTrace.GetName(type), "indirect", drawcount, stride);
#endif
}

void APIENTRY glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params)
{
    if (sglGetProgramInterfaceiv)
    {
        sglGetProgramInterfaceiv(program, programInterface, pname, params);
        ReportGLError("glGetProgramInterfaceiv");
    }
    else
    {
        ReportGLNullFunction("glGetProgramInterfaceiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramInterfaceiv", "",
        program, gsTrace.GetName(programInterface),
        gsTrace.GetName(pname), *params);
#endif
}

GLuint APIENTRY glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name)
{
    GLuint result;
    if (sglGetProgramResourceIndex)
    {
        result = sglGetProgramResourceIndex(program, programInterface, name);
        ReportGLError("glGetProgramResourceIndex");
    }
    else
    {
        ReportGLNullFunction("glGetProgramResourceIndex");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramResourceIndex", std::to_string(result),
        program, gsTrace.GetName(programInterface), name);
#endif
    return result;
}

void APIENTRY glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
    if (sglGetProgramResourceName)
    {
        sglGetProgramResourceName(program, programInterface, index, bufSize, length, name);
        ReportGLError("glGetProgramResourceName");
    }
    else
    {
        ReportGLNullFunction("glGetProgramResourceName");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramResourceName", "",
        program, gsTrace.GetName(programInterface), index, bufSize,
        (length != nullptr ? *length : 0), name);
#endif
}

void APIENTRY glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params)
{
    if (sglGetProgramResourceiv)
    {
        sglGetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
        ReportGLError("glGetProgramResourceiv");
    }
    else
    {
        ReportGLNullFunction("glGetProgramResourceiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramResourceiv", "",
        program, gsTrace.GetName(programInterface), index, propCount,
        propCount, gsTrace.GetEnumArray(propCount, props), bufSize,
        (length != nullptr ? *length : 0), gsTrace.GetArray(propCount, params));
#endif
}

GLint APIENTRY glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name)
{
    GLint result;
    if (sglGetProgramResourceLocation)
    {
        result = sglGetProgramResourceLocation(program, programInterface, name);
        ReportGLError("glGetProgramResourceLocation");
    }
    else
    {
        ReportGLNullFunction("glGetProgramResourceLocation");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramResourceLocation", std::to_string(result),
        program, gsTrace.GetName(programInterface), name);
#endif
    return result;
}

GLint APIENTRY glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name)
{
    GLint result;
    if (sglGetProgramResourceLocationIndex)
    {
        result = sglGetProgramResourceLocationIndex(program, programInterface, name);
        ReportGLError("glGetProgramResourceLocationIndex");
    }
    else
    {
        ReportGLNullFunction("glGetProgramResourceLocationIndex");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetProgramResourceLocationIndex", std::to_string(result),
        program, gsTrace.GetName(programInterface), name);
#endif
    return result;
}

void APIENTRY glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)
{
    if (sglShaderStorageBlockBinding)
    {
        sglShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding);
        ReportGLError("glShaderStorageBlockBinding");
    }
    else
    {
        ReportGLNullFunction("glShaderStorageBlockBinding");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glShaderStorageBlockBinding", "",
        program, storageBlockIndex, storageBlockBinding);
#endif
}

void APIENTRY glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if (sglTexBufferRange)
    {
        sglTexBufferRange(target, internalformat, buffer, offset, size);
        ReportGLError("glTexBufferRange");
    }
    else
    {
        ReportGLNullFunction("glTexBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexBufferRange", "",
        gsTrace.GetName(target), gsTrace.GetName(internalformat),
        buffer, offset, size);
#endif
}

void APIENTRY glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if (sglTexStorage2DMultisample)
    {
        sglTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
        ReportGLError("glTexStorage2DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTexStorage2DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexStorage2DMultisample", "",
        gsTrace.GetName(target), samples, gsTrace.GetName(internalformat),
        width, height, gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    if (sglTexStorage3DMultisample)
    {
        sglTexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
        ReportGLError("glTexStorage3DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTexStorage3DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTexStorage3DMultisample", "",
        gsTrace.GetName(target), samples, gsTrace.GetName(internalformat),
        width, height, depth, gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    if (sglTextureView)
    {
        sglTextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
        ReportGLError("glTextureView");
    }
    else
    {
        ReportGLNullFunction("glTextureView");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureView", "",
        texture, gsTrace.GetName(target), origtexture, gsTrace.GetName(internalformat),
        minlevel, numlevels, minlayer, numlayers);
#endif
}

void APIENTRY glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    if (sglBindVertexBuffer)
    {
        sglBindVertexBuffer(bindingindex, buffer, offset, stride);
        ReportGLError("glBindVertexBuffer");
    }
    else
    {
        ReportGLNullFunction("glBindVertexBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindVertexBuffer", "",
        bindingindex, buffer, offset, stride);
#endif
}

void APIENTRY glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    if (sglVertexAttribFormat)
    {
        sglVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
        ReportGLError("glVertexAttribFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    std::string isNormalized = (normalized != 0 ? "true" : "false");
    gsTrace.Call("glVertexAttribFormat", "",
        attribindex, size, gsTrace.GetName(type),
        gsTrace.GetBoolean(normalized), relativeoffset);
#endif
}

void APIENTRY glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if (sglVertexAttribIFormat)
    {
        sglVertexAttribIFormat(attribindex, size, type, relativeoffset);
        ReportGLError("glVertexAttribIFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribIFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribIFormat", "",
        attribindex, size, gsTrace.GetName(type), relativeoffset);
#endif
}

void APIENTRY glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if (sglVertexAttribLFormat)
    {
        sglVertexAttribLFormat(attribindex, size, type, relativeoffset);
        ReportGLError("glVertexAttribLFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribLFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribLFormat", "",
        attribindex, size, gsTrace.GetName(type), relativeoffset);
#endif
}

void APIENTRY glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    if (sglVertexAttribBinding)
    {
        sglVertexAttribBinding(attribindex, bindingindex);
        ReportGLError("glVertexAttribBinding");
    }
    else
    {
        ReportGLNullFunction("glVertexAttribBinding");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexAttribBinding", "",
        attribindex, bindingindex);
#endif
}

void APIENTRY glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
    if (sglVertexBindingDivisor)
    {
        sglVertexBindingDivisor(bindingindex, divisor);
        ReportGLError("glVertexBindingDivisor");
    }
    else
    {
        ReportGLNullFunction("glVertexBindingDivisor");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexBindingDivisor", "",
        bindingindex, divisor);
#endif
}

void APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    if (sglDebugMessageControl)
    {
        sglDebugMessageControl(source, type, severity, count, ids, enabled);
        ReportGLError("glDebugMessageControl");
    }
    else
    {
        ReportGLNullFunction("glDebugMessageControl");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDebugMessageControl", "",
        gsTrace.GetName(source), gsTrace.GetName(type), gsTrace.GetName(severity),
        count, gsTrace.GetArray(count, ids), gsTrace.GetBoolean(enabled));
#endif
}

void APIENTRY glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    if (sglDebugMessageInsert)
    {
        sglDebugMessageInsert(source, type, id, severity, length, buf);
        ReportGLError("glDebugMessageInsert");
    }
    else
    {
        ReportGLNullFunction("glDebugMessageInsert");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDebugMessageInsert", "",
        gsTrace.GetName(source), gsTrace.GetName(type), gsTrace.GetName(severity),
        length, buf);
#endif
}

void APIENTRY glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam)
{
    if (sglDebugMessageCallback)
    {
        sglDebugMessageCallback(callback, userParam);
        ReportGLError("glDebugMessageCallback");
    }
    else
    {
        ReportGLNullFunction("glDebugMessageCallback");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDebugMessageCallback", "",
        "GLDEBUGPROC", "userParam");
#endif
}

GLuint APIENTRY glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    GLuint result;
    if (sglGetDebugMessageLog)
    {
        result = sglGetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog);
        ReportGLError("glGetDebugMessageLog");
    }
    else
    {
        ReportGLNullFunction("glGetDebugMessageLog");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetDebugMessageLog", std::to_string(result),
        count, bufSize, gsTrace.GetEnumArray(count, sources),
        gsTrace.GetEnumArray(count, types), gsTrace.GetArray(count, ids),
        gsTrace.GetEnumArray(count, severities), "lengths", "messages");
#endif
    return result;
}

void APIENTRY glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar* message)
{
    if (sglPushDebugGroup)
    {
        sglPushDebugGroup(source, id, length, message);
        ReportGLError("glPushDebugGroup");
    }
    else
    {
        ReportGLNullFunction("glPushDebugGroup");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPushDebugGroup", "",
        gsTrace.GetName(source), id, length, message);
#endif
}

void APIENTRY glPopDebugGroup()
{
    if (sglPopDebugGroup)
    {
        sglPopDebugGroup();
        ReportGLError("glPopDebugGroup");
    }
    else
    {
        ReportGLNullFunction("glPopDebugGroup");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPopDebugGroup", "");
#endif
}

void APIENTRY glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label)
{
    if (sglObjectLabel)
    {
        sglObjectLabel(identifier, name, length, label);
        ReportGLError("glObjectLabel");
    }
    else
    {
        ReportGLNullFunction("glObjectLabel");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glObjectLabel", "",
        name, length, label);
#endif
}

void APIENTRY glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label)
{
    if (sglGetObjectLabel)
    {
        sglGetObjectLabel(identifier, name, bufSize, length, label);
        ReportGLError("glGetObjectLabel");
    }
    else
    {
        ReportGLNullFunction("glGetObjectLabel");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetObjectLabel", "",
        identifier, name, bufSize, *length, label);
#endif
}

void APIENTRY glObjectPtrLabel(const void* ptr, GLsizei length, const GLchar* label)
{
    if (sglObjectPtrLabel)
    {
        sglObjectPtrLabel(ptr, length, label);
        ReportGLError("glObjectPtrLabel");
    }
    else
    {
        ReportGLNullFunction("glObjectPtrLabel");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glObjectPtrLabel", "",
        "pointer", length, label);
#endif
}

void APIENTRY glGetObjectPtrLabel(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label)
{
    if (sglGetObjectPtrLabel)
    {
        sglGetObjectPtrLabel(ptr, bufSize, length, label);
        ReportGLError("glGetObjectPtrLabel");
    }
    else
    {
        ReportGLNullFunction("glGetObjectPtrLabel");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetObjectPtrLabel", "",
        "pointer", bufSize, *length, label);
#endif
}

static void Initialize_OPENGL_VERSION_4_3()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_3)
    {
        GetOpenGLFunction("glClearBufferData", sglClearBufferData);
        GetOpenGLFunction("glClearBufferSubData", sglClearBufferSubData);
        GetOpenGLFunction("glDispatchCompute", sglDispatchCompute);
        GetOpenGLFunction("glDispatchComputeIndirect", sglDispatchComputeIndirect);
        GetOpenGLFunction("glCopyImageSubData", sglCopyImageSubData);
        GetOpenGLFunction("glFramebufferParameteri", sglFramebufferParameteri);
        GetOpenGLFunction("glGetFramebufferParameteriv", sglGetFramebufferParameteriv);
        GetOpenGLFunction("glGetInternalformati64v", sglGetInternalformati64v);
        GetOpenGLFunction("glInvalidateTexSubImage", sglInvalidateTexSubImage);
        GetOpenGLFunction("glInvalidateTexImage", sglInvalidateTexImage);
        GetOpenGLFunction("glInvalidateBufferSubData", sglInvalidateBufferSubData);
        GetOpenGLFunction("glInvalidateBufferData", sglInvalidateBufferData);
        GetOpenGLFunction("glInvalidateFramebuffer", sglInvalidateFramebuffer);
        GetOpenGLFunction("glInvalidateSubFramebuffer", sglInvalidateSubFramebuffer);
        GetOpenGLFunction("glMultiDrawArraysIndirect", sglMultiDrawArraysIndirect);
        GetOpenGLFunction("glMultiDrawElementsIndirect", sglMultiDrawElementsIndirect);
        GetOpenGLFunction("glGetProgramInterfaceiv", sglGetProgramInterfaceiv);
        GetOpenGLFunction("glGetProgramResourceIndex", sglGetProgramResourceIndex);
        GetOpenGLFunction("glGetProgramResourceName", sglGetProgramResourceName);
        GetOpenGLFunction("glGetProgramResourceiv", sglGetProgramResourceiv);
        GetOpenGLFunction("glGetProgramResourceLocation", sglGetProgramResourceLocation);
        GetOpenGLFunction("glGetProgramResourceLocationIndex", sglGetProgramResourceLocationIndex);
        GetOpenGLFunction("glShaderStorageBlockBinding", sglShaderStorageBlockBinding);
        GetOpenGLFunction("glTexBufferRange", sglTexBufferRange);
        GetOpenGLFunction("glTexStorage2DMultisample", sglTexStorage2DMultisample);
        GetOpenGLFunction("glTexStorage3DMultisample", sglTexStorage3DMultisample);
        GetOpenGLFunction("glTextureView", sglTextureView);
        GetOpenGLFunction("glBindVertexBuffer", sglBindVertexBuffer);
        GetOpenGLFunction("glVertexAttribFormat", sglVertexAttribFormat);
        GetOpenGLFunction("glVertexAttribIFormat", sglVertexAttribIFormat);
        GetOpenGLFunction("glVertexAttribLFormat", sglVertexAttribLFormat);
        GetOpenGLFunction("glVertexAttribBinding", sglVertexAttribBinding);
        GetOpenGLFunction("glVertexBindingDivisor", sglVertexBindingDivisor);
        GetOpenGLFunction("glDebugMessageControl", sglDebugMessageControl);
        GetOpenGLFunction("glDebugMessageInsert", sglDebugMessageInsert);
        GetOpenGLFunction("glDebugMessageCallback", sglDebugMessageCallback);
        GetOpenGLFunction("glGetDebugMessageLog", sglGetDebugMessageLog);
        GetOpenGLFunction("glPushDebugGroup", sglPushDebugGroup);
        GetOpenGLFunction("glPopDebugGroup", sglPopDebugGroup);
        GetOpenGLFunction("glObjectLabel", sglObjectLabel);
        GetOpenGLFunction("glGetObjectLabel", sglGetObjectLabel);
        GetOpenGLFunction("glObjectPtrLabel", sglObjectPtrLabel);
        GetOpenGLFunction("glGetObjectPtrLabel", sglGetObjectPtrLabel);
    }
}

// GL_VERSION_4_4

static PFNGLBUFFERSTORAGEPROC sglBufferStorage = nullptr;
static PFNGLCLEARTEXIMAGEPROC sglClearTexImage = nullptr;
static PFNGLCLEARTEXSUBIMAGEPROC sglClearTexSubImage = nullptr;
static PFNGLBINDBUFFERSBASEPROC sglBindBuffersBase = nullptr;
static PFNGLBINDBUFFERSRANGEPROC sglBindBuffersRange = nullptr;
static PFNGLBINDTEXTURESPROC sglBindTextures = nullptr;
static PFNGLBINDSAMPLERSPROC sglBindSamplers = nullptr;
static PFNGLBINDIMAGETEXTURESPROC sglBindImageTextures = nullptr;
static PFNGLBINDVERTEXBUFFERSPROC sglBindVertexBuffers = nullptr;

void APIENTRY glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags)
{
    if (sglBufferStorage)
    {
        sglBufferStorage(target, size, data, flags);
        ReportGLError("glBufferStorage");
    }
    else
    {
        ReportGLNullFunction("glBufferStorage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBufferStorage", "",
        gsTrace.GetName(target), size, "data", flags);
#endif
}

void APIENTRY glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void* data)
{
    if (sglClearTexImage)
    {
        sglClearTexImage(texture, level, format, type, data);
        ReportGLError("glClearTexImage");
    }
    else
    {
        ReportGLNullFunction("glClearTexImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearTexImage", "",
        texture, level, gsTrace.GetName(format), gsTrace.GetName(type), "data");
#endif
}

void APIENTRY glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data)
{
    if (sglClearTexSubImage)
    {
        sglClearTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
        ReportGLError("glClearTexSubImage");
    }
    else
    {
        ReportGLNullFunction("glClearTexSubImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearTexSubImage", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth,
        gsTrace.GetName(format), gsTrace.GetName(type), "data");
#endif
}

void APIENTRY glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
    if (sglBindBuffersBase)
    {
        sglBindBuffersBase(target, first, count, buffers);
        ReportGLError("glBindBuffersBase");
    }
    else
    {
        ReportGLNullFunction("glBindBuffersBase");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindBuffersBase", "",
        gsTrace.GetName(target), first, count, gsTrace.GetArray(count, buffers + first));
#endif
}

void APIENTRY glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
    if (sglBindBuffersRange)
    {
        sglBindBuffersRange(target, first, count, buffers, offsets, sizes);
        ReportGLError("glBindBuffersRange");
    }
    else
    {
        ReportGLNullFunction("glBindBuffersRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindBuffersRange", "",
        gsTrace.GetName(target), first, count,
        gsTrace.GetArray(count, buffers + first),
        gsTrace.GetArray(count, offsets + first),
        gsTrace.GetArray(count, sizes + first));
#endif
}

void APIENTRY glBindTextures(GLuint first, GLsizei count, const GLuint* textures)
{
    if (sglBindTextures)
    {
        sglBindTextures(first, count, textures);
        ReportGLError("glBindTextures");
    }
    else
    {
        ReportGLNullFunction("glBindTextures");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindTextures", "",
        first, count, gsTrace.GetArray(count, textures + first));
#endif
}

void APIENTRY glBindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
    if (sglBindSamplers)
    {
        sglBindSamplers(first, count, samplers);
        ReportGLError("glBindSamplers");
    }
    else
    {
        ReportGLNullFunction("glBindSamplers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindSamplers", "",
        first, count, gsTrace.GetArray(count, samplers + first));
#endif
}

void APIENTRY glBindImageTextures(GLuint first, GLsizei count, const GLuint* textures)
{
    if (sglBindImageTextures)
    {
        sglBindImageTextures(first, count, textures);
        ReportGLError("glBindImageTextures");
    }
    else
    {
        ReportGLNullFunction("glBindImageTextures");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindImageTextures", "",
        first, count, gsTrace.GetArray(count, textures + first));
#endif
}

void APIENTRY glBindVertexBuffers(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides)
{
    if (sglBindVertexBuffers)
    {
        sglBindVertexBuffers(first, count, buffers, offsets, strides);
        ReportGLError("glBindVertexBuffers");
    }
    else
    {
        ReportGLNullFunction("glBindVertexBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindVertexBuffers", "",
        first, count, gsTrace.GetArray(count, buffers + first),
        gsTrace.GetArray(count, offsets + first), gsTrace.GetArray(count, strides + first));
#endif
}

static void Initialize_OPENGL_VERSION_4_4()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_4)
    {
        GetOpenGLFunction("glBufferStorage", sglBufferStorage);
        GetOpenGLFunction("glClearTexImage", sglClearTexImage);
        GetOpenGLFunction("glClearTexSubImage", sglClearTexSubImage);
        GetOpenGLFunction("glBindBuffersBase", sglBindBuffersBase);
        GetOpenGLFunction("glBindBuffersRange", sglBindBuffersRange);
        GetOpenGLFunction("glBindTextures", sglBindTextures);
        GetOpenGLFunction("glBindSamplers", sglBindSamplers);
        GetOpenGLFunction("glBindImageTextures", sglBindImageTextures);
        GetOpenGLFunction("glBindVertexBuffers", sglBindVertexBuffers);
    }
}

// GL_VERSION_4_5

static PFNGLCLIPCONTROLPROC sglClipControl = nullptr;
static PFNGLCREATETRANSFORMFEEDBACKSPROC sglCreateTransformFeedbacks = nullptr;
static PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC sglTransformFeedbackBufferBase = nullptr;
static PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC sglTransformFeedbackBufferRange = nullptr;
static PFNGLGETTRANSFORMFEEDBACKIVPROC sglGetTransformFeedbackiv = nullptr;
static PFNGLGETTRANSFORMFEEDBACKI_VPROC sglGetTransformFeedbacki_v = nullptr;
static PFNGLGETTRANSFORMFEEDBACKI64_VPROC sglGetTransformFeedbacki64_v = nullptr;
static PFNGLCREATEBUFFERSPROC sglCreateBuffers = nullptr;
static PFNGLNAMEDBUFFERSTORAGEPROC sglNamedBufferStorage = nullptr;
static PFNGLNAMEDBUFFERDATAPROC sglNamedBufferData = nullptr;
static PFNGLNAMEDBUFFERSUBDATAPROC sglNamedBufferSubData = nullptr;
static PFNGLCOPYNAMEDBUFFERSUBDATAPROC sglCopyNamedBufferSubData = nullptr;
static PFNGLCLEARNAMEDBUFFERDATAPROC sglClearNamedBufferData = nullptr;
static PFNGLCLEARNAMEDBUFFERSUBDATAPROC sglClearNamedBufferSubData = nullptr;
static PFNGLMAPNAMEDBUFFERPROC sglMapNamedBuffer = nullptr;
static PFNGLMAPNAMEDBUFFERRANGEPROC sglMapNamedBufferRange = nullptr;
static PFNGLUNMAPNAMEDBUFFERPROC sglUnmapNamedBuffer = nullptr;
static PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC sglFlushMappedNamedBufferRange = nullptr;
static PFNGLGETNAMEDBUFFERPARAMETERIVPROC sglGetNamedBufferParameteriv = nullptr;
static PFNGLGETNAMEDBUFFERPARAMETERI64VPROC sglGetNamedBufferParameteri64v = nullptr;
static PFNGLGETNAMEDBUFFERPOINTERVPROC sglGetNamedBufferPointerv = nullptr;
static PFNGLGETNAMEDBUFFERSUBDATAPROC sglGetNamedBufferSubData = nullptr;
static PFNGLCREATEFRAMEBUFFERSPROC sglCreateFramebuffers = nullptr;
static PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC sglNamedFramebufferRenderbuffer = nullptr;
static PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC sglNamedFramebufferParameteri = nullptr;
static PFNGLNAMEDFRAMEBUFFERTEXTUREPROC sglNamedFramebufferTexture = nullptr;
static PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC sglNamedFramebufferTextureLayer = nullptr;
static PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC sglNamedFramebufferDrawBuffer = nullptr;
static PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC sglNamedFramebufferDrawBuffers = nullptr;
static PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC sglNamedFramebufferReadBuffer = nullptr;
static PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC sglInvalidateNamedFramebufferData = nullptr;
static PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC sglInvalidateNamedFramebufferSubData = nullptr;
static PFNGLCLEARNAMEDFRAMEBUFFERIVPROC sglClearNamedFramebufferiv = nullptr;
static PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC sglClearNamedFramebufferuiv = nullptr;
static PFNGLCLEARNAMEDFRAMEBUFFERFVPROC sglClearNamedFramebufferfv = nullptr;
static PFNGLCLEARNAMEDFRAMEBUFFERFIPROC sglClearNamedFramebufferfi = nullptr;
static PFNGLBLITNAMEDFRAMEBUFFERPROC sglBlitNamedFramebuffer = nullptr;
static PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC sglCheckNamedFramebufferStatus = nullptr;
static PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC sglGetNamedFramebufferParameteriv = nullptr;
static PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC sglGetNamedFramebufferAttachmentParameteriv = nullptr;
static PFNGLCREATERENDERBUFFERSPROC sglCreateRenderbuffers = nullptr;
static PFNGLNAMEDRENDERBUFFERSTORAGEPROC sglNamedRenderbufferStorage = nullptr;
static PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC sglNamedRenderbufferStorageMultisample = nullptr;
static PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC sglGetNamedRenderbufferParameteriv = nullptr;
static PFNGLCREATETEXTURESPROC sglCreateTextures = nullptr;
static PFNGLTEXTUREBUFFERPROC sglTextureBuffer = nullptr;
static PFNGLTEXTUREBUFFERRANGEPROC sglTextureBufferRange = nullptr;
static PFNGLTEXTURESTORAGE1DPROC sglTextureStorage1D = nullptr;
static PFNGLTEXTURESTORAGE2DPROC sglTextureStorage2D = nullptr;
static PFNGLTEXTURESTORAGE3DPROC sglTextureStorage3D = nullptr;
static PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC sglTextureStorage2DMultisample = nullptr;
static PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC sglTextureStorage3DMultisample = nullptr;
static PFNGLTEXTURESUBIMAGE1DPROC sglTextureSubImage1D = nullptr;
static PFNGLTEXTURESUBIMAGE2DPROC sglTextureSubImage2D = nullptr;
static PFNGLTEXTURESUBIMAGE3DPROC sglTextureSubImage3D = nullptr;
static PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC sglCompressedTextureSubImage1D = nullptr;
static PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC sglCompressedTextureSubImage2D = nullptr;
static PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC sglCompressedTextureSubImage3D = nullptr;
static PFNGLCOPYTEXTURESUBIMAGE1DPROC sglCopyTextureSubImage1D = nullptr;
static PFNGLCOPYTEXTURESUBIMAGE2DPROC sglCopyTextureSubImage2D = nullptr;
static PFNGLCOPYTEXTURESUBIMAGE3DPROC sglCopyTextureSubImage3D = nullptr;
static PFNGLTEXTUREPARAMETERFPROC sglTextureParameterf = nullptr;
static PFNGLTEXTUREPARAMETERFVPROC sglTextureParameterfv = nullptr;
static PFNGLTEXTUREPARAMETERIPROC sglTextureParameteri = nullptr;
static PFNGLTEXTUREPARAMETERIIVPROC sglTextureParameterIiv = nullptr;
static PFNGLTEXTUREPARAMETERIUIVPROC sglTextureParameterIuiv = nullptr;
static PFNGLTEXTUREPARAMETERIVPROC sglTextureParameteriv = nullptr;
static PFNGLGENERATETEXTUREMIPMAPPROC sglGenerateTextureMipmap = nullptr;
static PFNGLBINDTEXTUREUNITPROC sglBindTextureUnit = nullptr;
static PFNGLGETTEXTUREIMAGEPROC sglGetTextureImage = nullptr;
static PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC sglGetCompressedTextureImage = nullptr;
static PFNGLGETTEXTURELEVELPARAMETERFVPROC sglGetTextureLevelParameterfv = nullptr;
static PFNGLGETTEXTURELEVELPARAMETERIVPROC sglGetTextureLevelParameteriv = nullptr;
static PFNGLGETTEXTUREPARAMETERFVPROC sglGetTextureParameterfv = nullptr;
static PFNGLGETTEXTUREPARAMETERIIVPROC sglGetTextureParameterIiv = nullptr;
static PFNGLGETTEXTUREPARAMETERIUIVPROC sglGetTextureParameterIuiv = nullptr;
static PFNGLGETTEXTUREPARAMETERIVPROC sglGetTextureParameteriv = nullptr;
static PFNGLCREATEVERTEXARRAYSPROC sglCreateVertexArrays = nullptr;
static PFNGLDISABLEVERTEXARRAYATTRIBPROC sglDisableVertexArrayAttrib = nullptr;
static PFNGLENABLEVERTEXARRAYATTRIBPROC sglEnableVertexArrayAttrib = nullptr;
static PFNGLVERTEXARRAYELEMENTBUFFERPROC sglVertexArrayElementBuffer = nullptr;
static PFNGLVERTEXARRAYVERTEXBUFFERPROC sglVertexArrayVertexBuffer = nullptr;
static PFNGLVERTEXARRAYVERTEXBUFFERSPROC sglVertexArrayVertexBuffers = nullptr;
static PFNGLVERTEXARRAYATTRIBBINDINGPROC sglVertexArrayAttribBinding = nullptr;
static PFNGLVERTEXARRAYATTRIBFORMATPROC sglVertexArrayAttribFormat = nullptr;
static PFNGLVERTEXARRAYATTRIBIFORMATPROC sglVertexArrayAttribIFormat = nullptr;
static PFNGLVERTEXARRAYATTRIBLFORMATPROC sglVertexArrayAttribLFormat = nullptr;
static PFNGLVERTEXARRAYBINDINGDIVISORPROC sglVertexArrayBindingDivisor = nullptr;
static PFNGLGETVERTEXARRAYIVPROC sglGetVertexArrayiv = nullptr;
static PFNGLGETVERTEXARRAYINDEXEDIVPROC sglGetVertexArrayIndexediv = nullptr;
static PFNGLGETVERTEXARRAYINDEXED64IVPROC sglGetVertexArrayIndexed64iv = nullptr;
static PFNGLCREATESAMPLERSPROC sglCreateSamplers = nullptr;
static PFNGLCREATEPROGRAMPIPELINESPROC sglCreateProgramPipelines = nullptr;
static PFNGLCREATEQUERIESPROC sglCreateQueries = nullptr;
static PFNGLGETQUERYBUFFEROBJECTI64VPROC sglGetQueryBufferObjecti64v = nullptr;
static PFNGLGETQUERYBUFFEROBJECTIVPROC sglGetQueryBufferObjectiv = nullptr;
static PFNGLGETQUERYBUFFEROBJECTUI64VPROC sglGetQueryBufferObjectui64v = nullptr;
static PFNGLGETQUERYBUFFEROBJECTUIVPROC sglGetQueryBufferObjectuiv = nullptr;
static PFNGLMEMORYBARRIERBYREGIONPROC sglMemoryBarrierByRegion = nullptr;
static PFNGLGETTEXTURESUBIMAGEPROC sglGetTextureSubImage = nullptr;
static PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC sglGetCompressedTextureSubImage = nullptr;
static PFNGLGETGRAPHICSRESETSTATUSPROC sglGetGraphicsResetStatus = nullptr;
static PFNGLGETNCOMPRESSEDTEXIMAGEPROC sglGetnCompressedTexImage = nullptr;
static PFNGLGETNTEXIMAGEPROC sglGetnTexImage = nullptr;
static PFNGLGETNUNIFORMDVPROC sglGetnUniformdv = nullptr;
static PFNGLGETNUNIFORMFVPROC sglGetnUniformfv = nullptr;
static PFNGLGETNUNIFORMIVPROC sglGetnUniformiv = nullptr;
static PFNGLGETNUNIFORMUIVPROC sglGetnUniformuiv = nullptr;
static PFNGLREADNPIXELSPROC sglReadnPixels = nullptr;
static PFNGLTEXTUREBARRIERPROC sglTextureBarrier = nullptr;

void APIENTRY glClipControl(GLenum origin, GLenum depth)
{
    if (sglClipControl)
    {
        sglClipControl(origin, depth);
        ReportGLError("glClipControl");
    }
    else
    {
        ReportGLNullFunction("glClipControl");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClipControl", "",
        gsTrace.GetName(origin), gsTrace.GetName(depth));
#endif
}

void APIENTRY glCreateTransformFeedbacks(GLsizei n, GLuint* ids)
{
    if (sglCreateTransformFeedbacks)
    {
        sglCreateTransformFeedbacks(n, ids);
        ReportGLError("glCreateTransformFeedbacks");
    }
    else
    {
        ReportGLNullFunction("glCreateTransformFeedbacks");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateTransformFeedbacks", "",
        n, gsTrace.GetArray(n, ids));
#endif
}

void APIENTRY glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer)
{
    if (sglTransformFeedbackBufferBase)
    {
        sglTransformFeedbackBufferBase(xfb, index, buffer);
        ReportGLError("glTransformFeedbackBufferBase");
    }
    else
    {
        ReportGLNullFunction("glTransformFeedbackBufferBase");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTransformFeedbackBufferBase", "",
        xfb, index, buffer);
#endif
}

void APIENTRY glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if (sglTransformFeedbackBufferRange)
    {
        sglTransformFeedbackBufferRange(xfb, index, buffer, offset, size);
        ReportGLError("glTransformFeedbackBufferRange");
    }
    else
    {
        ReportGLNullFunction("glTransformFeedbackBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTransformFeedbackBufferRange", "",
        xfb, index, buffer, offset, size);
#endif
}

void APIENTRY glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint* param)
{
    if (sglGetTransformFeedbackiv)
    {
        sglGetTransformFeedbackiv(xfb, pname, param);
        ReportGLError("glGetTransformFeedbackiv");
    }
    else
    {
        ReportGLNullFunction("glGetTransformFeedbackiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTransformFeedbackiv", "",
        xfb, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint* param)
{
    if (sglGetTransformFeedbacki_v)
    {
        sglGetTransformFeedbacki_v(xfb, pname, index, param);
        ReportGLError("glGetTransformFeedbacki_v");
    }
    else
    {
        ReportGLNullFunction("glGetTransformFeedbacki_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTransformFeedbacki_v", "",
        xfb, gsTrace.GetName(pname), index, *param);
#endif
}

void APIENTRY glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64* param)
{
    if (sglGetTransformFeedbacki64_v)
    {
        sglGetTransformFeedbacki64_v(xfb, pname, index, param);
        ReportGLError("glGetTransformFeedbacki64_v");
    }
    else
    {
        ReportGLNullFunction("glGetTransformFeedbacki64_v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTransformFeedbacki64_v", "",
        xfb, gsTrace.GetName(pname), index, *param);
#endif
}

void APIENTRY glCreateBuffers(GLsizei n, GLuint* buffers)
{
    if (sglCreateBuffers)
    {
        sglCreateBuffers(n, buffers);
        ReportGLError("glCreateBuffers");
    }
    else
    {
        ReportGLNullFunction("glCreateBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateBuffers", "",
        n, gsTrace.GetArray(n, buffers));
#endif
}

void APIENTRY glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags)
{
    if (sglNamedBufferStorage)
    {
        sglNamedBufferStorage(buffer, size, data, flags);
        ReportGLError("glNamedBufferStorage");
    }
    else
    {
        ReportGLNullFunction("glNamedBufferStorage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedBufferStorage", "",
        buffer, size, "data", flags);
#endif
}

void APIENTRY glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
{
    if (sglNamedBufferData)
    {
        sglNamedBufferData(buffer, size, data, usage);
        ReportGLError("glNamedBufferData");
    }
    else
    {
        ReportGLNullFunction("glNamedBufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedBufferData", "",
        buffer, size, "data", gsTrace.GetName(usage));
#endif
}

void APIENTRY glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data)
{
    if (sglNamedBufferSubData)
    {
        sglNamedBufferSubData(buffer, offset, size, data);
        ReportGLError("glNamedBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glNamedBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedBufferSubData", "",
        buffer, offset, size, "data");
#endif
}

void APIENTRY glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    if (sglCopyNamedBufferSubData)
    {
        sglCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
        ReportGLError("glCopyNamedBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glCopyNamedBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyNamedBufferSubData", "",
        readBuffer, writeBuffer, readOffset, writeOffset, size);
#endif
}

void APIENTRY glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
    if (sglClearNamedBufferData)
    {
        sglClearNamedBufferData(buffer, internalformat, format, type, data);
        ReportGLError("glClearNamedBufferData");
    }
    else
    {
        ReportGLNullFunction("glClearNamedBufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedBufferData", "",
        buffer, gsTrace.GetName(internalformat), gsTrace.GetName(format),
        gsTrace.GetName(type), "data");
#endif
}

void APIENTRY glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
    if (sglClearNamedBufferSubData)
    {
        sglClearNamedBufferSubData(buffer, internalformat, offset, size, format, type, data);
        ReportGLError("glClearNamedBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glClearNamedBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedBufferSubData", "",
        buffer, gsTrace.GetName(internalformat), offset, size, gsTrace.GetName(format),
        gsTrace.GetName(type), "data");
#endif
}

void* APIENTRY glMapNamedBuffer(GLuint buffer, GLenum access)
{
    void* result;
    if (sglMapNamedBuffer)
    {
        result = sglMapNamedBuffer(buffer, access);
        ReportGLError("glMapNamedBuffer");
    }
    else
    {
        ReportGLNullFunction("glMapNamedBuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMapNamedBuffer", "pointer",
        buffer, access);
#endif
    return result;
}

void* APIENTRY glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    void* result;
    if (sglMapNamedBufferRange)
    {
        result = sglMapNamedBufferRange(buffer, offset, length, access);
        ReportGLError("glMapNamedBufferRange");
    }
    else
    {
        ReportGLNullFunction("glMapNamedBufferRange");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMapNamedBufferRange", "pointer",
        buffer, offset, length, access);
#endif
    return result;
}

GLboolean APIENTRY glUnmapNamedBuffer(GLuint buffer)
{
    GLboolean result;
    if (sglUnmapNamedBuffer)
    {
        result = sglUnmapNamedBuffer(buffer);
        ReportGLError("glUnmapNamedBuffer");
    }
    else
    {
        ReportGLNullFunction("glUnmapNamedBuffer");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glUnmapNamedBuffer", gsTrace.GetBoolean(result),
        buffer);
#endif
    return result;
}

void APIENTRY glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    if (sglFlushMappedNamedBufferRange)
    {
        sglFlushMappedNamedBufferRange(buffer, offset, length);
        ReportGLError("glFlushMappedNamedBufferRange");
    }
    else
    {
        ReportGLNullFunction("glFlushMappedNamedBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glFlushMappedNamedBufferRange", "",
        buffer, offset, length);
#endif
}

void APIENTRY glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint* params)
{
    if (sglGetNamedBufferParameteriv)
    {
        sglGetNamedBufferParameteriv(buffer, pname, params);
        ReportGLError("glGetNamedBufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetNamedBufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedBufferParameteriv", "",
        buffer, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64* params)
{
    if (sglGetNamedBufferParameteri64v)
    {
        sglGetNamedBufferParameteri64v(buffer, pname, params);
        ReportGLError("glGetNamedBufferParameteri64v");
    }
    else
    {
        ReportGLNullFunction("glGetNamedBufferParameteri64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedBufferParameteri64v", "",
        buffer, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void** params)
{
    if (sglGetNamedBufferPointerv)
    {
        sglGetNamedBufferPointerv(buffer, pname, params);
        ReportGLError("glGetNamedBufferPointerv");
    }
    else
    {
        ReportGLNullFunction("glGetNamedBufferPointerv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedBufferPointerv", "",
        buffer, gsTrace.GetName(pname), *reinterpret_cast<std::uint64_t*>(*params));
#endif
}

void APIENTRY glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data)
{
    if (sglGetNamedBufferSubData)
    {
        sglGetNamedBufferSubData(buffer, offset, size, data);
        ReportGLError("glGetNamedBufferSubData");
    }
    else
    {
        ReportGLNullFunction("glGetNamedBufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedBufferSubData", "",
        buffer, offset, size, "data");
#endif
}

void APIENTRY glCreateFramebuffers(GLsizei n, GLuint* framebuffers)
{
    if (sglCreateFramebuffers)
    {
        sglCreateFramebuffers(n, framebuffers);
        ReportGLError("glCreateFramebuffers");
    }
    else
    {
        ReportGLNullFunction("glCreateFramebuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateFramebuffers", "",
        n, gsTrace.GetArray(n, framebuffers));
#endif
}

void APIENTRY glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    if (sglNamedFramebufferRenderbuffer)
    {
        sglNamedFramebufferRenderbuffer(framebuffer, attachment, renderbuffertarget, renderbuffer);
        ReportGLError("glNamedFramebufferRenderbuffer");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferRenderbuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferRenderbuffer", "",
        framebuffer, gsTrace.GetName(attachment), gsTrace.GetName(renderbuffertarget), renderbuffer);
#endif
}

void APIENTRY glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param)
{
    if (sglNamedFramebufferParameteri)
    {
        sglNamedFramebufferParameteri(framebuffer, pname, param);
        ReportGLError("glNamedFramebufferParameteri");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferRenderbuffer", "",
        framebuffer, gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
    if (sglNamedFramebufferTexture)
    {
        sglNamedFramebufferTexture(framebuffer, attachment, texture, level);
        ReportGLError("glNamedFramebufferTexture");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferTexture");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferTexture", "",
        framebuffer, gsTrace.GetName(attachment), texture, level);
#endif
}

void APIENTRY glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    if (sglNamedFramebufferTextureLayer)
    {
        sglNamedFramebufferTextureLayer(framebuffer, attachment, texture, level, layer);
        ReportGLError("glNamedFramebufferTextureLayer");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferTextureLayer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferTextureLayer", "",
        framebuffer, gsTrace.GetName(attachment), texture, level, layer);
#endif
}

void APIENTRY glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf)
{
    if (sglNamedFramebufferDrawBuffer)
    {
        sglNamedFramebufferDrawBuffer(framebuffer, buf);
        ReportGLError("glNamedFramebufferDrawBuffer");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferDrawBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferDrawBuffer", "",
        framebuffer, gsTrace.GetName(buf));
#endif
}

void APIENTRY glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum* bufs)
{
    if (sglNamedFramebufferDrawBuffers)
    {
        sglNamedFramebufferDrawBuffers(framebuffer, n, bufs);
        ReportGLError("glNamedFramebufferDrawBuffers");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferDrawBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferDrawBuffers", "",
        framebuffer, n, gsTrace.GetEnumArray(n, bufs));
#endif
}

void APIENTRY glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src)
{
    if (sglNamedFramebufferReadBuffer)
    {
        sglNamedFramebufferReadBuffer(framebuffer, src);
        ReportGLError("glNamedFramebufferReadBuffer");
    }
    else
    {
        ReportGLNullFunction("glNamedFramebufferReadBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedFramebufferReadBuffer", "",
        framebuffer, gsTrace.GetName(src));
#endif
}

void APIENTRY glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments)
{
    if (sglInvalidateNamedFramebufferData)
    {
        sglInvalidateNamedFramebufferData(framebuffer, numAttachments, attachments);
        ReportGLError("glInvalidateNamedFramebufferData");
    }
    else
    {
        ReportGLNullFunction("glInvalidateNamedFramebufferData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateNamedFramebufferData", "",
        framebuffer, numAttachments,
        gsTrace.GetEnumArray(numAttachments, attachments));
#endif
}

void APIENTRY glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglInvalidateNamedFramebufferSubData)
    {
        sglInvalidateNamedFramebufferSubData(framebuffer, numAttachments, attachments, x, y, width, height);
        ReportGLError("glInvalidateNamedFramebufferSubData");
    }
    else
    {
        ReportGLNullFunction("glInvalidateNamedFramebufferSubData");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glInvalidateNamedFramebufferSubData", "",
        framebuffer, numAttachments, gsTrace.GetEnumArray(numAttachments, attachments),
        x, y, width, height);
#endif
}

void APIENTRY glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value)
{
    if (sglClearNamedFramebufferiv)
    {
        sglClearNamedFramebufferiv(framebuffer, buffer, drawbuffer, value);
        ReportGLError("glClearNamedFramebufferiv");
    }
    else
    {
        ReportGLNullFunction("glClearNamedFramebufferiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedFramebufferiv", "",
        framebuffer, gsTrace.GetName(buffer), drawbuffer, "value");
#endif
}

void APIENTRY glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    if (sglClearNamedFramebufferuiv)
    {
        sglClearNamedFramebufferuiv(framebuffer, buffer, drawbuffer, value);
        ReportGLError("glClearNamedFramebufferuiv");
    }
    else
    {
        ReportGLNullFunction("glClearNamedFramebufferuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedFramebufferuiv", "",
        framebuffer, gsTrace.GetName(buffer), drawbuffer, "value");
#endif
}

void APIENTRY glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    if (sglClearNamedFramebufferfv)
    {
        sglClearNamedFramebufferfv(framebuffer, buffer, drawbuffer, value);
        ReportGLError("glClearNamedFramebufferfv");
    }
    else
    {
        ReportGLNullFunction("glClearNamedFramebufferfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedFramebufferfv", "",
        framebuffer, gsTrace.GetName(buffer), drawbuffer, "value");
#endif
}

void APIENTRY glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat depth, GLint stencil)
{
    if (sglClearNamedFramebufferfi)
    {
        sglClearNamedFramebufferfi(framebuffer, buffer, drawbuffer, depth, stencil);
        ReportGLError("glClearNamedFramebufferfi");
    }
    else
    {
        ReportGLNullFunction("glClearNamedFramebufferfi");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glClearNamedFramebufferfi", "",
        framebuffer, gsTrace.GetName(buffer), drawbuffer, depth, stencil);
#endif
}

void APIENTRY glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    if (sglBlitNamedFramebuffer)
    {
        sglBlitNamedFramebuffer(readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
        ReportGLError("glBlitNamedFramebuffer");
    }
    else
    {
        ReportGLNullFunction("glBlitNamedFramebuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBlitNamedFramebuffer", "",
        readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1,
        dstX0, dstY0, dstX1, dstY1, mask, gsTrace.GetName(filter));
#endif
}

GLenum APIENTRY glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target)
{
    GLenum result;
    if (sglCheckNamedFramebufferStatus)
    {
        result = sglCheckNamedFramebufferStatus(framebuffer, target);
        ReportGLError("glCheckNamedFramebufferStatus");
    }
    else
    {
        ReportGLNullFunction("glCheckNamedFramebufferStatus");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCheckNamedFramebufferStatus", "",
        framebuffer, gsTrace.GetName(target));
#endif
    return result;
}

void APIENTRY glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint* param)
{
    if (sglGetNamedFramebufferParameteriv)
    {
        sglGetNamedFramebufferParameteriv(framebuffer, pname, param);
        ReportGLError("glGetNamedFramebufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetNamedFramebufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedFramebufferParameteriv", "",
        framebuffer, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params)
{
    if (sglGetNamedFramebufferAttachmentParameteriv)
    {
        sglGetNamedFramebufferAttachmentParameteriv(framebuffer, attachment, pname, params);
        ReportGLError("glGetNamedFramebufferAttachmentParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetNamedFramebufferAttachmentParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedFramebufferAttachmentParameteriv", "",
        framebuffer, gsTrace.GetName(attachment), gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glCreateRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    if (sglCreateRenderbuffers)
    {
        sglCreateRenderbuffers(n, renderbuffers);
        ReportGLError("glCreateRenderbuffers");
    }
    else
    {
        ReportGLNullFunction("glCreateRenderbuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateRenderbuffers", "",
        n, gsTrace.GetArray(n, renderbuffers));
#endif
}

void APIENTRY glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglNamedRenderbufferStorage)
    {
        sglNamedRenderbufferStorage(renderbuffer, internalformat, width, height);
        ReportGLError("glNamedRenderbufferStorage");
    }
    else
    {
        ReportGLNullFunction("glNamedRenderbufferStorage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedRenderbufferStorage", "",
        renderbuffer, gsTrace.GetName(internalformat), width, height);
#endif
}

void APIENTRY glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglNamedRenderbufferStorageMultisample)
    {
        sglNamedRenderbufferStorageMultisample(renderbuffer, samples, internalformat, width, height);
        ReportGLError("glNamedRenderbufferStorageMultisample");
    }
    else
    {
        ReportGLNullFunction("glNamedRenderbufferStorageMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glNamedRenderbufferStorageMultisample", "",
        renderbuffer, samples, gsTrace.GetName(internalformat), width, height);
#endif
}

void APIENTRY glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint* params)
{
    if (sglGetNamedRenderbufferParameteriv)
    {
        sglGetNamedRenderbufferParameteriv(renderbuffer, pname, params);
        ReportGLError("glGetNamedRenderbufferParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetNamedRenderbufferParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetNamedRenderbufferParameteriv", "",
        renderbuffer, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glCreateTextures(GLenum target, GLsizei n, GLuint* textures)
{
    if (sglCreateTextures)
    {
        sglCreateTextures(target, n, textures);
        ReportGLError("glCreateTextures");
    }
    else
    {
        ReportGLNullFunction("glCreateTextures");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateTextures", "",
        gsTrace.GetName(target), n, gsTrace.GetArray(n, textures));
#endif
}

void APIENTRY glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer)
{
    if (sglTextureBuffer)
    {
        sglTextureBuffer(texture, internalformat, buffer);
        ReportGLError("glTextureBuffer");
    }
    else
    {
        ReportGLNullFunction("glTextureBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureBuffer", "",
        texture, gsTrace.GetName(internalformat), buffer);
#endif
}

void APIENTRY glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    if (sglTextureBufferRange)
    {
        sglTextureBufferRange(texture, internalformat, buffer, offset, size);
        ReportGLError("glTextureBufferRange");
    }
    else
    {
        ReportGLNullFunction("glTextureBufferRange");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureBufferRange", "",
        texture, gsTrace.GetName(internalformat), buffer, offset, size);
#endif
}

void APIENTRY glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    if (sglTextureStorage1D)
    {
        sglTextureStorage1D(texture, levels, internalformat, width);
        ReportGLError("glTextureStorage1D");
    }
    else
    {
        ReportGLNullFunction("glTextureStorage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureStorage1D", "",
        texture, levels, gsTrace.GetName(internalformat), width);
#endif
}

void APIENTRY glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    if (sglTextureStorage2D)
    {
        sglTextureStorage2D(texture, levels, internalformat, width, height);
        ReportGLError("glTextureStorage2D");
    }
    else
    {
        ReportGLNullFunction("glTextureStorage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureStorage2D", "",
        texture, levels, gsTrace.GetName(internalformat), width, height);
#endif
}

void APIENTRY glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    if (sglTextureStorage3D)
    {
        sglTextureStorage3D(texture, levels, internalformat, width, height, depth);
        ReportGLError("glTextureStorage3D");
    }
    else
    {
        ReportGLNullFunction("glTextureStorage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureStorage3D", "",
        texture, levels, gsTrace.GetName(internalformat), width, height, depth);
#endif
}

void APIENTRY glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    if (sglTextureStorage2DMultisample)
    {
        sglTextureStorage2DMultisample(texture, samples, internalformat, width, height, fixedsamplelocations);
        ReportGLError("glTextureStorage2DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTextureStorage2DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureStorage2DMultisample", "",
        texture, samples, gsTrace.GetName(internalformat), width, height,
        gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    if (sglTextureStorage3DMultisample)
    {
        sglTextureStorage3DMultisample(texture, samples, internalformat, width, height, depth, fixedsamplelocations);
        ReportGLError("glTextureStorage3DMultisample");
    }
    else
    {
        ReportGLNullFunction("glTextureStorage3DMultisample");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureStoragew3DMultisample", "",
        texture, samples, gsTrace.GetName(internalformat), width, height, depth,
        gsTrace.GetBoolean(fixedsamplelocations));
#endif
}

void APIENTRY glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
    if (sglTextureSubImage1D)
    {
        sglTextureSubImage1D(texture, level, xoffset, width, format, type, pixels);
        ReportGLError("glTextureSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glTextureSubImage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureSubImage1D", "",
        texture, level, xoffset, width, gsTrace.GetName(format),
        gsTrace.GetName(type), "pixels");
#endif
}

void APIENTRY glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    if (sglTextureSubImage2D)
    {
        sglTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, type, pixels);
        ReportGLError("glTextureSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glTextureSubImage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureSubImage2D", "",
        texture, level, xoffset, yoffset, width, height,
        gsTrace.GetName(format), gsTrace.GetName(type), "pixels");
#endif
}

void APIENTRY glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
    if (sglTextureSubImage3D)
    {
        sglTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
        ReportGLError("glTextureSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glTextureSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureSubImage3D", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth,
        gsTrace.GetName(format), gsTrace.GetName(type), "pixels");
#endif
}

void APIENTRY glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTextureSubImage1D)
    {
        sglCompressedTextureSubImage1D(texture, level, xoffset, width, format, imageSize, data);
        ReportGLError("glCompressedTextureSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTextureSubImage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTextureSubImage1D", "",
        texture, level, xoffset, width, gsTrace.GetName(format),
        imageSize, "data");
#endif
}

void APIENTRY glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTextureSubImage2D)
    {
        sglCompressedTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, imageSize, data);
        ReportGLError("glCompressedTextureSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTextureSubImage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTextureSubImage2D", "",
        texture, level, xoffset, yoffset, width, height, gsTrace.GetName(format),
        imageSize, "data");
#endif
}

void APIENTRY glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
    if (sglCompressedTextureSubImage3D)
    {
        sglCompressedTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
        ReportGLError("glCompressedTextureSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glCompressedTextureSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCompressedTextureSubImage3D", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth,
        gsTrace.GetName(format), imageSize, "data");
#endif
}

void APIENTRY glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    if (sglCopyTextureSubImage1D)
    {
        sglCopyTextureSubImage1D(texture, level, xoffset, x, y, width);
        ReportGLError("glCopyTextureSubImage1D");
    }
    else
    {
        ReportGLNullFunction("glCopyTextureSubImage1D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyTextureSubImage1D", "",
        texture, level, xoffset, x, y, width);
#endif
}

void APIENTRY glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglCopyTextureSubImage2D)
    {
        sglCopyTextureSubImage2D(texture, level, xoffset, yoffset, x, y, width, height);
        ReportGLError("glCopyTextureSubImage2D");
    }
    else
    {
        ReportGLNullFunction("glCopyTextureSubImage2D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyTextureSubImage2D", "",
        texture, level, xoffset, yoffset, x, y, width, height);
#endif
}

void APIENTRY glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (sglCopyTextureSubImage3D)
    {
        sglCopyTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, x, y, width, height);
        ReportGLError("glCopyTextureSubImage3D");
    }
    else
    {
        ReportGLNullFunction("glCopyTextureSubImage3D");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCopyTextureSubImage3D", "",
        texture, level, xoffset, yoffset, zoffset, x, y, width, height);
#endif
}

void APIENTRY glTextureParameterf(GLuint texture, GLenum pname, GLfloat param)
{
    if (sglTextureParameterf)
    {
        sglTextureParameterf(texture, pname, param);
        ReportGLError("glTextureParameterf");
    }
    else
    {
        ReportGLNullFunction("glTextureParameterf");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameterf", "",
        texture, gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat* param)
{
    if (sglTextureParameterfv)
    {
        sglTextureParameterfv(texture, pname, param);
        ReportGLError("glTextureParameterfv");
    }
    else
    {
        ReportGLNullFunction("glTextureParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameterfv", "",
        texture, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glTextureParameteri(GLuint texture, GLenum pname, GLint param)
{
    if (sglTextureParameteri)
    {
        sglTextureParameteri(texture, pname, param);
        ReportGLError("glTextureParameteri");
    }
    else
    {
        ReportGLNullFunction("glTextureParameteri");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameteri", "",
        texture, gsTrace.GetName(pname), param);
#endif
}

void APIENTRY glTextureParameterIiv(GLuint texture, GLenum pname, const GLint* params)
{
    if (sglTextureParameterIiv)
    {
        sglTextureParameterIiv(texture, pname, params);
        ReportGLError("glTextureParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glTextureParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameterIiv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint* params)
{
    if (sglTextureParameterIuiv)
    {
        sglTextureParameterIuiv(texture, pname, params);
        ReportGLError("glTextureParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glTextureParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameterIuiv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glTextureParameteriv(GLuint texture, GLenum pname, const GLint* param)
{
    if (sglTextureParameteriv)
    {
        sglTextureParameteriv(texture, pname, param);
        ReportGLError("glTextureParameteriv");
    }
    else
    {
        ReportGLNullFunction("glTextureParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureParameteriv", "",
        texture, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGenerateTextureMipmap(GLuint texture)
{
    if (sglGenerateTextureMipmap)
    {
        sglGenerateTextureMipmap(texture);
        ReportGLError("glGenerateTextureMipmap");
    }
    else
    {
        ReportGLNullFunction("glGenerateTextureMipmap");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGenerateTextureMipmap", "",
        texture);
#endif
}

void APIENTRY glBindTextureUnit(GLuint unit, GLuint texture)
{
    if (sglBindTextureUnit)
    {
        sglBindTextureUnit(unit, texture);
        ReportGLError("glBindTextureUnit");
    }
    else
    {
        ReportGLNullFunction("glBindTextureUnit");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glBindTextureUnit", "",
        unit, texture);
#endif
}

void APIENTRY glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
    if (sglGetTextureImage)
    {
        sglGetTextureImage(texture, level, format, type, bufSize, pixels);
        ReportGLError("glGetTextureImage");
    }
    else
    {
        ReportGLNullFunction("glGetTextureImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureImage", "",
        texture, level, gsTrace.GetName(format), gsTrace.GetName(type),
        bufSize, "pixels");
#endif
}

void APIENTRY glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void* pixels)
{
    if (sglGetCompressedTextureImage)
    {
        sglGetCompressedTextureImage(texture, level, bufSize, pixels);
        ReportGLError("glGetCompressedTextureImage");
    }
    else
    {
        ReportGLNullFunction("glGetCompressedTextureImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetCompressedTextureImage", "",
        texture, level, bufSize, "oixels");
#endif
}

void APIENTRY glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat* params)
{
    if (sglGetTextureLevelParameterfv)
    {
        sglGetTextureLevelParameterfv(texture, level, pname, params);
        ReportGLError("glGetTextureLevelParameterfv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureLevelParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureLevelParameterfv", "",
        texture, level, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint* params)
{
    if (sglGetTextureLevelParameteriv)
    {
        sglGetTextureLevelParameteriv(texture, level, pname, params);
        ReportGLError("glGetTextureLevelParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureLevelParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureLevelParameteriv", "",
        texture, level, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat* params)
{
    if (sglGetTextureParameterfv)
    {
        sglGetTextureParameterfv(texture, pname, params);
        ReportGLError("glGetTextureParameterfv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureParameterfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureLevelParameterfv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint* params)
{
    if (sglGetTextureParameterIiv)
    {
        sglGetTextureParameterIiv(texture, pname, params);
        ReportGLError("glGetTextureParameterIiv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureParameterIiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureParameterIiv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint* params)
{
    if (sglGetTextureParameterIuiv)
    {
        sglGetTextureParameterIuiv(texture, pname, params);
        ReportGLError("glGetTextureParameterIuiv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureParameterIuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureParameterIuiv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glGetTextureParameteriv(GLuint texture, GLenum pname, GLint* params)
{
    if (sglGetTextureParameteriv)
    {
        sglGetTextureParameteriv(texture, pname, params);
        ReportGLError("glGetTextureParameteriv");
    }
    else
    {
        ReportGLNullFunction("glGetTextureParameteriv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureParameteriv", "",
        texture, gsTrace.GetName(pname), *params);
#endif
}

void APIENTRY glCreateVertexArrays(GLsizei n, GLuint* arrays)
{
    if (sglCreateVertexArrays)
    {
        sglCreateVertexArrays(n, arrays);
        ReportGLError("glCreateVertexArrays");
    }
    else
    {
        ReportGLNullFunction("glCreateVertexArrays");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateVertexArrays", "",
        n, gsTrace.GetArray(n, arrays));
#endif
}

void APIENTRY glDisableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
    if (sglDisableVertexArrayAttrib)
    {
        sglDisableVertexArrayAttrib(vaobj, index);
        ReportGLError("glDisableVertexArrayAttrib");
    }
    else
    {
        ReportGLNullFunction("glDisableVertexArrayAttrib");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glDisableVertexArrayAttrib", "",
        vaobj, index);
#endif
}

void APIENTRY glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
    if (sglEnableVertexArrayAttrib)
    {
        sglEnableVertexArrayAttrib(vaobj, index);
        ReportGLError("glEnableVertexArrayAttrib");
    }
    else
    {
        ReportGLNullFunction("glEnableVertexArrayAttrib");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glEnableVertexArrayAttrib", "",
        vaobj, index);
#endif
}

void APIENTRY glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
{
    if (sglVertexArrayElementBuffer)
    {
        sglVertexArrayElementBuffer(vaobj, buffer);
        ReportGLError("glVertexArrayElementBuffer");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayElementBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayElementBuffer", "",
        vaobj, buffer);
#endif
}

void APIENTRY glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    if (sglVertexArrayVertexBuffer)
    {
        sglVertexArrayVertexBuffer(vaobj, bindingindex, buffer, offset, stride);
        ReportGLError("glVertexArrayVertexBuffer");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayVertexBuffer");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayVertexBuffer", "",
        vaobj, bindingindex, buffer, offset, stride);
#endif
}

void APIENTRY glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides)
{
    if (sglVertexArrayVertexBuffers)
    {
        sglVertexArrayVertexBuffers(vaobj, first, count, buffers, offsets, strides);
        ReportGLError("glVertexArrayVertexBuffers");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayVertexBuffers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayVertexBuffers", "",
        vaobj, first, count,
        gsTrace.GetArray(count, buffers + first),
        gsTrace.GetArray(count, offsets + first),
        gsTrace.GetArray(count, strides + first));
#endif
}

void APIENTRY glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
    if (sglVertexArrayAttribBinding)
    {
        sglVertexArrayAttribBinding(vaobj, attribindex, bindingindex);
        ReportGLError("glVertexArrayAttribBinding");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayAttribBinding");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayAttribBinding", "",
        vaobj, attribindex, bindingindex);
#endif
}

void APIENTRY glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    if (sglVertexArrayAttribFormat)
    {
        sglVertexArrayAttribFormat(vaobj, attribindex, size, type, normalized, relativeoffset);
        ReportGLError("glVertexArrayAttribFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayAttribFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayAttribFormat", "",
        vaobj, attribindex, size, gsTrace.GetName(type),
        gsTrace.GetBoolean(normalized), relativeoffset);
#endif
}

void APIENTRY glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if (sglVertexArrayAttribIFormat)
    {
        sglVertexArrayAttribIFormat(vaobj, attribindex, size, type, relativeoffset);
        ReportGLError("glVertexArrayAttribIFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayAttribIFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayAttribIFormat", "",
        vaobj, attribindex, size, gsTrace.GetName(type), relativeoffset);
#endif
}

void APIENTRY glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    if (sglVertexArrayAttribLFormat)
    {
        sglVertexArrayAttribLFormat(vaobj, attribindex, size, type, relativeoffset);
        ReportGLError("glVertexArrayAttribLFormat");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayAttribLFormat");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayAttribLFormat", "",
        vaobj, attribindex, size, gsTrace.GetName(type), relativeoffset);
#endif
}

void APIENTRY glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
    if (sglVertexArrayBindingDivisor)
    {
        sglVertexArrayBindingDivisor(vaobj, bindingindex, divisor);
        ReportGLError("glVertexArrayBindingDivisor");
    }
    else
    {
        ReportGLNullFunction("glVertexArrayBindingDivisor");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glVertexArrayBindingDivisor", "",
        vaobj, bindingindex, divisor);
#endif
}

void APIENTRY glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint* param)
{
    if (sglGetVertexArrayiv)
    {
        sglGetVertexArrayiv(vaobj, pname, param);
        ReportGLError("glGetVertexArrayiv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexArrayiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexArrayiv", "",
        vaobj, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint* param)
{
    if (sglGetVertexArrayIndexediv)
    {
        sglGetVertexArrayIndexediv(vaobj, index, pname, param);
        ReportGLError("glGetVertexArrayIndexediv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexArrayIndexediv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexArrayIndexediv", "",
        vaobj, index, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64* param)
{
    if (sglGetVertexArrayIndexed64iv)
    {
        sglGetVertexArrayIndexed64iv(vaobj, index, pname, param);
        ReportGLError("glGetVertexArrayIndexed64iv");
    }
    else
    {
        ReportGLNullFunction("glGetVertexArrayIndexed64iv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetVertexArrayIndexed64iv", "",
        vaobj, index, gsTrace.GetName(pname), *param);
#endif
}

void APIENTRY glCreateSamplers(GLsizei n, GLuint* samplers)
{
    if (sglCreateSamplers)
    {
        sglCreateSamplers(n, samplers);
        ReportGLError("glCreateSamplers");
    }
    else
    {
        ReportGLNullFunction("glCreateSamplers");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateSamplers", "",
        n, gsTrace.GetArray(n, samplers));
#endif
}

void APIENTRY glCreateProgramPipelines(GLsizei n, GLuint* pipelines)
{
    if (sglCreateProgramPipelines)
    {
        sglCreateProgramPipelines(n, pipelines);
        ReportGLError("glCreateProgramPipelines");
    }
    else
    {
        ReportGLNullFunction("glCreateProgramPipelines");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateProgramPipelines", "",
        n, gsTrace.GetArray(n, pipelines));
#endif
}

void APIENTRY glCreateQueries(GLenum target, GLsizei n, GLuint* ids)
{
    if (sglCreateQueries)
    {
        sglCreateQueries(target, n, ids);
        ReportGLError("glCreateQueries");
    }
    else
    {
        ReportGLNullFunction("glCreateQueries");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glCreateQueries", "",
        gsTrace.GetName(target), n, gsTrace.GetArray(n, ids));
#endif
}

void APIENTRY glGetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    if (sglGetQueryBufferObjecti64v)
    {
        sglGetQueryBufferObjecti64v(id, buffer, pname, offset);
        ReportGLError("glGetQueryBufferObjecti64v");
    }
    else
    {
        ReportGLNullFunction("glGetQueryBufferObjecti64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryBufferObjecti64v", "",
        id, buffer, gsTrace.GetName(pname), offset);
#endif
}

void APIENTRY glGetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    if (sglGetQueryBufferObjectiv)
    {
        sglGetQueryBufferObjectiv(id, buffer, pname, offset);
        ReportGLError("glGetQueryBufferObjectiv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryBufferObjectiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryBufferObjectiv", "",
        id, buffer, gsTrace.GetName(pname), offset);
#endif
}

void APIENTRY glGetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    if (sglGetQueryBufferObjectui64v)
    {
        sglGetQueryBufferObjectui64v(id, buffer, pname, offset);
        ReportGLError("glGetQueryBufferObjectui64v");
    }
    else
    {
        ReportGLNullFunction("glGetQueryBufferObjectui64v");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryBufferObjectui64v", "",
        id, buffer, gsTrace.GetName(pname), offset);
#endif
}

void APIENTRY glGetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    if (sglGetQueryBufferObjectuiv)
    {
        sglGetQueryBufferObjectuiv(id, buffer, pname, offset);
        ReportGLError("glGetQueryBufferObjectuiv");
    }
    else
    {
        ReportGLNullFunction("glGetQueryBufferObjectuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetQueryBufferObjectuiv", "",
        id, buffer, gsTrace.GetName(pname), offset);
#endif
}

void APIENTRY glMemoryBarrierByRegion(GLbitfield barriers)
{
    if (sglMemoryBarrierByRegion)
    {
        sglMemoryBarrierByRegion(barriers);
        ReportGLError("glMemoryBarrierByRegion");
    }
    else
    {
        ReportGLNullFunction("glMemoryBarrierByRegion");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMemoryBarrierByRegion", "",
        barriers);
#endif
}

void APIENTRY glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
    if (sglGetTextureSubImage)
    {
        sglGetTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels);
        ReportGLError("glGetTextureSubImage");
    }
    else
    {
        ReportGLNullFunction("glGetTextureSubImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetTextureSubImage", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth,
        gsTrace.GetName(format), gsTrace.GetName(type), bufSize, "pixels");
#endif
}

void APIENTRY glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels)
{
    if (sglGetCompressedTextureSubImage)
    {
        sglGetCompressedTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels);
        ReportGLError("glGetCompressedTextureSubImage");
    }
    else
    {
        ReportGLNullFunction("glGetCompressedTextureSubImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetCompressedTextureSubImage", "",
        texture, level, xoffset, yoffset, zoffset, width, height, depth,
        bufSize, "pixels");
#endif
}

GLenum APIENTRY glGetGraphicsResetStatus()
{
    GLenum result;
    if (sglGetGraphicsResetStatus)
    {
        result = sglGetGraphicsResetStatus();
        ReportGLError("glGetGraphicsResetStatus");
    }
    else
    {
        ReportGLNullFunction("glGetGraphicsResetStatus");
        result = 0;
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetGraphicsResetStatus", gsTrace.GetName(result));
#endif
    return result;
}

void APIENTRY glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void* pixels)
{
    if (sglGetnCompressedTexImage)
    {
        sglGetnCompressedTexImage(target, lod, bufSize, pixels);
        ReportGLError("glGetnCompressedTexImage");
    }
    else
    {
        ReportGLNullFunction("glGetnCompressedTexImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnCompressedTexImage", "",
        gsTrace.GetName(target), lod, bufSize, "pixels");
#endif
}

void APIENTRY glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
    if (sglGetnTexImage)
    {
        sglGetnTexImage(target, level, format, type, bufSize, pixels);
        ReportGLError("glGetnTexImage");
    }
    else
    {
        ReportGLNullFunction("glGetnTexImage");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnTexImage", "",
        gsTrace.GetName(target), level, gsTrace.GetName(format),
        gsTrace.GetName(type), bufSize, "pixels");
#endif
}

void APIENTRY glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble* params)
{
    if (sglGetnUniformdv)
    {
        sglGetnUniformdv(program, location, bufSize, params);
        ReportGLError("glGetnUniformdv");
    }
    else
    {
        ReportGLNullFunction("glGetnUniformdv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnUniformdv", "",
        program, location, bufSize, *params);
#endif
}

void APIENTRY glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
    if (sglGetnUniformfv)
    {
        sglGetnUniformfv(program, location, bufSize, params);
        ReportGLError("glGetnUniformfv");
    }
    else
    {
        ReportGLNullFunction("glGetnUniformfv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnUniformfv", "",
        program, location, bufSize, *params);
#endif
}

void APIENTRY glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
    if (sglGetnUniformiv)
    {
        sglGetnUniformiv(program, location, bufSize, params);
        ReportGLError("glGetnUniformiv");
    }
    else
    {
        ReportGLNullFunction("glGetnUniformiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnUniformiv", "",
        program, location, bufSize, *params);
#endif
}

void APIENTRY glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint* params)
{
    if (sglGetnUniformuiv)
    {
        sglGetnUniformuiv(program, location, bufSize, params);
        ReportGLError("glGetnUniformuiv");
    }
    else
    {
        ReportGLNullFunction("glGetnUniformuiv");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glGetnUniformuiv", "",
        program, location, bufSize, *params);
#endif
}

void APIENTRY glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
    if (sglReadnPixels)
    {
        sglReadnPixels(x, y, width, height, format, type, bufSize, data);
        ReportGLError("glReadnPixels");
    }
    else
    {
        ReportGLNullFunction("glReadnPixels");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glReadnPixels", "",
        x, y, width, height, gsTrace.GetName(format),
        gsTrace.GetName(type), bufSize, "data");
#endif
}

void APIENTRY glTextureBarrier()
{
    if (sglTextureBarrier)
    {
        sglTextureBarrier();
        ReportGLError("glTextureBarrier");
    }
    else
    {
        ReportGLNullFunction("glTextureBarrier");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glTextureBarrier", "");
#endif
}

static void Initialize_OPENGL_VERSION_4_5()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_5)
    {
        GetOpenGLFunction("glClipControl", sglClipControl);
        GetOpenGLFunction("glCreateTransformFeedbacks", sglCreateTransformFeedbacks);
        GetOpenGLFunction("glTransformFeedbackBufferBase", sglTransformFeedbackBufferBase);
        GetOpenGLFunction("glTransformFeedbackBufferRange", sglTransformFeedbackBufferRange);
        GetOpenGLFunction("glGetTransformFeedbackiv", sglGetTransformFeedbackiv);
        GetOpenGLFunction("glGetTransformFeedbacki_v", sglGetTransformFeedbacki_v);
        GetOpenGLFunction("glGetTransformFeedbacki64_v", sglGetTransformFeedbacki64_v);
        GetOpenGLFunction("glCreateBuffers", sglCreateBuffers);
        GetOpenGLFunction("glNamedBufferStorage", sglNamedBufferStorage);
        GetOpenGLFunction("glNamedBufferData", sglNamedBufferData);
        GetOpenGLFunction("glNamedBufferSubData", sglNamedBufferSubData);
        GetOpenGLFunction("glCopyNamedBufferSubData", sglCopyNamedBufferSubData);
        GetOpenGLFunction("glClearNamedBufferData", sglClearNamedBufferData);
        GetOpenGLFunction("glClearNamedBufferSubData", sglClearNamedBufferSubData);
        GetOpenGLFunction("glMapNamedBuffer", sglMapNamedBuffer);
        GetOpenGLFunction("glMapNamedBufferRange", sglMapNamedBufferRange);
        GetOpenGLFunction("glUnmapNamedBuffer", sglUnmapNamedBuffer);
        GetOpenGLFunction("glFlushMappedNamedBufferRange", sglFlushMappedNamedBufferRange);
        GetOpenGLFunction("glGetNamedBufferParameteriv", sglGetNamedBufferParameteriv);
        GetOpenGLFunction("glGetNamedBufferParameteri64v", sglGetNamedBufferParameteri64v);
        GetOpenGLFunction("glGetNamedBufferPointerv", sglGetNamedBufferPointerv);
        GetOpenGLFunction("glGetNamedBufferSubData", sglGetNamedBufferSubData);
        GetOpenGLFunction("glCreateFramebuffers", sglCreateFramebuffers);
        GetOpenGLFunction("glNamedFramebufferRenderbuffer", sglNamedFramebufferRenderbuffer);
        GetOpenGLFunction("glNamedFramebufferParameteri", sglNamedFramebufferParameteri);
        GetOpenGLFunction("glNamedFramebufferTexture", sglNamedFramebufferTexture);
        GetOpenGLFunction("glNamedFramebufferTextureLayer", sglNamedFramebufferTextureLayer);
        GetOpenGLFunction("glNamedFramebufferDrawBuffer", sglNamedFramebufferDrawBuffer);
        GetOpenGLFunction("glNamedFramebufferDrawBuffers", sglNamedFramebufferDrawBuffers);
        GetOpenGLFunction("glNamedFramebufferReadBuffer", sglNamedFramebufferReadBuffer);
        GetOpenGLFunction("glInvalidateNamedFramebufferData", sglInvalidateNamedFramebufferData);
        GetOpenGLFunction("glInvalidateNamedFramebufferSubData", sglInvalidateNamedFramebufferSubData);
        GetOpenGLFunction("glClearNamedFramebufferiv", sglClearNamedFramebufferiv);
        GetOpenGLFunction("glClearNamedFramebufferuiv", sglClearNamedFramebufferuiv);
        GetOpenGLFunction("glClearNamedFramebufferfv", sglClearNamedFramebufferfv);
        GetOpenGLFunction("glClearNamedFramebufferfi", sglClearNamedFramebufferfi);
        GetOpenGLFunction("glBlitNamedFramebuffer", sglBlitNamedFramebuffer);
        GetOpenGLFunction("glCheckNamedFramebufferStatus", sglCheckNamedFramebufferStatus);
        GetOpenGLFunction("glGetNamedFramebufferParameteriv", sglGetNamedFramebufferParameteriv);
        GetOpenGLFunction("glGetNamedFramebufferAttachmentParameteriv", sglGetNamedFramebufferAttachmentParameteriv);
        GetOpenGLFunction("glCreateRenderbuffers", sglCreateRenderbuffers);
        GetOpenGLFunction("glNamedRenderbufferStorage", sglNamedRenderbufferStorage);
        GetOpenGLFunction("glNamedRenderbufferStorageMultisample", sglNamedRenderbufferStorageMultisample);
        GetOpenGLFunction("glGetNamedRenderbufferParameteriv", sglGetNamedRenderbufferParameteriv);
        GetOpenGLFunction("glCreateTextures", sglCreateTextures);
        GetOpenGLFunction("glTextureBuffer", sglTextureBuffer);
        GetOpenGLFunction("glTextureBufferRange", sglTextureBufferRange);
        GetOpenGLFunction("glTextureStorage1D", sglTextureStorage1D);
        GetOpenGLFunction("glTextureStorage2D", sglTextureStorage2D);
        GetOpenGLFunction("glTextureStorage3D", sglTextureStorage3D);
        GetOpenGLFunction("glTextureStorage2DMultisample", sglTextureStorage2DMultisample);
        GetOpenGLFunction("glTextureStorage3DMultisample", sglTextureStorage3DMultisample);
        GetOpenGLFunction("glTextureSubImage1D", sglTextureSubImage1D);
        GetOpenGLFunction("glTextureSubImage2D", sglTextureSubImage2D);
        GetOpenGLFunction("glTextureSubImage3D", sglTextureSubImage3D);
        GetOpenGLFunction("glCompressedTextureSubImage1D", sglCompressedTextureSubImage1D);
        GetOpenGLFunction("glCompressedTextureSubImage2D", sglCompressedTextureSubImage2D);
        GetOpenGLFunction("glCompressedTextureSubImage3D", sglCompressedTextureSubImage3D);
        GetOpenGLFunction("glCopyTextureSubImage1D", sglCopyTextureSubImage1D);
        GetOpenGLFunction("glCopyTextureSubImage2D", sglCopyTextureSubImage2D);
        GetOpenGLFunction("glCopyTextureSubImage3D", sglCopyTextureSubImage3D);
        GetOpenGLFunction("glTextureParameterf", sglTextureParameterf);
        GetOpenGLFunction("glTextureParameterfv", sglTextureParameterfv);
        GetOpenGLFunction("glTextureParameteri", sglTextureParameteri);
        GetOpenGLFunction("glTextureParameterIiv", sglTextureParameterIiv);
        GetOpenGLFunction("glTextureParameterIuiv", sglTextureParameterIuiv);
        GetOpenGLFunction("glTextureParameteriv", sglTextureParameteriv);
        GetOpenGLFunction("glGenerateTextureMipmap", sglGenerateTextureMipmap);
        GetOpenGLFunction("glBindTextureUnit", sglBindTextureUnit);
        GetOpenGLFunction("glGetTextureImage", sglGetTextureImage);
        GetOpenGLFunction("glGetCompressedTextureImage", sglGetCompressedTextureImage);
        GetOpenGLFunction("glGetTextureLevelParameterfv", sglGetTextureLevelParameterfv);
        GetOpenGLFunction("glGetTextureLevelParameteriv", sglGetTextureLevelParameteriv);
        GetOpenGLFunction("glGetTextureParameterfv", sglGetTextureParameterfv);
        GetOpenGLFunction("glGetTextureParameterIiv", sglGetTextureParameterIiv);
        GetOpenGLFunction("glGetTextureParameterIuiv", sglGetTextureParameterIuiv);
        GetOpenGLFunction("glGetTextureParameteriv", sglGetTextureParameteriv);
        GetOpenGLFunction("glCreateVertexArrays", sglCreateVertexArrays);
        GetOpenGLFunction("glDisableVertexArrayAttrib", sglDisableVertexArrayAttrib);
        GetOpenGLFunction("glEnableVertexArrayAttrib", sglEnableVertexArrayAttrib);
        GetOpenGLFunction("glVertexArrayElementBuffer", sglVertexArrayElementBuffer);
        GetOpenGLFunction("glVertexArrayVertexBuffer", sglVertexArrayVertexBuffer);
        GetOpenGLFunction("glVertexArrayVertexBuffers", sglVertexArrayVertexBuffers);
        GetOpenGLFunction("glVertexArrayAttribBinding", sglVertexArrayAttribBinding);
        GetOpenGLFunction("glVertexArrayAttribFormat", sglVertexArrayAttribFormat);
        GetOpenGLFunction("glVertexArrayAttribIFormat", sglVertexArrayAttribIFormat);
        GetOpenGLFunction("glVertexArrayAttribLFormat", sglVertexArrayAttribLFormat);
        GetOpenGLFunction("glVertexArrayBindingDivisor", sglVertexArrayBindingDivisor);
        GetOpenGLFunction("glGetVertexArrayiv", sglGetVertexArrayiv);
        GetOpenGLFunction("glGetVertexArrayIndexediv", sglGetVertexArrayIndexediv);
        GetOpenGLFunction("glGetVertexArrayIndexed64iv", sglGetVertexArrayIndexed64iv);
        GetOpenGLFunction("glCreateSamplers", sglCreateSamplers);
        GetOpenGLFunction("glCreateProgramPipelines", sglCreateProgramPipelines);
        GetOpenGLFunction("glCreateQueries", sglCreateQueries);
        GetOpenGLFunction("glGetQueryBufferObjecti64v", sglGetQueryBufferObjecti64v);
        GetOpenGLFunction("glGetQueryBufferObjectiv", sglGetQueryBufferObjectiv);
        GetOpenGLFunction("glGetQueryBufferObjectui64v", sglGetQueryBufferObjectui64v);
        GetOpenGLFunction("glGetQueryBufferObjectuiv", sglGetQueryBufferObjectuiv);
        GetOpenGLFunction("glMemoryBarrierByRegion", sglMemoryBarrierByRegion);
        GetOpenGLFunction("glGetTextureSubImage", sglGetTextureSubImage);
        GetOpenGLFunction("glGetCompressedTextureSubImage", sglGetCompressedTextureSubImage);
        GetOpenGLFunction("glGetGraphicsResetStatus", sglGetGraphicsResetStatus);
        GetOpenGLFunction("glGetnCompressedTexImage", sglGetnCompressedTexImage);
        GetOpenGLFunction("glGetnTexImage", sglGetnTexImage);
        GetOpenGLFunction("glGetnUniformdv", sglGetnUniformdv);
        GetOpenGLFunction("glGetnUniformfv", sglGetnUniformfv);
        GetOpenGLFunction("glGetnUniformiv", sglGetnUniformiv);
        GetOpenGLFunction("glGetnUniformuiv", sglGetnUniformuiv);
        GetOpenGLFunction("glReadnPixels", sglReadnPixels);
        GetOpenGLFunction("glTextureBarrier", sglTextureBarrier);
    }
}

// GL_VERSION_4_6

PFNGLSPECIALIZESHADERPROC sglSpecializeShader = nullptr;
PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC sglMultiDrawArraysIndirectCount = nullptr;
PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC sglMultiDrawElementsIndirectCount = nullptr;
PFNGLPOLYGONOFFSETCLAMPPROC sglPolygonOffsetClamp = nullptr;

void APIENTRY glSpecializeShader(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue)
{
    if (sglSpecializeShader)
    {
        sglSpecializeShader(shader, pEntryPoint, numSpecializationConstants, pConstantIndex, pConstantValue);
        ReportGLError("glSpecializeShader");
    }
    else
    {
        ReportGLNullFunction("glSpecializeShader");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glSpecializeShader", "",
        shader, pEntryPoint, numSpecializationConstants,
        gsTrace.GetArray(numSpecializationConstants, pConstantIndex),
        gsTrace.GetArray(numSpecializationConstants, pConstantValue));
#endif
}

void APIENTRY glMultiDrawArraysIndirectCount(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
    if (sglMultiDrawArraysIndirectCount)
    {
        sglMultiDrawArraysIndirectCount(mode, indirect, drawcount, maxdrawcount, stride);
        ReportGLError("glMultiDrawArraysIndirectCount");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawArraysIndirectCount");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawArraysIndirectCount", "",
        gsTrace.GetName(mode), "indirect", drawcount, maxdrawcount, stride);
#endif
}

void APIENTRY glMultiDrawElementsIndirectCount(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
    if (sglMultiDrawElementsIndirectCount)
    {
        sglMultiDrawElementsIndirectCount(mode, type, indirect, drawcount, maxdrawcount, stride);
        ReportGLError("glMultiDrawElementsIndirectCount");
    }
    else
    {
        ReportGLNullFunction("glMultiDrawElementsIndirectCount");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glMultiDrawElementsIndirectCount", "",
        gsTrace.GetName(mode), gsTrace.GetName(type), "indirect", drawcount, maxdrawcount, stride);
#endif
}

void APIENTRY glPolygonOffsetClamp(GLfloat factor, GLfloat units, GLfloat clamp)
{
    if (sglPolygonOffsetClamp)
    {
        sglPolygonOffsetClamp(factor, units, clamp);
        ReportGLError("glPolygonOffsetClamp");
    }
    else
    {
        ReportGLNullFunction("glPolygonOffsetClamp");
    }

#if defined(GTE_ENABLE_GLTRACE)
    gsTrace.Call("glPolygonOffsetClamp", "",
        factor, units, clamp);
#endif
}

static void Initialize_OPENGL_VERSION_4_6()
{
    if (GetOpenGLVersion() >= OPENGL_VERSION_4_6)
    {
        GetOpenGLFunction("glMultiDrawArraysIndirectCount", sglMultiDrawArraysIndirectCount);
        GetOpenGLFunction("glMultiDrawArraysIndirectCount", sglMultiDrawArraysIndirectCount);
        GetOpenGLFunction("glMultiDrawElementsIndirectCount", sglMultiDrawElementsIndirectCount);
        GetOpenGLFunction("glPolygonOffsetClamp", sglPolygonOffsetClamp);
    }
}

void InitializeOpenGL(int& major, int& minor, char const* infofile)
{
#if !defined(GTE_USE_MSWINDOWS)
    Initialize_OPENGL_VERSION_1_0();
    Initialize_OPENGL_VERSION_1_1();
#endif

    Initialize_OPENGL_VERSION_1_2();
    Initialize_OPENGL_VERSION_1_3();
    Initialize_OPENGL_VERSION_1_4();
    Initialize_OPENGL_VERSION_1_5();
    Initialize_OPENGL_VERSION_2_0();
    Initialize_OPENGL_VERSION_2_1();
    Initialize_OPENGL_VERSION_3_0();
    Initialize_OPENGL_VERSION_3_1();
    Initialize_OPENGL_VERSION_3_2();
    Initialize_OPENGL_VERSION_3_3();
    Initialize_OPENGL_VERSION_4_0();
    Initialize_OPENGL_VERSION_4_1();
    Initialize_OPENGL_VERSION_4_2();
    Initialize_OPENGL_VERSION_4_3();
    Initialize_OPENGL_VERSION_4_4();
    Initialize_OPENGL_VERSION_4_5();
    Initialize_OPENGL_VERSION_4_6();

    if (infofile)
    {
        std::ofstream output(infofile);
        if (output)
        {
            char const* vendor = (char const*)glGetString(GL_VENDOR);
            if (vendor)
            {
                output << "vendor = " << vendor;
            }
            else
            {
                output << "vendor = <null>";
            }
            output << std::endl;

            char const* renderer = (char const*)glGetString(GL_RENDERER);
            if (vendor)
            {
                output << "renderer = " << renderer;
            }
            else
            {
                output << "renderer = <null>";
            }
            output << std::endl;

            char const* version = (char const*)glGetString(GL_VERSION);
            if (version)
            {
                output << "version = " << version;
            }
            else
            {
                output << "version = <null>";
            }
            output << std::endl;

            if (GetOpenGLVersion() >= OPENGL_VERSION_3_0)
            {
                GLint numExtensions;
                glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
                for (int i = 0; i < numExtensions; ++i)
                {
                    output << glGetStringi(GL_EXTENSIONS, i) << std::endl;
                }
            }
            else
            {
                char const* extensions = (char const*)glGetString(GL_EXTENSIONS);
                if (extensions)
                {
                    output << "extensions =" << std::endl;
                    std::string tokenString(extensions);
                    std::vector<std::string> tokens;
                    tokens.clear();
                    while (tokenString.length() > 0)
                    {
                        // Find the beginning of a token.
                        auto begin = tokenString.find_first_not_of(" \t");
                        if (begin == std::string::npos)
                        {
                            // All tokens have been found.
                            break;
                        }

                        // Strip off the white space.
                        if (begin > 0)
                        {
                            tokenString = tokenString.substr(begin);
                        }

                        // Find the end of the token.
                        auto end = tokenString.find_first_of(" \t");
                        if (end != std::string::npos)
                        {
                            std::string token = tokenString.substr(0, end);
                            tokens.push_back(token);
                            tokenString = tokenString.substr(end);
                        }
                        else
                        {
                            // This is the last token.
                            tokens.push_back(tokenString);
                            break;
                        }
                    }

                    for (auto const& token : tokens)
                    {
                        output << "    " << token << std::endl;
                    }
                }
                else
                {
                    output << "extensions = <null>" << std::endl;
                }
            }

            output.close();
        }
    }

    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
}

#if defined(GTE_ENABLE_GLTRACE)
std::array<std::string, 2> GLTrace::msBoolean
{
    "GL_FALSE",
    "GL_TRUE"
};

std::map<std::uint32_t, std::string> GLTrace::msTopology
{
    std::make_pair(0x0000, "GL_POINTS"),
    std::make_pair(0x0001, "GL_LINES"),
    std::make_pair(0x0002, "GL_LINE_LOOP"),
    std::make_pair(0x0003, "GL_LINE_STRIP"),
    std::make_pair(0x0004, "GL_TRIANGLES"),
    std::make_pair(0x0005, "GL_TRIANGLE_STRIP"),
    std::make_pair(0x0006, "GL_TRIANGLE_FAN"),
    std::make_pair(0x0007, "GL_QUADS"),
    std::make_pair(0x000A, "GL_LINES_ADJACENCY"),
    std::make_pair(0x000B, "GL_LINE_STRIP_ADJACENCY"),
    std::make_pair(0x000C, "GL_TRIANGLES_ADJACENCY"),
    std::make_pair(0x000D, "GL_TRIANGLE_STRIP_ADJACENCY"),
    std::make_pair(0x000E, "GL_PGL_PATCHESOINTS")
};

std::map<std::uint32_t, std::string> GLTrace::msName
{
    std::make_pair(0x0200, "GL_NEVER"),
    std::make_pair(0x0201, "GL_LESS"),
    std::make_pair(0x0202, "GL_EQUAL"),
    std::make_pair(0x0203, "GL_LEQUAL"),
    std::make_pair(0x0204, "GL_GREATER"),
    std::make_pair(0x0205, "GL_NOTEQUAL"),
    std::make_pair(0x0206, "GL_GEQUAL"),
    std::make_pair(0x0207, "GL_ALWAYS"),
    std::make_pair(0x0300, "GL_SRC_COLOR"),
    std::make_pair(0x0301, "GL_ONE_MINUS_SRC_COLOR"),
    std::make_pair(0x0302, "GL_SRC_ALPHA"),
    std::make_pair(0x0303, "GL_ONE_MINUS_SRC_ALPHA"),
    std::make_pair(0x0304, "GL_DST_ALPHA"),
    std::make_pair(0x0305, "GL_ONE_MINUS_DST_ALPHA"),
    std::make_pair(0x0306, "GL_DST_COLOR"),
    std::make_pair(0x0307, "GL_ONE_MINUS_DST_COLOR"),
    std::make_pair(0x0308, "GL_SRC_ALPHA_SATURATE"),
    std::make_pair(0x0400, "GL_FRONT_LEFT"),
    std::make_pair(0x0401, "GL_FRONT_RIGHT"),
    std::make_pair(0x0402, "GL_BACK_LEFT"),
    std::make_pair(0x0403, "GL_BACK_RIGHT"),
    std::make_pair(0x0404, "GL_FRONT"),
    std::make_pair(0x0405, "GL_BACK"),
    std::make_pair(0x0406, "GL_LEFT"),
    std::make_pair(0x0407, "GL_RIGHT"),
    std::make_pair(0x0408, "GL_FRONT_AND_BACK"),
    std::make_pair(0x0500, "GL_INVALID_ENUM"),
    std::make_pair(0x0501, "GL_INVALID_VALUE"),
    std::make_pair(0x0502, "GL_INVALID_OPERATION"),
    std::make_pair(0x0503, "GL_STACK_OVERFLOW"),
    std::make_pair(0x0504, "GL_STACK_UNDERFLOW"),
    std::make_pair(0x0505, "GL_OUT_OF_MEMORY"),
    std::make_pair(0x0506, "GL_INVALID_FRAMEBUFFER_OPERATION"),
    std::make_pair(0x0507, "GL_CONTEXT_LOST"),
    std::make_pair(0x0900, "GL_CW"),
    std::make_pair(0x0901, "GL_CCW"),
    std::make_pair(0x0B11, "GL_POINT_SIZE"),
    std::make_pair(0x0B12, "GL_POINT_SIZE_RANGE"),
    std::make_pair(0x0B12, "GL_SMOOTH_POINT_SIZE_RANGE"),
    std::make_pair(0x0B13, "GL_POINT_SIZE_GRANULARITY"),
    std::make_pair(0x0B13, "GL_SMOOTH_POINT_SIZE_GRANULARITY"),
    std::make_pair(0x0B20, "GL_LINE_SMOOTH"),
    std::make_pair(0x0B21, "GL_LINE_WIDTH"),
    std::make_pair(0x0B22, "GL_LINE_WIDTH_RANGE"),
    std::make_pair(0x0B22, "GL_SMOOTH_LINE_WIDTH_RANGE"),
    std::make_pair(0x0B23, "GL_LINE_WIDTH_GRANULARITY"),
    std::make_pair(0x0B23, "GL_SMOOTH_LINE_WIDTH_GRANULARITY"),
    std::make_pair(0x0B40, "GL_POLYGON_MODE"),
    std::make_pair(0x0B41, "GL_POLYGON_SMOOTH"),
    std::make_pair(0x0B44, "GL_CULL_FACE"),
    std::make_pair(0x0B45, "GL_CULL_FACE_MODE"),
    std::make_pair(0x0B46, "GL_FRONT_FACE"),
    std::make_pair(0x0B70, "GL_DEPTH_RANGE"),
    std::make_pair(0x0B71, "GL_DEPTH_TEST"),
    std::make_pair(0x0B72, "GL_DEPTH_WRITEMASK"),
    std::make_pair(0x0B73, "GL_DEPTH_CLEAR_VALUE"),
    std::make_pair(0x0B74, "GL_DEPTH_FUNC"),
    std::make_pair(0x0B90, "GL_STENCIL_TEST"),
    std::make_pair(0x0B91, "GL_STENCIL_CLEAR_VALUE"),
    std::make_pair(0x0B92, "GL_STENCIL_FUNC"),
    std::make_pair(0x0B93, "GL_STENCIL_VALUE_MASK"),
    std::make_pair(0x0B94, "GL_STENCIL_FAIL"),
    std::make_pair(0x0B95, "GL_STENCIL_PASS_DEPTH_FAIL"),
    std::make_pair(0x0B96, "GL_STENCIL_PASS_DEPTH_PASS"),
    std::make_pair(0x0B97, "GL_STENCIL_REF"),
    std::make_pair(0x0B98, "GL_STENCIL_WRITEMASK"),
    std::make_pair(0x0BA2, "GL_VIEWPORT"),
    std::make_pair(0x0BD0, "GL_DITHER"),
    std::make_pair(0x0BE0, "GL_BLEND_DST"),
    std::make_pair(0x0BE1, "GL_BLEND_SRC"),
    std::make_pair(0x0BE2, "GL_BLEND"),
    std::make_pair(0x0BF0, "GL_LOGIC_OP_MODE"),
    std::make_pair(0x0BF2, "GL_COLOR_LOGIC_OP"),
    std::make_pair(0x0C01, "GL_DRAW_BUFFER"),
    std::make_pair(0x0C02, "GL_READ_BUFFER"),
    std::make_pair(0x0C10, "GL_SCISSOR_BOX"),
    std::make_pair(0x0C11, "GL_SCISSOR_TEST"),
    std::make_pair(0x0C22, "GL_COLOR_CLEAR_VALUE"),
    std::make_pair(0x0C23, "GL_COLOR_WRITEMASK"),
    std::make_pair(0x0C32, "GL_DOUBLEBUFFER"),
    std::make_pair(0x0C33, "GL_STEREO"),
    std::make_pair(0x0C52, "GL_LINE_SMOOTH_HINT"),
    std::make_pair(0x0C53, "GL_POLYGON_SMOOTH_HINT"),
    std::make_pair(0x0CF0, "GL_UNPACK_SWAP_BYTES"),
    std::make_pair(0x0CF1, "GL_UNPACK_LSB_FIRST"),
    std::make_pair(0x0CF2, "GL_UNPACK_ROW_LENGTH"),
    std::make_pair(0x0CF3, "GL_UNPACK_SKIP_ROWS"),
    std::make_pair(0x0CF4, "GL_UNPACK_SKIP_PIXELS"),
    std::make_pair(0x0CF5, "GL_UNPACK_ALIGNMENT"),
    std::make_pair(0x0D00, "GL_PACK_SWAP_BYTES"),
    std::make_pair(0x0D01, "GL_PACK_LSB_FIRST"),
    std::make_pair(0x0D02, "GL_PACK_ROW_LENGTH"),
    std::make_pair(0x0D03, "GL_PACK_SKIP_ROWS"),
    std::make_pair(0x0D04, "GL_PACK_SKIP_PIXELS"),
    std::make_pair(0x0D05, "GL_PACK_ALIGNMENT"),
    std::make_pair(0x0D32, "GL_MAX_CLIP_DISTANCES"),
    std::make_pair(0x0D33, "GL_MAX_TEXTURE_SIZE"),
    std::make_pair(0x0D3A, "GL_MAX_VIEWPORT_DIMS"),
    std::make_pair(0x0D50, "GL_SUBPIXEL_BITS"),
    std::make_pair(0x0DE0, "GL_TEXTURE_1D"),
    std::make_pair(0x0DE1, "GL_TEXTURE_2D"),
    std::make_pair(0x1000, "GL_TEXTURE_WIDTH"),
    std::make_pair(0x1001, "GL_TEXTURE_HEIGHT"),
    std::make_pair(0x1003, "GL_TEXTURE_INTERNAL_FORMAT"),
    std::make_pair(0x1004, "GL_TEXTURE_BORDER_COLOR"),
    std::make_pair(0x1006, "GL_TEXTURE_TARGET"),
    std::make_pair(0x1100, "GL_DONT_CARE"),
    std::make_pair(0x1101, "GL_FASTEST"),
    std::make_pair(0x1102, "GL_NICEST"),
    std::make_pair(0x1400, "GL_BYTE"),
    std::make_pair(0x1401, "GL_UNSIGNED_BYTE"),
    std::make_pair(0x1402, "GL_SHORT"),
    std::make_pair(0x1403, "GL_UNSIGNED_SHORT"),
    std::make_pair(0x1404, "GL_INT"),
    std::make_pair(0x1405, "GL_UNSIGNED_INT"),
    std::make_pair(0x1406, "GL_FLOAT"),
    std::make_pair(0x140A, "GL_DOUBLE"),
    std::make_pair(0x140B, "GL_HALF_FLOAT"),
    std::make_pair(0x140C, "GL_FIXED"),
    std::make_pair(0x1500, "GL_CLEAR"),
    std::make_pair(0x1501, "GL_AND"),
    std::make_pair(0x1502, "GL_AND_REVERSE"),
    std::make_pair(0x1503, "GL_COPY"),
    std::make_pair(0x1504, "GL_AND_INVERTED"),
    std::make_pair(0x1505, "GL_NOOP"),
    std::make_pair(0x1506, "GL_XOR"),
    std::make_pair(0x1507, "GL_OR"),
    std::make_pair(0x1508, "GL_NOR"),
    std::make_pair(0x1509, "GL_EQUIV"),
    std::make_pair(0x150A, "GL_INVERT"),
    std::make_pair(0x150B, "GL_OR_REVERSE"),
    std::make_pair(0x150C, "GL_COPY_INVERTED"),
    std::make_pair(0x150D, "GL_OR_INVERTED"),
    std::make_pair(0x150E, "GL_NAND"),
    std::make_pair(0x150F, "GL_SET"),
    std::make_pair(0x1702, "GL_TEXTURE"),
    std::make_pair(0x1800, "GL_COLOR"),
    std::make_pair(0x1801, "GL_DEPTH"),
    std::make_pair(0x1802, "GL_STENCIL"),
    std::make_pair(0x1901, "GL_STENCIL_INDEX"),
    std::make_pair(0x1902, "GL_DEPTH_COMPONENT"),
    std::make_pair(0x1903, "GL_RED"),
    std::make_pair(0x1904, "GL_GREEN"),
    std::make_pair(0x1905, "GL_BLUE"),
    std::make_pair(0x1906, "GL_ALPHA"),
    std::make_pair(0x1907, "GL_RGB"),
    std::make_pair(0x1908, "GL_RGBA"),
    std::make_pair(0x1B00, "GL_POINT"),
    std::make_pair(0x1B01, "GL_LINE"),
    std::make_pair(0x1B02, "GL_FILL"),
    std::make_pair(0x1E00, "GL_KEEP"),
    std::make_pair(0x1E01, "GL_REPLACE"),
    std::make_pair(0x1E02, "GL_INCR"),
    std::make_pair(0x1E03, "GL_DECR"),
    std::make_pair(0x1F00, "GL_VENDOR"),
    std::make_pair(0x1F01, "GL_RENDERER"),
    std::make_pair(0x1F02, "GL_VERSION"),
    std::make_pair(0x1F03, "GL_EXTENSIONS"),
    std::make_pair(0x2600, "GL_NEAREST"),
    std::make_pair(0x2601, "GL_LINEAR"),
    std::make_pair(0x2700, "GL_NEAREST_MIPMAP_NEAREST"),
    std::make_pair(0x2701, "GL_LINEAR_MIPMAP_NEAREST"),
    std::make_pair(0x2702, "GL_NEAREST_MIPMAP_LINEAR"),
    std::make_pair(0x2703, "GL_LINEAR_MIPMAP_LINEAR"),
    std::make_pair(0x2800, "GL_TEXTURE_MAG_FILTER"),
    std::make_pair(0x2801, "GL_TEXTURE_MIN_FILTER"),
    std::make_pair(0x2802, "GL_TEXTURE_WRAP_S"),
    std::make_pair(0x2803, "GL_TEXTURE_WRAP_T"),
    std::make_pair(0x2901, "GL_REPEAT"),
    std::make_pair(0x2A00, "GL_POLYGON_OFFSET_UNITS"),
    std::make_pair(0x2A01, "GL_POLYGON_OFFSET_POINT"),
    std::make_pair(0x2A02, "GL_POLYGON_OFFSET_LINE"),
    std::make_pair(0x2A10, "GL_R3_G3_B2"),
    std::make_pair(0x3000, "GL_CLIP_DISTANCE0"),
    std::make_pair(0x3001, "GL_CLIP_DISTANCE1"),
    std::make_pair(0x3002, "GL_CLIP_DISTANCE2"),
    std::make_pair(0x3003, "GL_CLIP_DISTANCE3"),
    std::make_pair(0x3004, "GL_CLIP_DISTANCE4"),
    std::make_pair(0x3005, "GL_CLIP_DISTANCE5"),
    std::make_pair(0x3006, "GL_CLIP_DISTANCE6"),
    std::make_pair(0x3007, "GL_CLIP_DISTANCE7"),
    std::make_pair(0x8001, "GL_CONSTANT_COLOR"),
    std::make_pair(0x8002, "GL_ONE_MINUS_CONSTANT_COLOR"),
    std::make_pair(0x8003, "GL_CONSTANT_ALPHA"),
    std::make_pair(0x8004, "GL_ONE_MINUS_CONSTANT_ALPHA"),
    std::make_pair(0x8006, "GL_FUNC_ADD"),
    std::make_pair(0x8007, "GL_MIN"),
    std::make_pair(0x8008, "GL_MAX"),
    std::make_pair(0x8009, "GL_BLEND_EQUATION_RGB"),
    std::make_pair(0x800A, "GL_FUNC_SUBTRACT"),
    std::make_pair(0x800B, "GL_FUNC_REVERSE_SUBTRACT"),
    std::make_pair(0x8032, "GL_UNSIGNED_BYTE_3_3_2"),
    std::make_pair(0x8033, "GL_UNSIGNED_SHORT_4_4_4_4"),
    std::make_pair(0x8034, "GL_UNSIGNED_SHORT_5_5_5_1"),
    std::make_pair(0x8035, "GL_UNSIGNED_INT_8_8_8_8"),
    std::make_pair(0x8036, "GL_UNSIGNED_INT_10_10_10_2"),
    std::make_pair(0x8037, "GL_POLYGON_OFFSET_FILL"),
    std::make_pair(0x8038, "GL_POLYGON_OFFSET_FACTOR"),
    std::make_pair(0x804F, "GL_RGB4"),
    std::make_pair(0x8050, "GL_RGB5"),
    std::make_pair(0x8051, "GL_RGB8"),
    std::make_pair(0x8052, "GL_RGB10"),
    std::make_pair(0x8053, "GL_RGB12"),
    std::make_pair(0x8054, "GL_RGB16"),
    std::make_pair(0x8055, "GL_RGBA2"),
    std::make_pair(0x8056, "GL_RGBA4"),
    std::make_pair(0x8057, "GL_RGB5_A1"),
    std::make_pair(0x8058, "GL_RGBA8"),
    std::make_pair(0x8059, "GL_RGB10_A2"),
    std::make_pair(0x805A, "GL_RGBA12"),
    std::make_pair(0x805B, "GL_RGBA16"),
    std::make_pair(0x805C, "GL_TEXTURE_RED_SIZE"),
    std::make_pair(0x805D, "GL_TEXTURE_GREEN_SIZE"),
    std::make_pair(0x805E, "GL_TEXTURE_BLUE_SIZE"),
    std::make_pair(0x805F, "GL_TEXTURE_ALPHA_SIZE"),
    std::make_pair(0x8063, "GL_PROXY_TEXTURE_1D"),
    std::make_pair(0x8064, "GL_PROXY_TEXTURE_2D"),
    std::make_pair(0x8068, "GL_TEXTURE_BINDING_1D"),
    std::make_pair(0x8069, "GL_TEXTURE_BINDING_2D"),
    std::make_pair(0x806A, "GL_TEXTURE_BINDING_3D"),
    std::make_pair(0x806B, "GL_PACK_SKIP_IMAGES"),
    std::make_pair(0x806C, "GL_PACK_IMAGE_HEIGHT"),
    std::make_pair(0x806D, "GL_UNPACK_SKIP_IMAGES"),
    std::make_pair(0x806E, "GL_UNPACK_IMAGE_HEIGHT"),
    std::make_pair(0x806F, "GL_TEXTURE_3D"),
    std::make_pair(0x8070, "GL_PROXY_TEXTURE_3D"),
    std::make_pair(0x8071, "GL_TEXTURE_DEPTH"),
    std::make_pair(0x8072, "GL_TEXTURE_WRAP_R"),
    std::make_pair(0x8073, "GL_MAX_3D_TEXTURE_SIZE"),
    std::make_pair(0x8074, "GL_VERTEX_ARRAY"),
    std::make_pair(0x809D, "GL_MULTISAMPLE"),
    std::make_pair(0x809E, "GL_SAMPLE_ALPHA_TO_COVERAGE"),
    std::make_pair(0x809F, "GL_SAMPLE_ALPHA_TO_ONE"),
    std::make_pair(0x80A0, "GL_SAMPLE_COVERAGE"),
    std::make_pair(0x80A8, "GL_SAMPLE_BUFFERS"),
    std::make_pair(0x80A9, "GL_SAMPLES"),
    std::make_pair(0x80AA, "GL_SAMPLE_COVERAGE_VALUE"),
    std::make_pair(0x80AB, "GL_SAMPLE_COVERAGE_INVERT"),
    std::make_pair(0x80C8, "GL_BLEND_DST_RGB"),
    std::make_pair(0x80C9, "GL_BLEND_SRC_RGB"),
    std::make_pair(0x80CA, "GL_BLEND_DST_ALPHA"),
    std::make_pair(0x80CB, "GL_BLEND_SRC_ALPHA"),
    std::make_pair(0x80E0, "GL_BGR"),
    std::make_pair(0x80E1, "GL_BGRA"),
    std::make_pair(0x80E8, "GL_MAX_ELEMENTS_VERTICES"),
    std::make_pair(0x80E9, "GL_MAX_ELEMENTS_INDICES"),
    std::make_pair(0x8128, "GL_POINT_FADE_THRESHOLD_SIZE"),
    std::make_pair(0x812D, "GL_CLAMP_TO_BORDER"),
    std::make_pair(0x812F, "GL_CLAMP_TO_EDGE"),
    std::make_pair(0x813A, "GL_TEXTURE_MIN_LOD"),
    std::make_pair(0x813B, "GL_TEXTURE_MAX_LOD"),
    std::make_pair(0x813C, "GL_TEXTURE_BASE_LEVEL"),
    std::make_pair(0x813D, "GL_TEXTURE_MAX_LEVEL"),
    std::make_pair(0x81A5, "GL_DEPTH_COMPONENT16"),
    std::make_pair(0x81A6, "GL_DEPTH_COMPONENT24"),
    std::make_pair(0x81A7, "GL_DEPTH_COMPONENT32"),
    std::make_pair(0x8210, "GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING"),
    std::make_pair(0x8211, "GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE"),
    std::make_pair(0x8212, "GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE"),
    std::make_pair(0x8213, "GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE"),
    std::make_pair(0x8214, "GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE"),
    std::make_pair(0x8215, "GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE"),
    std::make_pair(0x8216, "GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE"),
    std::make_pair(0x8217, "GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE"),
    std::make_pair(0x8218, "GL_FRAMEBUFFER_DEFAULT"),
    std::make_pair(0x8219, "GL_FRAMEBUFFER_UNDEFINED"),
    std::make_pair(0x821A, "GL_DEPTH_STENCIL_ATTACHMENT"),
    std::make_pair(0x821B, "GL_MAJOR_VERSION"),
    std::make_pair(0x821C, "GL_MINOR_VERSION"),
    std::make_pair(0x821D, "GL_NUM_EXTENSIONS"),
    std::make_pair(0x821E, "GL_CONTEXT_FLAGS"),
    std::make_pair(0x821F, "GL_BUFFER_IMMUTABLE_STORAGE"),
    std::make_pair(0x8220, "GL_BUFFER_STORAGE_FLAGS"),
    std::make_pair(0x8221, "GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED"),
    std::make_pair(0x8225, "GL_COMPRESSED_RED"),
    std::make_pair(0x8226, "GL_COMPRESSED_RG"),
    std::make_pair(0x8227, "GL_RG"),
    std::make_pair(0x8228, "GL_RG_INTEGER"),
    std::make_pair(0x8229, "GL_R8"),
    std::make_pair(0x822A, "GL_R16"),
    std::make_pair(0x822B, "GL_RG8"),
    std::make_pair(0x822C, "GL_RG16"),
    std::make_pair(0x822D, "GL_R16F"),
    std::make_pair(0x822E, "GL_R32F"),
    std::make_pair(0x822F, "GL_RG16F"),
    std::make_pair(0x8230, "GL_RG32F"),
    std::make_pair(0x8231, "GL_R8I"),
    std::make_pair(0x8232, "GL_R8UI"),
    std::make_pair(0x8233, "GL_R16I"),
    std::make_pair(0x8234, "GL_R16UI"),
    std::make_pair(0x8235, "GL_R32I"),
    std::make_pair(0x8236, "GL_R32UI"),
    std::make_pair(0x8237, "GL_RG8I"),
    std::make_pair(0x8238, "GL_RG8UI"),
    std::make_pair(0x8239, "GL_RG16I"),
    std::make_pair(0x823A, "GL_RG16UI"),
    std::make_pair(0x823B, "GL_RG32I"),
    std::make_pair(0x823C, "GL_RG32UI"),
    std::make_pair(0x8242, "GL_DEBUG_OUTPUT_SYNCHRONOUS"),
    std::make_pair(0x8243, "GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH"),
    std::make_pair(0x8244, "GL_DEBUG_CALLBACK_FUNCTION"),
    std::make_pair(0x8245, "GL_DEBUG_CALLBACK_USER_PARAM"),
    std::make_pair(0x8246, "GL_DEBUG_SOURCE_API"),
    std::make_pair(0x8247, "GL_DEBUG_SOURCE_WINDOW_SYSTEM"),
    std::make_pair(0x8248, "GL_DEBUG_SOURCE_SHADER_COMPILER"),
    std::make_pair(0x8249, "GL_DEBUG_SOURCE_THIRD_PARTY"),
    std::make_pair(0x824A, "GL_DEBUG_SOURCE_APPLICATION"),
    std::make_pair(0x824B, "GL_DEBUG_SOURCE_OTHER"),
    std::make_pair(0x824C, "GL_DEBUG_TYPE_ERROR"),
    std::make_pair(0x824D, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"),
    std::make_pair(0x824E, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"),
    std::make_pair(0x824F, "GL_DEBUG_TYPE_PORTABILITY"),
    std::make_pair(0x8250, "GL_DEBUG_TYPE_PERFORMANCE"),
    std::make_pair(0x8251, "GL_DEBUG_TYPE_OTHER"),
    std::make_pair(0x8252, "GL_LOSE_CONTEXT_ON_RESET"),
    std::make_pair(0x8253, "GL_GUILTY_CONTEXT_RESET"),
    std::make_pair(0x8254, "GL_INNOCENT_CONTEXT_RESET"),
    std::make_pair(0x8255, "GL_UNKNOWN_CONTEXT_RESET"),
    std::make_pair(0x8256, "GL_RESET_NOTIFICATION_STRATEGY"),
    std::make_pair(0x8257, "GL_PROGRAM_BINARY_RETRIEVABLE_HINT"),
    std::make_pair(0x8258, "GL_PROGRAM_SEPARABLE"),
    std::make_pair(0x8259, "GL_ACTIVE_PROGRAM"),
    std::make_pair(0x825A, "GL_PROGRAM_PIPELINE_BINDING"),
    std::make_pair(0x825B, "GL_MAX_VIEWPORTS"),
    std::make_pair(0x825C, "GL_VIEWPORT_SUBPIXEL_BITS"),
    std::make_pair(0x825D, "GL_VIEWPORT_BOUNDS_RANGE"),
    std::make_pair(0x825E, "GL_LAYER_PROVOKING_VERTEX"),
    std::make_pair(0x825F, "GL_VIEWPORT_INDEX_PROVOKING_VERTEX"),
    std::make_pair(0x8260, "GL_UNDEFINED_VERTEX"),
    std::make_pair(0x8261, "GL_NO_RESET_NOTIFICATION"),
    std::make_pair(0x8262, "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE"),
    std::make_pair(0x8263, "GL_MAX_COMPUTE_UNIFORM_COMPONENTS"),
    std::make_pair(0x8264, "GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x8265, "GL_MAX_COMPUTE_ATOMIC_COUNTERS"),
    std::make_pair(0x8266, "GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS"),
    std::make_pair(0x8267, "GL_COMPUTE_WORK_GROUP_SIZE"),
    std::make_pair(0x8268, "GL_DEBUG_TYPE_MARKER"),
    std::make_pair(0x8269, "GL_DEBUG_TYPE_PUSH_GROUP"),
    std::make_pair(0x826A, "GL_DEBUG_TYPE_POP_GROUP"),
    std::make_pair(0x826B, "GL_DEBUG_SEVERITY_NOTIFICATION"),
    std::make_pair(0x826C, "GL_MAX_DEBUG_GROUP_STACK_DEPTH"),
    std::make_pair(0x826D, "GL_DEBUG_GROUP_STACK_DEPTH"),
    std::make_pair(0x826E, "GL_MAX_UNIFORM_LOCATIONS"),
    std::make_pair(0x826F, "GL_INTERNALFORMAT_SUPPORTED"),
    std::make_pair(0x8270, "GL_INTERNALFORMAT_PREFERRED"),
    std::make_pair(0x8271, "GL_INTERNALFORMAT_RED_SIZE"),
    std::make_pair(0x8272, "GL_INTERNALFORMAT_GREEN_SIZE"),
    std::make_pair(0x8273, "GL_INTERNALFORMAT_BLUE_SIZE"),
    std::make_pair(0x8274, "GL_INTERNALFORMAT_ALPHA_SIZE"),
    std::make_pair(0x8275, "GL_INTERNALFORMAT_DEPTH_SIZE"),
    std::make_pair(0x8276, "GL_INTERNALFORMAT_STENCIL_SIZE"),
    std::make_pair(0x8277, "GL_INTERNALFORMAT_SHARED_SIZE"),
    std::make_pair(0x8278, "GL_INTERNALFORMAT_RED_TYPE"),
    std::make_pair(0x8279, "GL_INTERNALFORMAT_GREEN_TYPE"),
    std::make_pair(0x827A, "GL_INTERNALFORMAT_BLUE_TYPE"),
    std::make_pair(0x827B, "GL_INTERNALFORMAT_ALPHA_TYPE"),
    std::make_pair(0x827C, "GL_INTERNALFORMAT_DEPTH_TYPE"),
    std::make_pair(0x827D, "GL_INTERNALFORMAT_STENCIL_TYPE"),
    std::make_pair(0x827E, "GL_MAX_WIDTH"),
    std::make_pair(0x827F, "GL_MAX_HEIGHT"),
    std::make_pair(0x8280, "GL_MAX_DEPTH"),
    std::make_pair(0x8281, "GL_MAX_LAYERS"),
    std::make_pair(0x8282, "GL_MAX_COMBINED_DIMENSIONS"),
    std::make_pair(0x8283, "GL_COLOR_COMPONENTS"),
    std::make_pair(0x8284, "GL_DEPTH_COMPONENTS"),
    std::make_pair(0x8285, "GL_STENCIL_COMPONENTS"),
    std::make_pair(0x8286, "GL_COLOR_RENDERABLE"),
    std::make_pair(0x8287, "GL_DEPTH_RENDERABLE"),
    std::make_pair(0x8288, "GL_STENCIL_RENDERABLE"),
    std::make_pair(0x8289, "GL_FRAMEBUFFER_RENDERABLE"),
    std::make_pair(0x828A, "GL_FRAMEBUFFER_RENDERABLE_LAYERED"),
    std::make_pair(0x828B, "GL_FRAMEBUFFER_BLEND"),
    std::make_pair(0x828C, "GL_READ_PIXELS"),
    std::make_pair(0x828D, "GL_READ_PIXELS_FORMAT"),
    std::make_pair(0x828E, "GL_READ_PIXELS_TYPE"),
    std::make_pair(0x828F, "GL_TEXTURE_IMAGE_FORMAT"),
    std::make_pair(0x8290, "GL_TEXTURE_IMAGE_TYPE"),
    std::make_pair(0x8291, "GL_GET_TEXTURE_IMAGE_FORMAT"),
    std::make_pair(0x8292, "GL_GET_TEXTURE_IMAGE_TYPE"),
    std::make_pair(0x8293, "GL_MIPMAP"),
    std::make_pair(0x8294, "GL_MANUAL_GENERATE_MIPMAP"),
    std::make_pair(0x8295, "GL_AUTO_GENERATE_MIPMAP"),
    std::make_pair(0x8296, "GL_COLOR_ENCODING"),
    std::make_pair(0x8297, "GL_SRGB_READ"),
    std::make_pair(0x8298, "GL_SRGB_WRITE"),
    std::make_pair(0x829A, "GL_FILTER"),
    std::make_pair(0x829B, "GL_VERTEX_TEXTURE"),
    std::make_pair(0x829C, "GL_TESS_CONTROL_TEXTURE"),
    std::make_pair(0x829D, "GL_TESS_EVALUATION_TEXTURE"),
    std::make_pair(0x829E, "GL_GEOMETRY_TEXTURE"),
    std::make_pair(0x829F, "GL_FRAGMENT_TEXTURE"),
    std::make_pair(0x82A0, "GL_COMPUTE_TEXTURE"),
    std::make_pair(0x82A1, "GL_TEXTURE_SHADOW"),
    std::make_pair(0x82A2, "GL_TEXTURE_GATHER"),
    std::make_pair(0x82A3, "GL_TEXTURE_GATHER_SHADOW"),
    std::make_pair(0x82A4, "GL_SHADER_IMAGE_LOAD"),
    std::make_pair(0x82A5, "GL_SHADER_IMAGE_STORE"),
    std::make_pair(0x82A6, "GL_SHADER_IMAGE_ATOMIC"),
    std::make_pair(0x82A7, "GL_IMAGE_TEXEL_SIZE"),
    std::make_pair(0x82A8, "GL_IMAGE_COMPATIBILITY_CLASS"),
    std::make_pair(0x82A9, "GL_IMAGE_PIXEL_FORMAT"),
    std::make_pair(0x82AA, "GL_IMAGE_PIXEL_TYPE"),
    std::make_pair(0x82AC, "GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST"),
    std::make_pair(0x82AD, "GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST"),
    std::make_pair(0x82AE, "GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE"),
    std::make_pair(0x82AF, "GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE"),
    std::make_pair(0x82B1, "GL_TEXTURE_COMPRESSED_BLOCK_WIDTH"),
    std::make_pair(0x82B2, "GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT"),
    std::make_pair(0x82B3, "GL_TEXTURE_COMPRESSED_BLOCK_SIZE"),
    std::make_pair(0x82B4, "GL_CLEAR_BUFFER"),
    std::make_pair(0x82B5, "GL_TEXTURE_VIEW"),
    std::make_pair(0x82B6, "GL_VIEW_COMPATIBILITY_CLASS"),
    std::make_pair(0x82B7, "GL_FULL_SUPPORT"),
    std::make_pair(0x82B8, "GL_CAVEAT_SUPPORT"),
    std::make_pair(0x82B9, "GL_IMAGE_CLASS_4_X_32"),
    std::make_pair(0x82BA, "GL_IMAGE_CLASS_2_X_32"),
    std::make_pair(0x82BB, "GL_IMAGE_CLASS_1_X_32"),
    std::make_pair(0x82BC, "GL_IMAGE_CLASS_4_X_16"),
    std::make_pair(0x82BD, "GL_IMAGE_CLASS_2_X_16"),
    std::make_pair(0x82BE, "GL_IMAGE_CLASS_1_X_16"),
    std::make_pair(0x82BF, "GL_IMAGE_CLASS_4_X_8"),
    std::make_pair(0x82C0, "GL_IMAGE_CLASS_2_X_8"),
    std::make_pair(0x82C1, "GL_IMAGE_CLASS_1_X_8"),
    std::make_pair(0x82C2, "GL_IMAGE_CLASS_11_11_10"),
    std::make_pair(0x82C3, "GL_IMAGE_CLASS_10_10_10_2"),
    std::make_pair(0x82C4, "GL_VIEW_CLASS_128_BITS"),
    std::make_pair(0x82C5, "GL_VIEW_CLASS_96_BITS"),
    std::make_pair(0x82C6, "GL_VIEW_CLASS_64_BITS"),
    std::make_pair(0x82C7, "GL_VIEW_CLASS_48_BITS"),
    std::make_pair(0x82C8, "GL_VIEW_CLASS_32_BITS"),
    std::make_pair(0x82C9, "GL_VIEW_CLASS_24_BITS"),
    std::make_pair(0x82CA, "GL_VIEW_CLASS_16_BITS"),
    std::make_pair(0x82CB, "GL_VIEW_CLASS_8_BITS"),
    std::make_pair(0x82CC, "GL_VIEW_CLASS_S3TC_DXT1_RGB"),
    std::make_pair(0x82CD, "GL_VIEW_CLASS_S3TC_DXT1_RGBA"),
    std::make_pair(0x82CE, "GL_VIEW_CLASS_S3TC_DXT3_RGBA"),
    std::make_pair(0x82CF, "GL_VIEW_CLASS_S3TC_DXT5_RGBA"),
    std::make_pair(0x82D0, "GL_VIEW_CLASS_RGTC1_RED"),
    std::make_pair(0x82D1, "GL_VIEW_CLASS_RGTC2_RG"),
    std::make_pair(0x82D2, "GL_VIEW_CLASS_BPTC_UNORM"),
    std::make_pair(0x82D3, "GL_VIEW_CLASS_BPTC_FLOAT"),
    std::make_pair(0x82D4, "GL_VERTEX_ATTRIB_BINDING"),
    std::make_pair(0x82D5, "GL_VERTEX_ATTRIB_RELATIVE_OFFSET"),
    std::make_pair(0x82D6, "GL_VERTEX_BINDING_DIVISOR"),
    std::make_pair(0x82D7, "GL_VERTEX_BINDING_OFFSET"),
    std::make_pair(0x82D8, "GL_VERTEX_BINDING_STRIDE"),
    std::make_pair(0x82D9, "GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET"),
    std::make_pair(0x82DA, "GL_MAX_VERTEX_ATTRIB_BINDINGS"),
    std::make_pair(0x82DB, "GL_TEXTURE_VIEW_MIN_LEVEL"),
    std::make_pair(0x82DC, "GL_TEXTURE_VIEW_NUM_LEVELS"),
    std::make_pair(0x82DD, "GL_TEXTURE_VIEW_MIN_LAYER"),
    std::make_pair(0x82DE, "GL_TEXTURE_VIEW_NUM_LAYERS"),
    std::make_pair(0x82DF, "GL_TEXTURE_IMMUTABLE_LEVELS"),
    std::make_pair(0x82E0, "GL_BUFFER"),
    std::make_pair(0x82E1, "GL_SHADER"),
    std::make_pair(0x82E2, "GL_PROGRAM"),
    std::make_pair(0x82E3, "GL_QUERY"),
    std::make_pair(0x82E4, "GL_PROGRAM_PIPELINE"),
    std::make_pair(0x82E5, "GL_MAX_VERTEX_ATTRIB_STRIDE"),
    std::make_pair(0x82E6, "GL_SAMPLER"),
    std::make_pair(0x82E8, "GL_MAX_LABEL_LENGTH"),
    std::make_pair(0x82E9, "GL_NUM_SHADING_LANGUAGE_VERSIONS"),
    std::make_pair(0x82EA, "GL_QUERY_TARGET"),
    std::make_pair(0x82EB, "GL_TEXTURE_BINDING"),
    std::make_pair(0x82F9, "GL_MAX_CULL_DISTANCES"),
    std::make_pair(0x82FA, "GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES"),
    std::make_pair(0x82FB, "GL_CONTEXT_RELEASE_BEHAVIOR"),
    std::make_pair(0x82FC, "GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH"),
    std::make_pair(0x8362, "GL_UNSIGNED_BYTE_2_3_3_REV"),
    std::make_pair(0x8363, "GL_UNSIGNED_SHORT_5_6_5"),
    std::make_pair(0x8364, "GL_UNSIGNED_SHORT_5_6_5_REV"),
    std::make_pair(0x8365, "GL_UNSIGNED_SHORT_4_4_4_4_REV"),
    std::make_pair(0x8366, "GL_UNSIGNED_SHORT_1_5_5_5_REV"),
    std::make_pair(0x8367, "GL_UNSIGNED_INT_8_8_8_8_REV"),
    std::make_pair(0x8368, "GL_UNSIGNED_INT_2_10_10_10_REV"),
    std::make_pair(0x8370, "GL_MIRRORED_REPEAT"),
    std::make_pair(0x846E, "GL_ALIASED_LINE_WIDTH_RANGE"),
    std::make_pair(0x84C0, "GL_TEXTURE0"),
    std::make_pair(0x84C1, "GL_TEXTURE1"),
    std::make_pair(0x84C2, "GL_TEXTURE2"),
    std::make_pair(0x84C3, "GL_TEXTURE3"),
    std::make_pair(0x84C4, "GL_TEXTURE4"),
    std::make_pair(0x84C5, "GL_TEXTURE5"),
    std::make_pair(0x84C6, "GL_TEXTURE6"),
    std::make_pair(0x84C7, "GL_TEXTURE7"),
    std::make_pair(0x84C8, "GL_TEXTURE8"),
    std::make_pair(0x84C9, "GL_TEXTURE9"),
    std::make_pair(0x84CA, "GL_TEXTURE10"),
    std::make_pair(0x84CB, "GL_TEXTURE11"),
    std::make_pair(0x84CC, "GL_TEXTURE12"),
    std::make_pair(0x84CD, "GL_TEXTURE13"),
    std::make_pair(0x84CE, "GL_TEXTURE14"),
    std::make_pair(0x84CF, "GL_TEXTURE15"),
    std::make_pair(0x84D0, "GL_TEXTURE16"),
    std::make_pair(0x84D1, "GL_TEXTURE17"),
    std::make_pair(0x84D2, "GL_TEXTURE18"),
    std::make_pair(0x84D3, "GL_TEXTURE19"),
    std::make_pair(0x84D4, "GL_TEXTURE20"),
    std::make_pair(0x84D5, "GL_TEXTURE21"),
    std::make_pair(0x84D6, "GL_TEXTURE22"),
    std::make_pair(0x84D7, "GL_TEXTURE23"),
    std::make_pair(0x84D8, "GL_TEXTURE24"),
    std::make_pair(0x84D9, "GL_TEXTURE25"),
    std::make_pair(0x84DA, "GL_TEXTURE26"),
    std::make_pair(0x84DB, "GL_TEXTURE27"),
    std::make_pair(0x84DC, "GL_TEXTURE28"),
    std::make_pair(0x84DD, "GL_TEXTURE29"),
    std::make_pair(0x84DE, "GL_TEXTURE30"),
    std::make_pair(0x84DF, "GL_TEXTURE31"),
    std::make_pair(0x84E0, "GL_ACTIVE_TEXTURE"),
    std::make_pair(0x84E8, "GL_MAX_RENDERBUFFER_SIZE"),
    std::make_pair(0x84ED, "GL_COMPRESSED_RGB"),
    std::make_pair(0x84EE, "GL_COMPRESSED_RGBA"),
    std::make_pair(0x84EF, "GL_TEXTURE_COMPRESSION_HINT"),
    std::make_pair(0x84F0, "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER"),
    std::make_pair(0x84F1, "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER"),
    std::make_pair(0x84F5, "GL_TEXTURE_RECTANGLE"),
    std::make_pair(0x84F6, "GL_TEXTURE_BINDING_RECTANGLE"),
    std::make_pair(0x84F7, "GL_PROXY_TEXTURE_RECTANGLE"),
    std::make_pair(0x84F8, "GL_MAX_RECTANGLE_TEXTURE_SIZE"),
    std::make_pair(0x84F9, "GL_DEPTH_STENCIL"),
    std::make_pair(0x84FA, "GL_UNSIGNED_INT_24_8"),
    std::make_pair(0x84FD, "GL_MAX_TEXTURE_LOD_BIAS"),
    std::make_pair(0x8501, "GL_TEXTURE_LOD_BIAS"),
    std::make_pair(0x8507, "GL_INCR_WRAP"),
    std::make_pair(0x8508, "GL_DECR_WRAP"),
    std::make_pair(0x8513, "GL_TEXTURE_CUBE_MAP"),
    std::make_pair(0x8514, "GL_TEXTURE_BINDING_CUBE_MAP"),
    std::make_pair(0x8515, "GL_TEXTURE_CUBE_MAP_POSITIVE_X"),
    std::make_pair(0x8516, "GL_TEXTURE_CUBE_MAP_NEGATIVE_X"),
    std::make_pair(0x8517, "GL_TEXTURE_CUBE_MAP_POSITIVE_Y"),
    std::make_pair(0x8518, "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y"),
    std::make_pair(0x8519, "GL_TEXTURE_CUBE_MAP_POSITIVE_Z"),
    std::make_pair(0x851A, "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z"),
    std::make_pair(0x851B, "GL_PROXY_TEXTURE_CUBE_MAP"),
    std::make_pair(0x851C, "GL_MAX_CUBE_MAP_TEXTURE_SIZE"),
    std::make_pair(0x8589, "GL_SRC1_ALPHA"),
    std::make_pair(0x85B5, "GL_VERTEX_ARRAY_BINDING"),
    std::make_pair(0x8622, "GL_VERTEX_ATTRIB_ARRAY_ENABLED"),
    std::make_pair(0x8623, "GL_VERTEX_ATTRIB_ARRAY_SIZE"),
    std::make_pair(0x8624, "GL_VERTEX_ATTRIB_ARRAY_STRIDE"),
    std::make_pair(0x8625, "GL_VERTEX_ATTRIB_ARRAY_TYPE"),
    std::make_pair(0x8626, "GL_CURRENT_VERTEX_ATTRIB"),
    std::make_pair(0x8642, "GL_VERTEX_PROGRAM_POINT_SIZE"),
    std::make_pair(0x8642, "GL_PROGRAM_POINT_SIZE"),
    std::make_pair(0x8645, "GL_VERTEX_ATTRIB_ARRAY_POINTER"),
    std::make_pair(0x864F, "GL_DEPTH_CLAMP"),
    std::make_pair(0x86A0, "GL_TEXTURE_COMPRESSED_IMAGE_SIZE"),
    std::make_pair(0x86A1, "GL_TEXTURE_COMPRESSED"),
    std::make_pair(0x86A2, "GL_NUM_COMPRESSED_TEXTURE_FORMATS"),
    std::make_pair(0x86A3, "GL_COMPRESSED_TEXTURE_FORMATS"),
    std::make_pair(0x8741, "GL_PROGRAM_BINARY_LENGTH"),
    std::make_pair(0x8743, "GL_MIRROR_CLAMP_TO_EDGE"),
    std::make_pair(0x874E, "GL_VERTEX_ATTRIB_ARRAY_LONG"),
    std::make_pair(0x8764, "GL_BUFFER_SIZE"),
    std::make_pair(0x8765, "GL_BUFFER_USAGE"),
    std::make_pair(0x87FE, "GL_NUM_PROGRAM_BINARY_FORMATS"),
    std::make_pair(0x87FF, "GL_PROGRAM_BINARY_FORMATS"),
    std::make_pair(0x8800, "GL_STENCIL_BACK_FUNC"),
    std::make_pair(0x8801, "GL_STENCIL_BACK_FAIL"),
    std::make_pair(0x8802, "GL_STENCIL_BACK_PASS_DEPTH_FAIL"),
    std::make_pair(0x8803, "GL_STENCIL_BACK_PASS_DEPTH_PASS"),
    std::make_pair(0x8814, "GL_RGBA32F"),
    std::make_pair(0x8815, "GL_RGB32F"),
    std::make_pair(0x881A, "GL_RGBA16F"),
    std::make_pair(0x881B, "GL_RGB16F"),
    std::make_pair(0x8824, "GL_MAX_DRAW_BUFFERS"),
    std::make_pair(0x8825, "GL_DRAW_BUFFER0"),
    std::make_pair(0x8826, "GL_DRAW_BUFFER1"),
    std::make_pair(0x8827, "GL_DRAW_BUFFER2"),
    std::make_pair(0x8828, "GL_DRAW_BUFFER3"),
    std::make_pair(0x8829, "GL_DRAW_BUFFER4"),
    std::make_pair(0x882A, "GL_DRAW_BUFFER5"),
    std::make_pair(0x882B, "GL_DRAW_BUFFER6"),
    std::make_pair(0x882C, "GL_DRAW_BUFFER7"),
    std::make_pair(0x882D, "GL_DRAW_BUFFER8"),
    std::make_pair(0x882E, "GL_DRAW_BUFFER9"),
    std::make_pair(0x882F, "GL_DRAW_BUFFER10"),
    std::make_pair(0x8830, "GL_DRAW_BUFFER11"),
    std::make_pair(0x8831, "GL_DRAW_BUFFER12"),
    std::make_pair(0x8832, "GL_DRAW_BUFFER13"),
    std::make_pair(0x8833, "GL_DRAW_BUFFER14"),
    std::make_pair(0x8834, "GL_DRAW_BUFFER15"),
    std::make_pair(0x883D, "GL_BLEND_EQUATION_ALPHA"),
    std::make_pair(0x884A, "GL_TEXTURE_DEPTH_SIZE"),
    std::make_pair(0x884C, "GL_TEXTURE_COMPARE_MODE"),
    std::make_pair(0x884D, "GL_TEXTURE_COMPARE_FUNC"),
    std::make_pair(0x884E, "GL_COMPARE_REF_TO_TEXTURE"),
    std::make_pair(0x884F, "GL_TEXTURE_CUBE_MAP_SEAMLESS"),
    std::make_pair(0x8864, "GL_QUERY_COUNTER_BITS"),
    std::make_pair(0x8865, "GL_CURRENT_QUERY"),
    std::make_pair(0x8866, "GL_QUERY_RESULT"),
    std::make_pair(0x8867, "GL_QUERY_RESULT_AVAILABLE"),
    std::make_pair(0x8869, "GL_MAX_VERTEX_ATTRIBS"),
    std::make_pair(0x886A, "GL_VERTEX_ATTRIB_ARRAY_NORMALIZED"),
    std::make_pair(0x886C, "GL_MAX_TESS_CONTROL_INPUT_COMPONENTS"),
    std::make_pair(0x886D, "GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS"),
    std::make_pair(0x8872, "GL_MAX_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x887F, "GL_GEOMETRY_SHADER_INVOCATIONS"),
    std::make_pair(0x8892, "GL_ARRAY_BUFFER"),
    std::make_pair(0x8893, "GL_ELEMENT_ARRAY_BUFFER"),
    std::make_pair(0x8894, "GL_ARRAY_BUFFER_BINDING"),
    std::make_pair(0x8895, "GL_ELEMENT_ARRAY_BUFFER_BINDING"),
    std::make_pair(0x889F, "GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING"),
    std::make_pair(0x88B8, "GL_READ_ONLY"),
    std::make_pair(0x88B9, "GL_WRITE_ONLY"),
    std::make_pair(0x88BA, "GL_READ_WRITE"),
    std::make_pair(0x88BB, "GL_BUFFER_ACCESS"),
    std::make_pair(0x88BC, "GL_BUFFER_MAPPED"),
    std::make_pair(0x88BD, "GL_BUFFER_MAP_POINTER"),
    std::make_pair(0x88BF, "GL_TIME_ELAPSED"),
    std::make_pair(0x88E0, "GL_STREAM_DRAW"),
    std::make_pair(0x88E1, "GL_STREAM_READ"),
    std::make_pair(0x88E2, "GL_STREAM_COPY"),
    std::make_pair(0x88E4, "GL_STATIC_DRAW"),
    std::make_pair(0x88E5, "GL_STATIC_READ"),
    std::make_pair(0x88E6, "GL_STATIC_COPY"),
    std::make_pair(0x88E8, "GL_DYNAMIC_DRAW"),
    std::make_pair(0x88E9, "GL_DYNAMIC_READ"),
    std::make_pair(0x88EA, "GL_DYNAMIC_COPY"),
    std::make_pair(0x88EB, "GL_PIXEL_PACK_BUFFER"),
    std::make_pair(0x88EC, "GL_PIXEL_UNPACK_BUFFER"),
    std::make_pair(0x88ED, "GL_PIXEL_PACK_BUFFER_BINDING"),
    std::make_pair(0x88EF, "GL_PIXEL_UNPACK_BUFFER_BINDING"),
    std::make_pair(0x88F0, "GL_DEPTH24_STENCIL8"),
    std::make_pair(0x88F1, "GL_TEXTURE_STENCIL_SIZE"),
    std::make_pair(0x88F9, "GL_SRC1_COLOR"),
    std::make_pair(0x88FA, "GL_ONE_MINUS_SRC1_COLOR"),
    std::make_pair(0x88FB, "GL_ONE_MINUS_SRC1_ALPHA"),
    std::make_pair(0x88FC, "GL_MAX_DUAL_SOURCE_DRAW_BUFFERS"),
    std::make_pair(0x88FD, "GL_VERTEX_ATTRIB_ARRAY_INTEGER"),
    std::make_pair(0x88FE, "GL_VERTEX_ATTRIB_ARRAY_DIVISOR"),
    std::make_pair(0x88FF, "GL_MAX_ARRAY_TEXTURE_LAYERS"),
    std::make_pair(0x8904, "GL_MIN_PROGRAM_TEXEL_OFFSET"),
    std::make_pair(0x8905, "GL_MAX_PROGRAM_TEXEL_OFFSET"),
    std::make_pair(0x8914, "GL_SAMPLES_PASSED"),
    std::make_pair(0x8916, "GL_GEOMETRY_VERTICES_OUT"),
    std::make_pair(0x8917, "GL_GEOMETRY_INPUT_TYPE"),
    std::make_pair(0x8918, "GL_GEOMETRY_OUTPUT_TYPE"),
    std::make_pair(0x8919, "GL_SAMPLER_BINDING"),
    std::make_pair(0x891C, "GL_CLAMP_READ_COLOR"),
    std::make_pair(0x891D, "GL_FIXED_ONLY"),
    std::make_pair(0x8A11, "GL_UNIFORM_BUFFER"),
    std::make_pair(0x8A28, "GL_UNIFORM_BUFFER_BINDING"),
    std::make_pair(0x8A29, "GL_UNIFORM_BUFFER_START"),
    std::make_pair(0x8A2A, "GL_UNIFORM_BUFFER_SIZE"),
    std::make_pair(0x8A2B, "GL_MAX_VERTEX_UNIFORM_BLOCKS"),
    std::make_pair(0x8A2C, "GL_MAX_GEOMETRY_UNIFORM_BLOCKS"),
    std::make_pair(0x8A2D, "GL_MAX_FRAGMENT_UNIFORM_BLOCKS"),
    std::make_pair(0x8A2E, "GL_MAX_COMBINED_UNIFORM_BLOCKS"),
    std::make_pair(0x8A2F, "GL_MAX_UNIFORM_BUFFER_BINDINGS"),
    std::make_pair(0x8A30, "GL_MAX_UNIFORM_BLOCK_SIZE"),
    std::make_pair(0x8A31, "GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS"),
    std::make_pair(0x8A32, "GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS"),
    std::make_pair(0x8A33, "GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS"),
    std::make_pair(0x8A34, "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT"),
    std::make_pair(0x8A35, "GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH"),
    std::make_pair(0x8A36, "GL_ACTIVE_UNIFORM_BLOCKS"),
    std::make_pair(0x8A37, "GL_UNIFORM_TYPE"),
    std::make_pair(0x8A38, "GL_UNIFORM_SIZE"),
    std::make_pair(0x8A39, "GL_UNIFORM_NAME_LENGTH"),
    std::make_pair(0x8A3A, "GL_UNIFORM_BLOCK_INDEX"),
    std::make_pair(0x8A3B, "GL_UNIFORM_OFFSET"),
    std::make_pair(0x8A3C, "GL_UNIFORM_ARRAY_STRIDE"),
    std::make_pair(0x8A3D, "GL_UNIFORM_MATRIX_STRIDE"),
    std::make_pair(0x8A3E, "GL_UNIFORM_IS_ROW_MAJOR"),
    std::make_pair(0x8A3F, "GL_UNIFORM_BLOCK_BINDING"),
    std::make_pair(0x8A40, "GL_UNIFORM_BLOCK_DATA_SIZE"),
    std::make_pair(0x8A41, "GL_UNIFORM_BLOCK_NAME_LENGTH"),
    std::make_pair(0x8A42, "GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS"),
    std::make_pair(0x8A43, "GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES"),
    std::make_pair(0x8A44, "GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER"),
    std::make_pair(0x8A45, "GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER"),
    std::make_pair(0x8A46, "GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER"),
    std::make_pair(0x8B30, "GL_FRAGMENT_SHADER"),
    std::make_pair(0x8B31, "GL_VERTEX_SHADER"),
    std::make_pair(0x8B49, "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS"),
    std::make_pair(0x8B4A, "GL_MAX_VERTEX_UNIFORM_COMPONENTS"),
    std::make_pair(0x8B4B, "GL_MAX_VARYING_FLOATS"),
    std::make_pair(0x8B4B, "GL_MAX_VARYING_COMPONENTS"),
    std::make_pair(0x8B4C, "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x8B4D, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x8B4F, "GL_SHADER_TYPE"),
    std::make_pair(0x8B50, "GL_FLOAT_VEC2"),
    std::make_pair(0x8B51, "GL_FLOAT_VEC3"),
    std::make_pair(0x8B52, "GL_FLOAT_VEC4"),
    std::make_pair(0x8B53, "GL_INT_VEC2"),
    std::make_pair(0x8B54, "GL_INT_VEC3"),
    std::make_pair(0x8B55, "GL_INT_VEC4"),
    std::make_pair(0x8B56, "GL_BOOL"),
    std::make_pair(0x8B57, "GL_BOOL_VEC2"),
    std::make_pair(0x8B58, "GL_BOOL_VEC3"),
    std::make_pair(0x8B59, "GL_BOOL_VEC4"),
    std::make_pair(0x8B5A, "GL_FLOAT_MAT2"),
    std::make_pair(0x8B5B, "GL_FLOAT_MAT3"),
    std::make_pair(0x8B5C, "GL_FLOAT_MAT4"),
    std::make_pair(0x8B5D, "GL_SAMPLER_1D"),
    std::make_pair(0x8B5E, "GL_SAMPLER_2D"),
    std::make_pair(0x8B5F, "GL_SAMPLER_3D"),
    std::make_pair(0x8B60, "GL_SAMPLER_CUBE"),
    std::make_pair(0x8B61, "GL_SAMPLER_1D_SHADOW"),
    std::make_pair(0x8B62, "GL_SAMPLER_2D_SHADOW"),
    std::make_pair(0x8B63, "GL_SAMPLER_2D_RECT"),
    std::make_pair(0x8B64, "GL_SAMPLER_2D_RECT_SHADOW"),
    std::make_pair(0x8B65, "GL_FLOAT_MAT2x3"),
    std::make_pair(0x8B66, "GL_FLOAT_MAT2x4"),
    std::make_pair(0x8B67, "GL_FLOAT_MAT3x2"),
    std::make_pair(0x8B68, "GL_FLOAT_MAT3x4"),
    std::make_pair(0x8B69, "GL_FLOAT_MAT4x2"),
    std::make_pair(0x8B6A, "GL_FLOAT_MAT4x3"),
    std::make_pair(0x8B80, "GL_DELETE_STATUS"),
    std::make_pair(0x8B81, "GL_COMPILE_STATUS"),
    std::make_pair(0x8B82, "GL_LINK_STATUS"),
    std::make_pair(0x8B83, "GL_VALIDATE_STATUS"),
    std::make_pair(0x8B84, "GL_INFO_LOG_LENGTH"),
    std::make_pair(0x8B85, "GL_ATTACHED_SHADERS"),
    std::make_pair(0x8B86, "GL_ACTIVE_UNIFORMS"),
    std::make_pair(0x8B87, "GL_ACTIVE_UNIFORM_MAX_LENGTH"),
    std::make_pair(0x8B88, "GL_SHADER_SOURCE_LENGTH"),
    std::make_pair(0x8B89, "GL_ACTIVE_ATTRIBUTES"),
    std::make_pair(0x8B8A, "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH"),
    std::make_pair(0x8B8B, "GL_FRAGMENT_SHADER_DERIVATIVE_HINT"),
    std::make_pair(0x8B8C, "GL_SHADING_LANGUAGE_VERSION"),
    std::make_pair(0x8B8D, "GL_CURRENT_PROGRAM"),
    std::make_pair(0x8B9A, "GL_IMPLEMENTATION_COLOR_READ_TYPE"),
    std::make_pair(0x8B9B, "GL_IMPLEMENTATION_COLOR_READ_FORMAT"),
    std::make_pair(0x8C10, "GL_TEXTURE_RED_TYPE"),
    std::make_pair(0x8C11, "GL_TEXTURE_GREEN_TYPE"),
    std::make_pair(0x8C12, "GL_TEXTURE_BLUE_TYPE"),
    std::make_pair(0x8C13, "GL_TEXTURE_ALPHA_TYPE"),
    std::make_pair(0x8C16, "GL_TEXTURE_DEPTH_TYPE"),
    std::make_pair(0x8C17, "GL_UNSIGNED_NORMALIZED"),
    std::make_pair(0x8C18, "GL_TEXTURE_1D_ARRAY"),
    std::make_pair(0x8C19, "GL_PROXY_TEXTURE_1D_ARRAY"),
    std::make_pair(0x8C1A, "GL_TEXTURE_2D_ARRAY"),
    std::make_pair(0x8C1B, "GL_PROXY_TEXTURE_2D_ARRAY"),
    std::make_pair(0x8C1C, "GL_TEXTURE_BINDING_1D_ARRAY"),
    std::make_pair(0x8C1D, "GL_TEXTURE_BINDING_2D_ARRAY"),
    std::make_pair(0x8C29, "GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x8C2A, "GL_TEXTURE_BUFFER"),
    std::make_pair(0x8C2A, "GL_TEXTURE_BUFFER_BINDING"),
    std::make_pair(0x8C2B, "GL_MAX_TEXTURE_BUFFER_SIZE"),
    std::make_pair(0x8C2C, "GL_TEXTURE_BINDING_BUFFER"),
    std::make_pair(0x8C2D, "GL_TEXTURE_BUFFER_DATA_STORE_BINDING"),
    std::make_pair(0x8C2F, "GL_ANY_SAMPLES_PASSED"),
    std::make_pair(0x8C36, "GL_SAMPLE_SHADING"),
    std::make_pair(0x8C37, "GL_MIN_SAMPLE_SHADING_VALUE"),
    std::make_pair(0x8C3A, "GL_R11F_G11F_B10F"),
    std::make_pair(0x8C3B, "GL_UNSIGNED_INT_10F_11F_11F_REV"),
    std::make_pair(0x8C3D, "GL_RGB9_E5"),
    std::make_pair(0x8C3E, "GL_UNSIGNED_INT_5_9_9_9_REV"),
    std::make_pair(0x8C3F, "GL_TEXTURE_SHARED_SIZE"),
    std::make_pair(0x8C40, "GL_SRGB"),
    std::make_pair(0x8C41, "GL_SRGB8"),
    std::make_pair(0x8C42, "GL_SRGB_ALPHA"),
    std::make_pair(0x8C43, "GL_SRGB8_ALPHA8"),
    std::make_pair(0x8C48, "GL_COMPRESSED_SRGB"),
    std::make_pair(0x8C49, "GL_COMPRESSED_SRGB_ALPHA"),
    std::make_pair(0x8C76, "GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH"),
    std::make_pair(0x8C7F, "GL_TRANSFORM_FEEDBACK_BUFFER_MODE"),
    std::make_pair(0x8C80, "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS"),
    std::make_pair(0x8C83, "GL_TRANSFORM_FEEDBACK_VARYINGS"),
    std::make_pair(0x8C84, "GL_TRANSFORM_FEEDBACK_BUFFER_START"),
    std::make_pair(0x8C85, "GL_TRANSFORM_FEEDBACK_BUFFER_SIZE"),
    std::make_pair(0x8C87, "GL_PRIMITIVES_GENERATED"),
    std::make_pair(0x8C88, "GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN"),
    std::make_pair(0x8C89, "GL_RASTERIZER_DISCARD"),
    std::make_pair(0x8C8A, "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS"),
    std::make_pair(0x8C8B, "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS"),
    std::make_pair(0x8C8C, "GL_INTERLEAVED_ATTRIBS"),
    std::make_pair(0x8C8D, "GL_SEPARATE_ATTRIBS"),
    std::make_pair(0x8C8E, "GL_TRANSFORM_FEEDBACK_BUFFER"),
    std::make_pair(0x8C8F, "GL_TRANSFORM_FEEDBACK_BUFFER_BINDING"),
    std::make_pair(0x8CA0, "GL_POINT_SPRITE_COORD_ORIGIN"),
    std::make_pair(0x8CA1, "GL_LOWER_LEFT"),
    std::make_pair(0x8CA2, "GL_UPPER_LEFT"),
    std::make_pair(0x8CA3, "GL_STENCIL_BACK_REF"),
    std::make_pair(0x8CA4, "GL_STENCIL_BACK_VALUE_MASK"),
    std::make_pair(0x8CA5, "GL_STENCIL_BACK_WRITEMASK"),
    std::make_pair(0x8CA6, "GL_FRAMEBUFFER_BINDING"),
    std::make_pair(0x8CA6, "GL_DRAW_FRAMEBUFFER_BINDING"),
    std::make_pair(0x8CA7, "GL_RENDERBUFFER_BINDING"),
    std::make_pair(0x8CA8, "GL_READ_FRAMEBUFFER"),
    std::make_pair(0x8CA9, "GL_DRAW_FRAMEBUFFER"),
    std::make_pair(0x8CAA, "GL_READ_FRAMEBUFFER_BINDING"),
    std::make_pair(0x8CAB, "GL_RENDERBUFFER_SAMPLES"),
    std::make_pair(0x8CAC, "GL_DEPTH_COMPONENT32F"),
    std::make_pair(0x8CAD, "GL_DEPTH32F_STENCIL8"),
    std::make_pair(0x8CD0, "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE"),
    std::make_pair(0x8CD1, "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME"),
    std::make_pair(0x8CD2, "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL"),
    std::make_pair(0x8CD3, "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE"),
    std::make_pair(0x8CD4, "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER"),
    std::make_pair(0x8CD5, "GL_FRAMEBUFFER_COMPLETE"),
    std::make_pair(0x8CD6, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"),
    std::make_pair(0x8CD7, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"),
    std::make_pair(0x8CDB, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"),
    std::make_pair(0x8CDC, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"),
    std::make_pair(0x8CDD, "GL_FRAMEBUFFER_UNSUPPORTED"),
    std::make_pair(0x8CDF, "GL_MAX_COLOR_ATTACHMENTS"),
    std::make_pair(0x8CE0, "GL_COLOR_ATTACHMENT0"),
    std::make_pair(0x8CE1, "GL_COLOR_ATTACHMENT1"),
    std::make_pair(0x8CE2, "GL_COLOR_ATTACHMENT2"),
    std::make_pair(0x8CE3, "GL_COLOR_ATTACHMENT3"),
    std::make_pair(0x8CE4, "GL_COLOR_ATTACHMENT4"),
    std::make_pair(0x8CE5, "GL_COLOR_ATTACHMENT5"),
    std::make_pair(0x8CE6, "GL_COLOR_ATTACHMENT6"),
    std::make_pair(0x8CE7, "GL_COLOR_ATTACHMENT7"),
    std::make_pair(0x8CE8, "GL_COLOR_ATTACHMENT8"),
    std::make_pair(0x8CE9, "GL_COLOR_ATTACHMENT9"),
    std::make_pair(0x8CEA, "GL_COLOR_ATTACHMENT10"),
    std::make_pair(0x8CEB, "GL_COLOR_ATTACHMENT11"),
    std::make_pair(0x8CEC, "GL_COLOR_ATTACHMENT12"),
    std::make_pair(0x8CED, "GL_COLOR_ATTACHMENT13"),
    std::make_pair(0x8CEE, "GL_COLOR_ATTACHMENT14"),
    std::make_pair(0x8CEF, "GL_COLOR_ATTACHMENT15"),
    std::make_pair(0x8D00, "GL_DEPTH_ATTACHMENT"),
    std::make_pair(0x8D20, "GL_STENCIL_ATTACHMENT"),
    std::make_pair(0x8D40, "GL_FRAMEBUFFER"),
    std::make_pair(0x8D41, "GL_RENDERBUFFER"),
    std::make_pair(0x8D42, "GL_RENDERBUFFER_WIDTH"),
    std::make_pair(0x8D43, "GL_RENDERBUFFER_HEIGHT"),
    std::make_pair(0x8D44, "GL_RENDERBUFFER_INTERNAL_FORMAT"),
    std::make_pair(0x8D46, "GL_STENCIL_INDEX1"),
    std::make_pair(0x8D47, "GL_STENCIL_INDEX4"),
    std::make_pair(0x8D48, "GL_STENCIL_INDEX8"),
    std::make_pair(0x8D49, "GL_STENCIL_INDEX16"),
    std::make_pair(0x8D50, "GL_RENDERBUFFER_RED_SIZE"),
    std::make_pair(0x8D51, "GL_RENDERBUFFER_GREEN_SIZE"),
    std::make_pair(0x8D52, "GL_RENDERBUFFER_BLUE_SIZE"),
    std::make_pair(0x8D53, "GL_RENDERBUFFER_ALPHA_SIZE"),
    std::make_pair(0x8D54, "GL_RENDERBUFFER_DEPTH_SIZE"),
    std::make_pair(0x8D55, "GL_RENDERBUFFER_STENCIL_SIZE"),
    std::make_pair(0x8D56, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"),
    std::make_pair(0x8D57, "GL_MAX_SAMPLES"),
    std::make_pair(0x8D62, "GL_RGB565"),
    std::make_pair(0x8D69, "GL_PRIMITIVE_RESTART_FIXED_INDEX"),
    std::make_pair(0x8D6A, "GL_ANY_SAMPLES_PASSED_CONSERVATIVE"),
    std::make_pair(0x8D6B, "GL_MAX_ELEMENT_INDEX"),
    std::make_pair(0x8D70, "GL_RGBA32UI"),
    std::make_pair(0x8D71, "GL_RGB32UI"),
    std::make_pair(0x8D76, "GL_RGBA16UI"),
    std::make_pair(0x8D77, "GL_RGB16UI"),
    std::make_pair(0x8D7C, "GL_RGBA8UI"),
    std::make_pair(0x8D7D, "GL_RGB8UI"),
    std::make_pair(0x8D82, "GL_RGBA32I"),
    std::make_pair(0x8D83, "GL_RGB32I"),
    std::make_pair(0x8D88, "GL_RGBA16I"),
    std::make_pair(0x8D89, "GL_RGB16I"),
    std::make_pair(0x8D8E, "GL_RGBA8I"),
    std::make_pair(0x8D8F, "GL_RGB8I"),
    std::make_pair(0x8D94, "GL_RED_INTEGER"),
    std::make_pair(0x8D95, "GL_GREEN_INTEGER"),
    std::make_pair(0x8D96, "GL_BLUE_INTEGER"),
    std::make_pair(0x8D98, "GL_RGB_INTEGER"),
    std::make_pair(0x8D99, "GL_RGBA_INTEGER"),
    std::make_pair(0x8D9A, "GL_BGR_INTEGER"),
    std::make_pair(0x8D9B, "GL_BGRA_INTEGER"),
    std::make_pair(0x8D9F, "GL_INT_2_10_10_10_REV"),
    std::make_pair(0x8DA7, "GL_FRAMEBUFFER_ATTACHMENT_LAYERED"),
    std::make_pair(0x8DA8, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"),
    std::make_pair(0x8DAD, "GL_FLOAT_32_UNSIGNED_INT_24_8_REV"),
    std::make_pair(0x8DB9, "GL_FRAMEBUFFER_SRGB"),
    std::make_pair(0x8DBB, "GL_COMPRESSED_RED_RGTC1"),
    std::make_pair(0x8DBC, "GL_COMPRESSED_SIGNED_RED_RGTC1"),
    std::make_pair(0x8DBD, "GL_COMPRESSED_RG_RGTC2"),
    std::make_pair(0x8DBE, "GL_COMPRESSED_SIGNED_RG_RGTC2"),
    std::make_pair(0x8DC0, "GL_SAMPLER_1D_ARRAY"),
    std::make_pair(0x8DC1, "GL_SAMPLER_2D_ARRAY"),
    std::make_pair(0x8DC2, "GL_SAMPLER_BUFFER"),
    std::make_pair(0x8DC3, "GL_SAMPLER_1D_ARRAY_SHADOW"),
    std::make_pair(0x8DC4, "GL_SAMPLER_2D_ARRAY_SHADOW"),
    std::make_pair(0x8DC5, "GL_SAMPLER_CUBE_SHADOW"),
    std::make_pair(0x8DC6, "GL_UNSIGNED_INT_VEC2"),
    std::make_pair(0x8DC7, "GL_UNSIGNED_INT_VEC3"),
    std::make_pair(0x8DC8, "GL_UNSIGNED_INT_VEC4"),
    std::make_pair(0x8DC9, "GL_INT_SAMPLER_1D"),
    std::make_pair(0x8DCA, "GL_INT_SAMPLER_2D"),
    std::make_pair(0x8DCB, "GL_INT_SAMPLER_3D"),
    std::make_pair(0x8DCC, "GL_INT_SAMPLER_CUBE"),
    std::make_pair(0x8DCD, "GL_INT_SAMPLER_2D_RECT"),
    std::make_pair(0x8DCE, "GL_INT_SAMPLER_1D_ARRAY"),
    std::make_pair(0x8DCF, "GL_INT_SAMPLER_2D_ARRAY"),
    std::make_pair(0x8DD0, "GL_INT_SAMPLER_BUFFER"),
    std::make_pair(0x8DD1, "GL_UNSIGNED_INT_SAMPLER_1D"),
    std::make_pair(0x8DD2, "GL_UNSIGNED_INT_SAMPLER_2D"),
    std::make_pair(0x8DD3, "GL_UNSIGNED_INT_SAMPLER_3D"),
    std::make_pair(0x8DD4, "GL_UNSIGNED_INT_SAMPLER_CUBE"),
    std::make_pair(0x8DD5, "GL_UNSIGNED_INT_SAMPLER_2D_RECT"),
    std::make_pair(0x8DD6, "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY"),
    std::make_pair(0x8DD7, "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY"),
    std::make_pair(0x8DD8, "GL_UNSIGNED_INT_SAMPLER_BUFFER"),
    std::make_pair(0x8DD9, "GL_GEOMETRY_SHADER"),
    std::make_pair(0x8DDF, "GL_MAX_GEOMETRY_UNIFORM_COMPONENTS"),
    std::make_pair(0x8DE0, "GL_MAX_GEOMETRY_OUTPUT_VERTICES"),
    std::make_pair(0x8DE1, "GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS"),
    std::make_pair(0x8DE5, "GL_ACTIVE_SUBROUTINES"),
    std::make_pair(0x8DE6, "GL_ACTIVE_SUBROUTINE_UNIFORMS"),
    std::make_pair(0x8DE7, "GL_MAX_SUBROUTINES"),
    std::make_pair(0x8DE8, "GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS"),
    std::make_pair(0x8DF0, "GL_LOW_FLOAT"),
    std::make_pair(0x8DF1, "GL_MEDIUM_FLOAT"),
    std::make_pair(0x8DF2, "GL_HIGH_FLOAT"),
    std::make_pair(0x8DF3, "GL_LOW_INT"),
    std::make_pair(0x8DF4, "GL_MEDIUM_INT"),
    std::make_pair(0x8DF5, "GL_HIGH_INT"),
    std::make_pair(0x8DF8, "GL_SHADER_BINARY_FORMATS"),
    std::make_pair(0x8DF9, "GL_NUM_SHADER_BINARY_FORMATS"),
    std::make_pair(0x8DFA, "GL_SHADER_COMPILER"),
    std::make_pair(0x8DFB, "GL_MAX_VERTEX_UNIFORM_VECTORS"),
    std::make_pair(0x8DFC, "GL_MAX_VARYING_VECTORS"),
    std::make_pair(0x8DFD, "GL_MAX_FRAGMENT_UNIFORM_VECTORS"),
    std::make_pair(0x8E13, "GL_QUERY_WAIT"),
    std::make_pair(0x8E14, "GL_QUERY_NO_WAIT"),
    std::make_pair(0x8E15, "GL_QUERY_BY_REGION_WAIT"),
    std::make_pair(0x8E16, "GL_QUERY_BY_REGION_NO_WAIT"),
    std::make_pair(0x8E17, "GL_QUERY_WAIT_INVERTED"),
    std::make_pair(0x8E18, "GL_QUERY_NO_WAIT_INVERTED"),
    std::make_pair(0x8E19, "GL_QUERY_BY_REGION_WAIT_INVERTED"),
    std::make_pair(0x8E1A, "GL_QUERY_BY_REGION_NO_WAIT_INVERTED"),
    std::make_pair(0x8E1E, "GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS"),
    std::make_pair(0x8E1F, "GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS"),
    std::make_pair(0x8E22, "GL_TRANSFORM_FEEDBACK"),
    std::make_pair(0x8E23, "GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED"),
    std::make_pair(0x8E23, "GL_TRANSFORM_FEEDBACK_PAUSED"),
    std::make_pair(0x8E24, "GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE"),
    std::make_pair(0x8E24, "GL_TRANSFORM_FEEDBACK_ACTIVE"),
    std::make_pair(0x8E25, "GL_TRANSFORM_FEEDBACK_BINDING"),
    std::make_pair(0x8E28, "GL_TIMESTAMP"),
    std::make_pair(0x8E42, "GL_TEXTURE_SWIZZLE_R"),
    std::make_pair(0x8E43, "GL_TEXTURE_SWIZZLE_G"),
    std::make_pair(0x8E44, "GL_TEXTURE_SWIZZLE_B"),
    std::make_pair(0x8E45, "GL_TEXTURE_SWIZZLE_A"),
    std::make_pair(0x8E46, "GL_TEXTURE_SWIZZLE_RGBA"),
    std::make_pair(0x8E47, "GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS"),
    std::make_pair(0x8E48, "GL_ACTIVE_SUBROUTINE_MAX_LENGTH"),
    std::make_pair(0x8E49, "GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH"),
    std::make_pair(0x8E4A, "GL_NUM_COMPATIBLE_SUBROUTINES"),
    std::make_pair(0x8E4B, "GL_COMPATIBLE_SUBROUTINES"),
    std::make_pair(0x8E4C, "GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION"),
    std::make_pair(0x8E4D, "GL_FIRST_VERTEX_CONVENTION"),
    std::make_pair(0x8E4E, "GL_LAST_VERTEX_CONVENTION"),
    std::make_pair(0x8E4F, "GL_PROVOKING_VERTEX"),
    std::make_pair(0x8E50, "GL_SAMPLE_POSITION"),
    std::make_pair(0x8E51, "GL_SAMPLE_MASK"),
    std::make_pair(0x8E52, "GL_SAMPLE_MASK_VALUE"),
    std::make_pair(0x8E59, "GL_MAX_SAMPLE_MASK_WORDS"),
    std::make_pair(0x8E5A, "GL_MAX_GEOMETRY_SHADER_INVOCATIONS"),
    std::make_pair(0x8E5B, "GL_MIN_FRAGMENT_INTERPOLATION_OFFSET"),
    std::make_pair(0x8E5C, "GL_MAX_FRAGMENT_INTERPOLATION_OFFSET"),
    std::make_pair(0x8E5D, "GL_FRAGMENT_INTERPOLATION_OFFSET_BITS"),
    std::make_pair(0x8E5E, "GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET"),
    std::make_pair(0x8E5F, "GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET"),
    std::make_pair(0x8E70, "GL_MAX_TRANSFORM_FEEDBACK_BUFFERS"),
    std::make_pair(0x8E71, "GL_MAX_VERTEX_STREAMS"),
    std::make_pair(0x8E72, "GL_PATCH_VERTICES"),
    std::make_pair(0x8E73, "GL_PATCH_DEFAULT_INNER_LEVEL"),
    std::make_pair(0x8E74, "GL_PATCH_DEFAULT_OUTER_LEVEL"),
    std::make_pair(0x8E75, "GL_TESS_CONTROL_OUTPUT_VERTICES"),
    std::make_pair(0x8E76, "GL_TESS_GEN_MODE"),
    std::make_pair(0x8E77, "GL_TESS_GEN_SPACING"),
    std::make_pair(0x8E78, "GL_TESS_GEN_VERTEX_ORDER"),
    std::make_pair(0x8E79, "GL_TESS_GEN_POINT_MODE"),
    std::make_pair(0x8E7A, "GL_ISOLINES"),
    std::make_pair(0x8E7B, "GL_FRACTIONAL_ODD"),
    std::make_pair(0x8E7C, "GL_FRACTIONAL_EVEN"),
    std::make_pair(0x8E7D, "GL_MAX_PATCH_VERTICES"),
    std::make_pair(0x8E7E, "GL_MAX_TESS_GEN_LEVEL"),
    std::make_pair(0x8E7F, "GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS"),
    std::make_pair(0x8E80, "GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS"),
    std::make_pair(0x8E81, "GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x8E82, "GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x8E83, "GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS"),
    std::make_pair(0x8E84, "GL_MAX_TESS_PATCH_COMPONENTS"),
    std::make_pair(0x8E85, "GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS"),
    std::make_pair(0x8E86, "GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS"),
    std::make_pair(0x8E87, "GL_TESS_EVALUATION_SHADER"),
    std::make_pair(0x8E88, "GL_TESS_CONTROL_SHADER"),
    std::make_pair(0x8E89, "GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS"),
    std::make_pair(0x8E8A, "GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS"),
    std::make_pair(0x8E8C, "GL_COMPRESSED_RGBA_BPTC_UNORM"),
    std::make_pair(0x8E8D, "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM"),
    std::make_pair(0x8E8E, "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT"),
    std::make_pair(0x8E8F, "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT"),
    std::make_pair(0x8F36, "GL_COPY_READ_BUFFER"),
    std::make_pair(0x8F36, "GL_COPY_READ_BUFFER_BINDING"),
    std::make_pair(0x8F37, "GL_COPY_WRITE_BUFFER"),
    std::make_pair(0x8F37, "GL_COPY_WRITE_BUFFER_BINDING"),
    std::make_pair(0x8F38, "GL_MAX_IMAGE_UNITS"),
    std::make_pair(0x8F39, "GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS"),
    std::make_pair(0x8F39, "GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES"),
    std::make_pair(0x8F3A, "GL_IMAGE_BINDING_NAME"),
    std::make_pair(0x8F3B, "GL_IMAGE_BINDING_LEVEL"),
    std::make_pair(0x8F3C, "GL_IMAGE_BINDING_LAYERED"),
    std::make_pair(0x8F3D, "GL_IMAGE_BINDING_LAYER"),
    std::make_pair(0x8F3E, "GL_IMAGE_BINDING_ACCESS"),
    std::make_pair(0x8F3F, "GL_DRAW_INDIRECT_BUFFER"),
    std::make_pair(0x8F43, "GL_DRAW_INDIRECT_BUFFER_BINDING"),
    std::make_pair(0x8F46, "GL_DOUBLE_MAT2"),
    std::make_pair(0x8F47, "GL_DOUBLE_MAT3"),
    std::make_pair(0x8F48, "GL_DOUBLE_MAT4"),
    std::make_pair(0x8F49, "GL_DOUBLE_MAT2x3"),
    std::make_pair(0x8F4A, "GL_DOUBLE_MAT2x4"),
    std::make_pair(0x8F4B, "GL_DOUBLE_MAT3x2"),
    std::make_pair(0x8F4C, "GL_DOUBLE_MAT3x4"),
    std::make_pair(0x8F4D, "GL_DOUBLE_MAT4x2"),
    std::make_pair(0x8F4E, "GL_DOUBLE_MAT4x3"),
    std::make_pair(0x8F4F, "GL_VERTEX_BINDING_BUFFER"),
    std::make_pair(0x8F94, "GL_R8_SNORM"),
    std::make_pair(0x8F95, "GL_RG8_SNORM"),
    std::make_pair(0x8F96, "GL_RGB8_SNORM"),
    std::make_pair(0x8F97, "GL_RGBA8_SNORM"),
    std::make_pair(0x8F98, "GL_R16_SNORM"),
    std::make_pair(0x8F99, "GL_RG16_SNORM"),
    std::make_pair(0x8F9A, "GL_RGB16_SNORM"),
    std::make_pair(0x8F9B, "GL_RGBA16_SNORM"),
    std::make_pair(0x8F9C, "GL_SIGNED_NORMALIZED"),
    std::make_pair(0x8F9D, "GL_PRIMITIVE_RESTART"),
    std::make_pair(0x8F9E, "GL_PRIMITIVE_RESTART_INDEX"),
    std::make_pair(0x8FFC, "GL_DOUBLE_VEC2"),
    std::make_pair(0x8FFD, "GL_DOUBLE_VEC3"),
    std::make_pair(0x8FFE, "GL_DOUBLE_VEC4"),
    std::make_pair(0x9009, "GL_TEXTURE_CUBE_MAP_ARRAY"),
    std::make_pair(0x900A, "GL_TEXTURE_BINDING_CUBE_MAP_ARRAY"),
    std::make_pair(0x900B, "GL_PROXY_TEXTURE_CUBE_MAP_ARRAY"),
    std::make_pair(0x900C, "GL_SAMPLER_CUBE_MAP_ARRAY"),
    std::make_pair(0x900D, "GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW"),
    std::make_pair(0x900E, "GL_INT_SAMPLER_CUBE_MAP_ARRAY"),
    std::make_pair(0x900F, "GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY"),
    std::make_pair(0x904C, "GL_IMAGE_1D"),
    std::make_pair(0x904D, "GL_IMAGE_2D"),
    std::make_pair(0x904E, "GL_IMAGE_3D"),
    std::make_pair(0x904F, "GL_IMAGE_2D_RECT"),
    std::make_pair(0x9050, "GL_IMAGE_CUBE"),
    std::make_pair(0x9051, "GL_IMAGE_BUFFER"),
    std::make_pair(0x9052, "GL_IMAGE_1D_ARRAY"),
    std::make_pair(0x9053, "GL_IMAGE_2D_ARRAY"),
    std::make_pair(0x9054, "GL_IMAGE_CUBE_MAP_ARRAY"),
    std::make_pair(0x9055, "GL_IMAGE_2D_MULTISAMPLE"),
    std::make_pair(0x9056, "GL_IMAGE_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x9057, "GL_INT_IMAGE_1D"),
    std::make_pair(0x9058, "GL_INT_IMAGE_2D"),
    std::make_pair(0x9059, "GL_INT_IMAGE_3D"),
    std::make_pair(0x905A, "GL_INT_IMAGE_2D_RECT"),
    std::make_pair(0x905B, "GL_INT_IMAGE_CUBE"),
    std::make_pair(0x905C, "GL_INT_IMAGE_BUFFER"),
    std::make_pair(0x905D, "GL_INT_IMAGE_1D_ARRAY"),
    std::make_pair(0x905E, "GL_INT_IMAGE_2D_ARRAY"),
    std::make_pair(0x905F, "GL_INT_IMAGE_CUBE_MAP_ARRAY"),
    std::make_pair(0x9060, "GL_INT_IMAGE_2D_MULTISAMPLE"),
    std::make_pair(0x9061, "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x9062, "GL_UNSIGNED_INT_IMAGE_1D"),
    std::make_pair(0x9063, "GL_UNSIGNED_INT_IMAGE_2D"),
    std::make_pair(0x9064, "GL_UNSIGNED_INT_IMAGE_3D"),
    std::make_pair(0x9065, "GL_UNSIGNED_INT_IMAGE_2D_RECT"),
    std::make_pair(0x9066, "GL_UNSIGNED_INT_IMAGE_CUBE"),
    std::make_pair(0x9067, "GL_UNSIGNED_INT_IMAGE_BUFFER"),
    std::make_pair(0x9068, "GL_UNSIGNED_INT_IMAGE_1D_ARRAY"),
    std::make_pair(0x9069, "GL_UNSIGNED_INT_IMAGE_2D_ARRAY"),
    std::make_pair(0x906A, "GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY"),
    std::make_pair(0x906B, "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE"),
    std::make_pair(0x906C, "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x906D, "GL_MAX_IMAGE_SAMPLES"),
    std::make_pair(0x906E, "GL_IMAGE_BINDING_FORMAT"),
    std::make_pair(0x906F, "GL_RGB10_A2UI"),
    std::make_pair(0x90BC, "GL_MIN_MAP_BUFFER_ALIGNMENT"),
    std::make_pair(0x90C7, "GL_IMAGE_FORMAT_COMPATIBILITY_TYPE"),
    std::make_pair(0x90C8, "GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE"),
    std::make_pair(0x90C9, "GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS"),
    std::make_pair(0x90CA, "GL_MAX_VERTEX_IMAGE_UNIFORMS"),
    std::make_pair(0x90CB, "GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS"),
    std::make_pair(0x90CC, "GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS"),
    std::make_pair(0x90CD, "GL_MAX_GEOMETRY_IMAGE_UNIFORMS"),
    std::make_pair(0x90CE, "GL_MAX_FRAGMENT_IMAGE_UNIFORMS"),
    std::make_pair(0x90CF, "GL_MAX_COMBINED_IMAGE_UNIFORMS"),
    std::make_pair(0x90D2, "GL_SHADER_STORAGE_BUFFER"),
    std::make_pair(0x90D3, "GL_SHADER_STORAGE_BUFFER_BINDING"),
    std::make_pair(0x90D4, "GL_SHADER_STORAGE_BUFFER_START"),
    std::make_pair(0x90D5, "GL_SHADER_STORAGE_BUFFER_SIZE"),
    std::make_pair(0x90D6, "GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90D7, "GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90D8, "GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90D9, "GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90DA, "GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90DB, "GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90DC, "GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS"),
    std::make_pair(0x90DD, "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS"),
    std::make_pair(0x90DE, "GL_MAX_SHADER_STORAGE_BLOCK_SIZE"),
    std::make_pair(0x90DF, "GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT"),
    std::make_pair(0x90EA, "GL_DEPTH_STENCIL_TEXTURE_MODE"),
    std::make_pair(0x90EB, "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS"),
    std::make_pair(0x90EC, "GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER"),
    std::make_pair(0x90ED, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER"),
    std::make_pair(0x90EE, "GL_DISPATCH_INDIRECT_BUFFER"),
    std::make_pair(0x90EF, "GL_DISPATCH_INDIRECT_BUFFER_BINDING"),
    std::make_pair(0x9100, "GL_TEXTURE_2D_MULTISAMPLE"),
    std::make_pair(0x9101, "GL_PROXY_TEXTURE_2D_MULTISAMPLE"),
    std::make_pair(0x9102, "GL_TEXTURE_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x9103, "GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x9104, "GL_TEXTURE_BINDING_2D_MULTISAMPLE"),
    std::make_pair(0x9105, "GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x9106, "GL_TEXTURE_SAMPLES"),
    std::make_pair(0x9107, "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS"),
    std::make_pair(0x9108, "GL_SAMPLER_2D_MULTISAMPLE"),
    std::make_pair(0x9109, "GL_INT_SAMPLER_2D_MULTISAMPLE"),
    std::make_pair(0x910A, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE"),
    std::make_pair(0x910B, "GL_SAMPLER_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x910C, "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x910D, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY"),
    std::make_pair(0x910E, "GL_MAX_COLOR_TEXTURE_SAMPLES"),
    std::make_pair(0x910F, "GL_MAX_DEPTH_TEXTURE_SAMPLES"),
    std::make_pair(0x9110, "GL_MAX_INTEGER_SAMPLES"),
    std::make_pair(0x9111, "GL_MAX_SERVER_WAIT_TIMEOUT"),
    std::make_pair(0x9112, "GL_OBJECT_TYPE"),
    std::make_pair(0x9113, "GL_SYNC_CONDITION"),
    std::make_pair(0x9114, "GL_SYNC_STATUS"),
    std::make_pair(0x9115, "GL_SYNC_FLAGS"),
    std::make_pair(0x9116, "GL_SYNC_FENCE"),
    std::make_pair(0x9117, "GL_SYNC_GPU_COMMANDS_COMPLETE"),
    std::make_pair(0x9118, "GL_UNSIGNALED"),
    std::make_pair(0x9119, "GL_SIGNALED"),
    std::make_pair(0x911A, "GL_ALREADY_SIGNALED"),
    std::make_pair(0x911B, "GL_TIMEOUT_EXPIRED"),
    std::make_pair(0x911C, "GL_CONDITION_SATISFIED"),
    std::make_pair(0x911D, "GL_WAIT_FAILED"),
    std::make_pair(0x911F, "GL_BUFFER_ACCESS_FLAGS"),
    std::make_pair(0x9120, "GL_BUFFER_MAP_LENGTH"),
    std::make_pair(0x9121, "GL_BUFFER_MAP_OFFSET"),
    std::make_pair(0x9122, "GL_MAX_VERTEX_OUTPUT_COMPONENTS"),
    std::make_pair(0x9123, "GL_MAX_GEOMETRY_INPUT_COMPONENTS"),
    std::make_pair(0x9124, "GL_MAX_GEOMETRY_OUTPUT_COMPONENTS"),
    std::make_pair(0x9125, "GL_MAX_FRAGMENT_INPUT_COMPONENTS"),
    std::make_pair(0x9126, "GL_CONTEXT_PROFILE_MASK"),
    std::make_pair(0x9127, "GL_UNPACK_COMPRESSED_BLOCK_WIDTH"),
    std::make_pair(0x9128, "GL_UNPACK_COMPRESSED_BLOCK_HEIGHT"),
    std::make_pair(0x9129, "GL_UNPACK_COMPRESSED_BLOCK_DEPTH"),
    std::make_pair(0x912A, "GL_UNPACK_COMPRESSED_BLOCK_SIZE"),
    std::make_pair(0x912B, "GL_PACK_COMPRESSED_BLOCK_WIDTH"),
    std::make_pair(0x912C, "GL_PACK_COMPRESSED_BLOCK_HEIGHT"),
    std::make_pair(0x912D, "GL_PACK_COMPRESSED_BLOCK_DEPTH"),
    std::make_pair(0x912E, "GL_PACK_COMPRESSED_BLOCK_SIZE"),
    std::make_pair(0x912F, "GL_TEXTURE_IMMUTABLE_FORMAT"),
    std::make_pair(0x9143, "GL_MAX_DEBUG_MESSAGE_LENGTH"),
    std::make_pair(0x9144, "GL_MAX_DEBUG_LOGGED_MESSAGES"),
    std::make_pair(0x9145, "GL_DEBUG_LOGGED_MESSAGES"),
    std::make_pair(0x9146, "GL_DEBUG_SEVERITY_HIGH"),
    std::make_pair(0x9147, "GL_DEBUG_SEVERITY_MEDIUM"),
    std::make_pair(0x9148, "GL_DEBUG_SEVERITY_LOW"),
    std::make_pair(0x9192, "GL_QUERY_BUFFER"),
    std::make_pair(0x9193, "GL_QUERY_BUFFER_BINDING"),
    std::make_pair(0x9194, "GL_QUERY_RESULT_NO_WAIT"),
    std::make_pair(0x919D, "GL_TEXTURE_BUFFER_OFFSET"),
    std::make_pair(0x919E, "GL_TEXTURE_BUFFER_SIZE"),
    std::make_pair(0x919F, "GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT"),
    std::make_pair(0x91B9, "GL_COMPUTE_SHADER"),
    std::make_pair(0x91BB, "GL_MAX_COMPUTE_UNIFORM_BLOCKS"),
    std::make_pair(0x91BC, "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS"),
    std::make_pair(0x91BD, "GL_MAX_COMPUTE_IMAGE_UNIFORMS"),
    std::make_pair(0x91BE, "GL_MAX_COMPUTE_WORK_GROUP_COUNT"),
    std::make_pair(0x91BF, "GL_MAX_COMPUTE_WORK_GROUP_SIZE"),
    std::make_pair(0x9270, "GL_COMPRESSED_R11_EAC"),
    std::make_pair(0x9271, "GL_COMPRESSED_SIGNED_R11_EAC"),
    std::make_pair(0x9272, "GL_COMPRESSED_RG11_EAC"),
    std::make_pair(0x9273, "GL_COMPRESSED_SIGNED_RG11_EAC"),
    std::make_pair(0x9274, "GL_COMPRESSED_RGB8_ETC2"),
    std::make_pair(0x9275, "GL_COMPRESSED_SRGB8_ETC2"),
    std::make_pair(0x9276, "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2"),
    std::make_pair(0x9277, "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2"),
    std::make_pair(0x9278, "GL_COMPRESSED_RGBA8_ETC2_EAC"),
    std::make_pair(0x9279, "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC"),
    std::make_pair(0x92C0, "GL_ATOMIC_COUNTER_BUFFER"),
    std::make_pair(0x92C1, "GL_ATOMIC_COUNTER_BUFFER_BINDING"),
    std::make_pair(0x92C2, "GL_ATOMIC_COUNTER_BUFFER_START"),
    std::make_pair(0x92C3, "GL_ATOMIC_COUNTER_BUFFER_SIZE"),
    std::make_pair(0x92C4, "GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE"),
    std::make_pair(0x92C5, "GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS"),
    std::make_pair(0x92C6, "GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES"),
    std::make_pair(0x92C7, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER"),
    std::make_pair(0x92C8, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER"),
    std::make_pair(0x92C9, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER"),
    std::make_pair(0x92CA, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER"),
    std::make_pair(0x92CB, "GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER"),
    std::make_pair(0x92CC, "GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92CD, "GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92CE, "GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92CF, "GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92D0, "GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92D1, "GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92D2, "GL_MAX_VERTEX_ATOMIC_COUNTERS"),
    std::make_pair(0x92D3, "GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS"),
    std::make_pair(0x92D4, "GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS"),
    std::make_pair(0x92D5, "GL_MAX_GEOMETRY_ATOMIC_COUNTERS"),
    std::make_pair(0x92D6, "GL_MAX_FRAGMENT_ATOMIC_COUNTERS"),
    std::make_pair(0x92D7, "GL_MAX_COMBINED_ATOMIC_COUNTERS"),
    std::make_pair(0x92D8, "GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE"),
    std::make_pair(0x92D9, "GL_ACTIVE_ATOMIC_COUNTER_BUFFERS"),
    std::make_pair(0x92DA, "GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX"),
    std::make_pair(0x92DB, "GL_UNSIGNED_INT_ATOMIC_COUNTER"),
    std::make_pair(0x92DC, "GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS"),
    std::make_pair(0x92E0, "GL_DEBUG_OUTPUT"),
    std::make_pair(0x92E1, "GL_UNIFORM"),
    std::make_pair(0x92E2, "GL_UNIFORM_BLOCK"),
    std::make_pair(0x92E3, "GL_PROGRAM_INPUT"),
    std::make_pair(0x92E4, "GL_PROGRAM_OUTPUT"),
    std::make_pair(0x92E5, "GL_BUFFER_VARIABLE"),
    std::make_pair(0x92E6, "GL_SHADER_STORAGE_BLOCK"),
    std::make_pair(0x92E7, "GL_IS_PER_PATCH"),
    std::make_pair(0x92E8, "GL_VERTEX_SUBROUTINE"),
    std::make_pair(0x92E9, "GL_TESS_CONTROL_SUBROUTINE"),
    std::make_pair(0x92EA, "GL_TESS_EVALUATION_SUBROUTINE"),
    std::make_pair(0x92EB, "GL_GEOMETRY_SUBROUTINE"),
    std::make_pair(0x92EC, "GL_FRAGMENT_SUBROUTINE"),
    std::make_pair(0x92ED, "GL_COMPUTE_SUBROUTINE"),
    std::make_pair(0x92EE, "GL_VERTEX_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92EF, "GL_TESS_CONTROL_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92F0, "GL_TESS_EVALUATION_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92F1, "GL_GEOMETRY_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92F2, "GL_FRAGMENT_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92F3, "GL_COMPUTE_SUBROUTINE_UNIFORM"),
    std::make_pair(0x92F4, "GL_TRANSFORM_FEEDBACK_VARYING"),
    std::make_pair(0x92F5, "GL_ACTIVE_RESOURCES"),
    std::make_pair(0x92F6, "GL_MAX_NAME_LENGTH"),
    std::make_pair(0x92F7, "GL_MAX_NUM_ACTIVE_VARIABLES"),
    std::make_pair(0x92F8, "GL_MAX_NUM_COMPATIBLE_SUBROUTINES"),
    std::make_pair(0x92F9, "GL_NAME_LENGTH"),
    std::make_pair(0x92FA, "GL_TYPE"),
    std::make_pair(0x92FB, "GL_ARRAY_SIZE"),
    std::make_pair(0x92FC, "GL_OFFSET"),
    std::make_pair(0x92FD, "GL_BLOCK_INDEX"),
    std::make_pair(0x92FE, "GL_ARRAY_STRIDE"),
    std::make_pair(0x92FF, "GL_MATRIX_STRIDE"),
    std::make_pair(0x9300, "GL_IS_ROW_MAJOR"),
    std::make_pair(0x9301, "GL_ATOMIC_COUNTER_BUFFER_INDEX"),
    std::make_pair(0x9302, "GL_BUFFER_BINDING"),
    std::make_pair(0x9303, "GL_BUFFER_DATA_SIZE"),
    std::make_pair(0x9304, "GL_NUM_ACTIVE_VARIABLES"),
    std::make_pair(0x9305, "GL_ACTIVE_VARIABLES"),
    std::make_pair(0x9306, "GL_REFERENCED_BY_VERTEX_SHADER"),
    std::make_pair(0x9307, "GL_REFERENCED_BY_TESS_CONTROL_SHADER"),
    std::make_pair(0x9308, "GL_REFERENCED_BY_TESS_EVALUATION_SHADER"),
    std::make_pair(0x9309, "GL_REFERENCED_BY_GEOMETRY_SHADER"),
    std::make_pair(0x930A, "GL_REFERENCED_BY_FRAGMENT_SHADER"),
    std::make_pair(0x930B, "GL_REFERENCED_BY_COMPUTE_SHADER"),
    std::make_pair(0x930C, "GL_TOP_LEVEL_ARRAY_SIZE"),
    std::make_pair(0x930D, "GL_TOP_LEVEL_ARRAY_STRIDE"),
    std::make_pair(0x930E, "GL_LOCATION"),
    std::make_pair(0x930F, "GL_LOCATION_INDEX"),
    std::make_pair(0x9310, "GL_FRAMEBUFFER_DEFAULT_WIDTH"),
    std::make_pair(0x9311, "GL_FRAMEBUFFER_DEFAULT_HEIGHT"),
    std::make_pair(0x9312, "GL_FRAMEBUFFER_DEFAULT_LAYERS"),
    std::make_pair(0x9313, "GL_FRAMEBUFFER_DEFAULT_SAMPLES"),
    std::make_pair(0x9314, "GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS"),
    std::make_pair(0x9315, "GL_MAX_FRAMEBUFFER_WIDTH"),
    std::make_pair(0x9316, "GL_MAX_FRAMEBUFFER_HEIGHT"),
    std::make_pair(0x9317, "GL_MAX_FRAMEBUFFER_LAYERS"),
    std::make_pair(0x9318, "GL_MAX_FRAMEBUFFER_SAMPLES"),
    std::make_pair(0x934A, "GL_LOCATION_COMPONENT"),
    std::make_pair(0x934B, "GL_TRANSFORM_FEEDBACK_BUFFER_INDEX"),
    std::make_pair(0x934C, "GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE"),
    std::make_pair(0x935C, "GL_CLIP_ORIGIN"),
    std::make_pair(0x935D, "GL_CLIP_DEPTH_MODE"),
    std::make_pair(0x935E, "GL_NEGATIVE_ONE_TO_ONE"),
    std::make_pair(0x935F, "GL_ZERO_TO_ONE"),
    std::make_pair(0x9365, "GL_CLEAR_TEXTURE"),
    std::make_pair(0x9380, "GL_NUM_SAMPLE_COUNTS"),
    std::make_pair(0x9551, "GL_SHADER_BINARY_FORMAT_SPIR_V"),
    std::make_pair(0x9552, "GL_SPIR_V_BINARY"),
    std::make_pair(0x9552, "GL_PARAMETER_BUFFER"),
    std::make_pair(0x80EF, "GL_PARAMETER_BUFFER_BINDING"),
    std::make_pair(0x82EE, "GL_VERTICES_SUBMITTED"),
    std::make_pair(0x82EF, "GL_PRIMITIVES_SUBMITTED"),
    std::make_pair(0x82F0, "GL_VERTEX_SHADER_INVOCATIONS"),
    std::make_pair(0x82F1, "GL_TESS_CONTROL_SHADER_PATCHES"),
    std::make_pair(0x82F2, "GL_TESS_EVALUATION_SHADER_INVOCATIONS"),
    std::make_pair(0x82F3, "GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED"),
    std::make_pair(0x82F4, "GL_FRAGMENT_SHADER_INVOCATIONS"),
    std::make_pair(0x82F5, "GL_COMPUTE_SHADER_INVOCATIONS"),
    std::make_pair(0x82F6, "GL_CLIPPING_INPUT_PRIMITIVES"),
    std::make_pair(0x82F7, "GL_CLIPPING_OUTPUT_PRIMITIVES"),
    std::make_pair(0x8E1B, "GL_POLYGON_OFFSET_CLAMP"),
    std::make_pair(0x9553, "GL_SPIR_V_EXTENSIONS"),
    std::make_pair(0x9554, "GL_NUM_SPIR_V_EXTENSIONS"),
    std::make_pair(0x84FE, "GL_TEXTURE_MAX_ANISOTROPY"),
    std::make_pair(0x84FF, "GL_MAX_TEXTURE_MAX_ANISOTROPY"),
    std::make_pair(0x82EC, "GL_TRANSFORM_FEEDBACK_OVERFLOW"),
    std::make_pair(0x82ED, "GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW")
};
#endif

#if 0
// TO BE CLASSIFIED AND STORE AS STATIC VARIABLES IN GLTrace.
// Constants
#define	GL_ZERO	0
#define	GL_ONE	1

// Bits
#define	GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT	0x00000001
#define	GL_CONTEXT_CORE_PROFILE_BIT	0x00000001
#define	GL_SYNC_FLUSH_COMMANDS_BIT	0x00000001
#define	GL_VERTEX_SHADER_BIT	0x00000001
#define	GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT	0x00000001
#define	GL_CONTEXT_COMPATIBILITY_PROFILE_BIT	0x00000002
#define	GL_FRAGMENT_SHADER_BIT	0x00000002
#define	GL_ELEMENT_ARRAY_BARRIER_BIT	0x00000002
#define	GL_CONTEXT_FLAG_DEBUG_BIT	0x00000002
#define	GL_GEOMETRY_SHADER_BIT	0x00000004
#define	GL_UNIFORM_BARRIER_BIT	0x00000004
#define	GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT	0x00000004
#define	GL_TESS_CONTROL_SHADER_BIT	0x00000008
#define	GL_TEXTURE_FETCH_BARRIER_BIT	0x00000008
#define	GL_TESS_EVALUATION_SHADER_BIT	0x00000010
#define	GL_SHADER_IMAGE_ACCESS_BARRIER_BIT	0x00000020
#define	GL_COMPUTE_SHADER_BIT	0x00000020
#define	GL_COMMAND_BARRIER_BIT	0x00000040
#define	GL_PIXEL_BUFFER_BARRIER_BIT	0x00000080
#define	GL_DEPTH_BUFFER_BIT	0x00000100
#define	GL_TEXTURE_UPDATE_BARRIER_BIT	0x00000100
#define	GL_BUFFER_UPDATE_BARRIER_BIT	0x00000200
#define	GL_STENCIL_BUFFER_BIT	0x00000400
#define	GL_FRAMEBUFFER_BARRIER_BIT	0x00000400
#define	GL_TRANSFORM_FEEDBACK_BARRIER_BIT	0x00000800
#define	GL_ATOMIC_COUNTER_BARRIER_BIT	0x00001000
#define	GL_SHADER_STORAGE_BARRIER_BIT	0x00002000
#define	GL_COLOR_BUFFER_BIT	0x00004000
#define	GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT	0x00004000
#define	GL_QUERY_BUFFER_BARRIER_BIT	0x00008000
#define	GL_ALL_SHADER_BITS	0xFFFFFFFF
#define	GL_ALL_BARRIER_BITS	0xFFFFFFFF
#define	GL_TIMEOUT_IGNORED	0xFFFFFFFFFFFFFFFFull
#define	GL_INVALID_INDEX	0xFFFFFFFFu
#define	GL_DYNAMIC_STORAGE_BIT	0x0100
#define	GL_CLIENT_STORAGE_BIT	0x0200

#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_MAP_COHERENT_BIT               0x0080

#define GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008

#endif

