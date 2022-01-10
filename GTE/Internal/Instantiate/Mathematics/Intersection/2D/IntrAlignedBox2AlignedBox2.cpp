#include <Mathematics/IntrAlignedBox2AlignedBox2.h>

namespace gte
{
    template class TIQuery<float, AlignedBox2<float>, AlignedBox2<float>>;
    template class FIQuery<float, AlignedBox2<float>, AlignedBox2<float>>;

    template class TIQuery<double, AlignedBox2<double>, AlignedBox2<double>>;
    template class FIQuery<double, AlignedBox2<double>, AlignedBox2<double>>;
}
