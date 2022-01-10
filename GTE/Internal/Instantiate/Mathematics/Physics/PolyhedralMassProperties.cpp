#include <Mathematics/PolyhedralMassProperties.h>

namespace gte
{
    template void gte::ComputeMassProperties(Vector3<float> const*, int, int const*, bool, float&, Vector3<float>&, Matrix3x3<float>&);
    template void gte::ComputeMassProperties(Vector3<double> const*, int, int const*, bool, double&, Vector3<double>&, Matrix3x3<double>&);
}
