#include <Mathematics/LDLTDecomposition.h>

namespace gte
{
    template class LDLTDecomposition<float, 4>;
    template class LDLTDecomposition<float>;
    template class BlockLDLTDecomposition<float, 4, 10>;
    template class BlockLDLTDecomposition<float>;

    template class LDLTDecomposition<double, 4>;
    template class LDLTDecomposition<double>;
    template class BlockLDLTDecomposition<double, 4, 10>;
    template class BlockLDLTDecomposition<double>;
}
