#pragma once

#include <Applications/Window2.h>
using namespace gte;

class BooleanPolygonWindow2 : public Window2
{
public:
    BooleanPolygonWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
};
