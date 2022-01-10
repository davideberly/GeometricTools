#include <Mathematics/IntrLine2Circle2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Circle2<float>>;
    template class FIQuery<float, Line2<float>, Circle2<float>>;

    template class TIQuery<double, Line2<double>, Circle2<double>>;
    template class FIQuery<double, Line2<double>, Circle2<double>>;
}
