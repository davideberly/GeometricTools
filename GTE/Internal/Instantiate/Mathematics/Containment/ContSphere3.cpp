#include <Mathematics/ContSphere3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, Sphere3<float>&);
    template bool GetContainer(std::vector<Vector3<float>> const&, Sphere3<float>&);
    template bool InContainer(Vector3<float> const&, Sphere3<float> const&);
    template bool MergeContainers(Sphere3<float> const&, Sphere3<float> const&, Sphere3<float>&);

    template bool GetContainer(int, Vector3<double> const*, Sphere3<double>&);
    template bool GetContainer(std::vector<Vector3<double>> const&, Sphere3<double>&);
    template bool InContainer(Vector3<double> const&, Sphere3<double> const&);
    template bool MergeContainers(Sphere3<double> const&, Sphere3<double> const&, Sphere3<double>&);
}
