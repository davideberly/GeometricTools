#include <Mathematics/IntrOrientedBox3Cone3.h>

namespace gte
{
    template class TIQuery<float, OrientedBox<3, float>, Cone<3, float>>;
    template class TIQuery<double, OrientedBox<3, double>, Cone<3, double>>;
}
