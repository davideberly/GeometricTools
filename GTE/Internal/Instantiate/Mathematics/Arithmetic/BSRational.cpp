#include <Mathematics/UIntegerAP32.h>
#include <Mathematics/UIntegerFP32.h>
#include <Mathematics/BSRational.h>

namespace gte
{
    template class BSRational<UIntegerAP32>;
    template void Convert(BSRational<UIntegerAP32> const&, int32_t, int32_t, BSNumber<UIntegerAP32>&);
    template void Convert(BSRational<UIntegerAP32> const&, int32_t, int32_t, BSRational<UIntegerAP32>&);
    template void gte::Convert(BSRational<UIntegerAP32> const&, int32_t, float&);
    template void gte::Convert(BSRational<UIntegerAP32> const&, int32_t, double&);

    // The MSVS code analysis generates these warnings. TODO: Figure out what
    // the analysis is doing so I can write a test program that leads to
    // failure.
    // 
    // UIntegerALU32.h(413) : warning C28020 : The expression '0<=_Param_(1)&&_Param_(1)<=4-1' is not true at this call.
    // UIntegerALU32.h(181) : warning C28020 : The expression '0<=_Param_(1)&&_Param_(1)<=4-1' is not true at this call.
    template class BSRational<UIntegerFP32<4>>;
    template void Convert(BSRational<UIntegerFP32<4>> const&, int32_t, int32_t, BSNumber<UIntegerFP32<4>>&);
    template void Convert(BSRational<UIntegerFP32<4>> const&, int32_t, int32_t, BSRational<UIntegerFP32<4>>&);
}

namespace std
{
    template gte::BSRational<gte::UIntegerAP32> acos(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> acos(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> acosh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> acosh(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> asin(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> asin(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> asinh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> asinh(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> atan(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> atan(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> atanh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> atanh(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> atan2(gte::BSRational<gte::UIntegerAP32> const&, gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> atan2(gte::BSRational<gte::UIntegerFP32<8>> const&, gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> ceil(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> ceil(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> cos(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> cos(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> cosh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> cosh(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> exp(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> exp(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> exp2(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> exp2(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> fabs(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> fabs(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> floor(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> floor(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> fmod(gte::BSRational<gte::UIntegerAP32> const&, gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> fmod(gte::BSRational<gte::UIntegerFP32<8>> const&, gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> frexp(gte::BSRational<gte::UIntegerAP32> const&, int*);
    template gte::BSRational<gte::UIntegerFP32<8>> frexp(gte::BSRational<gte::UIntegerFP32<8>> const&, int*);

    template gte::BSRational<gte::UIntegerAP32> ldexp(gte::BSRational<gte::UIntegerAP32> const&, int);
    template gte::BSRational<gte::UIntegerFP32<8>> ldexp(gte::BSRational<gte::UIntegerFP32<8>> const&, int);

    template gte::BSRational<gte::UIntegerAP32> log(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> log(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> log2(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> log2(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> log10(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> log10(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> pow(gte::BSRational<gte::UIntegerAP32> const&, gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> pow(gte::BSRational<gte::UIntegerFP32<8>> const&, gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> sin(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> sin(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> sinh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> sinh(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> sqrt(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> sqrt(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> tan(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> tan(gte::BSRational<gte::UIntegerFP32<8>> const&);

    template gte::BSRational<gte::UIntegerAP32> tanh(gte::BSRational<gte::UIntegerAP32> const&);
    template gte::BSRational<gte::UIntegerFP32<8>> tanh(gte::BSRational<gte::UIntegerFP32<8>> const&);
}

namespace gte
{
    template BSRational<UIntegerAP32> atandivpi(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> atandivpi(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> atan2divpi(BSRational<UIntegerAP32> const&, BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> atan2divpi(BSRational<UIntegerFP32<8>> const&, BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> cospi(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> cospi(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> exp10(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> exp10(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> invsqrt(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> invsqrt(BSRational<UIntegerFP32<8>> const&);

    template int isign(BSRational<UIntegerAP32> const&);
    template int isign(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> sign(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> sign(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> sinpi(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> sinpi(BSRational<UIntegerFP32<8>> const&);

    template BSRational<UIntegerAP32> sqr(BSRational<UIntegerAP32> const&);
    template BSRational<UIntegerFP32<8>> sqr(BSRational<UIntegerFP32<8>> const&);
}
