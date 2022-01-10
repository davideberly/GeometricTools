#include <Mathematics/APConversion.h>

namespace gte
{
    using Rational = BSRational<UIntegerAP32>;
    template class APConversion<Rational>;
}
