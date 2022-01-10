#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ConvexMesh3.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;
    using RationalN = BSRational<UIntegerFP32<16>>;
    template class ConvexMesh3<Rational>;
    template class ConvexMesh3<RationalN>;
}
