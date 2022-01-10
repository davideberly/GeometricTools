#include <Mathematics/SWInterval.h>

namespace gte
{
    template class SWInterval<float>;
    template SWInterval<float> operator+(SWInterval<float> const&);
    template SWInterval<float> operator-(SWInterval<float> const&);
    template SWInterval<float> gte::operator+(float, SWInterval<float> const&);
    template SWInterval<float> gte::operator+(SWInterval<float> const&, float);
    template SWInterval<float> operator+(SWInterval<float> const&, SWInterval<float> const&);
    template SWInterval<float> gte::operator-(float, SWInterval<float> const&);
    template SWInterval<float> gte::operator-(SWInterval<float> const&, float);
    template SWInterval<float> operator-(SWInterval<float> const&, SWInterval<float> const&);
    template SWInterval<float> gte::operator*(float, SWInterval<float> const&);
    template SWInterval<float> gte::operator*(SWInterval<float> const&, float);
    template SWInterval<float> operator*(SWInterval<float> const&, SWInterval<float> const&);
    template SWInterval<float> gte::operator/(float, SWInterval<float> const&);
    template SWInterval<float> gte::operator/(SWInterval<float> const&, float);
    template SWInterval<float> operator/(SWInterval<float> const&, SWInterval<float> const&);

    template class SWInterval<double>;
    template SWInterval<double> operator+(SWInterval<double> const&);
    template SWInterval<double> operator-(SWInterval<double> const&);
    template SWInterval<double> gte::operator+(double, SWInterval<double> const&);
    template SWInterval<double> gte::operator+(SWInterval<double> const&, double);
    template SWInterval<double> operator+(SWInterval<double> const&, SWInterval<double> const&);
    template SWInterval<double> gte::operator-(double, SWInterval<double> const&);
    template SWInterval<double> gte::operator-(SWInterval<double> const&, double);
    template SWInterval<double> operator-(SWInterval<double> const&, SWInterval<double> const&);
    template SWInterval<double> gte::operator*(double, SWInterval<double> const&);
    template SWInterval<double> gte::operator*(SWInterval<double> const&, double);
    template SWInterval<double> operator*(SWInterval<double> const&, SWInterval<double> const&);
    template SWInterval<double> gte::operator/(double, SWInterval<double> const&);
    template SWInterval<double> gte::operator/(SWInterval<double> const&, double);
    template SWInterval<double> operator/(SWInterval<double> const&, SWInterval<double> const&);
}
