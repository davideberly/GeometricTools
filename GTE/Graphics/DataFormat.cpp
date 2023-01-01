// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DataFormat.h>
using namespace gte;

// A string version of the DF_* enumeration.
std::string const& DataFormat::GetName(uint32_t type)
{
    return msName[type];
}

// The number of bytes per struct.
uint32_t DataFormat::GetNumBytesPerStruct(uint32_t type)
{
    return msNumBytesPerStruct[type];
}

// The number of channels per struct.
uint32_t DataFormat::GetNumChannels(uint32_t type)
{
    return msNumChannels[type];
}

// The type of the channel.
uint32_t DataFormat::GetChannelType(uint32_t type)
{
    return msChannelType[type];
}

// The conversion semantics for the channel.  When true, signed
// integers are converted to floats in [-1,1] and unsigned integers
// are converted to floats in [0,1].  When false, integer data is
// converted directly to floats.
bool DataFormat::ConvertChannel(uint32_t type)
{
    return msConvertChannel[type];
}

// Not all data formats are currently supported.
bool DataFormat::IsSupported(uint32_t type)
{
    return msSupported[type];
}

// The struct has a depth format.
bool DataFormat::IsDepth(uint32_t type)
{
    return type == DF_D32_FLOAT_S8X24_UINT
        || type == DF_D32_FLOAT
        || type == DF_D24_UNORM_S8_UINT
        || type == DF_D16_UNORM;
}

std::array<std::string, DF_NUM_FORMATS> const DataFormat::msName =
{
    "UNKNOWN",
    "R32G32B32A32_TYPELESS",
    "R32G32B32A32_FLOAT",
    "R32G32B32A32_UINT",
    "R32G32B32A32_SINT",
    "R32G32B32_TYPELESS",
    "R32G32B32_FLOAT",
    "R32G32B32_UINT",
    "R32G32B32_SINT",
    "R16G16B16A16_TYPELESS",
    "R16G16B16A16_FLOAT",
    "R16G16B16A16_UNORM",
    "R16G16B16A16_UINT",
    "R16G16B16A16_SNORM",
    "R16G16B16A16_SINT",
    "R32G32_TYPELESS",
    "R32G32_FLOAT",
    "R32G32_UINT",
    "R32G32_SINT",
    "R32G8X24_TYPELESS",
    "D32_FLOAT_S8X24_UINT",
    "R32_FLOAT_X8X24_TYPELESS",
    "X32_TYPELESS_G8X24_UINT",
    "R10G10B10A2_TYPELESS",
    "R10G10B10A2_UNORM",
    "R10G10B10A2_UINT",
    "R11G11B10_FLOAT",
    "R8G8B8A8_TYPELESS",
    "R8G8B8A8_UNORM",
    "R8G8B8A8_UNORM_SRGB",
    "R8G8B8A8_UINT",
    "R8G8B8A8_SNORM",
    "R8G8B8A8_SINT",
    "R16G16_TYPELESS",
    "R16G16_FLOAT",
    "R16G16_UNORM",
    "R16G16_UINT",
    "R16G16_SNORM",
    "R16G16_SINT",
    "R32_TYPELESS",
    "D32_FLOAT",
    "R32_FLOAT",
    "R32_UINT",
    "R32_SINT",
    "R24G8_TYPELESS",
    "D24_UNORM_S8_UINT",
    "R24_UNORM_X8_TYPELESS",
    "X24_TYPELESS_G8_UINT",
    "R8G8_TYPELESS",
    "R8G8_UNORM",
    "R8G8_UINT",
    "R8G8_SNORM",
    "R8G8_SINT",
    "R16_TYPELESS",
    "R16_FLOAT",
    "D16_UNORM",
    "R16_UNORM",
    "R16_UINT",
    "R16_SNORM",
    "R16_SINT",
    "R8_TYPELESS",
    "R8_UNORM",
    "R8_UINT",
    "R8_SNORM",
    "R8_SINT",
    "A8_UNORM",
    "R1_UNORM",
    "R9G9B9E5_SHAREDEXP",
    "R8G8_B8G8_UNORM",
    "G8R8_G8B8_UNORM",
    "BC1_TYPELESS",
    "BC1_UNORM",
    "BC1_UNORM_SRGB",
    "BC2_TYPELESS",
    "BC2_UNORM",
    "BC2_UNORM_SRGB",
    "BC3_TYPELESS",
    "BC3_UNORM",
    "BC3_UNORM_SRGB",
    "BC4_TYPELESS",
    "BC4_UNORM",
    "BC4_SNORM",
    "BC5_TYPELESS",
    "BC5_UNORM",
    "BC5_SNORM",
    "B5G6R5_UNORM",
    "B5G5R5A1_UNORM",
    "B8G8R8A8_UNORM",
    "B8G8R8X8_UNORM",
    "R10G10B10_XR_BIAS_A2_UNORM",
    "B8G8R8A8_TYPELESS",
    "B8G8R8A8_UNORM_SRGB",
    "B8G8R8X8_TYPELESS",
    "B8G8R8X8_UNORM_SRGB",
    "BC6H_TYPELESS",
    "BC6H_UF16",
    "BC6H_SF16",
    "BC7_TYPELESS",
    "BC7_UNORM",
    "BC7_UNORM_SRGB",
    "AYUV",
    "Y410",
    "Y416",
    "NV12",
    "P010",
    "P016",
    "OPAQUE_420",
    "YUY2",
    "Y210",
    "Y216",
    "NV11",
    "AI44",
    "IA44",
    "P8",
    "A8P8",
    "B4G4R4A4_UNORM"
};

