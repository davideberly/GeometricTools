#pragma once

#include <Applications/Window3.h>
using namespace gte;

class IntersectCylindersWindow3 : public Window3
{
public:
    IntersectCylindersWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
};
