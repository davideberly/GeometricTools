#include <Mathematics/DistAlignedBoxAlignedBox.h>

namespace gte
{
    template class DCPQuery<float, AlignedBox<3, float>, AlignedBox<3, float>>;
    template class DCPQuery<double, AlignedBox<2, double>, AlignedBox<2, double>>;
}