std::array<uint32_t, DF_NUM_FORMATS> const DataFormat::msNumBytesPerStruct =
{
    0,  // UNKNOWN
    16, // R32G32B32A32_TYPELESS
    16, // R32G32B32A32_FLOAT
    16, // R32G32B32A32_UINT
    16, // R32G32B32A32_SINT
    12, // R32G32B32_TYPELESS
    12, // R32G32B32_FLOAT
    12, // R32G32B32_UINT
    12, // R32G32B32_SINT
    8,  // R16G16B16A16_TYPELESS
    8,  // R16G16B16A16_FLOAT
    8,  // R16G16B16A16_UNORM
    8,  // R16G16B16A16_UINT
    8,  // R16G16B16A16_SNORM
    8,  // R16G16B16A16_SINT
    8,  // R32G32_TYPELESS
    8,  // R32G32_FLOAT
    8,  // R32G32_UINT
    8,  // R32G32_SINT
    8,  // R32G8X24_TYPELESS
    4,  // D32_FLOAT_S8X24_UINT
    4,  // R32_FLOAT_X8X24_TYPELESS
    4,  // X32_TYPELESS_G8X24_UINT
    4,  // R10G10B10A2_TYPELESS
    4,  // R10G10B10A2_UNORM
    4,  // R10G10B10A2_UINT
    4,  // R11G11B10_FLOAT
    4,  // R8G8B8A8_TYPELESS
    4,  // R8G8B8A8_UNORM
    4,  // R8G8B8A8_UNORM_SRGB
    4,  // R8G8B8A8_UINT
    4,  // R8G8B8A8_SNORM
    4,  // R8G8B8A8_SINT
    4,  // R16G16_TYPELESS
    4,  // R16G16_FLOAT
    4,  // R16G16_UNORM
    4,  // R16G16_UINT
    4,  // R16G16_SNORM
    4,  // R16G16_SINT
    4,  // R32_TYPELESS
    4,  // D32_FLOAT
    4,  // R32_FLOAT
    4,  // R32_UINT
    4,  // R32_SINT
    4,  // R24G8_TYPELESS
    4,  // D24_UNORM_S8_UINT
    4,  // R24_UNORM_X8_TYPELESS
    4,  // X24_TYPELESS_G8_UINT
    2,  // R8G8_TYPELESS
    2,  // R8G8_UNORM
    2,  // R8G8_UINT
    2,  // R8G8_SNORM
    2,  // R8G8_SINT
    2,  // R16_TYPELESS
    2,  // R16_FLOAT
    2,  // D16_UNORM
    2,  // R16_UNORM
    2,  // R16_UINT
    2,  // R16_SNORM
    2,  // R16_SINT
    1,  // R8_TYPELESS
    1,  // R8_UNORM
    1,  // R8_UINT
    1,  // R8_SNORM
    1,  // R8_SINT
    1,  // A8_UNORM
    0,  // R1_UNORM
    2,  // R9G9B9E5_SHAREDEXP
    2,  // R8G8_B8G8_UNORM
    2,  // G8R8_G8B8_UNORM
    0,  // BC1_TYPELESS
    0,  // BC1_UNORM
    0,  // BC1_UNORM_SRGB
    0,  // BC2_TYPELESS
    0,  // BC2_UNORM
    0,  // BC2_UNORM_SRGB
    0,  // BC3_TYPELESS
    0,  // BC3_UNORM
    0,  // BC3_UNORM_SRGB
    0,  // BC4_TYPELESS
    0,  // BC4_UNORM
    0,  // BC4_SNORM
    0,  // BC5_TYPELESS
    0,  // BC5_UNORM
    0,  // BC5_SNORM
    2,  // B5G6R5_UNORM
    2,  // B5G5R5A1_UNORM
    4,  // B8G8R8A8_UNORM
    4,  // B8G8R8X8_UNORM
    4,  // R10G10B10_XR_BIAS_A2_UNORM
    4,  // B8G8R8A8_TYPELESS
    4,  // B8G8R8A8_UNORM_SRGB
    4,  // B8G8R8X8_TYPELESS
    4,  // B8G8R8X8_UNORM_SRGB
    0,  // BC6H_TYPELESS
    0,  // BC6H_UF16
    0,  // BC6H_SF16
    0,  // BC7_TYPELESS
    0,  // BC7_UNORM
    0,  // BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine bytes per channel)
    0,  // AYUV
    0,  // Y410
    0,  // Y416
    0,  // NV12
    0,  // P010
    0,  // P016
    0,  // OPAQUE_420
    0,  // YUY2
    0,  // Y210
    0,  // Y216
    0,  // NV11
    0,  // AI44
    0,  // IA44
    0,  // P8
    0,  // A8P8
    0   // B4G4R4A4_UNORM
};

