// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

#include <GTL/Mathematics/Primitives/ND/Hyperellipsoid.h>

namespace gtl
{
    // Template alias to expose Ellipse2<T>. The name is more descriptive
    // than Hyperellipsoid2<T>.
    template <typename T> using Ellipse2 = Hyperellipsoid<T, 2>;
}
