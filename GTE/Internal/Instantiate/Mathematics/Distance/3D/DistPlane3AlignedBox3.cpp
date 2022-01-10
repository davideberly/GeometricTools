#include <Mathematics/DistPlane3AlignedBox3.h>

namespace gte
{
    template class DCPQuery<float, Plane3<float>, AlignedBox3<float>>;
    template class DCPQuery<double, Plane3<double>, AlignedBox3<double>>;
}
