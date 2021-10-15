#pragma
#include <vector>
#include <k4a/k4a.h>
#include <string>
#include "utils/DefUtil.h"
#include "utils/MathUtil.h"
namespace cv
{
    class Mat;
};
SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cRenderResource
{
public:
    cRenderResource();
    void ConvertFromEigen(const tMatrixXf &mat);
    void ConvertFromOpencv(const cv::Mat &image);
    void ConvertFromOpencvFloat3Channel(const cv::Mat &image);
    void ConvertFromOpencvFloat1Channel(const cv::Mat &image);

    virtual void DimmedByWindow(const tVector2i &st_pos, const tVector2i &window_size);
    // virtual cv::Mat ConvertToOpencvPresent(const tVector2i &window_st,
    //                                        const tVector2i &window_size);

    void ConvertFromAnotherResource(cRenderResourcePtr ptr);
    void Reset();
    int mHeight, mWidth;
    int mChannels;
    std::vector<float> mPixelData;
};
