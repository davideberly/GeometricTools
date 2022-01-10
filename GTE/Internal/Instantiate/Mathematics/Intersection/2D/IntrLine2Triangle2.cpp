#include <Mathematics/IntrLine2Triangle2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Triangle2<float>>;
    template class FIQuery<float, Line2<float>, Triangle2<float>>;

    template class TIQuery<double, Line2<double>, Triangle2<double>>;
    template class FIQuery<double, Line2<double>, Triangle2<double>>;
}
