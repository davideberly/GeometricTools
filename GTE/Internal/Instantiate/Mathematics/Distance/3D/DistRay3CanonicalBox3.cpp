#include <Mathematics/DistRay3CanonicalBox3.h>

namespace gte
{
    template class DCPQuery<float, Ray3<float>, CanonicalBox3<float>>;
    template class DCPQuery<double, Ray3<double>, CanonicalBox3<double>>;
}
