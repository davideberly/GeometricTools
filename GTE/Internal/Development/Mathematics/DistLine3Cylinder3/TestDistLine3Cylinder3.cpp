#include "UnitTestDistLine3Cylinder3.h"
#include <Mathematics/GteDistLine3Cylinder3.h>
#include <Mathematics/GteDistPoint3Cylinder3.h>
#include <Mathematics/GteRotation.h>
#include <Mathematics/GteMatrix3x3.h>
using namespace gte;

//----------------------------------------------------------------------------
UnitTestDistLine3Cylinder3::~UnitTestDistLine3Cylinder3()
{
}
//----------------------------------------------------------------------------
UnitTestDistLine3Cylinder3::UnitTestDistLine3Cylinder3()
    :
    mStrLogger("UnitTestDistLine3Cylinder3",
        Logger::Listener::LISTEN_FOR_ALL),
    mMbxLogger(Logger::Listener::LISTEN_FOR_ALL)
{
    Subscribe();
    DoTests();
    Unsubscribe();
}
//----------------------------------------------------------------------------
std::vector<std::string> const& UnitTestDistLine3Cylinder3::GetMessages()
    const
{
    return mStrLogger.GetMessages();
}
//----------------------------------------------------------------------------
std::vector<std::string>& UnitTestDistLine3Cylinder3::GetMessages()
{
    return mStrLogger.GetMessages();
}
//----------------------------------------------------------------------------
void UnitTestDistLine3Cylinder3::Subscribe()
{
    Logger::Subscribe(&mStrLogger);
    Logger::Subscribe(&mMbxLogger);
}
//----------------------------------------------------------------------------
void UnitTestDistLine3Cylinder3::Unsubscribe()
{
    Logger::Unsubscribe(&mStrLogger);
    Logger::Unsubscribe(&mMbxLogger);
}
//----------------------------------------------------------------------------
void UnitTestDistLine3Cylinder3::TestInfiniteCylinder()
{
    DCPQuery<double, Line3<double>, Cylinder3<double>> query;
    DCPQuery<double, Line3<double>, Cylinder3<double>>::Result result;

    Cylinder3<double> cylinder;
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    cylinder.radius = 1.0;
    cylinder.height = std::numeric_limits<double>::max();

    Line3<double> line;
    Quaternion<double> q{ 1.0f, 2.0f, 3.0f, 4.0f };
    Normalize(q);
    Matrix3x3<double> rotate = Rotation<3, double>(q);
    Vector3<double> translate = { 0.123, -4.567, 0.089012 };
    Vector3<double> lineClosest, cylinderClosest, point, delta;
    double lineLength, cylinderLength, value;

    // line outside the cylinder
    line.origin = { 1.0, 2.0, 3.0 };
    line.direction = { 1.0, -1.0, 1.0 };
    Normalize(line.direction);
    result = query(line, cylinder);
    lineClosest = Vector3<double>{1.5, 1.5, 3.5};
    cylinderClosest = Vector3<double>{ sqrt(0.5), sqrt(0.5), 3.5 };
    lineLength = Length(result.lineClosest - lineClosest);
    cylinderLength = Length(result.cylinderClosest - cylinderClosest);
    LogAssert(result.distance == 1.1213203435596428 &&
        lineLength == 0.0 &&
        cylinderLength == 0.0 &&
        result.parameter[0] == 0.86602540378443849 &&
        result.parameter[1] == 0.86602540378443849,
        "line-outside-cylinder query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    line.origin = rotate * line.origin + translate;
    line.direction = rotate * line.direction;
    lineClosest = rotate * lineClosest + translate;
    cylinderClosest = rotate * cylinderClosest + translate;
    result = query(line, cylinder);
    lineLength = Length(result.lineClosest - lineClosest);
    cylinderLength = Length(result.cylinderClosest - cylinderClosest);
    LogAssert(result.distance == 1.1213203435596424 &&
        result.type == 0 &&
        lineLength == 1.4895204919483639e-015 &&
        cylinderLength == 1.2560739669470201e-015 &&
        result.parameter[0] == 0.86602540378443948 &&
        result.parameter[1] == 0.86602540378443948,
        "rotated line-outside-cylinder query failed");

    // line intersects the cylinder transversely
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    line.origin = { 0.1, 0.2, 3.0 };
    line.direction = { 1.0, -1.0, 1.0 };
    Normalize(line.direction);
    result = query(line, cylinder);
    lineClosest = { -0.54101374805426261, 0.84101374805426254, 2.3589862519457374 };
    cylinderClosest = lineClosest;
    LogAssert(result.distance == 0.0 &&
        result.type == 1 &&
        result.lineClosest == lineClosest &&
        result.cylinderClosest == cylinderClosest &&
        result.parameter[0] == -1.1102683799801383 &&
        result.parameter[1] == 1.2834734607370262,
        "line-intersects-cylinder-transversely query failed");

    point = line.origin + result.parameter[0] * line.direction;
    delta = point - cylinder.axis.origin;
    delta -= Dot(delta, cylinder.axis.direction) * cylinder.axis.direction;
    value = Length(delta) - cylinder.radius;
    LogAssert(value == -2.2204460492503131e-016,
        "line-intersects-cylinder-transversely query failed");

    point = line.origin + result.parameter[1] * line.direction;
    delta = point - cylinder.axis.origin;
    delta -= Dot(delta, cylinder.axis.direction) * cylinder.axis.direction;
    value = Length(delta) - cylinder.radius;
    LogAssert(value == 0.0,
        "line-intersects-cylinder-transversely query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    line.origin = rotate * line.origin + translate;
    line.direction = rotate * line.direction;
    lineClosest = rotate * lineClosest + translate;
    cylinderClosest = rotate * cylinderClosest + translate;
    result = query(line, cylinder);
    lineLength = Length(result.lineClosest - lineClosest);
    cylinderLength = Length(result.cylinderClosest - cylinderClosest);
    LogAssert(result.distance == 0.0 &&
        result.type == 1 &&
        lineLength == 8.0059320849734419e-016 &&
        cylinderLength == 8.0059320849734419e-016 &&
        result.parameter[0] == -1.1102683799801387 &&
        result.parameter[1] == 1.2834734607370264,
        "rotated line-intersects-cylinder-transversely query failed");

    point = line.origin + result.parameter[0] * line.direction;
    delta = point - cylinder.axis.origin;
    delta -= Dot(delta, cylinder.axis.direction) * cylinder.axis.direction;
    value = Length(delta) - cylinder.radius;
    LogAssert(value == 2.2204460492503131e-016,
        "rotated line-intersects-cylinder-transversely query failed");

    point = line.origin + result.parameter[0] * line.direction;
    delta = point - cylinder.axis.origin;
    delta -= Dot(delta, cylinder.axis.direction) * cylinder.axis.direction;
    value = Length(delta) - cylinder.radius;
    LogAssert(value == 2.2204460492503131e-016,
        "rotated line-intersects-cylinder-transversely query failed");

    // parallel line outside the cylinder
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    line.origin = { 1.0, 2.0, 3.0 };
    line.direction = { 0.0, 0.0, 1.0 };
    result = query(line, cylinder);
    lineClosest = line.origin;
    cylinderClosest = { sqrt(0.2), sqrt(0.8), 3.0 };
    LogAssert(result.distance == 1.2360679774997898 &&
        result.type == 3 &&
        result.lineClosest == lineClosest &&
        result.cylinderClosest == cylinderClosest,
        "parallel line-outside-cylinder query failed");
}
//----------------------------------------------------------------------------
void UnitTestDistLine3Cylinder3::TestFiniteCylinder()
{
#if 0
    DCPQuery<double, Line3<double>, Cylinder3<double>> query;
    DCPQuery<double, Line3<double>, Cylinder3<double>>::Result result;

    Cylinder3<double> cylinder;
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    cylinder.radius = 1.0;
    cylinder.height = 3.0;

    Vector3<double> point;
    Quaternion<double> q{ 1.0f, 2.0f, 3.0f, 4.0f };
    Normalize(q);
    Matrix3x3<double> rotate = Rotation<3, double>(q);
    Vector3<double> translate = { 0.123, -4.567, 0.089012 };

    // There are 6 Voronoi regions to test.
    // 1. inside the cylinder, between planes of disks
    //      closest = point
    // 2. outside the cylinder, between planes of the disks
    //      closest = point projected to cylinder wall
    // 3. inside the cylinder, above plane of top disk
    //      closest = point projected to top disk
    // 4. outside the cylinder, above plane of top disk
    //      closest = point projected to circular boundary of top disk
    // 5. inside the cylinder, below plane of top disk
    //      closest = point projected to bottom disk
    // 6. outside the cylinder, below plane of top disk
    //      closest = point projected to circular boundary of bottom disk

    // region 1
    point = { 0.1, 0.2, 0.3 };
    result = query(point, cylinder);
    LogAssert(result.distance == 0.0 && result.cylinderClosest == point,
        "region 1 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    result = query(point, cylinder);
    double length = Length(result.cylinderClosest - point);
    LogAssert(result.distance == 0.0 && length == 5.5511151231257827e-017,
        "rotated region 1 query failed");

    // region 2
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    point = { 3.0, 4.0, 1.0 };
    result = query(point, cylinder);
    Vector3<double> closest({ 0.6, 0.8, 1.0 });
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 4.0 && length == 1.1102230246251565e-016,
        "region 2 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    closest = rotate * closest + translate;
    result = query(point, cylinder);
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 3.9999999999999991
        && length == 1.2803716525534355e-015,
        "rotated region 2 query failed");

    // region 3
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    point = { 0.1, 0.2, 4.0 };
    result = query(point, cylinder);
    closest = { point[0], point[1], cylinder.height };
    LogAssert(result.distance == 1.0 && result.cylinderClosest == closest,
        "region 3 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    closest = rotate * closest + translate;
    result = query(point, cylinder);
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 1.0 && length == 6.2803698347351007e-016,
        "rotated region 3 query failed");

    // region 4
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    point = { 3.0, 4.0, 4.0 };
    result = query(point, cylinder);
    closest = { 0.6, 0.8, cylinder.height };
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 4.1231056256176606
        && length == 1.1102230246251565e-016,
        "region 4 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    closest = rotate * closest + translate;
    result = query(point, cylinder);
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 4.1231056256176597
        && length == 4.9650683064945462e-016,
        "rotated region 4 query failed");

    // region 5
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    point = { 0.1, 0.2, -4.0 };
    result = query(point, cylinder);
    closest = { point[0], point[1], -cylinder.height };
    LogAssert(result.distance == 1.0 && result.cylinderClosest == closest,
        "region 5 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    closest = rotate * closest + translate;
    result = query(point, cylinder);
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 1.0 && length == 8.0059320849734419e-016,
        "rotated region 5 query failed");

    // region 6
    cylinder.axis.origin = { 0.0, 0.0, 0.0 };
    cylinder.axis.direction = { 0.0, 0.0, 1.0 };
    point = { 3.0, 4.0, -4.0 };
    result = query(point, cylinder);
    closest = { 0.6, 0.8, -cylinder.height };
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 4.1231056256176606
        && length == 1.1102230246251565e-016,
        "region 6 query failed");

    // repeat the query after a rigid transformation
    cylinder.axis.origin = rotate * cylinder.axis.origin + translate;
    cylinder.axis.direction = rotate * cylinder.axis.direction;
    point = rotate * point + translate;
    closest = rotate * closest + translate;
    result = query(point, cylinder);
    length = Length(result.cylinderClosest - closest);
    LogAssert(result.distance == 4.1231056256176597
        && length == 1.1102230246251565e-015,
        "rotated region 6 query failed");
#endif
}
//----------------------------------------------------------------------------
void UnitTestDistLine3Cylinder3::DoTests()
{
    TestInfiniteCylinder();
    TestFiniteCylinder();
}
//----------------------------------------------------------------------------
