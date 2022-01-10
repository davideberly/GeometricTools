#include <Mathematics/ATanEstimate.h>

namespace gte
{
    template<> template<> float ATanEstimate<float>::Degree<3>(float);
    template<> template<> float ATanEstimate<float>::Degree<5>(float);
    template<> template<> float ATanEstimate<float>::Degree<7>(float);
    template<> template<> float ATanEstimate<float>::Degree<9>(float);
    template<> template<> float ATanEstimate<float>::Degree<11>(float);
    template<> template<> float ATanEstimate<float>::Degree<13>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<3>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<5>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<7>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<9>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<11>(float);
    template<> template<> float ATanEstimate<float>::DegreeRR<13>(float);

    template<> template<> double ATanEstimate<double>::Degree<3>(double);
    template<> template<> double ATanEstimate<double>::Degree<5>(double);
    template<> template<> double ATanEstimate<double>::Degree<7>(double);
    template<> template<> double ATanEstimate<double>::Degree<9>(double);
    template<> template<> double ATanEstimate<double>::Degree<11>(double);
    template<> template<> double ATanEstimate<double>::Degree<13>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<3>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<5>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<7>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<9>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<11>(double);
    template<> template<> double ATanEstimate<double>::DegreeRR<13>(double);
}
