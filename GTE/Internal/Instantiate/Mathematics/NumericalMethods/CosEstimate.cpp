#include <Mathematics/CosEstimate.h>

namespace gte
{
    template<> template<> float CosEstimate<float>::Degree<2>(float);
    template<> template<> float CosEstimate<float>::Degree<4>(float);
    template<> template<> float CosEstimate<float>::Degree<6>(float);
    template<> template<> float CosEstimate<float>::Degree<8>(float);
    template<> template<> float CosEstimate<float>::Degree<10>(float);
    template<> template<> float CosEstimate<float>::DegreeRR<2>(float);
    template<> template<> float CosEstimate<float>::DegreeRR<4>(float);
    template<> template<> float CosEstimate<float>::DegreeRR<6>(float);
    template<> template<> float CosEstimate<float>::DegreeRR<8>(float);
    template<> template<> float CosEstimate<float>::DegreeRR<10>(float);

    template<> template<> double CosEstimate<double>::Degree<2>(double);
    template<> template<> double CosEstimate<double>::Degree<4>(double);
    template<> template<> double CosEstimate<double>::Degree<6>(double);
    template<> template<> double CosEstimate<double>::Degree<8>(double);
    template<> template<> double CosEstimate<double>::Degree<10>(double);
    template<> template<> double CosEstimate<double>::DegreeRR<2>(double);
    template<> template<> double CosEstimate<double>::DegreeRR<4>(double);
    template<> template<> double CosEstimate<double>::DegreeRR<6>(double);
    template<> template<> double CosEstimate<double>::DegreeRR<8>(double);
    template<> template<> double CosEstimate<double>::DegreeRR<10>(double);
}
