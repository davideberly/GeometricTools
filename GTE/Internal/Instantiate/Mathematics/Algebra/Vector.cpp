#include <Mathematics/Vector.h>

namespace gte
{
    template class Vector<2, float>;
    template Vector<2, float> operator+(Vector<2, float> const&);
    template Vector<2, float> operator-(Vector<2, float> const&);
    template Vector<2, float> operator+(Vector<2, float> const&, Vector<2, float> const&);
    template Vector<2, float> operator-(Vector<2, float> const&, Vector<2, float> const&);
    template Vector<2, float> gte::operator*(Vector<2, float> const&, float);
    template Vector<2, float> gte::operator*(float, Vector<2, float> const&);
    template Vector<2, float> gte::operator/(Vector<2, float> const&, float);
    template Vector<2, float>& operator+=(Vector<2, float>&, Vector<2, float> const&);
    template Vector<2, float>& operator-=(Vector<2, float>&, Vector<2, float> const&);
    template Vector<2, float>& gte::operator*=(Vector<2, float>&, float);
    template Vector<2, float>& gte::operator/=(Vector<2, float>&, float);
    template Vector<2, float> operator*(Vector<2, float> const&, Vector<2, float> const&);
    template Vector<2, float> operator/(Vector<2, float> const&, Vector<2, float> const&);
    template Vector<2, float>& operator*=(Vector<2, float>&, Vector<2, float> const&);
    template Vector<2, float>& operator/=(Vector<2, float>&, Vector<2, float> const&);
    template float Dot(Vector<2, float> const&, Vector<2, float> const&);
    template float Length(Vector<2, float> const&, bool);
    template float Normalize(Vector<2, float>&, bool);
    template float Orthonormalize(int, Vector<2, float>*, bool);
    template Vector<3, float> GetOrthogonal(Vector<3, float> const&, bool);
    template bool ComputeExtremes(int, Vector<2, float> const*, Vector<2, float>&, Vector<2, float>&);
    template Vector<4,float> gte::HLift(Vector<3,float> const&, float);
    template Vector<3,float> HProject(Vector<4,float> const&);
    template Vector<4,float> gte::Lift(Vector<3,float> const&, int, float);
    template Vector<3,float> Project(Vector<4,float> const&, int);

    template class Vector<2, double>;
    template Vector<2, double> operator+(Vector<2, double> const&);
    template Vector<2, double> operator-(Vector<2, double> const&);
    template Vector<2, double> operator+(Vector<2, double> const&, Vector<2, double> const&);
    template Vector<2, double> operator-(Vector<2, double> const&, Vector<2, double> const&);
    template Vector<2, double> gte::operator*(Vector<2, double> const&, double);
    template Vector<2, double> gte::operator*(double, Vector<2, double> const&);
    template Vector<2, double> gte::operator/(Vector<2, double> const&, double);
    template Vector<2, double>& operator+=(Vector<2, double>&, Vector<2, double> const&);
    template Vector<2, double>& operator-=(Vector<2, double>&, Vector<2, double> const&);
    template Vector<2, double>& gte::operator*=(Vector<2, double>&, double);
    template Vector<2, double>& gte::operator/=(Vector<2, double>&, double);
    template Vector<2, double> operator*(Vector<2, double> const&, Vector<2, double> const&);
    template Vector<2, double> operator/(Vector<2, double> const&, Vector<2, double> const&);
    template Vector<2, double>& operator*=(Vector<2, double>&, Vector<2, double> const&);
    template Vector<2, double>& operator/=(Vector<2, double>&, Vector<2, double> const&);
    template double Dot(Vector<2, double> const&, Vector<2, double> const&);
    template double Length(Vector<2, double> const&, bool);
    template double Normalize(Vector<2, double>&, bool);
    template double Orthonormalize(int, Vector<2, double>*, bool);
    template Vector<3, double> GetOrthogonal(Vector<3, double> const&, bool);
    template bool ComputeExtremes(int, Vector<2, double> const*, Vector<2, double>&, Vector<2, double>&);
    template Vector<4, double> gte::HLift(Vector<3, double> const&, double);
    template Vector<3, double> HProject(Vector<4, double> const&);
    template Vector<4, double> gte::Lift(Vector<3, double> const&, int, double);
    template Vector<3, double> Project(Vector<4, double> const&, int);
}
