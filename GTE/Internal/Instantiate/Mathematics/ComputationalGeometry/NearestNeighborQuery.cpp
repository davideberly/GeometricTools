#include <Mathematics/NearestNeighborQuery.h>
#include <Mathematics/Vector.h>

namespace gte
{
    using Site0 = PositionSite<3, float>;
    template class NearestNeighborQuery<3, float, Site0>;
    using NNQuery0 = NearestNeighborQuery<3, float, Site0>;
    template int NNQuery0::FindNeighbors<3>(Vector<3, float> const&, float, std::array<int, 3>&) const;
    template int NNQuery0::FindNeighbors<1>(Vector<3, float> const&, float, std::array<int, 1>&) const;

    using Site1 = PositionDirectionSite<3, float>;
    template class NearestNeighborQuery<3, float, Site1>;
    using NNQuery1 = NearestNeighborQuery<3, float, Site1>;
    template int NNQuery1::FindNeighbors<3>(Vector<3, float> const&, float, std::array<int, 3>&) const;
    template int NNQuery1::FindNeighbors<1>(Vector<3, float> const&, float, std::array<int, 1>&) const;
}
