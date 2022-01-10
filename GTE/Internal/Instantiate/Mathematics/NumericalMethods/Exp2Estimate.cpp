#include <Mathematics/Exp2Estimate.h>

namespace gte
{
    template<> template<> float Exp2Estimate<float>::Degree<1>(float);
    template<> template<> float Exp2Estimate<float>::Degree<2>(float);
    template<> template<> float Exp2Estimate<float>::Degree<3>(float);
    template<> template<> float Exp2Estimate<float>::Degree<4>(float);
    template<> template<> float Exp2Estimate<float>::Degree<5>(float);
    template<> template<> float Exp2Estimate<float>::Degree<6>(float);
    template<> template<> float Exp2Estimate<float>::Degree<7>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<1>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<2>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<3>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<4>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<5>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<6>(float);
    template<> template<> float Exp2Estimate<float>::DegreeRR<7>(float);

    template<> template<> double Exp2Estimate<double>::Degree<1>(double);
    template<> template<> double Exp2Estimate<double>::Degree<2>(double);
    template<> template<> double Exp2Estimate<double>::Degree<3>(double);
    template<> template<> double Exp2Estimate<double>::Degree<4>(double);
    template<> template<> double Exp2Estimate<double>::Degree<5>(double);
    template<> template<> double Exp2Estimate<double>::Degree<6>(double);
    template<> template<> double Exp2Estimate<double>::Degree<7>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<1>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<2>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<3>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<4>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<5>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<6>(double);
    template<> template<> double Exp2Estimate<double>::DegreeRR<7>(double);
}
