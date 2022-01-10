#include <Mathematics/ExpEstimate.h>

namespace gte
{
    template<> template<> float ExpEstimate<float>::Degree<1>(float);
    template<> template<> float ExpEstimate<float>::Degree<2>(float);
    template<> template<> float ExpEstimate<float>::Degree<3>(float);
    template<> template<> float ExpEstimate<float>::Degree<4>(float);
    template<> template<> float ExpEstimate<float>::Degree<5>(float);
    template<> template<> float ExpEstimate<float>::Degree<6>(float);
    template<> template<> float ExpEstimate<float>::Degree<7>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<1>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<2>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<3>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<4>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<5>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<6>(float);
    template<> template<> float ExpEstimate<float>::DegreeRR<7>(float);

    template<> template<> double ExpEstimate<double>::Degree<1>(double);
    template<> template<> double ExpEstimate<double>::Degree<2>(double);
    template<> template<> double ExpEstimate<double>::Degree<3>(double);
    template<> template<> double ExpEstimate<double>::Degree<4>(double);
    template<> template<> double ExpEstimate<double>::Degree<5>(double);
    template<> template<> double ExpEstimate<double>::Degree<6>(double);
    template<> template<> double ExpEstimate<double>::Degree<7>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<1>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<2>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<3>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<4>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<5>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<6>(double);
    template<> template<> double ExpEstimate<double>::DegreeRR<7>(double);
}
