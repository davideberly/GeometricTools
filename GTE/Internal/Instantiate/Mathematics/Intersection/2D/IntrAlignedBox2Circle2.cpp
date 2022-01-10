#include <Mathematics/IntrAlignedBox2Circle2.h>

namespace gte
{
    template class TIQuery<float, AlignedBox2<float>, Circle2<float>>;
    template class FIQuery<float, AlignedBox2<float>, Circle2<float>>;

    template class TIQuery<double, AlignedBox2<double>, Circle2<double>>;
    template class FIQuery<double, AlignedBox2<double>, Circle2<double>>;
}
