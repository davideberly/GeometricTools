#include <Mathematics/ImageUtility2.h>

namespace gte
{
    template void ImageUtility2::FloodFill4(Image2<unsigned int>&, int, int, unsigned int, unsigned int);
    template void ImageUtility2::DrawFloodFill4(int, int, int, int, unsigned int, unsigned int, std::function<void(int, int, unsigned int)> const&, std::function<unsigned int(int, int)> const&);
}
