#include <Mathematics/Projection.h>

namespace gte
{
    template void gte::Project(Ellipse2<float> const&, Line2<float> const&, float&, float&);
    template void gte::Project(Ellipsoid3<float> const&, Line3<float> const&, float&, float&);

    template void gte::Project(Ellipse2<double> const&, Line2<double> const&, double&, double&);
    template void gte::Project(Ellipsoid3<double> const&, Line3<double> const&, double&, double&);
}
