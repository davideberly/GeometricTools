#include <Mathematics/IntrConvexPolygonHyperplane.h>

namespace gte
{
    template class TIQuery<float, std::vector<Vector<2, float>>, Hyperplane<2, float>>;
    template class FIQuery<float, std::vector<Vector<2, float>>, Hyperplane<2, float>>;
    template class TIQuery<float, std::vector<Vector<3, float>>, Hyperplane<3, float>>;
    template class FIQuery<float, std::vector<Vector<3, float>>, Hyperplane<3, float>>;

    template class TIQuery<double, std::vector<Vector<2, double>>, Hyperplane<2, double>>;
    template class FIQuery<double, std::vector<Vector<2, double>>, Hyperplane<2, double>>;
    template class TIQuery<double, std::vector<Vector<3, double>>, Hyperplane<3, double>>;
    template class FIQuery<double, std::vector<Vector<3, double>>, Hyperplane<3, double>>;
}
