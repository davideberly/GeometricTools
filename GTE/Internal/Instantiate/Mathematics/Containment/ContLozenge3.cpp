#include <Mathematics/ContLozenge3.h>

namespace gte
{
    template bool GetContainer(int, Vector3<float> const*, Lozenge3<float>&);
    template bool InContainer(Vector3<float> const& point, Lozenge3<float> const&);

    template bool GetContainer(int, Vector3<double> const*, Lozenge3<double>&);
    template bool InContainer(Vector3<double> const& point, Lozenge3<double> const&);
}
