#include <Mathematics/CircleThroughPointSpecifiedTangentAndRadius.h>

namespace gte
{
    template size_t gte::CircleThroughPointSpecifiedTangentAndRadius(
        Vector2<float> const&, Vector2<float> const&, Vector2<float>,
        float const&, std::array<Circle2<float>, 2>&);

    template size_t gte::CircleThroughPointSpecifiedTangentAndRadius(
        Vector2<double> const&, Vector2<double> const&, Vector2<double>,
        double const&, std::array<Circle2<double>, 2>&);
}