std::array<uint32_t, DF_NUM_FORMATS> const DataFormat::msNumChannels =
{
    0,  // UNKNOWN
    4,  // R32G32B32A32_TYPELESS
    4,  // R32G32B32A32_FLOAT
    4,  // R32G32B32A32_UINT
    4,  // R32G32B32A32_SINT
    3,  // R32G32B32_TYPELESS
    3,  // R32G32B32_FLOAT
    3,  // R32G32B32_UINT
    3,  // R32G32B32_SINT
    4,  // R16G16B16A16_TYPELESS
    4,  // R16G16B16A16_FLOAT
    4,  // R16G16B16A16_UNORM
    4,  // R16G16B16A16_UINT
    4,  // R16G16B16A16_SNORM
    4,  // R16G16B16A16_SINT
    2,  // R32G32_TYPELESS
    2,  // R32G32_FLOAT
    2,  // R32G32_UINT
    2,  // R32G32_SINT
    2,  // R32G8X24_TYPELESS
    2,  // D32_FLOAT_S8X24_UINT
    2,  // R32_FLOAT_X8X24_TYPELESS
    2,  // X32_TYPELESS_G8X24_UINT
    4,  // R10G10B10A2_TYPELESS
    4,  // R10G10B10A2_UNORM
    4,  // R10G10B10A2_UINT
    3,  // R11G11B10_FLOAT
    4,  // R8G8B8A8_TYPELESS
    4,  // R8G8B8A8_UNORM
    4,  // R8G8B8A8_UNORM_SRGB
    4,  // R8G8B8A8_UINT
    4,  // R8G8B8A8_SNORM
    4,  // R8G8B8A8_SINT
    2,  // R16G16_TYPELESS
    2,  // R16G16_FLOAT
    2,  // R16G16_UNORM
    2,  // R16G16_UINT
    2,  // R16G16_SNORM
    2,  // R16G16_SINT
    1,  // R32_TYPELESS
    1,  // D32_FLOAT
    1,  // R32_FLOAT
    1,  // R32_UINT
    1,  // R32_SINT
    2,  // R24G8_TYPELESS
    2,  // D24_UNORM_S8_UINT
    2,  // R24_UNORM_X8_TYPELESS
    2,  // X24_TYPELESS_G8_UINT
    2,  // R8G8_TYPELESS
    2,  // R8G8_UNORM
    2,  // R8G8_UINT
    2,  // R8G8_SNORM
    2,  // R8G8_SINT
    1,  // R16_TYPELESS
    1,  // R16_FLOAT
    1,  // D16_UNORM
    1,  // R16_UNORM
    1,  // R16_UINT
    1,  // R16_SNORM
    1,  // R16_SINT
    1,  // R8_TYPELESS
    1,  // R8_UNORM
    1,  // R8_UINT
    1,  // R8_SNORM
    1,  // R8_SINT
    1,  // A8_UNORM
    1,  // R1_UNORM
    4,  // R9G9B9E5_SHAREDEXP
    4,  // R8G8_B8G8_UNORM
    4,  // G8R8_G8B8_UNORM
    0,  // BC1_TYPELESS
    0,  // BC1_UNORM
    0,  // BC1_UNORM_SRGB
    0,  // BC2_TYPELESS
    0,  // BC2_UNORM
    0,  // BC2_UNORM_SRGB
    0,  // BC3_TYPELESS
    0,  // BC3_UNORM
    0,  // BC3_UNORM_SRGB
    0,  // BC4_TYPELESS
    0,  // BC4_UNORM
    0,  // BC4_SNORM
    0,  // BC5_TYPELESS
    0,  // BC5_UNORM
    0,  // BC5_SNORM
    2,  // B5G6R5_UNORM
    4,  // B5G5R5A1_UNORM
    4,  // B8G8R8A8_UNORM
    4,  // B8G8R8X8_UNORM
    4,  // R10G10B10_XR_BIAS_A2_UNORM
    4,  // B8G8R8A8_TYPELESS
    4,  // B8G8R8A8_UNORM_SRGB
    4,  // B8G8R8X8_TYPELESS
    4,  // B8G8R8X8_UNORM_SRGB
    0,  // BC6H_TYPELESS
    0,  // BC6H_UF16
    0,  // BC6H_SF16
    0,  // BC7_TYPELESS
    0,  // BC7_UNORM
    0,  // BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine number of channels)
    0,  // AYUV
    0,  // Y410
    0,  // Y416
    0,  // NV12
    0,  // P010
    0,  // P016
    0,  // OPAQUE_420
    0,  // YUY2
    0,  // Y210
    0,  // Y216
    0,  // NV11
    0,  // AI44
    0,  // IA44
    0,  // P8
    0,  // A8P8
    0   // B4G4R4A4_UNORM
};

