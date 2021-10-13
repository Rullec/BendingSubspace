#pragma once
#include "render/RenderableBase.h"
#include "bending/load/BendingDataLoader.h"
#include <string>
class cBendingGui : cRenderable
{
public:
    explicit cBendingGui();
    virtual void Init(std::string root_path);
    virtual void UpdateGui();
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() override;

protected:
    std::string mRawBendingDataRootDir;
    tBendingClothArray mBendingData;
};

SIM_DECLARE_PTR(cBendingGui);
