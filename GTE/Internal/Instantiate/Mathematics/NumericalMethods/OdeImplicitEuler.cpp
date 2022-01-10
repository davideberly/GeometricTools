#include <Mathematics/OdeImplicitEuler.h>
#include <Mathematics/Matrix2x2.h>

namespace gte
{
    template class OdeImplicitEuler<float, Vector2<float>, Matrix2x2<float>>;
    template class OdeImplicitEuler<double, Vector2<double>, Matrix2x2<double>>;
}
