#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ConstrainedDelaunay2.h>

namespace gte
{
    template class ConstrainedDelaunay2<float, float>;
    template class ConstrainedDelaunay2<double, double>;
    template class ConstrainedDelaunay2<double, BSNumber<UIntegerFP32<526>>>;

    template class ConstrainedDelaunay2<float>;
    template class ConstrainedDelaunay2<double>;
}
