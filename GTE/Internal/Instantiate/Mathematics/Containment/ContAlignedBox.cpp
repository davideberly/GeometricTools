#include <Mathematics/ContAlignedBox.h>

namespace gte
{
    template bool GetContainer(int, Vector<3, float> const*, AlignedBox<3, float>&);
    template bool InContainer(Vector<3, float> const&, AlignedBox<3, float> const&);
    template bool MergeContainers(AlignedBox<3, float> const&, AlignedBox<3, float> const&, AlignedBox<3, float>&);

    template bool GetContainer(int, Vector<3, double> const*, AlignedBox<3, double>&);
    template bool InContainer(Vector<3, double> const&, AlignedBox<3, double> const&);
    template bool MergeContainers(AlignedBox<3, double> const&, AlignedBox<3, double> const&, AlignedBox<3, double>&);
}
