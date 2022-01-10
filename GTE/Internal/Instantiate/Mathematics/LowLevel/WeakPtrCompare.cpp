#include <Mathematics/WeakPtrCompare.h>

namespace gte
{
    template struct WeakPtrEQ<int>;
    template struct WeakPtrNEQ<int>;
    template struct WeakPtrLT<int>;
    template struct WeakPtrLTE<int>;
    template struct WeakPtrGT<int>;
    template struct WeakPtrGTE<int>;
}
