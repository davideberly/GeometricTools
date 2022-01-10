#include <Mathematics/TanEstimate.h>

namespace gte
{
    template<> template<> float TanEstimate<float>::Degree<3>(float);
    template<> template<> float TanEstimate<float>::Degree<5>(float);
    template<> template<> float TanEstimate<float>::Degree<7>(float);
    template<> template<> float TanEstimate<float>::Degree<9>(float);
    template<> template<> float TanEstimate<float>::Degree<11>(float);
    template<> template<> float TanEstimate<float>::Degree<13>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<3>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<5>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<7>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<9>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<11>(float);
    template<> template<> float TanEstimate<float>::DegreeRR<13>(float);

    template<> template<> double TanEstimate<double>::Degree<3>(double);
    template<> template<> double TanEstimate<double>::Degree<5>(double);
    template<> template<> double TanEstimate<double>::Degree<7>(double);
    template<> template<> double TanEstimate<double>::Degree<9>(double);
    template<> template<> double TanEstimate<double>::Degree<11>(double);
    template<> template<> double TanEstimate<double>::Degree<13>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<3>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<5>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<7>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<9>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<11>(double);
    template<> template<> double TanEstimate<double>::DegreeRR<13>(double);
}