std::array<DFChannelType, DF_NUM_FORMATS> const DataFormat::msChannelType =
{
    DF_UNSUPPORTED,     // UNKNOWN
    DF_UNSUPPORTED,     // R32G32B32A32_TYPELESS
    DF_FLOAT,           // R32G32B32A32_FLOAT
    DF_UINT,            // R32G32B32A32_UINT
    DF_INT,             // R32G32B32A32_SINT
    DF_UNSUPPORTED,     // R32G32B32_TYPELESS
    DF_FLOAT,           // R32G32B32_FLOAT
    DF_UINT,            // R32G32B32_UINT
    DF_INT,             // R32G32B32_SINT
    DF_UNSUPPORTED,     // R16G16B16A16_TYPELESS
    DF_HALF_FLOAT,      // R16G16B16A16_FLOAT
    DF_USHORT,          // R16G16B16A16_UNORM
    DF_USHORT,          // R16G16B16A16_UINT
    DF_SHORT,           // R16G16B16A16_SNORM
    DF_SHORT,           // R16G16B16A16_SINT
    DF_UNSUPPORTED,     // R32G32_TYPELESS
    DF_FLOAT,           // R32G32_FLOAT
    DF_UINT,            // R32G32_UINT
    DF_INT,             // R32G32_SINT
    DF_UNSUPPORTED,     // R32G8X24_TYPELESS
    DF_UNSUPPORTED,     // D32_FLOAT_S8X24_UINT
    DF_UNSUPPORTED,     // R32_FLOAT_X8X24_TYPELESS
    DF_UNSUPPORTED,     // X32_TYPELESS_G8X24_UINT
    DF_UNSUPPORTED,     // R10G10B10A2_TYPELESS
    DF_UINT_10_10_2,    // R10G10B10A2_UNORM
    DF_UINT_10_10_2,    // R10G10B10A2_UINT
    DF_FLOAT_11_11_10,  // R11G11B10_FLOAT
    DF_UNSUPPORTED,     // R8G8B8A8_TYPELESS
    DF_UBYTE,           // R8G8B8A8_UNORM
    DF_UBYTE,           // R8G8B8A8_UNORM_SRGB
    DF_UBYTE,           // R8G8B8A8_UINT
    DF_BYTE,            // R8G8B8A8_SNORM
    DF_BYTE,            // R8G8B8A8_SINT
    DF_UNSUPPORTED,     // R16G16_TYPELESS
    DF_FLOAT,           // R16G16_FLOAT
    DF_USHORT,          // R16G16_UNORM
    DF_USHORT,          // R16G16_UINT
    DF_SHORT,           // R16G16_SNORM
    DF_SHORT,           // R16G16_SINT
    DF_UNSUPPORTED,     // R32_TYPELESS
    DF_FLOAT,           // D32_FLOAT
    DF_FLOAT,           // R32_FLOAT
    DF_UINT,            // R32_UINT
    DF_INT,             // R32_SINT
    DF_UNSUPPORTED,     // R24G8_TYPELESS
    DF_UINT_24_8,       // D24_UNORM_S8_UINT
    DF_UNSUPPORTED,     // R24_UNORM_X8_TYPELESS
    DF_UNSUPPORTED,     // X24_TYPELESS_G8_UINT
    DF_UNSUPPORTED,     // R8G8_TYPELESS
    DF_UBYTE,           // R8G8_UNORM
    DF_UBYTE,           // R8G8_UINT
    DF_BYTE,            // R8G8_SNORM
    DF_BYTE,            // R8G8_SINT
    DF_UNSUPPORTED,     // R16_TYPELESS
    DF_HALF_FLOAT,      // R16_FLOAT
    DF_USHORT,          // D16_UNORM
    DF_USHORT,          // R16_UNORM
    DF_USHORT,          // R16_UINT
    DF_SHORT,           // R16_SNORM
    DF_SHORT,           // R16_SINT
    DF_UNSUPPORTED,     // R8_TYPELESS
    DF_UBYTE,           // R8_UNORM
    DF_UBYTE,           // R8_UINT
    DF_BYTE,            // R8_SNORM
    DF_BYTE,            // R8_SINT
    DF_UNSUPPORTED,     // A8_UNORM
    DF_UNSUPPORTED,     // R1_UNORM
    DF_UNSUPPORTED,     // R9G9B9E5_SHAREDEXP
    DF_UNSUPPORTED,     // R8G8_B8G8_UNORM
    DF_UNSUPPORTED,     // G8R8_G8B8_UNORM
    DF_UNSUPPORTED,     // BC1_TYPELESS
    DF_UNSUPPORTED,     // BC1_UNORM
    DF_UNSUPPORTED,     // BC1_UNORM_SRGB
    DF_UNSUPPORTED,     // BC2_TYPELESS
    DF_UNSUPPORTED,     // BC2_UNORM
    DF_UNSUPPORTED,     // BC2_UNORM_SRGB
    DF_UNSUPPORTED,     // BC3_TYPELESS
    DF_UNSUPPORTED,     // BC3_UNORM
    DF_UNSUPPORTED,     // BC3_UNORM_SRGB
    DF_UNSUPPORTED,     // BC4_TYPELESS
    DF_UNSUPPORTED,     // BC4_UNORM
    DF_UNSUPPORTED,     // BC4_SNORM
    DF_UNSUPPORTED,     // BC5_TYPELESS
    DF_UNSUPPORTED,     // BC5_UNORM
    DF_UNSUPPORTED,     // BC5_SNORM
    DF_UNSUPPORTED,     // B5G6R5_UNORM
    DF_UNSUPPORTED,     // B5G5R5A1_UNORM
    DF_UNSUPPORTED,     // B8G8R8A8_UNORM
    DF_UNSUPPORTED,     // B8G8R8X8_UNORM
    DF_UNSUPPORTED,     // R10G10B10_XR_BIAS_A2_UNORM
    DF_UNSUPPORTED,     // B8G8R8A8_TYPELESS
    DF_UNSUPPORTED,     // B8G8R8A8_UNORM_SRGB
    DF_UNSUPPORTED,     // B8G8R8X8_TYPELESS
    DF_UNSUPPORTED,     // B8G8R8X8_UNORM_SRGB
    DF_UNSUPPORTED,     // BC6H_TYPELESS
    DF_UNSUPPORTED,     // BC6H_UF16
    DF_UNSUPPORTED,     // BC6H_SF16
    DF_UNSUPPORTED,     // BC7_TYPELESS
    DF_UNSUPPORTED,     // BC7_UNORM
    DF_UNSUPPORTED,      // BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine whether we will support these)
    DF_UNSUPPORTED,     // AYUV
    DF_UNSUPPORTED,     // Y410
    DF_UNSUPPORTED,     // Y416
    DF_UNSUPPORTED,     // NV12
    DF_UNSUPPORTED,     // P010
    DF_UNSUPPORTED,     // P016
    DF_UNSUPPORTED,     // OPAQUE_420
    DF_UNSUPPORTED,     // YUY2
    DF_UNSUPPORTED,     // Y210
    DF_UNSUPPORTED,     // Y216
    DF_UNSUPPORTED,     // NV11
    DF_UNSUPPORTED,     // AI44
    DF_UNSUPPORTED,     // IA44
    DF_UNSUPPORTED,     // P8
    DF_UNSUPPORTED,     // A8P8
    DF_UNSUPPORTED      // B4G4R4A4_UNORM
};

