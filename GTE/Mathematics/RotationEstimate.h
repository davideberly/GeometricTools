// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2024.04.26

#pragma once

// Rotation matrices can be constructed using estimates of the coefficients
// that involve trigonometric and polynomial terms. See
// https://www.geometrictools.com/Documentation/ApproximateRotationMatrix.pdf
// for the length details.

#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    // Constants for rotc0(t) = sin(t)/t.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC0_EST_COEFF =
    { {
        {   // degree 4
            +1.00000000000000000e+00,
            -1.58971650732578684e-01,
            +5.84121356311684790e-03
        },
        {   // degree 6
            +1.00000000000000000e+00,
            -1.66218398161274539e-01,
            +8.06129151017077016e-03,
            -1.50545944866583496e-04
        },
        {   // degree 8
            +1.00000000000000000e+00,
            -1.66651290458553397e-01,
            +8.31836205080888937e-03,
            -1.93853969255209339e-04,
            +2.19921657358978346e-06
        },
        {   // degree 10
            +1.00000000000000000e+00,
            -1.66666320608302304e-01,
            +8.33284074932796014e-03,
            -1.98184457544372085e-04,
            +2.70931602688878442e-06,
            -2.07033154672609224e-08
        },
        {   // degree 12
            +1.00000000000000000e+00,
            -1.66666661172424652e-01,
            +8.33332258782261241e-03,
            -1.98405693280411671e-04,
            +2.75362742462233601e-06,
            -2.47308402132623094e-08,
            +1.36149931873379692e-10
        },
        {   // degree 14
            +1.00000000000000000e+00,
            -1.66666666641878403e-01,
            +8.33333324542580994e-03,
            -1.98412602287003715e-04,
            +2.75568576745228666e-06,
            -2.50407933908690801e-08,
            +1.59105811932465814e-10,
            -6.64696382424593659e-13
        },
        {   // degree 16
            +1.00000000000000000e+00,
            -1.66666666666648478e-01,
            +8.33333333318112164e-03,
            -1.98412698077537775e-04,
            +2.75573162083557394e-06,
            -2.50519743096581360e-08,
            +1.60558314470477309e-10,
            -7.60488921303402553e-13,
            +2.52255089807125025e-15
        }
    } };

    std::array<double, 7> constexpr C_ROTC0_EST_MAX_ERROR =
    {
        6.9656371186750e-03,    // degree 4
        2.2379506089580e-04,    // degree 6
        4.8670096434722e-06,    // degree 8
        7.5654711606532e-08,    // degree 10
        8.7939167753293e-10,    // degree 12
        1.8030021919913e-12,    // degree 14
        6.8001160258291e-16     // degree 16
    };

    // Constants for rotc1(t) = (1-cos(t))/t^2.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC1_EST_COEFF =
    { {
        {   // degree 4
            +5.00000000000000000e-01,
            -4.06593520914583922e-02,
            +1.06698549928666312e-03
        },
        {   // degree 6
            +5.00000000000000000e-01,
            -4.16202835017619524e-02,
            +1.36087417563353699e-03,
            -1.99122437404000405e-05
        },
        {   // degree 8
            +5.00000000000000000e-01,
            -4.16653520191245796e-02,
            +1.38761160375298095e-03,
            -2.44138380330618480e-05,
            +2.28499434819148172e-07
        },
        {   // degree 10
            +5.00000000000000000e-01,
            -4.16666414534323168e-02,
            +1.38885303988547384e-03,
            -2.47850001122907892e-05,
            +2.72207208415419378e-07,
            -1.77358008604198045e-09
        },
        {   // degree 12
            +5.00000000000000000e-01,
            -4.16666663178411056e-02,
            +1.38888820709637153e-03,
            -2.48011431705276915e-05,
            +2.75439902957067131e-07,
            -2.06736081072201315e-09,
            +9.93003616566537400e-12
        },
        {   // degree 14
            +5.00000000000000000e-01,
            -4.16666666664263635e-02,
            +1.38888888750799658e-03,
            -2.48015851902670717e-05,
            +2.75571871163332658e-07,
            -2.08727380201649381e-09,
            +1.14076763269827225e-11,
            -4.28619236995285237e-14
        },
        {   // degree 16
            +5.00000000000000000e-01,
            -4.16666666666571719e-02,
            +1.38888888885105744e-03,
            -2.48015872513761947e-05,
            +2.75573160474227648e-07,
            -2.08766469798137579e-09,
            +1.14685460418668139e-11,
            -4.75415775440997119e-14,
            +1.40555891469552795e-16
        }
    } };

    std::array<double, 7> constexpr C_ROTC1_EST_MAX_ERROR =
    {
        9.2119010150538e-04,    // degree 4
        2.3251261806301e-05,    // degree 6
        4.1693160884870e-07,    // degree 8
        5.5177887814395e-09,    // degree 10
        5.5865700954172e-11,    // degree 12
        7.1609385088323e-15,    // degree 14
        7.2164496600635e-16     // degree 16
    };

    // Constants for rotc2(t) = (sin(t) - t*cos(t))/t^3.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC2_EST_COEFF =
    { {
        {   // degree 4
            +3.33333333333333315e-01,
            -3.24417271573718483e-02,
            +9.05201583387763454e-04
        },
        {   // degree 6
            +3.33333333333333315e-01,
            -3.32912781805089902e-02,
            +1.16506615743456146e-03,
            -1.76083105011587047e-05
        },
        {   // degree 8
            +3.33333333333333315e-01,
            -3.33321218985461534e-02,
            +1.18929901553194335e-03,
            -2.16884239911580259e-05,
            +2.07111898922214621e-07
        },
        {   // degree 10
            +3.33333333333333315e-01,
            -3.33333098285276269e-02,
            +1.19044276839769606e-03,
            -2.20303898189124444e-05,
            +2.47382309403030923e-07,
            -1.63412179616686230e-09
        },
        {   // degree 12
            +3.33333333333333315e-01,
            -3.33333330053041110e-02,
            +1.19047554930712374e-03,
            -2.20454376929804210e-05,
            +2.50395723867477426e-07,
            -1.90797722371463640e-09,
            +9.25661071605387496e-12
        },
        {   // degree 14
            +3.33333333333333315e-01,
            -3.33333333331133561e-02,
            +1.19047618918715682e-03,
            -2.20458533943125258e-05,
            +2.50519837811549507e-07,
            -1.92670551155064303e-09,
            +1.06463697865186991e-11,
            -4.03135292145519115e-14
        },
        {   // degree 16
            +3.33333333333333315e-01,
            -3.33333333333034956e-02,
            +1.19047619036920628e-03,
            -2.20458552540489507e-05,
            +2.50521015434838418e-07,
            -1.92706504721931338e-09,
            +1.07026043656398707e-11,
            -4.46498739610373537e-14,
            +1.30526089083317312e-16
        }
    } };

    std::array<double, 7> constexpr C_ROTC2_EST_MAX_ERROR =
    {
        8.1461508460229e-04,    // degree 4
        2.1075025784856e-05,    // degree 6
        3.8414838612888e-07,    // degree 8
        5.1435966597069e-09,    // degree 10
        5.2533449812486e-11,    // degree 12
        7.7715611723761e-15,    // degree 14
        2.2759572004816e-15     // degree 16
    };

    // Constants for rotc3(t) = (2*(1-cos(t)) - t*sin(t))/t^4.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC3_EST_COEFF =
    { {
        {   // degree 4
            +8.33333333333333287e-02,
            -5.46357009138465424e-03,
            +1.19638433962248889e-04
        },
        {   // degree 6
            +8.33333333333333287e-02,
            -5.55196372993948303e-03,
            +1.46646667516630680e-04,
            -1.82905866698780768e-06
        },
        {   // degree 8
            +8.33333333333333287e-02,
            -5.55546733314307706e-03,
            +1.48723933698110248e-04,
            -2.17865651989456709e-06,
            +1.77408035681006169e-08
        },
        {   // degree 10
            +8.33333333333333287e-02,
            -5.55555406357867952e-03,
            +1.48807404154064368e-04,
            -2.20360578135342372e-06,
            +2.06782449868995703e-08,
            -1.19178563894098852e-10
        },
        {   // degree 12
            +8.33333333333333287e-02,
            -5.55555555324832757e-03,
            +1.48809514798423797e-04,
            -2.20457622072950518e-06,
            +2.08728631685852690e-08,
            -1.36888190776165574e-10,
            +5.99292681875750821e-13
        },
        {   // degree 14
            +8.33333333333333287e-02,
            -5.55555555528319030e-03,
            +1.48809523101214977e-04,
            -2.20458493798151629e-06,
            +2.08765224186559757e-08,
            -1.37600800115177215e-10,
            +6.63762129016229865e-13,
            -2.19044013684859942e-15
        },
        {   // degree 16
            +8.33333333333333287e-02,
            -5.55555555501025672e-03,
            +1.48809521898935978e-04,
            -2.20458342827337994e-06,
            +2.08757075326674457e-08,
            -1.37379825035843510e-10,
            +6.32209097599974706e-13,
            +7.39204014316007136e-17,
            -6.43236558920699052e-17
        }
    } };

    std::array<double, 7> constexpr C_ROTC3_EST_MAX_ERROR =
    {
        8.4612036888886e-05,    // degree 4
        1.8051973185995e-06,    // degree 6
        2.8016103950645e-08,    // degree 8
        3.2675391559156e-10,    // degree 10
        1.3714029911682e-13,    // degree 12
        3.2078506517763e-14,    // degree 14
        4.7774284528401e-14     // degree 16
    };

    // Constants for rotc4(t) = (t - sin(t))/t^3.
    std::array<std::array<double, 9>, 7> constexpr C_ROTC4_EST_COEFF =
    { {
        {   // degree 4
            +1.666661813592689343e-01,
            -8.328498974646202896e-03,
            +1.911744425053129385e-04
        },
        {   // degree 6
            +1.666666647041342428e-01,
            -8.333298484927197715e-03,
            +1.983151947669098215e-04,
            -2.667880597988307407e-06
        },
        {   // degree 8
            +1.666666666610747703e-01,
            -8.333333177830641014e-03,
            +1.984120026488328388e-04,
            -2.754638167851095519e-06,
            +2.434739842192313304e-08
        },
        {   // degree 10
            +1.666666666666548375e-01,
            -8.333333332858779118e-03,
            +1.984126953155111963e-04,
            -2.755724509529648579e-06,
            +2.504406966203295571e-08,
            -1.565598362112428459e-10
        },
        {   // degree 12
            +1.666666666666666474e-01,
            -8.333333333332277341e-03,
            +1.984126984032422657e-04,
            -2.755731890577082639e-06,
            +2.505205730565559701e-08,
            -1.605482763895217150e-10,
            +7.474149667203868085e-13
        },
        {   // degree 14
            +1.666666666666666667e-01,
            -8.333333333333331542e-03,
            +1.984126984126773376e-04,
            -2.755731922304019196e-06,
            +2.505210817677023291e-08,
            -1.605901880350512037e-10,
            +7.645497840414198876e-13,
            -2.753603733509153125e-14
        },
        {   // degree 16
            +1.666666666666666667e-01,
            -8.333333333333333331e-03,
            +1.984126984126983769e-04,
            -2.755731922398382640e-06,
            +2.505210838484547384e-08
            -1.605904373998917420e-10,
            +7.647154485773692262e-13,
            -2.810942149025141692e-15,
            +8.065584188855569398e-18
        }
    } };

    std::array<double, 7> constexpr C_ROTC4_EST_MAX_ERROR =
    {
        2.9118443863947e-06,    // degree 4
        1.1775194543557e-08,    // degree 6
        3.3551378409470e-11,    // degree 8
        7.0975185732742e-14,    // degree 10
        1.1586733901401e-16,    // degree 12
        1.5038700542518e-19,    // degree 14
        1.5889884416916e-22     // degree 16
    };
}

