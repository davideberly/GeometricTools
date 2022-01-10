#include <Mathematics/ContCircle2.h>

namespace gte
{
    template bool GetContainer(int, Vector2<float> const*, Circle2<float>&);
    template bool GetContainer(std::vector<Vector2<float>> const&, Circle2<float>&);
    template bool InContainer(Vector2<float> const&, Circle2<float> const&);
    template bool MergeContainers(Circle2<float> const&, Circle2<float> const&, Circle2<float>&);

    template bool GetContainer(int, Vector2<double> const*, Circle2<double>&);
    template bool GetContainer(std::vector<Vector2<double>> const&, Circle2<double>&);
    template bool InContainer(Vector2<double> const&, Circle2<double> const&);
    template bool MergeContainers(Circle2<double> const&, Circle2<double> const&, Circle2<double>&);
}
