#include <Mathematics/DistPointTriangle.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, Triangle<2, float>>;
    template class DCPQuery<double, Vector<3, double>, Triangle<3, double>>;
}
