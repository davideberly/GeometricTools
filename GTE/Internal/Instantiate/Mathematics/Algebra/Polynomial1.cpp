#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Polynomial1.h>

namespace gte
{
    template class Polynomial1<float>;
    template Polynomial1<float> operator+(Polynomial1<float> const&);
    template Polynomial1<float> operator-(Polynomial1<float> const&);
    template Polynomial1<float> operator+(Polynomial1<float> const&, Polynomial1<float> const&);
    template Polynomial1<float> operator-(Polynomial1<float> const&, Polynomial1<float> const&);
    template Polynomial1<float> operator*(Polynomial1<float> const&, Polynomial1<float> const&);
    template Polynomial1<float> gte::operator+(Polynomial1<float> const&, float);
    template Polynomial1<float> gte::operator+(float, Polynomial1<float> const&);
    template Polynomial1<float> gte::operator-(Polynomial1<float> const&, float);
    template Polynomial1<float> gte::operator-(float, Polynomial1<float> const&);
    template Polynomial1<float> gte::operator*(Polynomial1<float> const&, float);
    template Polynomial1<float> gte::operator*(float, Polynomial1<float> const&);
    template Polynomial1<float> gte::operator/(Polynomial1<float> const&, float);
    template Polynomial1<float>& operator+=(Polynomial1<float>&, Polynomial1<float> const&);
    template Polynomial1<float>& operator-=(Polynomial1<float>&, Polynomial1<float> const&);
    template Polynomial1<float>& operator*=(Polynomial1<float>&, Polynomial1<float> const&);
    template Polynomial1<float>& gte::operator+=(Polynomial1<float>&, float);
    template Polynomial1<float>& gte::operator-=(Polynomial1<float>&, float);
    template Polynomial1<float>& gte::operator*=(Polynomial1<float>&, float);
    template Polynomial1<float>& gte::operator/=(Polynomial1<float>&, float);
    template Polynomial1<float> GreatestCommonDivisor(Polynomial1<float> const&, Polynomial1<float> const&);
    template void SquareFreeFactorization(Polynomial1<float> const&, std::vector<Polynomial1<float>>&);

    template class Polynomial1<double>;
    template Polynomial1<double> operator+(Polynomial1<double> const&);
    template Polynomial1<double> operator-(Polynomial1<double> const&);
    template Polynomial1<double> operator+(Polynomial1<double> const&, Polynomial1<double> const&);
    template Polynomial1<double> operator-(Polynomial1<double> const&, Polynomial1<double> const&);
    template Polynomial1<double> operator*(Polynomial1<double> const&, Polynomial1<double> const&);
    template Polynomial1<double> gte::operator+(Polynomial1<double> const&, double);
    template Polynomial1<double> gte::operator+(double, Polynomial1<double> const&);
    template Polynomial1<double> gte::operator-(Polynomial1<double> const&, double);
    template Polynomial1<double> gte::operator-(double, Polynomial1<double> const&);
    template Polynomial1<double> gte::operator*(Polynomial1<double> const&, double);
    template Polynomial1<double> gte::operator*(double, Polynomial1<double> const&);
    template Polynomial1<double> gte::operator/(Polynomial1<double> const&, double);
    template Polynomial1<double>& operator+=(Polynomial1<double>&, Polynomial1<double> const&);
    template Polynomial1<double>& operator-=(Polynomial1<double>&, Polynomial1<double> const&);
    template Polynomial1<double>& operator*=(Polynomial1<double>&, Polynomial1<double> const&);
    template Polynomial1<double>& gte::operator+=(Polynomial1<double>&, double);
    template Polynomial1<double>& gte::operator-=(Polynomial1<double>&, double);
    template Polynomial1<double>& gte::operator*=(Polynomial1<double>&, double);
    template Polynomial1<double>& gte::operator/=(Polynomial1<double>&, double);
    template Polynomial1<double> GreatestCommonDivisor(Polynomial1<double> const&, Polynomial1<double> const&);
    template void SquareFreeFactorization(Polynomial1<double> const&, std::vector<Polynomial1<double>>&);

    typedef BSRational<UIntegerAP32> Rational;
    template class Polynomial1<Rational>;
    template Polynomial1<Rational> operator+(Polynomial1<Rational> const&);
    template Polynomial1<Rational> operator-(Polynomial1<Rational> const&);
    template Polynomial1<Rational> operator+(Polynomial1<Rational> const&, Polynomial1<Rational> const&);
    template Polynomial1<Rational> operator-(Polynomial1<Rational> const&, Polynomial1<Rational> const&);
    template Polynomial1<Rational> operator*(Polynomial1<Rational> const&, Polynomial1<Rational> const&);
    template Polynomial1<Rational> gte::operator+(Polynomial1<Rational> const&, Rational);
    template Polynomial1<Rational> gte::operator+(Rational, Polynomial1<Rational> const&);
    template Polynomial1<Rational> gte::operator-(Polynomial1<Rational> const&, Rational);
    template Polynomial1<Rational> gte::operator-(Rational, Polynomial1<Rational> const&);
    template Polynomial1<Rational> gte::operator*(Polynomial1<Rational> const&, Rational);
    template Polynomial1<Rational> gte::operator*(Rational, Polynomial1<Rational> const&);
    template Polynomial1<Rational> gte::operator/(Polynomial1<Rational> const&, Rational);
    template Polynomial1<Rational>& operator+=(Polynomial1<Rational>&, Polynomial1<Rational> const&);
    template Polynomial1<Rational>& operator-=(Polynomial1<Rational>&, Polynomial1<Rational> const&);
    template Polynomial1<Rational>& operator*=(Polynomial1<Rational>&, Polynomial1<Rational> const&);
    template Polynomial1<Rational>& gte::operator+=(Polynomial1<Rational>&, Rational);
    template Polynomial1<Rational>& gte::operator-=(Polynomial1<Rational>&, Rational);
    template Polynomial1<Rational>& gte::operator*=(Polynomial1<Rational>&, Rational);
    template Polynomial1<Rational>& gte::operator/=(Polynomial1<Rational>&, Rational);
    template Polynomial1<Rational> GreatestCommonDivisor(Polynomial1<Rational> const&, Polynomial1<Rational> const&);
    template void SquareFreeFactorization(Polynomial1<Rational> const&, std::vector<Polynomial1<Rational>>&);
}
