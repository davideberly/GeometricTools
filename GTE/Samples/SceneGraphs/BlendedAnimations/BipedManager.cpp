// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BipedManager.h"
#include <Applications/WICFileIO.h>
#include <Graphics/BlendTransformController.h>
#include <Graphics/KeyframeController.h>
#include <Mathematics/Logger.h>
#include <fstream>

BipedManager::BipedManager(std::string const& rootPath, std::string const& bname,
    std::shared_ptr<ProgramFactory> const& factory, BufferUpdater const& postUpdate)
{
    // Vertex format shared by the two skins.
    mVFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    // Create the texture effects for the two skins.
    std::shared_ptr<Texture2> texture[2];
    std::shared_ptr<Texture2Effect> effect[2];
    for (int32_t i = 0; i < 2; ++i)
    {
        std::string name = rootPath + "Skins/Skins" + std::to_string(i) + ".texture.png";
        texture[i] = WICFileIO::Load(name, true);
        texture[i]->AutogenerateMipmaps();
        effect[i] = std::make_shared<Texture2Effect>(factory, texture[i],
            SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
            SamplerState::Mode::WRAP);
    }

    PreSpatialArray preSpatialArray;
    PreSkinArray preSkinArray;
    SpatialMap spatialMap;

    std::string filename = rootPath + bname + ".txt";
    std::ifstream inFile(filename);
    while (!inFile.eof())
    {
        std::string line;
        getline(inFile, line);
        if (line == "")
        {
            // The file contains no blank lines, except for the very last one.
            break;
        }

        // Strip off initial white space.
        std::string::size_type begin = line.find_first_not_of(" ");
        if (begin > 0)
        {
            line = line.substr(begin);
        }

        std::string::size_type end;
        std::string name;
        if (line.find("Node") == 0)
        {
            // Load the node.
            begin = line.find("<");
            end = line.find(">");
            name = line.substr(begin + 1, end - 1 - begin);
            if (name.back() == 'X')
            {
                // TODO: These nodes are not necessary.  Remove them from the
                // data sets.
                continue;
            }

            auto preSpatial = LoadNode(rootPath, name);
            preSpatialArray.push_back(preSpatial);
            std::shared_ptr<Node> node = std::static_pointer_cast<Node>(preSpatial->Associate);
            spatialMap[name] = node;

            // Load the transform controllers.
            NodeCtrl nc;
            nc.first = node;

            nc.second = LoadTransformController(rootPath, name, "Idle");
            mIdleArray.push_back(nc);

            nc.second = LoadTransformController(rootPath, name, "Walk");
            mWalkArray.push_back(nc);

            nc.second = LoadTransformController(rootPath, name, "Run");
            mRunArray.push_back(nc);
        }
        else if (line.find("TriMesh") == 0)
        {
            // Load the mesh.
            begin = line.find("<");
            end = line.find(">");
            name = line.substr(begin + 1, end - 1 - begin);
            int32_t suffix = name[name.length() - 1] - '0';
            auto preSpatial = LoadMesh(rootPath, name, effect[suffix]);
            preSpatialArray.push_back(preSpatial);
            spatialMap[name] = preSpatial->Associate;

            // Load the skin controller.
            auto preSkin = LoadSkinController(rootPath, name, postUpdate);
            preSkinArray.push_back(preSkin);

            // Attach the skin controller to the mesh.
            preSpatial->Associate->AttachController(preSkin->Associate);
        }
    }

    // Resolve the bone links.
    for (auto const& preSkin : preSkinArray)
    {
        std::shared_ptr<SkinController> ctrl = preSkin->Associate;
        std::vector<std::weak_ptr<Node>>& bones = ctrl->GetBones();
        int32_t i = 0;
        for (auto const& boneName : preSkin->BoneNames)
        {
            auto iter = spatialMap.find(boneName);
            std::shared_ptr<Node> node = std::static_pointer_cast<Node>(iter->second);
            bones[i] = node;
            ++i;
        }
    }

    // Assemble the biped hierarchy.
    for (auto const& preSpatial : preSpatialArray)
    {
        std::shared_ptr<Node> node = std::dynamic_pointer_cast<Node>(preSpatial->Associate);
        if (node)
        {
            for (auto const& childName : preSpatial->ChildNames)
            {
                if (childName.back() == 'X')
                {
                    // TODO: These nodes are not necessary.  Remove them from
                    // the data sets.
                    continue;
                }

                auto iter = spatialMap.find(childName);
                std::shared_ptr<Visual> mesh = std::dynamic_pointer_cast<Visual>(iter->second);
                if (mesh)
                {
                    node->AttachChild(mesh);
                    if (mesh->GetEffect().get() == effect[0].get())
                    {
                        mSubscribers[0].first = mesh;
                        mSubscribers[0].second = effect[0]->GetPVWMatrixConstant();
                    }
                    else
                    {
                        mSubscribers[1].first = mesh;
                        mSubscribers[1].second = effect[1]->GetPVWMatrixConstant();
                    }
                }
                else
                {
                    node->AttachChild(std::shared_ptr<Spatial>(iter->second));
                }
            }
        }
    }

    mRoot = std::static_pointer_cast<Node>(preSpatialArray[0]->Associate);

    // Create the blend controllers.
    int32_t const numControllers = static_cast<int32_t>(mIdleArray.size());
    mIdleWalkArray.resize(numControllers);
    mWalkRunArray.resize(numControllers);
    for (int32_t i = 0; i < numControllers; ++i)
    {
        NodeCtrl const& nc0 = mIdleArray[i];
        NodeCtrl const& nc1 = mWalkArray[i];
        NodeCtrl const& nc2 = mRunArray[i];

        mIdleWalkArray[i].first = nc0.first;
        mIdleWalkArray[i].second = std::make_shared<BlendTransformController>(
            nc0.second, nc1.second, true, false);

        mWalkRunArray[i].first = nc0.first;
        mWalkRunArray[i].second = std::make_shared<BlendTransformController>(
            nc1.second, nc2.second, true, false);
    }
}

