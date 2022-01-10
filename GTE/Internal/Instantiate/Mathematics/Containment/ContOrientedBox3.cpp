#include <Mathematics/ContOrientedBox3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, OrientedBox3<float>&);
    template bool GetContainer(std::vector<Vector3<float>> const&, OrientedBox3<float>&);
    template bool InContainer(Vector3<float> const&, OrientedBox3<float> const&);
    template bool MergeContainers(OrientedBox3<float> const&, OrientedBox3<float> const&, OrientedBox3<float>&);

    template bool GetContainer(int, Vector3<double> const*, OrientedBox3<double>&);
    template bool GetContainer(std::vector<Vector3<double>> const&, OrientedBox3<double>&);
    template bool InContainer(Vector3<double> const&, OrientedBox3<double> const&);
    template bool MergeContainers(OrientedBox3<double> const&, OrientedBox3<double> const&, OrientedBox3<double>&);
}
