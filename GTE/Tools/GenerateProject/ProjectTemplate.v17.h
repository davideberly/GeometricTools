// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.01.12

#pragma once

#include "ProjectTemplate.h"
#include <string>

class TemplateV17 : public Template
{
public:
    TemplateV17(std::string const& gteRelativePath);
    virtual ~TemplateV17() = default;

private:
    virtual std::string GetMSVSVersion() const { return "v17"; }

    virtual std::string GetGTMathematicsGUID() const override
    {
        return msGTMathematicsGUID;
    }

    virtual std::string GetGTGraphicsGUID() const override
    {
        return msGTGraphicsGUID;
    }

    virtual std::string GetGTGraphicsDX11GUID() const override
    {
        return msGTGraphicsDX11GUID;
    }

    virtual std::string GetGTGraphicsGL46GUID() const override
    {
        return msGTGraphicsGL46GUID;
    }

    virtual std::string GetGTApplicationsDX11GUID() const override
    {
        return msGTApplicationsDX11GUID;
    }

    virtual std::string GetGTApplicationsGL46GUID() const override
    {
        return msGTApplicationsGL46GUID;
    }

    virtual std::string GetSolutionLines() const override
    {
        return msSolutionLines;
    }

    virtual std::string GetProjectLines() const override
    {
        return msProjectLines;
    }

    virtual std::string GetFilterLines() const override
    {
        return msFilterLines;
    }

    static std::string const msGTMathematicsGUID;
    static std::string const msGTGraphicsGUID;
    static std::string const msGTGraphicsDX11GUID;
    static std::string const msGTApplicationsDX11GUID;
    static std::string const msGTGraphicsGL46GUID;
    static std::string const msGTApplicationsGL46GUID;
    static std::string const msSolutionLines;
    static std::string const msProjectLines;
    static std::string const msFilterLines;
};
