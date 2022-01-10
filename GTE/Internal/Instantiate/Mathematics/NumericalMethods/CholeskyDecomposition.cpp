#include <Mathematics/CholeskyDecomposition.h>

namespace gte
{
    template class CholeskyDecomposition<float, 4>;
    template class CholeskyDecomposition<float>;
    template class BlockCholeskyDecomposition<float, 4, 10>;
    template class BlockCholeskyDecomposition<float>;

    template class CholeskyDecomposition<double, 4>;
    template class CholeskyDecomposition<double>;
    template class BlockCholeskyDecomposition<double, 4, 10>;
    template class BlockCholeskyDecomposition<double>;
}
