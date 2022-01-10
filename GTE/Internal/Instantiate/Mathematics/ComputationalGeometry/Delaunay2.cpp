#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay2.h>

namespace gte
{
    template class Delaunay2<float, float>;
    template class Delaunay2<double, double>;
    template class Delaunay2<float, BSNumber<UIntegerAP32>>;
    template class Delaunay2<double, BSNumber<UIntegerFP32<526>>>;

    template class Delaunay2<float>;
    template class Delaunay2<double>;
}
