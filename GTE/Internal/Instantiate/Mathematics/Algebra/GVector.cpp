#include <Mathematics/GVector.h>

namespace gte
{
    template class GVector<float>;
    template GVector<float> operator+(GVector<float> const&);
    template GVector<float> operator-(GVector<float> const&);
    template GVector<float> operator+(GVector<float> const&, GVector<float> const&);
    template GVector<float> operator-(GVector<float> const&, GVector<float> const&);
    template GVector<float> gte::operator*(GVector<float> const&, float);
    template GVector<float> gte::operator*(float, GVector<float> const&);
    template GVector<float> gte::operator/(GVector<float> const&, float);
    template GVector<float>& operator+=(GVector<float>&, GVector<float> const&);
    template GVector<float>& operator-=(GVector<float>&, GVector<float> const&);
    template GVector<float>& gte::operator*=(GVector<float>&, float);
    template GVector<float>& gte::operator/=(GVector<float>&, float);
    template float Dot(GVector<float> const&, GVector<float> const&);
    template float Length(GVector<float> const&, bool);
    template float Normalize(GVector<float>&, bool);
    template float Orthonormalize(int, GVector<float>*, bool);
    template bool ComputeExtremes(int, GVector<float> const*, GVector<float>&, GVector<float>&);
    template GVector<float> gte::HLift(GVector<float> const&, float);
    template GVector<float> HProject(GVector<float> const&);
    template GVector<float> gte::Lift(GVector<float> const&, int, float);
    template GVector<float> Project(GVector<float> const&, int reject);

    template class GVector<double>;
    template GVector<double> operator+(GVector<double> const&);
    template GVector<double> operator-(GVector<double> const&);
    template GVector<double> operator+(GVector<double> const&, GVector<double> const&);
    template GVector<double> operator-(GVector<double> const&, GVector<double> const&);
    template GVector<double> gte::operator*(GVector<double> const&, double);
    template GVector<double> gte::operator*(double, GVector<double> const&);
    template GVector<double> gte::operator/(GVector<double> const&, double);
    template GVector<double>& operator+=(GVector<double>&, GVector<double> const&);
    template GVector<double>& operator-=(GVector<double>&, GVector<double> const&);
    template GVector<double>& gte::operator*=(GVector<double>&, double);
    template GVector<double>& gte::operator/=(GVector<double>&, double);
    template double Dot(GVector<double> const&, GVector<double> const&);
    template double Length(GVector<double> const&, bool);
    template double Normalize(GVector<double>&, bool);
    template double Orthonormalize(int, GVector<double>*, bool);
    template bool ComputeExtremes(int, GVector<double> const*, GVector<double>&, GVector<double>&);
    template GVector<double> gte::HLift(GVector<double> const&, double);
    template GVector<double> HProject(GVector<double> const&);
    template GVector<double> gte::Lift(GVector<double> const&, int, double);
    template GVector<double> Project(GVector<double> const&, int reject);
}
