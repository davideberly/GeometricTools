#include <Mathematics/IntrConvexMesh3Plane3.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;
    using RationalN = BSRational<UIntegerFP32<16>>;
    template class FIQuery<Rational, ConvexMesh3<Rational>, Plane3<Rational>>;
    template class FIQuery<RationalN, ConvexMesh3<RationalN>, Plane3<RationalN>>;
}
