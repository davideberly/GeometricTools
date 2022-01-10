#include <Mathematics/CircleThroughTwoPointsSpecifiedRadius.h>

namespace gte
{
    template size_t gte::CircleThroughTwoPointsSpecifiedRadius(
        Vector2<float> const&, Vector2<float> const&, float const&,
        std::array<Circle2<float>, 2>&);

    template size_t gte::CircleThroughTwoPointsSpecifiedRadius(
        Vector2<double> const&, Vector2<double> const&, double const&,
        std::array<Circle2<double>, 2>&);
}
