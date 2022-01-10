#define GTE_THROW_ON_IMAGE2_ERRORS
#include <Mathematics/Image2.h>

namespace gte
{
    template class Image2<float>;

    struct Dummy
    {
        Dummy()
            :
            i(0),
            d(0.0)
        {
        }

        int i;
        double d;
    };

    template class Image2<Dummy>;
}
