#pragma once
#include "utils/DefUtil.h"
#include <memory>
#include <vector>

SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cRenderable : std::enable_shared_from_this<cRenderable>
{
public:
    virtual void UpdateGui() = 0;
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() = 0;
};