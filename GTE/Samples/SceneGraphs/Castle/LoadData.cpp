// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "CastleWindow3.h"

std::shared_ptr<Visual> CastleWindow3::LoadMeshPNT1(std::string const& name)
{
    // Get the positions, normals, and texture coordinates.
    std::string filename = mEnvironment.GetPath(name);
    std::ifstream inFile(filename);
    std::vector<Vector3<float>> positions, normals;
    std::vector<Vector2<float>> tcoords;
    GetTuple3(inFile, positions);
    GetTuple3(inFile, normals);
    GetTuple2(inFile, tcoords);

    // Get the vertices and indices.
    unsigned int numTriangles;
    inFile >> numTriangles;
    std::vector<LookupPNT1> lookups(3 * numTriangles);
    std::vector<LookupPNT1> PNT1Array;
    std::map<LookupPNT1, unsigned int> PNT1Map;
    std::vector<unsigned int> indices;
    for (unsigned int t = 0; t < numTriangles; ++t)
    {
        for (unsigned int j = 0, k = 3 * t; j < 3; ++j, ++k)
        {
            LookupPNT1& lookup = lookups[k];
            inFile >> lookup.PIndex;
            inFile >> lookup.NIndex;
            inFile >> lookup.TIndex;

            auto iter = PNT1Map.find(lookup);
            unsigned int index;
            if (iter != PNT1Map.end())
            {
                // Second or later time the vertex is encountered.
                index = iter->second;
            }
            else
            {
                // First time the vertex is encountered.
                index = static_cast<unsigned int>(PNT1Array.size());
                PNT1Map.insert(std::make_pair(lookup, index));
                PNT1Array.push_back(lookup);
            }
            indices.push_back(index);
        }
    }
    inFile.close();

    // Build the meshes.  Generate the unique vertices.  Keep track of indices
    // to remap the index buffers.
    unsigned int numVertices = static_cast<unsigned int>(PNT1Array.size());
    std::vector<unsigned int> remap(numVertices);
    std::map<VertexPNT1, std::vector<unsigned int>> uniqueVertices;
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        LookupPNT1& lookup = PNT1Array[i];
        VertexPNT1 vertex;
        vertex.position = positions[lookup.PIndex];
        vertex.normal = normals[lookup.NIndex];
        vertex.tcoord = tcoords[lookup.TIndex];
        auto iter = uniqueVertices.find(vertex);
        if (iter == uniqueVertices.end())
        {
            uniqueVertices.insert(std::make_pair(vertex, std::vector<unsigned int>{ i }));
        }
        else
        {
            iter->second.push_back(i);
        }
    }

    numVertices = static_cast<unsigned int>(uniqueVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(mPNT1Format, numVertices);
    VertexPNT1* vertex = vbuffer->Get<VertexPNT1>();
    unsigned int v = 0;
    for (auto const& element : uniqueVertices)
    {
        vertex[v] = element.first;
        for (auto index : element.second)
        {
            remap[index] = v;
        }
        ++v;
    }

    unsigned int numIndices = static_cast<unsigned int>(indices.size());
    for (auto& index : indices)
    {
        index = remap[index];
    }
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices, sizeof(unsigned int));
    std::memcpy(ibuffer->GetData(), indices.data(), numIndices * sizeof(unsigned int));

    std::shared_ptr<Visual> mesh = std::make_shared<Visual>(vbuffer, ibuffer);
    return mesh;
}

