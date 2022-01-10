#include <Mathematics/Image3.h>

namespace gte
{
    template class Image3<float>;

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

    template class Image3<Dummy>;
}
