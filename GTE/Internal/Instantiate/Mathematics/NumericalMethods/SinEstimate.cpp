#include <Mathematics/SinEstimate.h>

namespace gte
{
    template<> template<> float SinEstimate<float>::Degree<3>(float);
    template<> template<> float SinEstimate<float>::Degree<5>(float);
    template<> template<> float SinEstimate<float>::Degree<7>(float);
    template<> template<> float SinEstimate<float>::Degree<9>(float);
    template<> template<> float SinEstimate<float>::Degree<11>(float);
    template<> template<> float SinEstimate<float>::DegreeRR<3>(float);
    template<> template<> float SinEstimate<float>::DegreeRR<5>(float);
    template<> template<> float SinEstimate<float>::DegreeRR<7>(float);
    template<> template<> float SinEstimate<float>::DegreeRR<9>(float);
    template<> template<> float SinEstimate<float>::DegreeRR<11>(float);

    template<> template<> double SinEstimate<double>::Degree<3>(double);
    template<> template<> double SinEstimate<double>::Degree<5>(double);
    template<> template<> double SinEstimate<double>::Degree<7>(double);
    template<> template<> double SinEstimate<double>::Degree<9>(double);
    template<> template<> double SinEstimate<double>::Degree<11>(double);
    template<> template<> double SinEstimate<double>::DegreeRR<3>(double);
    template<> template<> double SinEstimate<double>::DegreeRR<5>(double);
    template<> template<> double SinEstimate<double>::DegreeRR<7>(double);
    template<> template<> double SinEstimate<double>::DegreeRR<9>(double);
    template<> template<> double SinEstimate<double>::DegreeRR<11>(double);
}
