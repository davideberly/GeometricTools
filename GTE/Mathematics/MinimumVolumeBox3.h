// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.1.2025.11.23

#pragma once

// Compute a minimum volume oriented box containing a set of points. The box
// is necessarily the minimum volume box for the convex hull of the points.
// The algorithm is described in
//   J.O'Rourke, "Finding minimal enclosing boxes",
//   Internat. J. Comput. Inform. Sci., 14:183-199, 1985.
// for a set of points whose convex hull is 3-dimensional; that is, the points
// are not all the same point, not colinear, and not coplanar, which makes the
// hull a 3-dimensional polytope (convex polyhedron). According to the paper,
// the minimum volume oriented box must have at least two adjacent faces flush
// with edges of the polytope. The implementation processes all pairs of
// edges, determining for each pair the relevant box-face normals that are
// candidates for the minimum volume box. The approach of the paper involves
// computing roots of polynomials of large degree, so it is not possible to
// obtain the theoretically correct box. Instead, an estimate of the box is
// computed within floating-point tolerances.
// 
// I use an approach different from that of the details in the paper; see
//   https://www.geometrictools.com/Documentation/MinimumVolumeBox.pdf
// The computations involve an iterative minimizer for volume functions with
// domains given by line segments or hyperbolic curves. The default minimizer
// simply selects sample points on the domain of a volume function and returns
// the sample point of minimum volume. This provides a good approximation to
// the box based on the number of samples the minimizer uses in its search.
// The caller of the minimum volume box queries chooses the number of samples
// to be 2^{lgMaxSample}, where lgMaxSample is an input to the query. The
// implementation allows you to provide your own iterative minimizer using
// overrides by virtual functions; see member functions MinimizerConstantS,
// MinimizerConstantT, MinimizerVariableS, and MinimizerVariableT.
// 
// Four queries are provided to compute the box. In all cases, the minimum
// volume box and its volume are returned.
// 
// An arbitrary set of points is passed to the query. The convex hull is
// computed. If the dimension of the hull is 0, all points are the same, in
// which case a box with extents all 0 is returned. If the dimension is 1, all
// points are on a line, in which case a box with 1 positive extent and 2 zero
// extents is returned. If the dimension is 2, all points are on a plane, in
// which case a box with 2 positive extents and 1 zero extent is returned. If
// the dimension is 3, the points are volumetric, in which case a box with 3
// positive extents is returned. For dimensions 0, 1, and 2, the returned
// volume is 0. For dimension 3, the returned volume is positive.
// 
//   1. std::size_t operator()(
//          std::size_t numPoints,
//          Vector3<T> const* points,
//          std::size_t lgMaxSample,
//          OrientedBox3<T>& box,
//          T& volume);
// 
//   2. std::size_t operator()(
//          std::vector<TVector3> const& points,
//          std::size_t lgMaxSample,
//          OrientedBox3<T>& box,
//          T& volume);
// 
// The points must be the vertices of a 3-dimensional convex hull. The indices
// stores 3-tuples of indices into vertices[], each 3-tuple representing a
// triangle face of the hull. The number of triangles is numIndices/3. The
// type of the indices elements is the template parameter IndexType. The
// returned box has 3 positive extents and the returned volume is positive.
// NOTE: It is the caller's responsibility to ensure the vertices and indices
// represent a 3-dimensional convex hull with no duplicate or unused vertices
// and no duplicate or unused triangles.
// 
//   3. void operator()(
//          std::size_t numVertices,
//          Vector3<T> const* vertices,
//          std::size_t lgMaxSample,
//          std::size_t numIndices,
//          IndexType const* indices,
//          OrientedBox3<T>& box,
//          T& volume);
//
//   4. void operator()(
//          std::vector<TVector3> const& vertices,
//          std::vector<IndexType> const& indices,
//          std::size_t lgMaxSample,
//          OrientedBox3<T>& box,
//          T& volume);

#include <cstddef>
#include <type_traits>

namespace gte
{
    // Compute types for partial template specialization.
    using MVB3FloatingPoint = std::integral_constant<std::size_t, 0>;
    using MVB3Rational = std::integral_constant<std::size_t, 1>;
    using MVB3GPU = std::integral_constant<std::size_t, 2>;

    // The primary template for MinimumVolumeBox3 classes.
    template <typename T, typename IndexType, typename ComputeType>
    class MinimumVolumeBox3 {};
}

