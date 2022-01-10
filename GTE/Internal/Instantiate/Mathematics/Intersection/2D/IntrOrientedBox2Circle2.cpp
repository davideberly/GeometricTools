#include <Mathematics/IntrOrientedBox2Circle2.h>

namespace gte
{
    template class TIQuery<float, OrientedBox2<float>, Circle2<float>>;
    template class FIQuery<float, OrientedBox2<float>, Circle2<float>>;

    template class TIQuery<double, OrientedBox2<double>, Circle2<double>>;
    template class FIQuery<double, OrientedBox2<double>, Circle2<double>>;
}
