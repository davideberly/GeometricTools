#include <Mathematics/Quaternion.h>

namespace gte
{
    template class Quaternion<float>;
    template Quaternion<float> operator+(Quaternion<float> const&);
    template Quaternion<float> operator-(Quaternion<float> const&);
    template Quaternion<float> operator+(Quaternion<float> const&, Quaternion<float> const&);
    template Quaternion<float> operator-(Quaternion<float> const&, Quaternion<float> const&);
    template Quaternion<float> gte::operator*(Quaternion<float> const&, float);
    template Quaternion<float> gte::operator*(float, Quaternion<float> const&);
    template Quaternion<float> gte::operator/(Quaternion<float> const&, float);
    template Quaternion<float>& operator+=(Quaternion<float>&, Quaternion<float> const&);
    template Quaternion<float>& operator-=(Quaternion<float>&, Quaternion<float> const&);
    template Quaternion<float>& gte::operator*=(Quaternion<float>&, float);
    template Quaternion<float>& gte::operator/=(Quaternion<float>&, float);
    template float Dot(Quaternion<float> const&, Quaternion<float> const&);
    template float Length(Quaternion<float> const&);
    template float Normalize(Quaternion<float>&);
    template Quaternion<float> operator*(Quaternion<float> const&, Quaternion<float> const&);
    template Quaternion<float> Inverse(Quaternion<float> const&);
    template Quaternion<float> Conjugate(Quaternion<float> const&);
    template Vector<3, float> Rotate(Quaternion<float> const&, Vector<3, float> const&);
    template Vector<4, float> Rotate(Quaternion<float> const&, Vector<4, float> const&);
    template Quaternion<float> gte::Slerp(float, Quaternion<float> const&, Quaternion<float> const&);
    template Quaternion<float> gte::SlerpR(float, Quaternion<float> const&, Quaternion<float> const&);
    template Quaternion<float> gte::SlerpRP(float, Quaternion<float> const&, Quaternion<float> const&, float);
    template Quaternion<float> gte::SlerpRPH(float, Quaternion<float> const&, Quaternion<float> const&, Quaternion<float> const&, float);

    template class Quaternion<double>;
    template Quaternion<double> operator+(Quaternion<double> const&);
    template Quaternion<double> operator-(Quaternion<double> const&);
    template Quaternion<double> operator+(Quaternion<double> const&, Quaternion<double> const&);
    template Quaternion<double> operator-(Quaternion<double> const&, Quaternion<double> const&);
    template Quaternion<double> gte::operator*(Quaternion<double> const&, double);
    template Quaternion<double> gte::operator*(double, Quaternion<double> const&);
    template Quaternion<double> gte::operator/(Quaternion<double> const&, double);
    template Quaternion<double>& operator+=(Quaternion<double>&, Quaternion<double> const&);
    template Quaternion<double>& operator-=(Quaternion<double>&, Quaternion<double> const&);
    template Quaternion<double>& gte::operator*=(Quaternion<double>&, double);
    template Quaternion<double>& gte::operator/=(Quaternion<double>&, double);
    template double Dot(Quaternion<double> const&, Quaternion<double> const&);
    template double Length(Quaternion<double> const&);
    template double Normalize(Quaternion<double>&);
    template Quaternion<double> operator*(Quaternion<double> const&, Quaternion<double> const&);
    template Quaternion<double> Inverse(Quaternion<double> const&);
    template Quaternion<double> Conjugate(Quaternion<double> const&);
    template Vector<3, double> Rotate(Quaternion<double> const&, Vector<3, double> const&);
    template Vector<4, double> Rotate(Quaternion<double> const&, Vector<4, double> const&);
    template Quaternion<double> gte::Slerp(double, Quaternion<double> const&, Quaternion<double> const&);
    template Quaternion<double> gte::SlerpR(double, Quaternion<double> const&, Quaternion<double> const&);
    template Quaternion<double> gte::SlerpRP(double, Quaternion<double> const&, Quaternion<double> const&, double);
    template Quaternion<double> gte::SlerpRPH(double, Quaternion<double> const&, Quaternion<double> const&, Quaternion<double> const&, double);
}
