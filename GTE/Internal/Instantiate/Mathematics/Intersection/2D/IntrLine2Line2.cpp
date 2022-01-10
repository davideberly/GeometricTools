#include <Mathematics/IntrLine2Line2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Line2<float>>;
    template class FIQuery<float, Line2<float>, Line2<float>>;

    template class TIQuery<double, Line2<double>, Line2<double>>;
    template class FIQuery<double, Line2<double>, Line2<double>>;
}
