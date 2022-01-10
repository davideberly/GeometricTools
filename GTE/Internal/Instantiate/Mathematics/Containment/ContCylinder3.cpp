#include <Mathematics/ContCylinder3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, Cylinder3<float>&);
    template bool InContainer(Vector3<float> const&, Cylinder3<float> const&);

    template bool GetContainer(int, Vector3<double> const*, Cylinder3<double>&);
    template bool InContainer(Vector3<double> const&, Cylinder3<double> const&);
}
