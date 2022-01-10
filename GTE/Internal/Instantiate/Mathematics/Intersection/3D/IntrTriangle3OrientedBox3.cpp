#include <Mathematics/IntrTriangle3OrientedBox3.h>

namespace gte
{
    template class TIQuery<float, Triangle3<float>, OrientedBox3<float>>;
    template class FIQuery<float, Triangle3<float>, OrientedBox3<float>>;

    template class TIQuery<double, Triangle3<double>, OrientedBox3<double>>;
    template class FIQuery<double, Triangle3<double>, OrientedBox3<double>>;
}