void BipedManager::Initialize(int32_t idleWalkCount, int32_t walkCount,
    int32_t walkRunCount)
{
    mState = Animation::IDLE;
    mCount = 0;
    mCountMax[Animation::IDLE] = 0;
    mCountMax[Animation::IDLE_WALK] = idleWalkCount;
    mCountMax[Animation::WALK] = mCountMax[Animation::IDLE_WALK] + walkCount;
    mCountMax[Animation::WALK_RUN] = mCountMax[Animation::WALK] + walkRunCount;
    mCountMax[Animation::RUN] = mCountMax[Animation::WALK_RUN];
    mWeight = 0.0f;
    mDeltaWeight0 = 1.0f / static_cast<float>(idleWalkCount);
    mDeltaWeight1 = 1.0f / static_cast<float>(walkRunCount);

    DoIdle();
}

void BipedManager::Update(bool blendIdleToWalk, bool blendWalkToRun)
{
    if (blendIdleToWalk)
    {
        if (mState == Animation::IDLE)
        {
            TransitionIdleToIdleWalk();
            mCount = 1;
            return;
        }

        if (mState == Animation::IDLE_WALK)
        {
            if (mCount++ < mCountMax[Animation::IDLE_WALK])
            {
                ContinueIdleWalk();
            }
            else
            {
                TransitionIdleWalkToWalk();
            }
            return;
        }

        if (mState == Animation::WALK)
        {
            if (blendWalkToRun)
            {
                if (mCount++ == mCountMax[Animation::WALK])
                {
                    TransitionWalkToWalkRun();
                }
            }
            else
            {
                // continue walk
            }
            return;
        }

        if (mState == Animation::WALK_RUN)
        {
            if (blendWalkToRun)
            {
                if (mCount++ < mCountMax[Animation::WALK_RUN])
                {
                    ContinueWalkRun();
                }
                else
                {
                    TransitionWalkRunToRun();
                }
            }
            else
            {
                if (--mCount > mCountMax[Animation::WALK])
                {
                    ContinueRunWalk();
                }
                else
                {
                    TransitionRunWalkToWalk();
                }
            }
            return;
        }

        if (mState == Animation::RUN)
        {
            if (blendWalkToRun)
            {
                // continue run
            }
            else
            {
                --mCount;
                TransitionRunToRunWalk();
            }
            return;
        }
    }
    else
    {
        if (mState == Animation::RUN)
        {
            --mCount;
            TransitionRunToRunWalk();
            return;
        }

        if (mState == Animation::WALK_RUN)
        {
            if (--mCount > mCountMax[Animation::WALK])
            {
                ContinueRunWalk();
            }
            else
            {
                TransitionRunWalkToWalk();
            }
            return;
        }

        if (mState == Animation::WALK)
        {
            if (--mCount == mCountMax[Animation::IDLE_WALK])
            {
                TransitionWalkToWalkIdle();
            }
            else
            {
                // continue walk
            }
            return;
        }

        if (mState == Animation::IDLE_WALK)
        {
            if (--mCount > mCountMax[Animation::IDLE])
            {
                ContinueWalkIdle();
            }
            else
            {
                TransitionWalkIdleToIdle();
            }
            return;
        }

        if (mState == Animation::IDLE)
        {
            // continue idle
            return;
        }
    }
}

