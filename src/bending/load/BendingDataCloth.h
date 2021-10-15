#pragma once
#include "BendingData.h"
class tBendingDataCloth
{
public:
    tBendingDataCloth();
    virtual bool Init(std::string data_dir);
    virtual int GetId() const { return mId; }
    virtual std::string GetDir() const { return mDataDir; }
    virtual tVectorXf GetFrontAngleList();
    virtual tVectorXf GetBackAngleList();

protected:
    int mId;
    std::string mDataDir;
    tBendingDataList mFrontData, mBackData;
};

SIM_DECLARE_PTR(tBendingDataCloth);
int LoadClothData(std::string data_dir,
                  std::vector<std::string> &img_list,
                  std::vector<std::string> &mat_list);