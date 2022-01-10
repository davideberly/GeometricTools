#include <Mathematics/DistPointAlignedBox.h>

namespace gte
{
    template class DCPQuery<float, Vector<2, float>, AlignedBox<2, float>>;
    template class DCPQuery<double, Vector<3, double>, AlignedBox<3, double>>;
}
