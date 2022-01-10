#include <Mathematics/UIntegerAP32.h>
#include <Mathematics/UIntegerFP32.h>
#include <Mathematics/BSNumber.h>

namespace gte
{
    template class BSNumber<UIntegerAP32>;
    template class BSNumber<UIntegerFP32<4>>;
    template void Convert(BSNumber<UIntegerAP32> const&, int32_t, int32_t, BSNumber<UIntegerAP32>&);

    // The MSVS code analysis generates these warnings. TODO: Figure out what
    // the analysis is doing so I can write a test program that leads to
    // failure.
    // 
    // UIntegerALU32.h(413) : warning C28020 : The expression '0<=_Param_(1)&&_Param_(1)<=4-1' is not true at this call.
    // UIntegerALU32.h(181) : warning C28020 : The expression '0<=_Param_(1)&&_Param_(1)<=4-1' is not true at this call.
    // BSNumber.h(1113) : warning C28020 : The expression '0<=_Param_(1)&&_Param_(1)<=4-1' is not true at this call.

    template void Convert(BSNumber<UIntegerFP32<4>> const&, int32_t, int32_t, BSNumber<UIntegerFP32<4>>&);
}

namespace std
{
    template gte::BSNumber<gte::UIntegerAP32> acos(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> acos(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> acosh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> acosh(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> asin(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> asin(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> asinh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> asinh(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> atan(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> atan(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> atanh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> atanh(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> atan2(gte::BSNumber<gte::UIntegerAP32> const&, gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> atan2(gte::BSNumber<gte::UIntegerFP32<8>> const&, gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> ceil(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> ceil(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> cos(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> cos(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> cosh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> cosh(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> exp(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> exp(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> exp2(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> exp2(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> fabs(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> fabs(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> floor(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> floor(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> fmod(gte::BSNumber<gte::UIntegerAP32> const&, gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> fmod(gte::BSNumber<gte::UIntegerFP32<8>> const&, gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> frexp(gte::BSNumber<gte::UIntegerAP32> const&, int*);
    template gte::BSNumber<gte::UIntegerFP32<8>> frexp(gte::BSNumber<gte::UIntegerFP32<8>> const&, int*);

    template gte::BSNumber<gte::UIntegerAP32> ldexp(gte::BSNumber<gte::UIntegerAP32> const&, int);
    template gte::BSNumber<gte::UIntegerFP32<8>> ldexp(gte::BSNumber<gte::UIntegerFP32<8>> const&, int);

    template gte::BSNumber<gte::UIntegerAP32> log(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> log(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> log2(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> log2(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> log10(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> log10(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> pow(gte::BSNumber<gte::UIntegerAP32> const&, gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> pow(gte::BSNumber<gte::UIntegerFP32<8>> const&, gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> sin(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> sin(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> sinh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> sinh(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> sqrt(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> sqrt(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> tan(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> tan(gte::BSNumber<gte::UIntegerFP32<8>> const&);

    template gte::BSNumber<gte::UIntegerAP32> tanh(gte::BSNumber<gte::UIntegerAP32> const&);
    template gte::BSNumber<gte::UIntegerFP32<8>> tanh(gte::BSNumber<gte::UIntegerFP32<8>> const&);
}

namespace gte
{
    template BSNumber<UIntegerAP32> atandivpi(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> atandivpi(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> atan2divpi(BSNumber<UIntegerAP32> const&, BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> atan2divpi(BSNumber<UIntegerFP32<8>> const&, BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> clamp(BSNumber<UIntegerAP32> const&, BSNumber<UIntegerAP32> const&, BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> clamp(BSNumber<UIntegerFP32<8>> const&, BSNumber<UIntegerFP32<8>> const&, BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> cospi(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> cospi(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> exp10(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> exp10(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> invsqrt(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> invsqrt(BSNumber<UIntegerFP32<8>> const&);

    template int isign(BSNumber<UIntegerAP32> const&);
    template int isign(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> saturate(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> saturate(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> sign(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> sign(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> sinpi(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> sinpi(BSNumber<UIntegerFP32<8>> const&);

    template BSNumber<UIntegerAP32> sqr(BSNumber<UIntegerAP32> const&);
    template BSNumber<UIntegerFP32<8>> sqr(BSNumber<UIntegerFP32<8>> const&);
}
