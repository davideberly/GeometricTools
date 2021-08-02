#pragma once

#include <Applications/Console.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/GaussianElimination.h>
#include <array>
using namespace gte;

using Rational = BSRational<UIntegerAP32>;

class FiniteDifferencesConsole : public Console
{
public:
    FiniteDifferencesConsole(Parameters& parameters);

    virtual void Execute() override;

private:
    // TODO: Determine the multiplier that is used in the
    // GenerateMiPj() functions by using L'Hopital's rule.
    // This needs to be reported as D in the C[i] = N[i]/D,
    // and report N[i] also. The equation used for L'Hopital's
    // rule is the one corresponding to k = m (the right-hand
    // side has a 1 in that location).
    //
    // IS THIS POSSIBLE? L'Hopital's rule was used once I
    // manually determined the common denominator.
    template <int m, int p>
    bool Generate(int imin, int imax,
        std::array<Rational, m + p>& C, int& errorOrder)
    {
        constexpr size_t size = m + p;
        std::array<std::array<Rational, size>, size> M;  // M[row][col]

        int i = imin;
        for (size_t col = 0; col < size; ++col, ++i)
        {
            M[0][col] = 1;
            M[1][col] = i;
        }

        for (size_t row = 2; row < size; ++row)
        {
            for (size_t col = 0; col < size; ++col)
            {
                M[row][col] = M[row - 1][col] * M[1][col];
            }
        }

        std::array<Rational, size> B;  // B[row]
        B.fill(Rational(0));
        B[m] = Rational(1);

        GaussianElimination<Rational> solver;
        Rational determinant;
        bool success = solver(size, M[0].data(), nullptr, determinant,
            B.data(), C.data(), nullptr, size, nullptr);

        if (success)
        {
            // Compute m!.
            Rational factorial(1);
            for (int j = 2; j <= m; ++j)
            {
                factorial *= Rational(j);
            }

            // In solving for the derivative approximation, we need to
            // multiply the linear system coefficients by m!.
            for (size_t col = 0; col < size; ++col)
            {
                C[col] *= factorial;
            }

            // Test whether we have a centered difference for which the
            // power is effectively p+1.
            errorOrder = p;
            if (imax == -imin)
            {
                Rational sum(0);
                for (size_t col = 0; col < size; ++col)
                {
                    Rational power = M[size - 1][col] * M[1][col];
                    sum += power * C[col];
                }
                if (sum == Rational(0))
                {
                    ++errorOrder;
                }
            }
        }

        return success;
    }

    void GenerateM1P1();
    void GenerateM1P2();
    void GenerateM1P3();
    void GenerateM1P4();

    void GenerateM2P1();
    void GenerateM2P2();
    void GenerateM2P3();
    void GenerateM2P4();

    void GenerateM3P1();
    void GenerateM3P2();
    void GenerateM3P3();
    void GenerateM3P4();

    void GenerateM4P1();
    void GenerateM4P2();
    void GenerateM4P3();
    void GenerateM4P4();
};
