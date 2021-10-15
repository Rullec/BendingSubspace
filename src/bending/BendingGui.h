#pragma once
#include "render/RenderableBase.h"
#include "bending/load/BendingDataLoader.h"
#include <string>

SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cBendingGui : public cRenderable
{
public:
    explicit cBendingGui();
    virtual void Init(std::string root_path);
    virtual void UpdateGui();
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() override;

protected:
    std::string mRawBendingDataRootDir;
    tBendingClothArray mBendingData;
    std::vector<std::string> mClothName, mAngleName;
    std::vector<std::string> mFaceName = {
        "Front", "Back"};
    struct
    {
        int mCurClothId, mCurFaceId, mCurAngleId;
        bool mEnableDrawBezier;
        void Init()
        {
            mCurClothId = 0, mCurFaceId = 0, mCurAngleId = 0;
            mEnableDrawBezier = false;
            Sync();
        }
        bool IsChanged()
        {
            return (mCurClothId != mBeforeClothId) || (mCurFaceId != mBeforeFaceId) || (mCurAngleId != mBeforeAngleId) || (mEnableDrawBezier != mEnableDrawBezierBefore);
        }
        void Sync()
        {
            mBeforeClothId = mCurClothId;
            mBeforeFaceId = mCurFaceId;
            mBeforeAngleId = mCurAngleId;
            mEnableDrawBezierBefore = mEnableDrawBezier;
        }

    protected:
        int mBeforeClothId, mBeforeFaceId, mBeforeAngleId;
        bool mEnableDrawBezierBefore;
    } mSelectState;

    cRenderResourcePtr mEmptyResource, mRealPictureResource;
    void UpdateClothResource();
};

SIM_DECLARE_PTR(cBendingGui);
