// Test for Issues 6.2, 6.3, 6.4: Operator precedence bug in grid max computation
//
// Bug: mXMax = mXMin + mXSpacing * static_cast<Real>(mXBound) - static_cast<Real>(1)
// evaluates as (mXMin + mXSpacing*mXBound) - 1.0 instead of
// mXMin + mXSpacing*(mXBound - 1).
//
// For xMin=0, spacing=0.5, bound=5: buggy gives 1.5, correct gives 2.0.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <Mathematics/IntpBilinear2.h>
#include <Mathematics/IntpBicubic2.h>
#include <Mathematics/IntpTrilinear3.h>
#include <Mathematics/IntpTricubic3.h>

int main()
{
    std::cout << "=== Issues 6.2/6.3/6.4: Interpolation grid max precedence ===\n\n";
    bool ok = true;

    // Test 1: IntpBilinear2 (issue 6.2)
    {
        int32_t xBound = 5, yBound = 4;
        double xMin = 1.0, yMin = 2.0;
        double xSpacing = 0.5, ySpacing = 0.25;
        // Expected: xMax = 1.0 + 0.5*(5-1) = 3.0
        //           yMax = 2.0 + 0.25*(4-1) = 2.75
        double expectedXMax = xMin + xSpacing * (xBound - 1);  // 3.0
        double expectedYMax = yMin + ySpacing * (yBound - 1);  // 2.75

        std::vector<double> F(xBound * yBound, 1.0);
        gte::IntpBilinear2<double> interp(xBound, yBound, xMin, xSpacing, yMin, ySpacing, F.data());

        double gotXMax = interp.GetXMax();
        double gotYMax = interp.GetYMax();

        std::cout << "  IntpBilinear2:\n";
        std::cout << "    XMax: got=" << gotXMax << "  expected=" << expectedXMax << "\n";
        std::cout << "    YMax: got=" << gotYMax << "  expected=" << expectedYMax << "\n";

        if (std::abs(gotXMax - expectedXMax) > 1e-10)
        {
            std::cout << "    FAIL: XMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotYMax - expectedYMax) > 1e-10)
        {
            std::cout << "    FAIL: YMax is wrong\n";
            ok = false;
        }
    }

    // Test 2: IntpBicubic2 (same bug as 6.2)
    {
        int32_t xBound = 5, yBound = 5;
        double xMin = 1.0, yMin = 2.0;
        double xSpacing = 0.5, ySpacing = 0.25;
        double expectedXMax = xMin + xSpacing * (xBound - 1);  // 3.0
        double expectedYMax = yMin + ySpacing * (yBound - 1);  // 3.0

        std::vector<double> F(xBound * yBound, 1.0);
        gte::IntpBicubic2<double> interp(xBound, yBound, xMin, xSpacing, yMin, ySpacing, F.data(), true);

        double gotXMax = interp.GetXMax();
        double gotYMax = interp.GetYMax();

        std::cout << "  IntpBicubic2:\n";
        std::cout << "    XMax: got=" << gotXMax << "  expected=" << expectedXMax << "\n";
        std::cout << "    YMax: got=" << gotYMax << "  expected=" << expectedYMax << "\n";

        if (std::abs(gotXMax - expectedXMax) > 1e-10)
        {
            std::cout << "    FAIL: XMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotYMax - expectedYMax) > 1e-10)
        {
            std::cout << "    FAIL: YMax is wrong\n";
            ok = false;
        }
    }

    // Test 3: IntpTrilinear3 (issue 6.4)
    {
        int32_t xBound = 5, yBound = 4, zBound = 3;
        double xMin = 1.0, yMin = 2.0, zMin = 3.0;
        double xSpacing = 0.5, ySpacing = 0.25, zSpacing = 0.1;
        double expectedXMax = xMin + xSpacing * (xBound - 1);  // 3.0
        double expectedYMax = yMin + ySpacing * (yBound - 1);  // 2.75
        double expectedZMax = zMin + zSpacing * (zBound - 1);  // 3.2

        std::vector<double> F(xBound * yBound * zBound, 1.0);
        gte::IntpTrilinear3<double> interp(xBound, yBound, zBound,
            xMin, xSpacing, yMin, ySpacing, zMin, zSpacing, F.data());

        double gotXMax = interp.GetXMax();
        double gotYMax = interp.GetYMax();
        double gotZMax = interp.GetZMax();

        std::cout << "  IntpTrilinear3:\n";
        std::cout << "    XMax: got=" << gotXMax << "  expected=" << expectedXMax << "\n";
        std::cout << "    YMax: got=" << gotYMax << "  expected=" << expectedYMax << "\n";
        std::cout << "    ZMax: got=" << gotZMax << "  expected=" << expectedZMax << "\n";

        if (std::abs(gotXMax - expectedXMax) > 1e-10)
        {
            std::cout << "    FAIL: XMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotYMax - expectedYMax) > 1e-10)
        {
            std::cout << "    FAIL: YMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotZMax - expectedZMax) > 1e-10)
        {
            std::cout << "    FAIL: ZMax is wrong\n";
            ok = false;
        }
    }

    // Test 3: IntpTricubic3 (issue 6.3)
    {
        int32_t xBound = 5, yBound = 5, zBound = 5;
        double xMin = 1.0, yMin = 2.0, zMin = 3.0;
        double xSpacing = 0.5, ySpacing = 0.25, zSpacing = 0.1;
        double expectedXMax = xMin + xSpacing * (xBound - 1);  // 3.0
        double expectedYMax = yMin + ySpacing * (yBound - 1);  // 3.0
        double expectedZMax = zMin + zSpacing * (zBound - 1);  // 3.4

        std::vector<double> F(xBound * yBound * zBound, 1.0);
        gte::IntpTricubic3<double> interp(xBound, yBound, zBound,
            xMin, xSpacing, yMin, ySpacing, zMin, zSpacing, F.data(), true);

        double gotXMax = interp.GetXMax();
        double gotYMax = interp.GetYMax();
        double gotZMax = interp.GetZMax();

        std::cout << "  IntpTricubic3:\n";
        std::cout << "    XMax: got=" << gotXMax << "  expected=" << expectedXMax << "\n";
        std::cout << "    YMax: got=" << gotYMax << "  expected=" << expectedYMax << "\n";
        std::cout << "    ZMax: got=" << gotZMax << "  expected=" << expectedZMax << "\n";

        if (std::abs(gotXMax - expectedXMax) > 1e-10)
        {
            std::cout << "    FAIL: XMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotYMax - expectedYMax) > 1e-10)
        {
            std::cout << "    FAIL: YMax is wrong\n";
            ok = false;
        }
        if (std::abs(gotZMax - expectedZMax) > 1e-10)
        {
            std::cout << "    FAIL: ZMax is wrong\n";
            ok = false;
        }
    }

    std::cout << "\n=== Summary ===\n";
    std::cout << "Grid max test: " << (ok ? "passed" : "BUG DETECTED") << "\n";
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
