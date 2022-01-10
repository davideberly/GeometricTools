#include <Mathematics/Image.h>

namespace gte
{
    template class Image<float>;

    struct Dummy
    {
        Dummy()
            :
            i(0),
            d(0.0),
            v{}
        {
        }

        int i;
        double d;
        std::vector<int> v;
    };

    template class Image<Dummy>;
}
