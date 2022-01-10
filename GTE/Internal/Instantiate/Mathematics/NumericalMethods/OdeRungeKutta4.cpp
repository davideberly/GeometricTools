#include <Mathematics/OdeRungeKutta4.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    template class OdeRungeKutta4<float, Vector2<float>>;
    template class OdeRungeKutta4<double, Vector2<double>>;
}