std::shared_ptr<Visual> CastleWindow3::LoadMeshPNT2(std::string const& name)
{
    // Get the positions, normals, and texture coordinates.
    std::string filename = mEnvironment.GetPath(name);
    std::ifstream inFile(filename);
    std::vector<Vector3<float>> positions, normals;
    std::vector<Vector2<float>> tcoords0, tcoords1;
    GetTuple3(inFile, positions);
    GetTuple3(inFile, normals);
    GetTuple2(inFile, tcoords0);
    GetTuple2(inFile, tcoords1);

    // Get the vertices and indices.
    unsigned int numTriangles;
    inFile >> numTriangles;
    std::vector<LookupPNT2> lookups(3 * numTriangles);
    std::vector<LookupPNT2> PNT2Array;
    std::map<LookupPNT2, unsigned int> PNT2Map;
    std::vector<unsigned int> indices;
    for (unsigned int t = 0; t < numTriangles; ++t)
    {
        for (unsigned int j = 0, k = 3 * t; j < 3; ++j, ++k)
        {
            LookupPNT2& lookup = lookups[k];
            inFile >> lookup.PIndex;
            inFile >> lookup.NIndex;
            inFile >> lookup.T0Index;
            inFile >> lookup.T1Index;

            auto iter = PNT2Map.find(lookup);
            unsigned int index;
            if (iter != PNT2Map.end())
            {
                // Second or later time the vertex is encountered.
                index = iter->second;
            }
            else
            {
                // First time the vertex is encountered.
                index = static_cast<unsigned int>(PNT2Array.size());
                PNT2Map.insert(std::make_pair(lookup, index));
                PNT2Array.push_back(lookup);
            }
            indices.push_back(index);
        }
    }
    inFile.close();

    // Build the meshes.  Generate the unique vertices.  Keep track of indices
    // to remap the index buffers.
    unsigned int numVertices = static_cast<unsigned int>(PNT2Array.size());
    std::vector<unsigned int> remap(numVertices);
    std::map<VertexPNT2, std::vector<unsigned int>> uniqueVertices;
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        LookupPNT2& lookup = PNT2Array[i];
        VertexPNT2 vertex;
        vertex.position = positions[lookup.PIndex];
        vertex.normal = normals[lookup.NIndex];
        vertex.tcoord0 = tcoords0[lookup.T0Index];
        vertex.tcoord1 = tcoords1[lookup.T1Index];
        auto iter = uniqueVertices.find(vertex);
        if (iter == uniqueVertices.end())
        {
            uniqueVertices.insert(std::make_pair(vertex, std::vector<unsigned int>{ i }));
        }
        else
        {
            iter->second.push_back(i);
        }
    }

    numVertices = static_cast<unsigned int>(uniqueVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(mPNT2Format, numVertices);
    VertexPNT2* vertex = vbuffer->Get<VertexPNT2>();
    unsigned int v = 0;
    for (auto const& element : uniqueVertices)
    {
        vertex[v] = element.first;
        for (auto index : element.second)
        {
            remap[index] = v;
        }
        ++v;
    }

    unsigned int numIndices = static_cast<unsigned int>(indices.size());
    for (auto& index : indices)
    {
        index = remap[index];
    }
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices, sizeof(unsigned int));
    std::memcpy(ibuffer->GetData(), indices.data(), numIndices * sizeof(unsigned int));

    std::shared_ptr<Visual> mesh = std::make_shared<Visual>(vbuffer, ibuffer);
    return mesh;
}