float BipedManager::GetSpeed() const
{
    return
        static_cast<float>(mCount) / static_cast<float>(mCountMax[Animation::RUN]);
}

std::shared_ptr<BipedManager::PreSpatial> BipedManager::LoadNode(
    std::string const& rootPath, std::string const& name)
{
    std::shared_ptr<PreSpatial> preSpatial = std::make_shared<PreSpatial>();
    preSpatial->Associate = std::make_shared<Node>();
    preSpatial->Associate->name = name;

    std::string filename = rootPath + "Bones/" + name + ".node.raw";
    std::ifstream input(filename, std::ios::binary);

    int32_t numChildren{};
    input.read((char*)&numChildren, sizeof(numChildren));
    for (int32_t i = 0; i < numChildren; ++i)
    {
        int32_t length{};
        input.read((char*)&length, sizeof(length));
        std::vector<char> text(static_cast<size_t>(length) + 1);
        input.read(text.data(), sizeof(char) * length);
        text[length] = 0;
        preSpatial->ChildNames.push_back(std::string(text.data()));
    }

    input.close();
    return preSpatial;
}

std::shared_ptr<BipedManager::PreSpatial> BipedManager::LoadMesh(std::string const& rootPath,
    std::string const& name, std::shared_ptr<Texture2Effect> const& effect)
{
    std::string filename = rootPath + "Skins/" + name + ".mesh.raw";
    std::ifstream input(filename, std::ios::binary);

    struct VertexOnDisk
    {
        Vector3<float> position;
        Vector3<float> normal;  // Artist generated values, keeping on disk for now.
        Vector2<float> tcoord;
    };

    uint32_t numVertices{};
    input.read((char*)&numVertices, sizeof(numVertices));
    std::vector<VertexOnDisk> temp(numVertices);
    input.read((char*)temp.data(), numVertices * sizeof(VertexOnDisk));

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(
        mVFormat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertex[i].position = temp[i].position;
        vertex[i].tcoord = temp[i].tcoord;
    }

    int32_t numIndices{};
    input.read((char*)&numIndices, sizeof(numIndices));
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(
        IP_TRIMESH, numIndices / 3, sizeof(int32_t));
    input.read(ibuffer->GetData(), numIndices * sizeof(int32_t));
    input.close();

    std::shared_ptr<Visual> mesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mesh->name = name;

    std::shared_ptr<PreSpatial> preSpatial = std::make_shared<PreSpatial>();
    preSpatial->Associate = mesh;
    return preSpatial;
}

