#include <Mathematics/Vector3.h>

namespace gte
{
    template class Vector<3, float>;
    template Vector<3, float> Cross(Vector<3, float> const&, Vector<3, float> const&);
    template Vector<3, float> UnitCross(Vector<3, float> const&, Vector<3, float> const&, bool);
    template float DotCross(Vector<3, float> const&, Vector<3, float> const&, Vector<3, float> const&);
    template float ComputeOrthogonalComplement(int, Vector3<float>*, bool);
    template bool gte::ComputeBarycentrics(Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, Vector3<float> const&, float[4], float);
    template class IntrinsicsVector3<float>;

    template class Vector<3, double>;
    template Vector<3, double> Cross(Vector<3, double> const&, Vector<3, double> const&);
    template Vector<3, double> UnitCross(Vector<3, double> const&, Vector<3, double> const&, bool);
    template double DotCross(Vector<3, double> const&, Vector<3, double> const&, Vector<3, double> const&);
    template double ComputeOrthogonalComplement(int, Vector3<double>*, bool);
    template bool gte::ComputeBarycentrics(Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, Vector3<double> const&, double[4], double);
    template class IntrinsicsVector3<double>;
}
