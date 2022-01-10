#include <Mathematics/UIntegerAP32.h>
#include <Mathematics/BSRational.h>
#include <Mathematics/RootsPolynomial.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;

    template class RootsPolynomial<float>;
    template void RootsPolynomial<float>::SolveQuadratic<Rational>(Rational const&, Rational const&, Rational const&, std::map<float, int>&);
    template void RootsPolynomial<float>::SolveCubic<Rational>(Rational const&, Rational const&, Rational const&, Rational const&, std::map<float, int>&);
    template void RootsPolynomial<float>::SolveQuartic<Rational>(Rational const&, Rational const&, Rational const&, Rational const&, Rational const&, std::map<float, int>&);

    template class RootsPolynomial<double>;
    template void RootsPolynomial<double>::SolveQuadratic<Rational>(Rational const&, Rational const&, Rational const&, std::map<double, int>&);
    template void RootsPolynomial<double>::SolveCubic<Rational>(Rational const&, Rational const&, Rational const&, Rational const&, std::map<double, int>&);
    template void RootsPolynomial<double>::SolveQuartic<Rational>(Rational const&, Rational const&, Rational const&, Rational const&, Rational const&, std::map<double, int>&);
}
