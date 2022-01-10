#include <Mathematics/ContScribeCircle3Sphere3.h>

namespace gte
{
    template bool Circumscribe(Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Circle3<float>&);
    template bool Circumscribe(Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Sphere3<float>&);
    template bool Inscribe(Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Circle3<float>&);
    template bool Inscribe(Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Sphere3<float>&);

    template bool Circumscribe(Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Circle3<double>&);
    template bool Circumscribe(Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Sphere3<double>&);
    template bool Inscribe(Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Circle3<double>&);
    template bool Inscribe(Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Sphere3<double>&);
}
