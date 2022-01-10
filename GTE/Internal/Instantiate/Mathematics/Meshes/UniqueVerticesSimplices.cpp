#include <Mathematics/UniqueVerticesSimplices.h>
#include <Mathematics/Vector.h>

namespace gte
{
    struct Vertex
    {
        Vector<3, float> position;
        Vector<4, float> color;
        bool operator< (Vertex const& vertex) const
        {
            if (position < vertex.position) { return true; }
            if (position > vertex.position) { return false; }
            if (color < vertex.color) { return true; }
            return false;
        }
    };

    template class UniqueVerticesSimplices<Vertex, size_t, 3>;
}
