#include <Mathematics/ContOrientedBox2.h>

namespace gte
{
    template bool GetContainer(int, Vector2<float> const*, OrientedBox2<float>&);
    template bool GetContainer(std::vector<Vector2<float>> const&, OrientedBox2<float>&);
    template bool InContainer(Vector2<float> const&, OrientedBox2<float> const&);
    template bool MergeContainers(OrientedBox2<float> const&, OrientedBox2<float> const&, OrientedBox2<float>&);

    template bool GetContainer(int, Vector2<double> const*, OrientedBox2<double>&);
    template bool GetContainer(std::vector<Vector2<double>> const&, OrientedBox2<double>&);
    template bool InContainer(Vector2<double> const&, OrientedBox2<double> const&);
    template bool MergeContainers(OrientedBox2<double> const&, OrientedBox2<double> const&, OrientedBox2<double>&);
}