std::shared_ptr<BipedManager::PreSkin> BipedManager::LoadSkinController(
    std::string const& rootPath, std::string const& name, BufferUpdater const& postUpdate)
{
    std::string filename = rootPath + "Skins/" + name + ".skinctrl.raw";
    std::ifstream input(filename, std::ios::binary);

    int32_t repeatType{}, active{};
    double minTime{}, maxTime{}, phase{}, frequency{};
    input.read((char*)&repeatType, sizeof(repeatType));
    input.read((char*)&minTime, sizeof(minTime));
    input.read((char*)&maxTime, sizeof(maxTime));
    input.read((char*)&phase, sizeof(phase));
    input.read((char*)&frequency, sizeof(frequency));
    input.read((char*)&active, sizeof(active));

    int32_t numVertices{}, numBones{};
    input.read((char*)&numVertices, sizeof(numVertices));
    input.read((char*)&numBones, sizeof(numBones));

    std::shared_ptr<SkinController> ctrl = std::make_shared<SkinController>(
        numVertices, numBones, postUpdate);
    ctrl->name = name;
    std::shared_ptr<PreSkin> preSkin = std::make_shared<PreSkin>();
    preSkin->Associate = ctrl;

    ctrl->repeat = static_cast<Controller::RepeatType>(repeatType);
    ctrl->minTime = minTime;
    ctrl->maxTime = maxTime;
    ctrl->phase = phase;
    ctrl->frequency = frequency;
    ctrl->active = (active > 0);

    int32_t b, v;
    for (b = 0; b < numBones; ++b)
    {
        int32_t length{};
        input.read((char*)&length, sizeof(length));
        std::vector<char> text(static_cast<size_t>(length) + 1);
        input.read(text.data(), sizeof(char) * length);
        text[length] = 0;
        preSkin->BoneNames.push_back(std::string(text.data()));
    }

    float* weights = ctrl->GetWeights().data();
    Vector4<float>* offsets = ctrl->GetOffsets().data();
    for (v = 0; v < numVertices; ++v)
    {
        for (b = 0; b < numBones; ++b)
        {
            float weight{};
            input.read((char*)&weight, sizeof(weight));
            *weights++ = weight;
        }
    }

    Vector4<float> offset{ 0.0f, 0.0f, 0.0f, 1.0f };
    for (v = 0; v < numVertices; ++v)
    {
        for (b = 0; b < numBones; ++b)
        {
            input.read((char*)&offset[0], 3 * sizeof(offset[0]));
            *offsets++ = offset;
        }
    }

    input.close();
    return preSkin;
}

