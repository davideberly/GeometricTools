#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay3.h>

namespace gte
{
    template class Delaunay3<float, BSNumber<UIntegerAP32>>;
    template class Delaunay3<float, float>;

    template class Delaunay3<double, BSNumber<UIntegerAP32>>;
    template class Delaunay3<double, double>;

    template class Delaunay3<float>;
    template class Delaunay3<double>;
}
