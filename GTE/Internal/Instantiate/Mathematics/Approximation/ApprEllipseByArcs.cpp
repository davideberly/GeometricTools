#include <Mathematics/ApprEllipseByArcs.h>

namespace gte
{
    template bool gte::ApproximateEllipseByArcs(float, float, int, std::vector<Vector2<float>>&,
        std::vector<Vector2<float>>&, std::vector<float>&);

    template bool gte::ApproximateEllipseByArcs(double, double, int, std::vector<Vector2<double>>&,
        std::vector<Vector2<double>>&, std::vector<double>&);
}
