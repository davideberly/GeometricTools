#include <Mathematics/Matrix4x4.h>

namespace gte
{
    template class Matrix<4, 4, float>;
    template Matrix4x4<float> Inverse(Matrix4x4<float> const&, bool*);
    template Matrix4x4<float> Adjoint(Matrix4x4<float> const&);
    template float Determinant(Matrix4x4<float> const&);
    template float Trace(Matrix4x4<float> const&);
    template Vector4<float> DoTransform(Matrix4x4<float> const&, Vector4<float> const&);
    template Matrix4x4<float> DoTransform(Matrix4x4<float> const&, Matrix4x4<float> const&);
    template void SetBasis(Matrix4x4<float>&, int, Vector4<float> const&);
    template Vector4<float> GetBasis(Matrix4x4<float> const&, int);
    template Matrix4x4<float> MakeObliqueProjection(Vector4<float> const&, Vector4<float> const&, Vector4<float> const&);
    template Matrix4x4<float> MakePerspectiveProjection(Vector4<float> const&, Vector4<float> const&, Vector4<float> const&);
    template Matrix4x4<float> MakeReflection(Vector4<float> const&, Vector4<float> const&);

    template class Matrix<4, 4, double>;
    template Matrix4x4<double> Inverse(Matrix4x4<double> const&, bool*);
    template Matrix4x4<double> Adjoint(Matrix4x4<double> const&);
    template double Determinant(Matrix4x4<double> const&);
    template double Trace(Matrix4x4<double> const&);
    template Vector4<double> DoTransform(Matrix4x4<double> const&, Vector4<double> const&);
    template Matrix4x4<double> DoTransform(Matrix4x4<double> const&, Matrix4x4<double> const&);
    template void SetBasis(Matrix4x4<double>&, int, Vector4<double> const&);
    template Vector4<double> GetBasis(Matrix4x4<double> const&, int);
    template Matrix4x4<double> MakeObliqueProjection(Vector4<double> const&, Vector4<double> const&, Vector4<double> const&);
    template Matrix4x4<double> MakePerspectiveProjection(Vector4<double> const&, Vector4<double> const&, Vector4<double> const&);
    template Matrix4x4<double> MakeReflection(Vector4<double> const&, Vector4<double> const&);
}
