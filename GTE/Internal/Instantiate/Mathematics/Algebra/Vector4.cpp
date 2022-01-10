#include <Mathematics/Vector4.h>

namespace gte
{
    template class Vector<4, float>;
    template Vector4<float> HyperCross(Vector4<float> const&, Vector4<float> const&, Vector4<float> const&);
    template Vector4<float> UnitHyperCross(Vector4<float> const&, Vector4<float> const&, Vector4<float> const&, bool);
    template float DotHyperCross(Vector4<float> const&, Vector4<float> const&, Vector4<float> const&, Vector4<float> const&);
    template float ComputeOrthogonalComplement(int, Vector4<float>*, bool);
    template Vector<4, float> Cross(Vector<4, float> const&, Vector<4, float> const&);
    template Vector<4, float> UnitCross(Vector<4, float> const&, Vector<4, float> const&, bool);
    template float DotCross(Vector<4, float> const&, Vector<4, float> const&, Vector<4, float> const&);

    template class Vector<4, double>;
    template Vector4<double> HyperCross(Vector4<double> const&, Vector4<double> const&, Vector4<double> const&);
    template Vector4<double> UnitHyperCross(Vector4<double> const&, Vector4<double> const&, Vector4<double> const&, bool);
    template double DotHyperCross(Vector4<double> const&, Vector4<double> const&, Vector4<double> const&, Vector4<double> const&);
    template double ComputeOrthogonalComplement(int, Vector4<double>*, bool);
    template Vector<4, double> Cross(Vector<4, double> const&, Vector<4, double> const&);
    template Vector<4, double> UnitCross(Vector<4, double> const&, Vector<4, double> const&, bool);
    template double DotCross(Vector<4, double> const&, Vector<4, double> const&, Vector<4, double> const&);
}
