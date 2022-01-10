#include <Mathematics/IntrLine2OrientedBox2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, OrientedBox2<float>>;
    template class FIQuery<float, Line2<float>, OrientedBox2<float>>;

    template class TIQuery<double, Line2<double>, OrientedBox2<double>>;
    template class FIQuery<double, Line2<double>, OrientedBox2<double>>;
}
