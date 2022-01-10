#include <Mathematics/DistPointCanonicalBox.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, CanonicalBox<2, float>>;
    template class DCPQuery<double, Vector<3, double>, CanonicalBox<3, double>>;
}
