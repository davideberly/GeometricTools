#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/TriangulateEC.h>

namespace gte
{
    template class TriangulateEC<float, BSRational<UIntegerAP32>>;
    template class TriangulateEC<float, float>;

    template class TriangulateEC<double, BSRational<UIntegerAP32>>;
    template class TriangulateEC<double, double>;
}
