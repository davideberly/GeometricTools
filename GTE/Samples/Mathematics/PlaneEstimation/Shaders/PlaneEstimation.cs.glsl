// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(rgba32f) uniform readonly image2D positions;
layout(rgba32f) uniform writeonly image2D planes;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);

    // Valid positions are of the form (x,y,z,1), where z > 0.  Missing data
    // are of the form (x,y,0,0).  We use this information in the computations.
    vec4 position = imageLoad(positions, t);
    if (position.w > 0.0f)
    {
        // This is the center of the neighborhood, which is the xy-mean when
        // the neighborhood has no missing values.  However, it is only the
        // approximate xy-mean when the neighborhood has missing values.
        // Subtracting the approximate xy-mean will still help us avoid the
        // catastrophic cancellation discussed in the book.
        vec4 center = vec4(position.xy, 0, 0);

        // Compute the covariance matrix based on the approximate xy-mean.
        vec4 sums0 = vec4(0.0f);  // (sumXX, sumXY, sumX, sumXZ)
        vec4 sums1 = vec4(0.0f);  // (sumYX, sumYY, sumY, sumYZ)
        vec4 sums2 = vec4(0.0f);  // (sumX,  sumY,  sum1, sumZ )
        ivec2 offset;
        for (offset.y = -RADIUS; offset.y <= RADIUS; ++offset.y)
        {
            for (offset.x = -RADIUS; offset.x <= RADIUS; ++offset.x)
            {
                // Compute (X,Y,Z,W) - (CX,CY,0,0) = (X-CX,Y-CY,Z,W).
                vec4 diff = imageLoad(positions, t + offset) - center;

                // If the position is valid, its w-channel is 1 and the
                // center's w-channel is 0, so diff.w = 1.  If the position
                // is invalid, its w-channel is 0 and the center's w-channel
                // is 0, so diff.w = 0.  It suffices to use sign(diff.w) as
                // an indicator of valid or missing data.
                float valid = sign(diff.w);

                // Efficient summation using vectorization.  The matching
                // swizzles indicate the particular sum.  For example, the
                // swizzle pairs of diff.xxxx*diff.xywz are xx, xy, xw, xz.
                // For valid positions, the w-channel is 1, so the pair xw
                // corresponds to an x sum.  For diff.yyyy*diff.xywz the pairs
                // are yx, yy, yw, yz.  For valid positions, the w-channel is
                // 1, so the pair yw corresponds to a y sum.  For diff.xywz
                // and valid positions, the w swizzle corresponds to a sum of
                // the number 1, in which case sums2.z is the number of valid
                // positions in the neighborhood.
                sums0 += valid * diff.xxxx * diff.xywz;
                sums1 += valid * diff.yyyy * diff.xywz;
                sums2 += valid * diff.xywz;
            }
        }

        // Compute an estimate of the normal vector.
        if (sums2.z >= 3.0f)
        {
            // We are trying to fit z = A*x + B*y + C.  The linear system is
            // +-                  -+ +- -+   +-     -+
            // | sumXX  sumXY  sumX | | A |   | sumXZ |
            // | sumYX  sumYY  sumY | | B | = | sumYZ |
            // | sumX   sumY   sum1 | | C |   | sumZ  |
            // +-                  -+ +- -+   +-     -+
            //
            // If M is the 3x3 matrix on the left-hand side, then
            //     +-    -+
            //     | V0^T |
            // M = | V1^T |
            //     | V2^T |
            //     +-    -+
            // where V0, V1, and V2 are 3x1 vectors and their transposes form
            // the rows of M.  We know Inverse(M) = Adjoint(M)/Determinant(M).
            // It can be shown that
            //              +-                                      -+
            // Adjoint(M) = | Cross(V1,V2) Cross(V2,V0) Cross(V0,V1) |
            //              +-                                      -+
            // and Determinant(M) = Dot(V0,Cross(V1,V2)).
            vec3 V0xV1 = cross(sums0.xyz, sums1.xyz);
            vec3 V1xV2 = cross(sums1.xyz, sums2.xyz);
            vec3 V2xV0 = cross(sums2.xyz, sums0.xyz);
            float determinant = dot(sums0.xyz, V1xV2);
            vec3 detTimesABC = sums0.w * V1xV2 + sums1.w * V2xV0 + sums2.w * V0xV1;

            // Return the plane det*(A*x + B*y - z + C) = 0.  The application
            // can extract a unit-length normal (A,B,-1)/sqrt(A*A+B*B+1).
            imageStore(planes, t, vec4(detTimesABC.xy, -determinant, detTimesABC.z));
        }
        else
        {
            // The neighborhood has no valid positions.
            imageStore(planes, t, vec4(0.0f));
        }
    }
    else
    {
        // The current position is missing data.
        imageStore(planes, t, vec4(0.0f));
    }
};
