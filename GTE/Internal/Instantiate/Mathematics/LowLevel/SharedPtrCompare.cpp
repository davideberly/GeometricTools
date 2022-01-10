#include <Mathematics/SharedPtrCompare.h>

namespace gte
{
    template struct SharedPtrEQ<int>;
    template struct SharedPtrNEQ<int>;
    template struct SharedPtrLT<int>;
    template struct SharedPtrLTE<int>;
    template struct SharedPtrGT<int>;
    template struct SharedPtrGTE<int>;
}
