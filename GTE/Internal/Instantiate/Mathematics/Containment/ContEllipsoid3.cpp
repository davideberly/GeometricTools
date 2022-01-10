#include <Mathematics/ContEllipsoid3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, Ellipsoid3<float>&);
    template bool InContainer(Vector3<float> const&, Ellipsoid3<float> const&);
    template bool MergeContainers(Ellipsoid3<float> const&, Ellipsoid3<float> const&, Ellipsoid3<float>&);

    template bool GetContainer(int, Vector3<double> const*, Ellipsoid3<double>&);
    template bool InContainer(Vector3<double> const&, Ellipsoid3<double> const&);
    template bool MergeContainers(Ellipsoid3<double> const&, Ellipsoid3<double> const&, Ellipsoid3<double>&);
}
