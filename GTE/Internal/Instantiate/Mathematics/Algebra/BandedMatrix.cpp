#include <Mathematics/BandedMatrix.h>

namespace gte
{
    template class BandedMatrix<float>;
    template bool BandedMatrix<float>::SolveSystem<true>(float*, int);
    template bool BandedMatrix<float>::ComputeInverse<true>(float*) const;

    template class BandedMatrix<double>;
    template bool BandedMatrix<double>::SolveSystem<true>(double*, int);
    template bool BandedMatrix<double>::ComputeInverse<true>(double*) const;
}
