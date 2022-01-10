#include <Mathematics/IntrAlignedBox3AlignedBox3.h>

namespace gte
{
    template class TIQuery<float, AlignedBox3<float>, AlignedBox3<float>>;
    template class FIQuery<float, AlignedBox3<float>, AlignedBox3<float>>;

    template class TIQuery<double, AlignedBox3<double>, AlignedBox3<double>>;
    template class FIQuery<double, AlignedBox3<double>, AlignedBox3<double>>;
}
