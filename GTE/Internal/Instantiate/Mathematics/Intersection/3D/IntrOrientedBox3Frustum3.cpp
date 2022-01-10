#include <Mathematics/IntrOrientedBox3Frustum3.h>

namespace gte
{
    template class TIQuery<float, OrientedBox3<float>, Frustum3<float>>;
    template class TIQuery<double, OrientedBox3<double>, Frustum3<double>>;
}