std::array<bool, DF_NUM_FORMATS> const DataFormat::msConvertChannel =
{
    false,  // UNKNOWN
    false,  // R32G32B32A32_TYPELESS
    false,  // R32G32B32A32_FLOAT
    false,  // R32G32B32A32_UINT
    false,  // R32G32B32A32_SINT
    false,  // R32G32B32_TYPELESS
    false,  // R32G32B32_FLOAT
    false,  // R32G32B32_UINT
    false,  // R32G32B32_SINT
    false,  // R16G16B16A16_TYPELESS
    false,  // R16G16B16A16_FLOAT
    true,   // R16G16B16A16_UNORM
    false,  // R16G16B16A16_UINT
    true,   // R16G16B16A16_SNORM
    false,  // R16G16B16A16_SINT
    false,  // R32G32_TYPELESS
    false,  // R32G32_FLOAT
    false,  // R32G32_UINT
    false,  // R32G32_SINT
    false,  // R32G8X24_TYPELESS
    false,  // D32_FLOAT_S8X24_UINT
    false,  // R32_FLOAT_X8X24_TYPELESS
    false,  // X32_TYPELESS_G8X24_UINT
    false,  // R10G10B10A2_TYPELESS
    true,   // R10G10B10A2_UNORM
    false,  // R10G10B10A2_UINT
    false,  // R11G11B10_FLOAT
    false,  // R8G8B8A8_TYPELESS
    true,   // R8G8B8A8_UNORM
    true,   // R8G8B8A8_UNORM_SRGB
    false,  // R8G8B8A8_UINT
    true,   // R8G8B8A8_SNORM
    false,  // R8G8B8A8_SINT
    false,  // R16G16_TYPELESS
    false,  // R16G16_FLOAT
    true,   // R16G16_UNORM
    false,  // R16G16_UINT
    true,   // R16G16_SNORM
    false,  // R16G16_SINT
    false,  // R32_TYPELESS
    false,  // D32_FLOAT
    false,  // R32_FLOAT
    false,  // R32_UINT
    false,  // R32_SINT
    false,  // R24G8_TYPELESS
    false,  // D24_UNORM_S8_UINT
    false,  // R24_UNORM_X8_TYPELESS
    false,  // X24_TYPELESS_G8_UINT
    false,  // R8G8_TYPELESS
    true,   // R8G8_UNORM
    false,  // R8G8_UINT
    true,   // R8G8_SNORM
    false,  // R8G8_SINT
    false,  // R16_TYPELESS
    false,  // R16_FLOAT
    true,   // D16_UNORM
    true,   // R16_UNORM
    false,  // R16_UINT
    true,   // R16_SNORM
    false,  // R16_SINT
    false,  // R8_TYPELESS
    true,   // R8_UNORM
    false,  // R8_UINT
    true,   // R8_SNORM
    false,  // R8_SINT
    true,   // A8_UNORM
    true,   // R1_UNORM
    false,  // R9G9B9E5_SHAREDEXP
    true,   // R8G8_B8G8_UNORM
    true,   // G8R8_G8B8_UNORM
    false,  // BC1_TYPELESS
    true,   // BC1_UNORM
    true,   // BC1_UNORM_SRGB
    false,  // BC2_TYPELESS
    true,   // BC2_UNORM
    true,   // BC2_UNORM_SRGB
    false,  // BC3_TYPELESS
    true,   // BC3_UNORM
    true,   // BC3_UNORM_SRGB
    false,  // BC4_TYPELESS
    true,   // BC4_UNORM
    true,   // BC4_SNORM
    false,  // BC5_TYPELESS
    true,   // BC5_UNORM
    true,   // BC5_SNORM
    true,   // B5G6R5_UNORM
    true,   // B5G5R5A1_UNORM
    true,   // B8G8R8A8_UNORM
    true,   // B8G8R8X8_UNORM
    true,   // R10G10B10_XR_BIAS_A2_UNORM
    false,  // B8G8R8A8_TYPELESS
    true,   // B8G8R8A8_UNORM_SRGB
    false,  // B8G8R8X8_TYPELESS
    true,   // B8G8R8X8_UNORM_SRGB
    false,  // BC6H_TYPELESS
    false,  // BC6H_UF16
    false,  // BC6H_SF16
    false,  // BC7_TYPELESS
    true,   // BC7_UNORM
    true,   // BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine the appropriate bool value)
    false,  // AYUV
    false,  // Y410
    false,  // Y416
    false,  // NV12
    false,  // P010
    false,  // P016
    false,  // OPAQUE_420
    false,  // YUY2
    false,  // Y210
    false,  // Y216
    false,  // NV11
    false,  // AI44
    false,  // IA44
    false,  // P8
    false,  // A8P8
    false   // B4G4R4A4_UNORM
};

