#include <Mathematics/DistPointHyperplane.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, Hyperplane<2, float>>;
    template class DCPQuery<double, Vector<3, double>, Hyperplane<3, double>>;
}
