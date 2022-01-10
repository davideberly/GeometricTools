#include <Mathematics/Matrix3x3.h>

namespace gte
{
    template class Matrix<3, 3, float>;
    template Matrix3x3<float> Inverse(Matrix3x3<float> const&, bool*);
    template Matrix3x3<float> Adjoint(Matrix3x3<float> const&);
    template float Determinant(Matrix3x3<float> const&);
    template float Trace(Matrix3x3<float> const&);
    template Vector3<float> DoTransform(Matrix3x3<float> const&, Vector3<float> const&);
    template Matrix3x3<float> DoTransform(Matrix3x3<float> const&, Matrix3x3<float> const&);
    template void SetBasis(Matrix3x3<float>&, int, Vector3<float> const&);
    template Vector3<float> GetBasis(Matrix3x3<float> const&, int);

    template class Matrix<3, 3, double>;
    template Matrix3x3<double> Inverse(Matrix3x3<double> const&, bool*);
    template Matrix3x3<double> Adjoint(Matrix3x3<double> const&);
    template double Determinant(Matrix3x3<double> const&);
    template double Trace(Matrix3x3<double> const&);
    template Vector3<double> DoTransform(Matrix3x3<double> const&, Vector3<double> const&);
    template Matrix3x3<double> DoTransform(Matrix3x3<double> const&, Matrix3x3<double> const&);
    template void SetBasis(Matrix3x3<double>&, int, Vector3<double> const&);
    template Vector3<double> GetBasis(Matrix3x3<double> const&, int);
}
