// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

#include <GTL/Mathematics/Primitives/ND/Hypersphere.h>

namespace gtl
{
    // Template alias to expose Circle2<T>. The name is more descriptive
    // than Hypersphere2<T>.
    template <typename T> using Circle2 = Hypersphere<T, 2>;
}
