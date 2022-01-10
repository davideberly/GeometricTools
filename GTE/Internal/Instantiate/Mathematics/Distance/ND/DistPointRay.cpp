#include <Mathematics/DistPointRay.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, Ray<2, float>>;
    template class DCPQuery<double, Vector<3, double>, Ray<3, double>>;
}
