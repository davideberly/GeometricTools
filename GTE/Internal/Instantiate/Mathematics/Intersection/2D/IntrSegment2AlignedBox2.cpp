#include <Mathematics/IntrSegment2AlignedBox2.h>

namespace gte
{
    template class TIQuery<float, Segment2<float>, AlignedBox2<float>>;
    template class FIQuery<float, Segment2<float>, AlignedBox2<float>>;

    template class TIQuery<double, Segment2<double>, AlignedBox2<double>>;
    template class FIQuery<double, Segment2<double>, AlignedBox2<double>>;
}
