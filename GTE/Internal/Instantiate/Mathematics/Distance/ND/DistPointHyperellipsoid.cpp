#include <Mathematics/DistPointHyperellipsoid.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, Hyperellipsoid<2, float>>;
    template class DCPQuery<double, Vector<3, double>, Hyperellipsoid<3, double>>;
}