std::vector<std::shared_ptr<Visual>> CastleWindow3::LoadMeshPNT1Multi(std::string const& name)
{
    // Get the positions, normals, and texture coordinates.
    std::string filename = mEnvironment.GetPath(name);
    std::ifstream inFile(filename);
    std::vector<Vector3<float>> positions, normals;
    std::vector<Vector2<float>> tcoords;
    GetTuple3(inFile, positions);
    GetTuple3(inFile, normals);
    GetTuple2(inFile, tcoords);

    // Get the vertices and indices.
    unsigned int numMeshes;
    inFile >> numMeshes;
    std::vector<unsigned int> numTriangles(numMeshes);
    unsigned int numTotalTriangles = 0;
    for (unsigned int m = 0; m < numMeshes; ++m)
    {
        inFile >> numTriangles[m];
        numTotalTriangles += numTriangles[m];
    }

    std::vector<std::vector<unsigned int>> indices(numMeshes);
    std::vector<LookupPNT1> lookups(3 * numTotalTriangles);
    std::vector<LookupPNT1> PNT1Array;
    std::map<LookupPNT1, unsigned int> PNT1Map;
    for (unsigned int m = 0; m < numMeshes; ++m)
    {
        for (unsigned int t = 0; t < numTriangles[m]; ++t)
        {
            for (unsigned int j = 0, k = 3 * t; j < 3; ++j, ++k)
            {
                LookupPNT1& lookup = lookups[k];
                inFile >> lookup.PIndex;
                inFile >> lookup.NIndex;
                inFile >> lookup.TIndex;

                auto iter = PNT1Map.find(lookup);
                unsigned int index;
                if (iter != PNT1Map.end())
                {
                    // Second or later time the vertex is encountered.
                    index = iter->second;
                }
                else
                {
                    // First time the vertex is encountered.
                    index = static_cast<unsigned int>(PNT1Array.size());
                    PNT1Map.insert(std::make_pair(lookup, index));
                    PNT1Array.push_back(lookup);
                }
                indices[m].push_back(index);
            }
        }
    }
    inFile.close();

    // Build the meshes.  Generate the unique vertices.  Keep track of indices
    // to remap the index buffers.
    unsigned int numVertices = static_cast<unsigned int>(PNT1Array.size());
    std::vector<unsigned int> remap(numVertices);
    std::map<VertexPNT1, std::vector<unsigned int>> uniqueVertices;
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        LookupPNT1& lookup = PNT1Array[i];
        VertexPNT1 vertex;
        vertex.position = positions[lookup.PIndex];
        vertex.normal = normals[lookup.NIndex];
        vertex.tcoord = tcoords[lookup.TIndex];
        auto iter = uniqueVertices.find(vertex);
        if (iter == uniqueVertices.end())
        {
            uniqueVertices.insert(std::make_pair(vertex, std::vector<unsigned int>{ i }));
        }
        else
        {
            iter->second.push_back(i);
        }
    }

    numVertices = static_cast<unsigned int>(uniqueVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(mPNT1Format, numVertices);
    VertexPNT1* vertex = vbuffer->Get<VertexPNT1>();
    unsigned int v = 0;
    for (auto const& element : uniqueVertices)
    {
        vertex[v] = element.first;
        for (auto index : element.second)
        {
            remap[index] = v;
        }
        ++v;
    }

    std::vector<std::shared_ptr<Visual>> meshes(numMeshes);
    for (unsigned int m = 0; m < numMeshes; ++m)
    {
        unsigned int numIndices = static_cast<unsigned int>(indices[m].size());
        for (auto& index : indices[m])
        {
            index = remap[index];
        }
        auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numIndices, sizeof(unsigned int));
        std::memcpy(ibuffer->GetData(), indices[m].data(), numIndices * sizeof(unsigned int));
        meshes[m] = std::make_shared<Visual>(vbuffer, ibuffer);
    }

    return meshes;
}

void CastleWindow3::GetTuple3(std::ifstream& inFile, std::vector<Vector3<float>>& elements)
{
    int numElements;
    inFile >> numElements;
    elements.resize(numElements);
    for (int i = 0; i < numElements; ++i)
    {
        inFile >> elements[i][0];
        inFile >> elements[i][1];
        inFile >> elements[i][2];
    }
}

void CastleWindow3::GetTuple2(std::ifstream& inFile, std::vector<Vector2<float>>& elements)
{
    int numElements;
    inFile >> numElements;
    elements.resize(numElements);
    for (int i = 0; i < numElements; ++i)
    {
        inFile >> elements[i][0];
        inFile >> elements[i][1];
    }
}

CastleWindow3::LookupPNT1::LookupPNT1()
    :
    PIndex(-1),
    NIndex(-1),
    TIndex(-1)
{
}

bool CastleWindow3::LookupPNT1::operator< (LookupPNT1 const& other) const
{
    if (PIndex < other.PIndex) { return true; }
    if (PIndex > other.PIndex) { return false; }
    if (NIndex < other.NIndex) { return true; }
    if (NIndex > other.NIndex) { return false; }
    return TIndex < other.TIndex;
}

CastleWindow3::LookupPNT2::LookupPNT2()
    :
    PIndex(-1),
    NIndex(-1),
    T0Index(-1),
    T1Index(-1)
{
}

bool CastleWindow3::LookupPNT2::operator< (LookupPNT2 const& other) const
{
    if (PIndex  < other.PIndex) { return true; }
    if (PIndex  > other.PIndex) { return false; }
    if (NIndex  < other.NIndex) { return true; }
    if (NIndex  > other.NIndex) { return false; }
    if (T0Index < other.T0Index) { return true; }
    if (T0Index > other.T0Index) { return false; }
    return T1Index < other.T1Index;
}