std::array<bool, DF_NUM_FORMATS> const DataFormat::msSupported =
{
    false,  // UNKNOWN
    true,   // R32G32B32A32_TYPELESS
    true,   // R32G32B32A32_FLOAT
    true,   // R32G32B32A32_UINT
    true,   // R32G32B32A32_SINT
    true,   // R32G32B32_TYPELESS
    true,   // R32G32B32_FLOAT
    true,   // R32G32B32_UINT
    true,   // R32G32B32_SINT
    true,   // R16G16B16A16_TYPELESS
    true,   // R16G16B16A16_FLOAT
    true,   // R16G16B16A16_UNORM
    true,   // R16G16B16A16_UINT
    true,   // R16G16B16A16_SNORM
    true,   // R16G16B16A16_SINT
    true,   // R32G32_TYPELESS
    true,   // R32G32_FLOAT
    true,   // R32G32_UINT
    true,   // R32G32_SINT
    true,   // R32G8X24_TYPELESS
    true,   // D32_FLOAT_S8X24_UINT
    true,   // R32_FLOAT_X8X24_TYPELESS
    true,   // X32_TYPELESS_G8X24_UINT
    true,   // R10G10B10A2_TYPELESS
    true,   // R10G10B10A2_UNORM
    true,   // R10G10B10A2_UINT
    true,   // R11G11B10_FLOAT
    true,   // R8G8B8A8_TYPELESS
    true,   // R8G8B8A8_UNORM
    true,   // R8G8B8A8_UNORM_SRGB
    true,   // R8G8B8A8_UINT
    true,   // R8G8B8A8_SNORM
    true,   // R8G8B8A8_SINT
    true,   // R16G16_TYPELESS
    true,   // R16G16_FLOAT
    true,   // R16G16_UNORM
    true,   // R16G16_UINT
    true,   // R16G16_SNORM
    true,   // R16G16_SINT
    true,   // R32_TYPELESS
    true,   // D32_FLOAT
    true,   // R32_FLOAT
    true,   // R32_UINT
    true,   // R32_SINT
    true,   // R24G8_TYPELESS
    true,   // D24_UNORM_S8_UINT
    true,   // R24_UNORM_X8_TYPELESS
    true,   // X24_TYPELESS_G8_UINT
    true,   // R8G8_TYPELESS
    true,   // R8G8_UNORM
    true,   // R8G8_UINT
    true,   // R8G8_SNORM
    true,   // R8G8_SINT
    true,   // R16_TYPELESS
    true,   // R16_FLOAT
    true,   // D16_UNORM
    true,   // R16_UNORM
    true,   // R16_UINT
    true,   // R16_SNORM
    true,   // R16_SINT
    true,   // R8_TYPELESS
    true,   // R8_UNORM
    true,   // R8_UINT
    true,   // R8_SNORM
    true,   // R8_SINT
    true,   // A8_UNORM
    false,  // R1_UNORM
    true,   // R9G9B9E5_SHAREDEXP
    true,   // R8G8_B8G8_UNORM
    true,   // G8R8_G8B8_UNORM
    false,  // BC1_TYPELESS
    false,  // BC1_UNORM
    false,  // BC1_UNORM_SRGB
    false,  // BC2_TYPELESS
    false,  // BC2_UNORM
    false,  // BC2_UNORM_SRGB
    false,  // BC3_TYPELESS
    false,  // BC3_UNORM
    false,  // BC3_UNORM_SRGB
    false,  // BC4_TYPELESS
    false,  // BC4_UNORM
    false,  // BC4_SNORM
    false,  // BC5_TYPELESS
    false,  // BC5_UNORM
    false,  // BC5_SNORM
    true,   // B5G6R5_UNORM
    true,   // B5G5R5A1_UNORM
    true,   // B8G8R8A8_UNORM
    true,   // B8G8R8X8_UNORM
    true,   // R10G10B10_XR_BIAS_A2_UNORM
    true,   // B8G8R8A8_TYPELESS
    true,   // B8G8R8A8_UNORM_SRGB
    true,   // B8G8R8X8_TYPELESS
    true,   // B8G8R8X8_UNORM_SRGB
    false,  // BC6H_TYPELESS
    false,  // BC6H_UF16
    false,  // BC6H_SF16
    false,  // BC7_TYPELESS
    false,  // BC7_UNORM
    false,  // BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine whether we will support these)
    false,  // AYUV
    false,  // Y410
    false,  // Y416
    false,  // NV12
    false,  // P010
    false,  // P016
    false,  // OPAQUE_420
    false,  // YUY2
    false,  // Y210
    false,  // Y216
    false,  // NV11
    false,  // AI44
    false,  // IA44
    false,  // P8
    false,  // A8P8
    false   // B4G4R4A4_UNORM
};
