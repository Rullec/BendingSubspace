#pragma once
#include "BendingData.h"
#include "BendingDataFace.h"
#include <map>


class tBendingDataCloth
{
public:
    tBendingDataCloth();
    virtual bool Init(std::string data_dir, const std::map<int, double> &density_map);
    virtual int GetId() const { return mId; }
    virtual std::string GetDir() const { return mDataDir; }
    virtual tVectorXf GetFrontAngleList();
    virtual tVectorXf GetBackAngleList();
    virtual tBendingDataPtr GetFrontDataByIdx(int idx);
    virtual tBendingDataList GetFrontDataList() const;
    virtual tBendingDataList GetBackDataList() const;

    virtual tBendingDataPtr GetBackDataByIdx(int idx);
    virtual tBendingDataPtr GetFrontDataByAngle(float angle);
    virtual tBendingDataPtr GetBackDataByAngle(float angle);
    virtual double GetRhoG() const;

protected:
    int mId;
    double mRhoG;
    std::string mDataDir;
    tBendingDataList mFrontData, mBackData;
    tBendingStiffnessClothPtr mFrontBendingStiffness, mBackBendingStiffness;
    virtual void InitBendingStiffness();
};

SIM_DECLARE_PTR(tBendingDataCloth);
int LoadClothData(std::string data_dir,
                  std::vector<std::string> &img_list,
                  std::vector<std::string> &mat_list);