#include <Mathematics/DistRay3AlignedBox3.h>

namespace gte
{
    template class DCPQuery<float, Ray3<float>, AlignedBox3<float>>;
    template class DCPQuery<double, Ray3<double>, AlignedBox3<double>>;
}
