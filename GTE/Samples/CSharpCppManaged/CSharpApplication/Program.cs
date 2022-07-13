using System;
using CLI;

// Before building, change the configuration to Release and the architecture
// type to x64. If you choose to run x86, you must launch the properties
// dialog for the CSharpApplication project. Select the Build item and change
// the platform architecture from x64 to x86. I do not know enough about
// mixed-language project management in Visual Studio to figure out whether
// this can happen automatically by selecting the config/architecture UI
// control from the Visual Studio toolbar.
//
// Before running, set the start-up project to CSharpApplication; right-click
// that project and choose "Set as Startup Project".

namespace CSharpApplication
{
    class Program
    {
        static void Main(string[] args)
        {
            int numPoints = 2048;
            double[] points = new double[3 * numPoints];

            double[] center = new double[] { 0.0, 0.0, 0.0 };
            double[] extent = new double[] { 1.0, 0.25, 0.125 };
            double sqrtHalf = Math.Sqrt(0.5);
            double[] axis = new double[] {
                sqrtHalf, sqrtHalf, 0.0,
                -sqrtHalf, sqrtHalf, 0.0,
                0.0, 0.0, 1.0 };
            double[] volume = new double[] { 0.0 };

            Random generator = new Random();

            for (int i = 0; i < numPoints; ++i)
            {
                double rnd = 2.0 * generator.NextDouble() - 1.0;
                double theta = rnd * 2.0 * Math.PI;
                rnd = 2.0 * generator.NextDouble() - 1.0;
                double phi = rnd * Math.PI;
                double radius = generator.NextDouble();
                double x = extent[0] * Math.Cos(theta) * Math.Sin(phi);
                double y = extent[1] * Math.Sin(theta) * Math.Sin(phi);
                double z = extent[2] * Math.Cos(phi);
                for (int j = 0; j < 3; ++j)
                {
                    points[3 * i + j] = center[j] +
                        radius * (x * axis[j] + y * axis[j + 3] + z * axis[j + 6]);
                }
            }

            MVB3 mvb = new MVB3();
            uint numThreads = 4;
            uint lgMaxSample = 10;
            mvb.ComputeMinimumVolumeBoxFromPoints(numThreads, numPoints, points,
                lgMaxSample, center, axis, extent, volume);

            Console.WriteLine("center  = " + center[0] + ", " + center[1] + ", " + center[2]);
            Console.WriteLine("extent  = " + extent[0] + ", " + extent[1] + ", " + extent[2]);
            Console.WriteLine("axis[0] = " + axis[0] + ", " + axis[1] + ", " + axis[2]);
            Console.WriteLine("axis[1] = " + axis[3] + ", " + axis[4] + ", " + axis[5]);
            Console.WriteLine("axis[2] = " + axis[6] + ", " + axis[7] + ", " + axis[8]);
            Console.WriteLine("volume  = " + volume[0]);
            Console.WriteLine("Press enter to exit program.");
            Console.Read();
        }
    }
}
