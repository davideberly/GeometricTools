#include <Mathematics/IntrCircle2Circle2.h>

namespace gte
{
    template class TIQuery<float, Circle2<float>, Circle2<float>>;
    template class FIQuery<float, Circle2<float>, Circle2<float>>;

    template class TIQuery<double, Circle2<double>, Circle2<double>>;
    template class FIQuery<double, Circle2<double>, Circle2<double>>;
}
