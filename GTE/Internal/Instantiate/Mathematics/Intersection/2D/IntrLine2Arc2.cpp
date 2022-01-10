#include <Mathematics/IntrLine2Arc2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, Arc2<float>>;
    template class FIQuery<float, Line2<float>, Arc2<float>>;

    template class TIQuery<double, Line2<double>, Arc2<double>>;
    template class FIQuery<double, Line2<double>, Arc2<double>>;
}
