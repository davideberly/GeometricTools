#include <Mathematics/OdeEuler.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    template class OdeEuler<float, Vector2<float>>;
    template class OdeEuler<double, Vector2<double>>;
}
