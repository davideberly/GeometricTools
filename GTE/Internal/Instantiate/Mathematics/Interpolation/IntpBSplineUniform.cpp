#include <Mathematics/IntpBSplineUniform.h>

namespace gte
{
    template <typename T>
    struct Controls1
    {
        using Type = T;

        int GetSize(int) const
        {
            return static_cast<int>(signal.size());
        }

        T operator() (int const* tuple) const
        {
            return signal[tuple[0]];
        }

#if !defined(GTE_INTP_BSPLINE_UNIFORM_NO_SPECIALIZATION)
        T operator() (int x) const
        {
            return signal[x];
        }
#endif

        std::vector<T> signal;
    };

    template <typename T>
    struct Controls2
    {
        using Type = T;

        int GetSize(int i) const
        {
            return size[i];
        }

        T operator() (int const* tuple) const
        {
            return image[tuple[0] + static_cast<size_t>(size[0]) * tuple[1]];
        }

#if !defined(GTE_INTP_BSPLINE_UNIFORM_NO_SPECIALIZATION)
        T operator() (int x, int y) const
        {
            return image[x + static_cast<size_t>(size[0]) * y];
        }
#endif

        std::array<int, 2> size;
        std::vector<T> image;
    };

    template <typename T>
    struct Controls3
    {
        using Type = T;

        int GetSize(int i) const
        {
            return size[i];
        }

        T operator() (int const* tuple) const
        {
            return image[tuple[0] + static_cast<size_t>(size[0]) * (tuple[1] + static_cast<size_t>(size[1]) * tuple[2])];
        }

#if !defined(GTE_INTP_BSPLINE_UNIFORM_NO_SPECIALIZATION)
        T operator() (int x, int y, int z) const
        {
            return image[x + static_cast<size_t>(size[0]) * (y + static_cast<size_t>(size[1]) * z)];
        }
#endif

        std::array<int, 3> size;
        std::vector<T> image;
    };
}

namespace gte
{
    template class IntpBSplineUniform<float, Controls1<float>>;
    template class IntpBSplineUniform<float, Controls1<float>, 1>;
    template class IntpBSplineUniform<float, Controls2<float>>;
    template class IntpBSplineUniform<float, Controls2<float>, 2>;
    template class IntpBSplineUniform<float, Controls3<float>>;
    template class IntpBSplineUniform<float, Controls3<float>, 3>;

    template class IntpBSplineUniform<double, Controls1<double>>;
    template class IntpBSplineUniform<double, Controls1<double>, 1>;
    template class IntpBSplineUniform<double, Controls2<double>>;
    template class IntpBSplineUniform<double, Controls2<double>, 2>;
    template class IntpBSplineUniform<double, Controls3<double>>;
    template class IntpBSplineUniform<double, Controls3<double>, 3>;
}
