#include <Mathematics/ContCone.h>

namespace gte
{
    template bool InContainer(Vector<2, float> const&, Cone<2, float> const&);
    template bool InContainer(Vector<3, float> const&, Cone<3, float> const&);
    template bool InContainer(Vector<4, float> const&, Cone<4, float> const&);

    template bool InContainer(Vector<2, double> const&, Cone<2, double> const&);
    template bool InContainer(Vector<3, double> const&, Cone<3, double> const&);
    template bool InContainer(Vector<4, double> const&, Cone<4, double> const&);
}
