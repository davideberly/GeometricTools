// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct Segment
{
    VECREAL end0, end1;
};

struct Result
{
    REAL sqrDistance;
    REAL parameter[3];
#if GET_CLOSEST
    VECREAL closest[2];
#endif
};

REAL GetClampedRoot(REAL slope, REAL h0, REAL h1)
{
    REAL r;
    if (h0 < 0)
    {
        if (h1 > 0)
        {
            r = -h0 / slope;
            if (r > 1)
            {
                r = 0.5;
            }
        }
        else
        {
            r = 1;
        }
    }
    else
    {
        r = 0;
    }
    return r;
}

void ComputeIntersection(REAL b, REAL f00, REAL f10, REAL sValue[2],
    int classify[2], out int edge[2], out REAL end[2][2])
{
    if (classify[0] < 0)
    {
        edge[0] = 0;
        end[0][0] = 0;
        end[0][1] = f00 / b;
        if (end[0][1] < 0 || end[0][1] > 1)
        {
            end[0][1] = 0.5;
        }

        if (classify[1] == 0)
        {
            edge[1] = 3;
            end[1][0] = sValue[1];
            end[1][1] = 1;
        }
        else
        {
            edge[1] = 1;
            end[1][0] = 1;
            end[1][1] = f10 / b;
            if (end[1][1] < 0 || end[1][1] > 1)
            {
                end[1][1] = 0.5;
            }
        }
    }
    else if (classify[0] == 0)
    {
        edge[0] = 2;
        end[0][0] = sValue[0];
        end[0][1] = 0;

        if (classify[1] < 0)
        {
            edge[1] = 0;
            end[1][0] = 0;
            end[1][1] = f00 / b;
            if (end[1][1] < 0 || end[1][1] > 1)
            {
                end[1][1] = 0.5;
            }
        }
        else if (classify[1] == 0)
        {
            edge[1] = 3;
            end[1][0] = sValue[1];
            end[1][1] = 1;
        }
        else
        {
            edge[1] = 1;
            end[1][0] = 1;
            end[1][1] = f10 / b;
            if (end[1][1] < 0 || end[1][1] > 1)
            {
                end[1][1] = 0.5;
            }
        }
    }
    else
    {
        edge[0] = 1;
        end[0][0] = 1;
        end[0][1] = f10 / b;
        if (end[0][1] < 0 || end[0][1] > 1)
        {
            end[0][1] = 0.5;
        }

        if (classify[1] == 0)
        {
            edge[1] = 3;
            end[1][0] = sValue[1];
            end[1][1] = 1;
        }
        else
        {
            edge[1] = 0;
            end[1][0] = 0;
            end[1][1] = f00 / b;
            if (end[1][1] < 0 || end[1][1] > 1)
            {
                end[1][1] = 0.5;
            }
        }
    }
}

void ComputeMinimumParameters(REAL b, REAL c, REAL e,
    REAL g00, REAL g10, REAL g01, REAL g11,
    int edge[2], REAL end[2][2], out REAL parameter[3])
{
    REAL delta = end[1][1] - end[0][1];
    REAL h0 = delta * (-b * end[0][0] + c * end[0][1] - e);
    if (h0 >= 0)
    {
        if (edge[0] == 0)
        {
            parameter[0] = 0;
            parameter[1] = GetClampedRoot(c, g00, g01);
        }
        else if (edge[0] == 1)
        {
            parameter[0] = 1;
            parameter[1] = GetClampedRoot(c, g10, g11);
        }
        else
        {
            parameter[0] = end[0][0];
            parameter[1] = end[0][1];
        }
    }
    else
    {
        REAL h1 = delta * (-b * end[1][0] + c * end[1][1] - e);
        if (h1 <= 0)
        {
            if (edge[1] == 0)
            {
                parameter[0] = 0;
                parameter[1] = GetClampedRoot(c, g00, g01);
            }
            else if (edge[1] == 1)
            {
                parameter[0] = 1;
                parameter[1] = GetClampedRoot(c, g10, g11);
            }
            else
            {
                parameter[0] = end[1][0];
                parameter[1] = end[1][1];
            }
        }
        else
        {
            REAL z = clamp(h0 / (h0 - h1), 0.0, 1.0);
            REAL omz = 1 - z;
            parameter[0] = omz * end[0][0] + z * end[1][0];
            parameter[1] = omz * end[0][1] + z * end[1][1];
        }
    }
    parameter[2] = 0;
}

