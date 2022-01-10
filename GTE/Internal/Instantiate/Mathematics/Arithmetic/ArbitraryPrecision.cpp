#include <Mathematics/ArbitraryPrecision.h>
using namespace gte;

// Disable the warning "conditional expression is constant".
#if defined(__MSWINDOWS__)
#pragma warning(disable : 4127)
#endif

int TestIsArbitraryPrecision()
{
    int count = 0;

    if (is_arbitrary_precision<float>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<double>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<long double>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<BSNumber<UIntegerAP32>>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<BSRational<UIntegerAP32>>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<BSNumber<UIntegerFP32<4>>>::value)
    {
        ++count;
    }

    if (is_arbitrary_precision<BSRational<UIntegerFP32<4>>>::value)
    {
        ++count;
    }

    return count;
}

int TestHasDivisionOperator()
{
    int count = 0;

    if (has_division_operator<float>::value)
    {
        ++count;
    }

    if (has_division_operator<double>::value)
    {
        ++count;
    }

    if (has_division_operator<long double>::value)
    {
        ++count;
    }

    if (has_division_operator<BSNumber<UIntegerAP32>>::value)
    {
        ++count;
    }

    if (has_division_operator<BSRational<UIntegerAP32>>::value)
    {
        ++count;
    }

    if (has_division_operator<BSNumber<UIntegerFP32<4>>>::value)
    {
        ++count;
    }

    if (has_division_operator<BSRational<UIntegerFP32<4>>>::value)
    {
        ++count;
    }

    return count;
}