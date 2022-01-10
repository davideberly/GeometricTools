#include <Mathematics/IntrLine2AlignedBox2.h>

namespace gte
{
    template class TIQuery<float, Line2<float>, AlignedBox2<float>>;
    template class FIQuery<float, Line2<float>, AlignedBox2<float>>;

    template class TIQuery<double, Line2<double>, AlignedBox2<double>>;
    template class FIQuery<double, Line2<double>, AlignedBox2<double>>;
}
