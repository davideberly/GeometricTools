#include <Mathematics/SurfaceExtractorTetrahedra.h>

namespace gte
{
    template void SurfaceExtractor<int8_t, float>::Extract(int8_t, bool, std::vector<std::array<float, 3>>&, std::vector<SurfaceExtractor<int8_t, float>::Triangle>&);
    template void SurfaceExtractor<int16_t, float>::Extract(int16_t, bool, std::vector<std::array<float, 3>>&, std::vector<SurfaceExtractor<int16_t, float>::Triangle>&);
    template void SurfaceExtractor<int32_t, float>::Extract(int32_t, bool, std::vector<std::array<float, 3>>&, std::vector<SurfaceExtractor<int32_t, float>::Triangle>&);
    template class SurfaceExtractorTetrahedra<int8_t, float>;
    template class SurfaceExtractorTetrahedra<int16_t, float>;
    template class SurfaceExtractorTetrahedra<int32_t, float>;

    template void SurfaceExtractor<int8_t, double>::Extract(int8_t, bool, std::vector<std::array<double, 3>>&, std::vector<SurfaceExtractor<int8_t, double>::Triangle>&);
    template void SurfaceExtractor<int16_t, double>::Extract(int16_t, bool, std::vector<std::array<double, 3>>&, std::vector<SurfaceExtractor<int16_t, double>::Triangle>&);
    template void SurfaceExtractor<int32_t, double>::Extract(int32_t, bool, std::vector<std::array<double, 3>>&, std::vector<SurfaceExtractor<int32_t, double>::Triangle>&);
    template class SurfaceExtractorTetrahedra<int8_t, double>;
    template class SurfaceExtractorTetrahedra<int16_t, double>;
    template class SurfaceExtractorTetrahedra<int32_t, double>;
}
