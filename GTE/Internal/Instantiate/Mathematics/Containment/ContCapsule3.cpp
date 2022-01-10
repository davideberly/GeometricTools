#include <Mathematics/ContCapsule3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, Capsule3<float>&);
    template bool InContainer(Vector3<float> const&, Capsule3<float> const&);
    template bool InContainer(Sphere3<float> const&, Capsule3<float> const&);
    template bool InContainer(Capsule3<float> const&, Capsule3<float> const&);
    template bool MergeContainers(Capsule3<float> const&, Capsule3<float> const&, Capsule3<float>&);

    template bool GetContainer(int, Vector3<double> const*, Capsule3<double>&);
    template bool InContainer(Vector3<double> const&, Capsule3<double> const&);
    template bool InContainer(Sphere3<double> const&, Capsule3<double> const&);
    template bool InContainer(Capsule3<double> const&, Capsule3<double> const&);
    template bool MergeContainers(Capsule3<double> const&, Capsule3<double> const&, Capsule3<double>&);
}
