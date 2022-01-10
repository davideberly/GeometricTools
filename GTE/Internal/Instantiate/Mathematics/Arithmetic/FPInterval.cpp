#include <Mathematics/FPInterval.h>

namespace gte
{
    template class FPInterval<float>;
    template FPInterval<float> operator+(FPInterval<float> const&);
    template FPInterval<float> operator-(FPInterval<float> const&);
    template FPInterval<float> gte::operator+(float, FPInterval<float> const&);
    template FPInterval<float> gte::operator+(FPInterval<float> const&, float);
    template FPInterval<float> operator+(FPInterval<float> const&, FPInterval<float> const&);
    template FPInterval<float> gte::operator-(float, FPInterval<float> const&);
    template FPInterval<float> gte::operator-(FPInterval<float> const&, float);
    template FPInterval<float> operator-(FPInterval<float> const&, FPInterval<float> const&);
    template FPInterval<float> gte::operator*(float, FPInterval<float> const&);
    template FPInterval<float> gte::operator*(FPInterval<float> const&, float);
    template FPInterval<float> operator*(FPInterval<float> const&, FPInterval<float> const&);
    template FPInterval<float> gte::operator/(float, FPInterval<float> const&);
    template FPInterval<float> gte::operator/(FPInterval<float> const&, float);
    template FPInterval<float> operator/(FPInterval<float> const&, FPInterval<float> const&);

    template class FPInterval<double>;
    template FPInterval<double> operator+(FPInterval<double> const&);
    template FPInterval<double> operator-(FPInterval<double> const&);
    template FPInterval<double> gte::operator+(double, FPInterval<double> const&);
    template FPInterval<double> gte::operator+(FPInterval<double> const&, double);
    template FPInterval<double> operator+(FPInterval<double> const&, FPInterval<double> const&);
    template FPInterval<double> gte::operator-(double, FPInterval<double> const&);
    template FPInterval<double> gte::operator-(FPInterval<double> const&, double);
    template FPInterval<double> operator-(FPInterval<double> const&, FPInterval<double> const&);
    template FPInterval<double> gte::operator*(double, FPInterval<double> const&);
    template FPInterval<double> gte::operator*(FPInterval<double> const&, double);
    template FPInterval<double> operator*(FPInterval<double> const&, FPInterval<double> const&);
    template FPInterval<double> gte::operator/(double, FPInterval<double> const&);
    template FPInterval<double> gte::operator/(FPInterval<double> const&, double);
    template FPInterval<double> operator/(FPInterval<double> const&, FPInterval<double> const&);
}
