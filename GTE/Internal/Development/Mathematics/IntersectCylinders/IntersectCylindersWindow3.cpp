#include "IntersectCylindersWindow3.h"
#include "IntrCylinder3Cylinder3.h"

namespace gte
{
    template class FIQuery<float, Cylinder3<float>, Cylinder3<float>>;
}

IntersectCylindersWindow3::IntersectCylindersWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
}

void IntersectCylindersWindow3::OnIdle()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}
