#include <Mathematics/IntrLine2Ray2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Ray2<float>>;
    template class FIQuery<float, Line2<float>, Ray2<float>>;

    template class TIQuery<double, Line2<double>, Ray2<double>>;
    template class FIQuery<double, Line2<double>, Ray2<double>>;
}
