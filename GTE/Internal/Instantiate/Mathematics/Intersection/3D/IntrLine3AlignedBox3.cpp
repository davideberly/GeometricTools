#include <Mathematics/IntrLine3AlignedBox3.h>

namespace gte
{
    template class TIQuery<float, Line3<float>, AlignedBox3<float>>;
    template class FIQuery<float, Line3<float>, AlignedBox3<float>>;

    template class TIQuery<double, Line3<double>, AlignedBox3<double>>;
    template class FIQuery<double, Line3<double>, AlignedBox3<double>>;
}