std::shared_ptr<TransformController> BipedManager::LoadTransformController(
    std::string const& rootPath, std::string const& name, std::string const& animation)
{
    std::string filename = rootPath + "Animations/" + animation + "/" +
        name + ".xfrmctrl.raw";
    std::ifstream input(filename, std::ios::binary);

    int32_t isKeyframeController{};
    input.read((char*)&isKeyframeController, sizeof(isKeyframeController));

    std::shared_ptr<TransformController> ctrl;
    int32_t repeatType{}, active{};
    double minTime{}, maxTime{}, phase{}, frequency{};
    input.read((char*)&repeatType, sizeof(repeatType));
    input.read((char*)&minTime, sizeof(minTime));
    input.read((char*)&maxTime, sizeof(maxTime));
    input.read((char*)&phase, sizeof(phase));
    input.read((char*)&frequency, sizeof(frequency));
    input.read((char*)&active, sizeof(active));

    std::array<float, 9> mat{};
    std::array<float, 3> trn{};
    std::array<float, 3> sca{};
    char isIdentity{}, isRSMatrix{}, isUniformScale{}, dummy{};
    input.read((char*)mat.data(), 9 * sizeof(float));
    input.read((char*)trn.data(), 3 * sizeof(float));
    input.read((char*)sca.data(), 3 * sizeof(float));
    input.read((char*)&isIdentity, sizeof(isIdentity));
    input.read((char*)&isRSMatrix, sizeof(isRSMatrix));
    input.read((char*)&isUniformScale, sizeof(isUniformScale));
    input.read((char*)&dummy, sizeof(dummy));

    Transform<float> localTransform;
    localTransform.SetTranslation(trn[0], trn[1], trn[2]);

    if (isUniformScale)
    {
        localTransform.SetUniformScale(sca[0]);
    }
    else
    {
        localTransform.SetScale(sca[0], sca[1], sca[2]);
    }

#if defined(GTE_USE_MAT_VEC)
    if (isRSMatrix)
    {
        localTransform.SetRotation(Matrix4x4<float>{
            mat[0], mat[1], mat[2], 0.0f,
            mat[3], mat[4], mat[5], 0.0f,
            mat[6], mat[7], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f });
    }
    else
    {
        localTransform.SetMatrix(Matrix4x4<float>{
            mat[0], mat[1], mat[2], 0.0f,
            mat[3], mat[4], mat[5], 0.0f,
            mat[6], mat[7], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f });
    }
#else
    if (isRSMatrix)
    {
        localTransform.SetRotation(Matrix4x4<float>{
            mat[0], mat[3], mat[6], 0.0f,
            mat[1], mat[4], mat[7], 0.0f,
            mat[2], mat[5], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f });
    }
    else
    {
        localTransform.SetMatrix(Matrix4x4<float>{
            mat[0], mat[3], mat[6], 0.0f,
            mat[1], mat[4], mat[7], 0.0f,
            mat[2], mat[5], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f });
    }
#endif

    if (isKeyframeController)
    {
        int32_t numTranslations{}, numRotations{}, numScales{};
        input.read((char*)&numTranslations, sizeof(numTranslations));
        input.read((char*)&numRotations, sizeof(numRotations));
        input.read((char*)&numScales, sizeof(numScales));

        std::shared_ptr<KeyframeController> keyfctrl = std::make_shared<KeyframeController>(
            0, numTranslations, numRotations, numScales, localTransform);

        if (numTranslations > 0)
        {
            input.read((char*)keyfctrl->GetTranslationTimes(),
                sizeof(float) * numTranslations);

            input.read((char*)keyfctrl->GetTranslations(),
                sizeof(Vector4<float>) * numTranslations);
        }

        if (numRotations > 0)
        {
            input.read((char*)keyfctrl->GetRotationTimes(),
                sizeof(float) * numRotations);

            input.read((char*)keyfctrl->GetRotations(),
                sizeof(Quaternion<float>) * numRotations);
        }

        if (numScales > 0)
        {
            input.read((char*)keyfctrl->GetScaleTimes(),
                sizeof(float) * numScales);

            input.read((char*)keyfctrl->GetScales(),
                sizeof(float) * numScales);
        }

        ctrl = keyfctrl;
    }
    else
    {
        // The adjustment to the "Biped" root node is an attempt to get the
        // biped to idle/walk/run on a floor at height zero.  The biped model
        // should have minimally been created so that the foot steps occur at
        // the same height for all animations.
        if (name == "Biped")
        {
            Vector3<float> translate = localTransform.GetTranslation();
            if (animation == "Idle")
            {
                translate[2] += 3.5f;
            }
            else if (animation == "Walk")
            {
                translate[2] += 2.0f;
            }
            else // animation == "Run"
            {
                translate[2] += 2.5f;
            }
            localTransform.SetTranslation(translate);
        }

        std::shared_ptr<TransformController> xfrmctrl =
            std::make_shared<TransformController>(localTransform);

        ctrl = xfrmctrl;
    }

    ctrl->name = name;
    ctrl->repeat = Controller::RepeatType::WRAP;
    ctrl->minTime = minTime;
    ctrl->maxTime = maxTime;
    ctrl->phase = phase;
    ctrl->frequency = frequency;
    ctrl->active = (active > 0);

    input.close();
    return ctrl;
}

