#include <Mathematics/Vector2.h>

namespace gte
{
    template class Vector<2, float>;
    template Vector2<float> Perp(Vector2<float> const&);
    template Vector2<float> UnitPerp(Vector2<float> const&, bool);
    template float DotPerp(Vector2<float> const&, Vector2<float> const&);
    template float ComputeOrthogonalComplement(int, Vector2<float>*, bool);
    template bool gte::ComputeBarycentrics(Vector2<float> const&, Vector2<float> const&, Vector2<float> const&, Vector2<float> const&, float[3], float);
    template class IntrinsicsVector2<float>;

    template class Vector<2, double>;
    template Vector2<double> Perp(Vector2<double> const&);
    template Vector2<double> UnitPerp(Vector2<double> const&, bool);
    template double DotPerp(Vector2<double> const&, Vector2<double> const&);
    template double ComputeOrthogonalComplement(int, Vector2<double>*, bool);
    template bool gte::ComputeBarycentrics(Vector2<double> const&, Vector2<double> const&, Vector2<double> const&, Vector2<double> const&, double[3], double);
    template class IntrinsicsVector2<double>;
}
