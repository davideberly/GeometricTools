#include <Mathematics/LexicoArray2.h>

namespace gte
{
    template class LexicoArray2<true, float>;
    template class LexicoArray2<false, float>;
    template class LexicoArray2<true, double, 2, 3>;
    template class LexicoArray2<false, double, 2, 3>;
}
