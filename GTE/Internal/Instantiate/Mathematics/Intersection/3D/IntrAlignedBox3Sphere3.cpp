#include <Mathematics/IntrAlignedBox3Sphere3.h>

namespace gte
{
    template class TIQuery<float, AlignedBox3<float>, Sphere3<float>>;
    template class FIQuery<float, AlignedBox3<float>, Sphere3<float>>;

    template class TIQuery<double, AlignedBox3<double>, Sphere3<double>>;
    template class FIQuery<double, AlignedBox3<double>, Sphere3<double>>;
}
