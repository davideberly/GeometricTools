#include <Mathematics/ContEllipse2.h>

namespace gte
{
    template bool GetContainer(int, Vector2<float> const*, Ellipse2<float>&);
    template bool InContainer(Vector2<float> const&, Ellipse2<float> const&);
    template bool MergeContainers(Ellipse2<float> const&, Ellipse2<float> const&, Ellipse2<float>&);

    template bool GetContainer(int, Vector2<double> const*, Ellipse2<double>&);
    template bool InContainer(Vector2<double> const&, Ellipse2<double> const&);
    template bool MergeContainers(Ellipse2<double> const&, Ellipse2<double> const&, Ellipse2<double>&);
}