void BipedManager::GetAnimation(NodeCtrlArray const& ncArray,
    double& minTime, double& maxTime) const
{
    minTime = std::numeric_limits<double>::max();
    maxTime = -minTime;
    for (auto const& nc : ncArray)
    {
        TransformController const* ctrl = nc.second.get();
        if (ctrl->minTime < minTime)
        {
            minTime = ctrl->minTime;
        }
        else if (ctrl->maxTime > maxTime)
        {
            maxTime = ctrl->maxTime;
        }
    }
}

void BipedManager::SetAnimation(NodeCtrlArray& ncArray, double frequency,
    double phase)
{
    for (auto& nc : ncArray)
    {
        TransformController* ctrl = nc.second.get();
        ctrl->frequency = frequency;
        ctrl->phase = phase;
    }
}

void BipedManager::SetBlendAnimation(NodeCtrlArray& ncArray, float weight)
{
    for (auto& nc : ncArray)
    {
        std::shared_ptr<BlendTransformController> ctrl =
            std::static_pointer_cast<BlendTransformController>(nc.second);
        ctrl->SetWeight(weight);
    }
}

void BipedManager::DoAnimation(NodeCtrlArray& ncArray)
{
    for (auto& nc : ncArray)
    {
        nc.second->repeat = Controller::RepeatType::WRAP;
        nc.first->DetachAllControllers();
        nc.first->AttachController(nc.second);
    }
}

void BipedManager::ContinueIdleWalk()
{
    SetIdleWalk(mWeight);
    mWeight += mDeltaWeight0;
    if (mWeight > 1.0f)
    {
        mWeight = 1.0f;
    }
}

void BipedManager::ContinueWalkRun()
{
    SetWalkRun(mWeight);
    mWeight += mDeltaWeight1;
    if (mWeight > 1.0f)
    {
        mWeight = 1.0f;
    }
}

void BipedManager::ContinueRunWalk()
{
    SetWalkRun(mWeight);
    mWeight -= mDeltaWeight1;
    if (mWeight < 0.0f)
    {
        mWeight = 0.0f;
    }
}

void BipedManager::ContinueWalkIdle()
{
    SetIdleWalk(mWeight);
    mWeight -= mDeltaWeight0;
    if (mWeight < 0.0f)
    {
        mWeight = 0.0f;
    }
}

void BipedManager::TransitionIdleToIdleWalk()
{
    mState = Animation::IDLE_WALK;
    DoIdleWalk();
    SetIdleWalk(0.0f);
    mWeight = mDeltaWeight0;
}

void BipedManager::TransitionIdleWalkToWalk()
{
    mState = Animation::WALK;
    DoWalk();
}

void BipedManager::TransitionWalkToWalkRun()
{
    mState = Animation::WALK_RUN;
    DoWalkRun();
    SetWalkRun(0.0f);
    mWeight = mDeltaWeight1;
}

void BipedManager::TransitionWalkRunToRun()
{
    mState = Animation::RUN;
    DoRun();
}

void BipedManager::TransitionRunToRunWalk()
{
    mState = Animation::WALK_RUN;
    DoWalkRun();
    SetWalkRun(1.0f);
    mWeight = 1.0f - mDeltaWeight1;
}

void BipedManager::TransitionRunWalkToWalk()
{
    mState = Animation::WALK;
    DoWalk();
}

void BipedManager::TransitionWalkToWalkIdle()
{
    mState = Animation::IDLE_WALK;
    DoIdleWalk();
    SetIdleWalk(1.0f);
    mWeight = 1.0f - mDeltaWeight0;
}

void BipedManager::TransitionWalkIdleToIdle()
{
    mState = Animation::IDLE;
    DoIdle();
}
