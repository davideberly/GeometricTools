#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/PolygonTree.h>

namespace gte
{
    using Rational = BSNumber<UIntegerAP32>;

    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<float> const&, Vector2<float> const*);
    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<double> const&, Vector2<double> const*);
    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<Rational> const&, Vector2<Rational> const*);

    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<float> const&,
        std::vector<std::array<int, 3>> const&, std::vector<size_t> const&, Vector2<float> const*);
    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<double> const&,
        std::vector<std::array<int, 3>> const&, std::vector<size_t> const&, Vector2<double> const*);
    template std::pair<size_t, size_t> PolygonTreeEx::GetContainingTriangle(Vector2<Rational> const&,
        std::vector<std::array<int, 3>> const&, std::vector<size_t> const&, Vector2<Rational> const*);

    template size_t PolygonTreeEx::GetContainingTriangle(Vector2<float> const&,
        std::vector<std::array<int, 3>> const&, int64_t, Vector2<float> const* points);
    template size_t PolygonTreeEx::GetContainingTriangle(Vector2<double> const&,
        std::vector<std::array<int, 3>> const&, int64_t, Vector2<double> const* points);
    template size_t PolygonTreeEx::GetContainingTriangle(Vector2<Rational> const&,
        std::vector<std::array<int, 3>> const&, int64_t, Vector2<Rational> const* points);
}
