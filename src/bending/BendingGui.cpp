#include "BendingGui.h"
#include "bending/load/BendingDataLoader.h"
cBendingGui::cBendingGui()
{
}
void cBendingGui::UpdateGui()
{
}
std::vector<cRenderResourcePtr> cBendingGui::GetRenderingResource()
{
    return {};
}

void cBendingGui::Init(std::string root_path)
{
    this->mRawBendingDataRootDir = root_path;
    this->mBendingData = BuildBendingClothArray(mRawBendingDataRootDir);
}