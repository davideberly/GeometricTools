#include <Mathematics/ASinEstimate.h>

namespace gte
{
    template<> template<> float ASinEstimate<float>::Degree<1>(float);
    template<> template<> float ASinEstimate<float>::Degree<2>(float);
    template<> template<> float ASinEstimate<float>::Degree<3>(float);
    template<> template<> float ASinEstimate<float>::Degree<4>(float);
    template<> template<> float ASinEstimate<float>::Degree<5>(float);
    template<> template<> float ASinEstimate<float>::Degree<6>(float);
    template<> template<> float ASinEstimate<float>::Degree<7>(float);
    template<> template<> float ASinEstimate<float>::Degree<8>(float);

    template<> template<> double ASinEstimate<double>::Degree<1>(double);
    template<> template<> double ASinEstimate<double>::Degree<2>(double);
    template<> template<> double ASinEstimate<double>::Degree<3>(double);
    template<> template<> double ASinEstimate<double>::Degree<4>(double);
    template<> template<> double ASinEstimate<double>::Degree<5>(double);
    template<> template<> double ASinEstimate<double>::Degree<6>(double);
    template<> template<> double ASinEstimate<double>::Degree<7>(double);
    template<> template<> double ASinEstimate<double>::Degree<8>(double);
}
