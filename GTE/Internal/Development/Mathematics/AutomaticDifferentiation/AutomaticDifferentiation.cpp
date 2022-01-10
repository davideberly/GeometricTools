#include "AutomaticDifferentiation.h"
#include "KbsInstructionSet.h"
#include <set>
#include <vector>
using namespace gte;

namespace gte
{
    template class DualNumber<float>;
    template class DualNumber<double>;
}

class MyFunction
{
public:
    template <typename Real>
    Real operator() (Real const& x) const
    {
        return exp(sin(x));
    }
};

template <typename T>
T MyDerivative(T x)
{
    return exp(sin(x)) * cos(x);
}

int main()
{
    InstructionSet instructionSet;

    MyFunction F;
    AutoDifferentiator<float, MyFunction> evaluator(F);
    float y, dy;
    evaluator(1.0f, y, dy);
    float dy0 = MyDerivative(1.0f);
    float diff = dy - dy0;
    (void)diff;

    return 0;
}
