#include <Mathematics/DistAlignedBox3OrientedBox3.h>

namespace gte
{
    template class DCPQuery<float, AlignedBox3<float>, OrientedBox3<float>>;
    template class DCPQuery<double, AlignedBox3<double>, OrientedBox3<double>>;
}