void DoCompute(Segment segment0, Segment segment1, inout Result result)
{
    VECREAL P0 = segment0.end0;
    VECREAL P1 = segment0.end1;
    VECREAL Q0 = segment1.end0;
    VECREAL Q1 = segment1.end1;

    VECREAL P1mP0 = P1 - P0;
    VECREAL Q1mQ0 = Q1 - Q0;
    VECREAL P0mQ0 = P0 - Q0;
    REAL a = dot(P1mP0, P1mP0);
    REAL b = dot(P1mP0, Q1mQ0);
    REAL c = dot(Q1mQ0, Q1mQ0);
    REAL d = dot(P1mP0, P0mQ0);
    REAL e = dot(Q1mQ0, P0mQ0);

    REAL f00 = d;
    REAL f10 = f00 + a;
    REAL f01 = f00 - b;
    REAL f11 = f10 - b;
    REAL g00 = -e;
    REAL g01 = g00 + c;
    REAL g10 = g00 - b;
    REAL g11 = g10 + c;

    if (a > 0 && c > 0)
    {
        REAL sValue[2];
        sValue[0] = GetClampedRoot(a, f00, f10);
        sValue[1] = GetClampedRoot(a, f01, f11);

        int classify[2];
        for (int i = 0; i < 2; ++i)
        {
            if (sValue[i] <= 0)
            {
                classify[i] = -1;
            }
            else if (sValue[i] >= 1)
            {
                classify[i] = 1;
            }
            else
            {
                classify[i] = 0;
            }
        }

        if (classify[0] == -1 && classify[1] == -1)
        {
            result.parameter[0] = 0;
            result.parameter[1] = GetClampedRoot(c, g00, g01);
            result.parameter[2] = 0;
        }
        else if (classify[0] == 1 && classify[1] == 1)
        {
            result.parameter[0] = 1;
            result.parameter[1] = GetClampedRoot(c, g10, g11);
            result.parameter[2] = 0;
        }
        else
        {
            int edge[2];
            REAL end[2][2];
            ComputeIntersection(b, f00, f10, sValue, classify, edge, end);
            ComputeMinimumParameters(b, c, e, g00, g10, g01, g11,
                edge, end, result.parameter);
        }
    }
    else
    {
        if (a > 0)
        {
            result.parameter[0] = GetClampedRoot(a, f00, f10);
            result.parameter[1] = 0;
            result.parameter[2] = 0;
        }
        else if (c > 0)
        {
            result.parameter[0] = 0;
            result.parameter[1] = GetClampedRoot(c, g00, g01);
            result.parameter[2] = 0;
        }
        else
        {
            result.parameter[0] = 0;
            result.parameter[1] = 0;
            result.parameter[2] = 0;
        }
    }

    VECREAL closest0 = (1 - result.parameter[0]) * P0 + result.parameter[0] * P1;
    VECREAL closest1 = (1 - result.parameter[1]) * Q0 + result.parameter[1] * Q1;
    VECREAL diff = closest0 - closest1;
    result.sqrDistance = dot(diff, diff);
#if GET_CLOSEST
    result.closest[0] = closest0;
    result.closest[1] = closest1;
#endif
}

uniform Block
{
    uvec2 origin;
};

buffer inSegment { Segment data[]; } inSegmentSB;
buffer outResult { Result data[]; } outResultSB;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    Segment segment0 = inSegmentSB.data[origin.x + dt.x];
    Segment segment1 = inSegmentSB.data[origin.y + dt.y];

    Result result;
    DoCompute(segment0, segment1, result);

    outResultSB.data[dt.x + BLOCK_SIZE * dt.y] = result;
}
