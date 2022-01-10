#include <Mathematics/ACosEstimate.h>

namespace gte
{
    template<> template<> float ACosEstimate<float>::Degree<1>(float);
    template<> template<> float ACosEstimate<float>::Degree<2>(float);
    template<> template<> float ACosEstimate<float>::Degree<3>(float);
    template<> template<> float ACosEstimate<float>::Degree<4>(float);
    template<> template<> float ACosEstimate<float>::Degree<5>(float);
    template<> template<> float ACosEstimate<float>::Degree<6>(float);
    template<> template<> float ACosEstimate<float>::Degree<7>(float);
    template<> template<> float ACosEstimate<float>::Degree<8>(float);

    template<> template<> double ACosEstimate<double>::Degree<1>(double);
    template<> template<> double ACosEstimate<double>::Degree<2>(double);
    template<> template<> double ACosEstimate<double>::Degree<3>(double);
    template<> template<> double ACosEstimate<double>::Degree<4>(double);
    template<> template<> double ACosEstimate<double>::Degree<5>(double);
    template<> template<> double ACosEstimate<double>::Degree<6>(double);
    template<> template<> double ACosEstimate<double>::Degree<7>(double);
    template<> template<> double ACosEstimate<double>::Degree<8>(double);
}
