#include <Mathematics/CurveExtractorSquares.h>

namespace gte
{
    template void CurveExtractor<int8_t, float>::Extract(int8_t, bool, std::vector<std::array<float, 2>>&, std::vector<CurveExtractor<int8_t, float>::Edge>&);
    template void CurveExtractor<int16_t, float>::Extract(int16_t, bool, std::vector<std::array<float, 2>>&, std::vector<CurveExtractor<int16_t, float>::Edge>&);
    template void CurveExtractor<int32_t, float>::Extract(int32_t, bool, std::vector<std::array<float, 2>>&, std::vector<CurveExtractor<int32_t, float>::Edge>&);
    template class CurveExtractorSquares<int8_t, float>;
    template class CurveExtractorSquares<int16_t, float>;
    template class CurveExtractorSquares<int32_t, float>;

    template void CurveExtractor<int8_t, double>::Extract(int8_t, bool, std::vector<std::array<double, 2>>&, std::vector<CurveExtractor<int8_t, double>::Edge>&);
    template void CurveExtractor<int16_t, double>::Extract(int16_t, bool, std::vector<std::array<double, 2>>&, std::vector<CurveExtractor<int16_t, double>::Edge>&);
    template void CurveExtractor<int32_t, double>::Extract(int32_t, bool, std::vector<std::array<double, 2>>&, std::vector<CurveExtractor<int32_t, double>::Edge>&);
    template class CurveExtractorSquares<int8_t, double>;
    template class CurveExtractorSquares<int16_t, double>;
    template class CurveExtractorSquares<int32_t, double>;
}
