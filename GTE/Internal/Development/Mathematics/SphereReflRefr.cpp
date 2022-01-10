#include <GTEngine.h>
using namespace gte;

// Quantities in SphereReflRefr.png
//
// ray : E + t*D, E is the ray origin (eyepoint) and D is the unit-length direction
// sphere : center C, radius r
// indexOfRefractionMedium : n1
// indexOfRefractionSphere : n2
// intersectionPoint : P, |P-C|^2 = r^2
// intersectionNormal : N, unit-length normal to the sphere at P
// reflection : U, unit-length direction
// refraction : V, unit-length direction
// angleReflection : theta1
// angleRefraction : theta2
//
// The function returns 'true' if and only if the following conditions hold:
//
// 1. E is outside the sphere.  Your comment was that the ray comes from
//   'outside' the system.
//
// 2. The ray intersects the sphere and is not tangent to the sphere.
//
// When the function returns 'true', the outputs are all valid quantities;
// otherwise, they should not be read.
bool Compute(
    // inputs
    Ray3<double> const& ray, Sphere3<double> const& sphere,
    double indexOfRefractionMedium, double indexOfRefractionSphere,
    // outputs
    Vector3<double>& intersectionPoint, Vector3<double>& intersectionNormal,
    Vector3<double>& reflection, Vector3<double>& refraction,
    double& angleReflection, double& angleRefraction)
{
    Vector3<double> EmC = ray.origin - sphere.center;
    double length = Length(EmC);
    if (length <= sphere.radius)
    {
        // The ray origin is inside the sphere, which we do not process.
        return false;
    }

    // The ray origin is outside the sphere.  Now determine whether the ray
    // intersects the sphere.
    typedef FIQuery<double, Ray3<double>, Sphere3<double>> RaySphereQuery;
    RaySphereQuery query;
    RaySphereQuery::Result result = query(ray, sphere);
    if (!result.intersect)
    {
        // The ray does not intersect the sphere.
        return false;
    }

    if (result.numIntersections != 2)
    {
        // result.numIntersections is 1 and the ray is tangent to the sphere,
        // which we do not process.
        return false;
    }

    // We know the ray intersects the sphere transversely.
    intersectionPoint = result.point[0];  // Intersection closest to ray origin.
    intersectionNormal = intersectionPoint - sphere.center;
    Normalize(intersectionNormal);  // unit-length outer-pointing normal to sphere
    double NdD = Dot(intersectionNormal, ray.direction);

    if (NdD > -1.0)
    {
        reflection = ray.direction - (2.0 * NdD) * intersectionNormal;
        angleReflection = acos(-NdD);

        // V = a*N + b*D is the unit-length reflection vector.  Define
        // c = cos(theta2) and d = Dot(N,D) = -cos(theta1); then
        //   1 = Dot(V,V) = a^2 + 2*a*b*d + b^2
        //   -c = Dot(N,V) = a + b*d
        // which has solution
        //   b = sqrt((1-c^2)/(1-d^2)) = n1/n2
        //   a = -(c + d*b)
        double b = indexOfRefractionMedium / indexOfRefractionSphere;

        // Compute the angle of refraction only when sinTheta2 is in the
        // range of the sine function.
        double sinTheta1 = sin(angleReflection);
        double sinTheta2 = b * sinTheta1;
        double cosTheta2Sqr = 1.0 - sinTheta2 * sinTheta2;
        if (0.0 <= cosTheta2Sqr && cosTheta2Sqr <= 1.0)
        {
            double cosTheta2 = sqrt(cosTheta2Sqr);
            angleRefraction = acos(cosTheta2);
            double a = -(cosTheta2 + NdD * b);
            refraction = a * intersectionNormal + b * ray.direction;
        }
    }
    else  // Dot(N,D) = -1
    {
        // N = -D (the sphere normal and ray direction are parallel)
        reflection = intersectionNormal;
        refraction = -intersectionNormal;
        angleReflection = 0.0;
        angleRefraction = 0.0;
    }

    return true;
}

int main()
{
    Ray3<double> ray;
    Sphere3<double> sphere;
    double indexOfRefractionMedium = 1.0;
    double indexOfRefractionSphere = 1.125;

    ray.origin = { 1.0, 1.5, 2.0 };
    ray.direction = { -1.0, -1.0, -1.0 };
    Normalize(ray.direction);
    sphere.center = { 0.0, 0.0, 0.0 };
    sphere.radius = 1.0;

    bool result;
    Vector3<double> intersectionPoint, intersectionNormal, reflection, refraction;
    double angleReflection, angleRefraction;
    result = Compute(ray, sphere, indexOfRefractionMedium, indexOfRefractionSphere,
        intersectionPoint, intersectionNormal, reflection, refraction,
        angleReflection, angleRefraction);
    return 0;
}
