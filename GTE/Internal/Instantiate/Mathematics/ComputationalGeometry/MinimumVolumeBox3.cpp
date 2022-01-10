#include <Mathematics/MinimumVolumeBox3.h>

namespace gte
{
    template class MinimumVolumeBox3<float, true>;
    template class MinimumVolumeBox3<double, true>;
    template class MinimumVolumeBox3<float, false>;
    template class MinimumVolumeBox3<double, false>;
}
