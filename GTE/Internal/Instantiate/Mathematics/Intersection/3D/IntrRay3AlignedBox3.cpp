#include <Mathematics/IntrRay3AlignedBox3.h>

namespace gte
{
    template class TIQuery<float, Ray3<float>, AlignedBox3<float>>;
    template class FIQuery<float, Ray3<float>, AlignedBox3<float>>;

    template class TIQuery<double, Ray3<double>, AlignedBox3<double>>;
    template class FIQuery<double, Ray3<double>, AlignedBox3<double>>;
}
