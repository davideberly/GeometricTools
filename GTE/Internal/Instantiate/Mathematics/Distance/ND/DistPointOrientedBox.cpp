#include <Mathematics/DistPointOrientedBox.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, OrientedBox<2, float>>;
    template class DCPQuery<double, Vector<3, double>, OrientedBox<3, double>>;
}
