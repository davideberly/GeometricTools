#include <Mathematics/IntrRay2AlignedBox2.h>

namespace gte
{
    template class TIQuery<float, Ray2<float>, AlignedBox2<float>>;
    template class FIQuery<float, Ray2<float>, AlignedBox2<float>>;

    template class TIQuery<double, Ray2<double>, AlignedBox2<double>>;
    template class FIQuery<double, Ray2<double>, AlignedBox2<double>>;
}
