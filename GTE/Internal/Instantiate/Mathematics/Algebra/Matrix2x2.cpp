#include <Mathematics/Matrix2x2.h>

namespace gte
{
    template class Matrix<2, 2, float>;
    template void gte::MakeRotation(float, Matrix2x2<float>&);
    template float GetRotationAngle(Matrix2x2<float> const&);
    template Matrix2x2<float> Inverse(Matrix2x2<float> const&, bool*);
    template Matrix2x2<float> Adjoint(Matrix2x2<float> const&);
    template float Determinant(Matrix2x2<float> const&);
    template float Trace(Matrix2x2<float> const&);
    template Vector2<float> DoTransform(Matrix2x2<float> const&, Vector2<float> const&);
    template Matrix2x2<float> DoTransform(Matrix2x2<float> const&, Matrix2x2<float> const&);
    template void SetBasis(Matrix2x2<float>&, int, Vector2<float> const&);
    template Vector2<float> GetBasis(Matrix2x2<float> const&, int);

    template class Matrix<2, 2, double>;
    template void gte::MakeRotation(double, Matrix2x2<double>&);
    template double GetRotationAngle(Matrix2x2<double> const&);
    template Matrix2x2<double> Inverse(Matrix2x2<double> const&, bool*);
    template Matrix2x2<double> Adjoint(Matrix2x2<double> const&);
    template double Determinant(Matrix2x2<double> const&);
    template double Trace(Matrix2x2<double> const&);
    template Vector2<double> DoTransform(Matrix2x2<double> const&, Vector2<double> const&);
    template Matrix2x2<double> DoTransform(Matrix2x2<double> const&, Matrix2x2<double> const&);
    template void SetBasis(Matrix2x2<double>&, int, Vector2<double> const&);
    template Vector2<double> GetBasis(Matrix2x2<double> const&, int);
}