namespace gte
{
    // Estimate rotc0(t) = sin(t)/t for t in [0,pi]. For example, a degree-6
    // estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC0Estimate<float, 6>(t);
    template <typename T, size_t Degree>
    inline T RotC0Estimate(T t)
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC0_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        T tsqr = t * t;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc1(t) = (1 - cos(t))/t^2 for t in [0,pi]. For example,
    // a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC1Estimate<float, 6>(t);
    template <typename T, size_t Degree>
    inline T RotC1Estimate(T t)
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC1_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        T tsqr = t * t;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc2(t) = (sin(t) - t*cos(t))/t^3 for t in [0,pi]. For
    // example, a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC2Estimate<float, 6>(t);
    template <typename T, size_t Degree>
    inline T RotC2Estimate(T t)
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC2_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        T tsqr = t * t;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc3(t) = (2*(1-cos(t)) - t*sin(t))/t^4 for t in
    // [0,pi]. For example, a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC3Estimate<float, 6>(t);
    template <typename T, size_t Degree>
    inline T RotC3Estimate(T t)
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC3_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        T tsqr = t * t;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    // Estimate rotc4(t) = (t - sin(t))/t^3 for t in
    // [0,pi]. For example, a degree-6 estimate is
    //   float t;  // in [0,pi]
    //   float result = RotC4Estimate<float, 6>(t);
    template <typename T, size_t Degree>
    inline T RotC4Estimate(T t)
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        size_t constexpr select = (Degree - 4) / 2;
        auto constexpr& coeff = C_ROTC4_EST_COEFF[select];
        size_t constexpr last = Degree / 2;
        T tsqr = t * t;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * tsqr;
        }
        return poly;
    }

    template <typename T, size_t Degree>
    T constexpr GetRotC0EstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        return static_cast<T>(C_ROTC0_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename T, size_t Degree>
    T constexpr GetRotC1EstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        return static_cast<T>(C_ROTC1_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename T, size_t Degree>
    T constexpr GetRotC2EstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        return static_cast<T>(C_ROTC2_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename T, size_t Degree>
    T constexpr GetRotC3EstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        return static_cast<T>(C_ROTC3_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    template <typename T, size_t Degree>
    T constexpr GetRotC4EstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 4 <= Degree && Degree <= 16,
            "Invalid degree.");

        return static_cast<T>(C_ROTC4_EST_MAX_ERROR[(Degree - 4) / 2]);
    }

    // Construct the estimate for the rotation matrix
    //   R = exp(S) = I + rotc0(t) * S + rotc1(t) * S^2
    // from a vector (p0,p1,p2) with length t = |(p0,p1,p2)| and
    // skew-symmetric matrix S = {{0,-p2,p1},{p2,0,-p0},{-p1,p0,0}}.
    template <typename T, size_t Degree>
    void RotationEstimate(Vector3<T> const& p,
        Matrix3x3<T>& R)
    {
        T const zero = static_cast<T>(0);
        T const one = static_cast<T>(1);

        Matrix3x3<T> I
        {
            one, zero, zero,
            zero, one, zero,
            zero, zero, one
        };

        Matrix3x3<T> S
        {
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        T p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        T p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix3x3<T> Ssqr
        {
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        T t = Length(p);
        T a = RotC0Estimate<T, Degree>(t);
        T b = RotC1Estimate<T, Degree>(t);
        R = I + a * S + b * Ssqr;
    };

    template <typename T, size_t Degree>
    void RotationDerivativeEstimate(Vector3<T> const& p,
        std::array<Matrix3x3<T>, 3>& Rder)
    {
        T const zero = static_cast<T>(0);
        T const one = static_cast<T>(1);

        std::array<Matrix3x3<T>, 3> skewE
        {
            Matrix3x3<T>
            {
                zero, zero, zero,
                zero, zero, -one,
                zero, one, zero
            },

            Matrix3x3<T>
            {
                zero, zero, one,
                zero, zero, zero,
                -one, zero, zero
            },

            Matrix3x3<T>
            {
                zero, -one, zero,
                one, zero, zero,
                zero, zero, zero
            }
        };

        Matrix3x3<T> S
        {
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        T p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        T p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix3x3<T> Ssqr
        {
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        T t = Length(p);
        T a = RotC0Estimate<T, Degree>(t);
        T b = RotC1Estimate<T, Degree>(t);
        T c = RotC2Estimate<T, Degree>(t);
        T d = RotC3Estimate<T, Degree>(t);
        for (int32_t i = 0; i < 3; ++i)
        {
            Rder[i] = a * skewE[i] + b * (S * skewE[i] + skewE[i] * S) - p[i] * (c * S + d * Ssqr);
        }
    }

    template <typename T, size_t Degree>
    void RotationAndDerivativeEstimate(Vector3<T> const& p,
        Matrix3x3<T>& R, std::array<Matrix3x3<T>, 3>& Rder)
    {
        T const zero = static_cast<T>(0);
        T const one = static_cast<T>(1);

        Matrix3x3<T> I
        {
            one, zero, zero,
            zero, one, zero,
            zero, zero, one
        };

        std::array<Matrix3x3<T>, 3> skewE
        {
            Matrix3x3<T>
            {
                zero, zero, zero,
                zero, zero, -one,
                zero, one, zero
            },

            Matrix3x3<T>
            {
                zero, zero, one,
                zero, zero, zero,
                -one, zero, zero
            },

            Matrix3x3<T>
            {
                zero, -one, zero,
                one, zero, zero,
                zero, zero, zero
            }
        };

        Matrix3x3<T> S
        {
            zero, -p[2], p[1],
            p[2], zero, -p[0],
            -p[1], p[0], zero
        };

        T p0p0 = p[0] * p[0], p0p1 = p[0] * p[1], p0p2 = p[0] * p[2];
        T p1p1 = p[1] * p[1], p1p2 = p[1] * p[2], p2p2 = p[2] * p[2];
        Matrix3x3<T> Ssqr
        {
            -(p1p1 + p2p2), p0p1, p0p2,
            p0p1, -(p0p0 + p2p2), p1p2,
            p0p2, p1p2, -(p0p0 + p1p1)
        };

        T t = Length(p);
        T a = RotC0Estimate<T, Degree>(t);
        T b = RotC1Estimate<T, Degree>(t);
        T c = RotC2Estimate<T, Degree>(t);
        T d = RotC3Estimate<T, Degree>(t);

        R = I + a * S + b * Ssqr;
        for (int32_t i = 0; i < 3; ++i)
        {
            Rder[i] = a * skewE[i] + b * (S * skewE[i] + skewE[i] * S) - p[i] * (c * S + d * Ssqr);
        }
    }
}
