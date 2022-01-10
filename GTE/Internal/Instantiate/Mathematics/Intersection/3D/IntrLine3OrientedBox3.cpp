#include <Mathematics/IntrLine3OrientedBox3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, OrientedBox3<float>>;
    template class FIQuery<float, Line3<float>, OrientedBox3<float>>;

    template class TIQuery<double, Line3<double>, OrientedBox3<double>>;
    template class FIQuery<double, Line3<double>, OrientedBox3<double>>;
}
