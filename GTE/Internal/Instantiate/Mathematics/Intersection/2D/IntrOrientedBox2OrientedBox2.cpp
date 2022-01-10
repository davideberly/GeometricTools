#include <Mathematics/IntrOrientedBox2OrientedBox2.h>

namespace gte
{
    template class TIQuery<float, OrientedBox2<float>, OrientedBox2<float>>;
    template class FIQuery<float, OrientedBox2<float>, OrientedBox2<float>>;

    template class TIQuery<double, OrientedBox2<double>, OrientedBox2<double>>;
    template class FIQuery<double, OrientedBox2<double>, OrientedBox2<double>>;
}
