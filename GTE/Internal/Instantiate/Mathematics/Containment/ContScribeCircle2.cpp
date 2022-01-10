#include <Mathematics/ContScribeCircle2.h>

namespace gte
{
    template bool Circumscribe(Vector2<float> const&, Vector2<float> const&, Vector2<float> const&, Circle2<float>& circle);
    template bool Inscribe(Vector2<float> const&, Vector2<float> const&, Vector2<float> const&, Circle2<float>& circle);

    template bool Circumscribe(Vector2<double> const&, Vector2<double> const&, Vector2<double> const&, Circle2<double>& circle);
    template bool Inscribe(Vector2<double> const&, Vector2<double> const&, Vector2<double> const&, Circle2<double>& circle);
}
