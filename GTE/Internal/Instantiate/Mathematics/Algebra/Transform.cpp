#include <Mathematics/Transform.h>

namespace gte
{
    template class Transform<float>;
    template Vector4<float> operator*(Transform<float> const&, Vector4<float> const&);
    template Vector4<float> operator*(Vector4<float> const&, Transform<float> const&);
    template Transform<float> operator*(Transform<float> const&, Transform<float> const&);
    template Matrix4x4<float> operator*(Matrix4x4<float> const&, Transform<float> const&);
    template Matrix4x4<float> operator*(Transform<float> const&, Matrix4x4<float> const&);

    template class Transform<double>;
    template Vector4<double> operator*(Transform<double> const&, Vector4<double> const&);
    template Vector4<double> operator*(Vector4<double> const&, Transform<double> const&);
    template Transform<double> operator*(Transform<double> const&, Transform<double> const&);
    template Matrix4x4<double> operator*(Matrix4x4<double> const&, Transform<double> const&);
    template Matrix4x4<double> operator*(Transform<double> const&, Matrix4x4<double> const&);
}
