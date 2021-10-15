#include "BendingDataCloth.h"
#include "utils/FileUtil.h"
#include "utils/RegexUtil.h"
#include <iostream>
bool GetFrontAndBackMat(std::string root_dir,
                        std::vector<std::string> &front_mat_path,
                        std::vector<std::string> &back_mat_path)
{
    front_mat_path.clear();
    back_mat_path.clear();
    bool failed = false;
    for (auto file_with_dir : cFileUtil::ListDir(root_dir))
    {
        auto file = cFileUtil::GetFilename(file_with_dir);
        if ("mat" == cFileUtil::GetExtension(file))
        {
            if ("front" == cFileUtil::ConvertToLowerCase(file.substr(0, 5)))
            {
                front_mat_path.push_back(file);
            }
            else if ("back" == cFileUtil::ConvertToLowerCase(file.substr(0, 4)))
            {
                back_mat_path.push_back(file);
            }
            else
            {
                failed = true;
                SIM_WARN("unclassified mat path {}", file_with_dir);
                break;
            }
        }
    }
    return failed;
}

bool CompareMat(std::string img_path0, std::string img_path1)
{
    double angle0 = GetWarpWeftAngleFromFilename(img_path0);
    double angle1 = GetWarpWeftAngleFromFilename(img_path1);
    return angle0 < angle1;
}

tBendingDataCloth::tBendingDataCloth()
{
}

bool tBendingDataCloth::Init(std::string data_dir)
{
    mDataDir = data_dir;
    // begin to init
    // 1. parse the index
    std::string subdir_single = cFileUtil::GetFilename(data_dir);
    auto res = cRegexUtil::FindAllMatch(subdir_single, "^[0-9]+");
    SIM_ASSERT(res.size() == 1);
    mId = std::stoi(res[0]);
    // 2. create the front and back data
    std::vector<std::string> front_paths, back_paths;
    if (true == GetFrontAndBackMat(data_dir, front_paths, back_paths))
    {
        std::cout << "failed in dir " << data_dir << std::endl;
        return false;
    }

    // 2. split them into front and backs
    std::sort(front_paths.begin(), front_paths.end(), CompareMat);
    std::sort(back_paths.begin(), back_paths.end(), CompareMat);
    SIM_ASSERT(front_paths.size() == back_paths.size());

    //
    mFrontData.clear();
    mBackData.clear();

    for (int i = 0; i < front_paths.size(); i++)
    {
        auto data = std::make_shared<tBendingData>();
        data->Init(cFileUtil::ConcatFilename(mDataDir, front_paths[i]));
        mFrontData.push_back(data);
    }

    for (int i = 0; i < back_paths.size(); i++)
    {
        auto data = std::make_shared<tBendingData>();
        data->Init(cFileUtil::ConcatFilename(mDataDir, back_paths[i]));
        mBackData.push_back(data);
    }

    return true;
}

tVectorXf tBendingDataCloth::GetFrontAngleList()
{
    tVectorXf front_angle_lst(mFrontData.size());
    for (int i = 0; i < mFrontData.size(); i++)
    {
        auto data = mFrontData[i];
        front_angle_lst[i] = data->GetWarpWeftAngle();
    }
    return front_angle_lst;
}
tVectorXf tBendingDataCloth::GetBackAngleList()
{
    tVectorXf back_angle_lst(mBackData.size());
    for (int i = 0; i < mBackData.size(); i++)
    {
        auto data = mBackData[i];
        back_angle_lst[i] = data->GetWarpWeftAngle();
    }
    return back_angle_lst;
}