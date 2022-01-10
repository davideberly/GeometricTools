#include <Mathematics/IntrHalfspace2Polygon2.h>

namespace gte
{
    template class FIQuery<float, Halfspace<2, float>, std::vector<Vector2<float>>>;
    template class FIQuery<double, Halfspace<2, double>, std::vector<Vector2<double>>>;
}
