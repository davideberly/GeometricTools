#include <Mathematics/ChebyshevRatio.h>

namespace gte
{
    template<> void ChebyshevRatio<float>::Get(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<1>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<2>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<3>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<4>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<5>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<6>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<7>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<8>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<9>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<10>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<11>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<12>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<13>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<14>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<15>(float, float, float&, float&);
    template<> template<> void ChebyshevRatio<float>::GetEstimate<16>(float, float, float&, float&);

    template<> void ChebyshevRatio<double>::Get(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<1>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<2>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<3>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<4>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<5>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<6>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<7>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<8>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<9>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<10>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<11>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<12>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<13>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<14>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<15>(double, double, double&, double&);
    template<> template<> void ChebyshevRatio<double>::GetEstimate<16>(double, double, double&, double&);
}
