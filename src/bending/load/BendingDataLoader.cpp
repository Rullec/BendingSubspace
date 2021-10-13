#include "BendingDataLoader.h"
#include <iostream>
#include "utils/FileUtil.h"
#include "utils/LogUtil.h"

#include "utils/RegexUtil.h"
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

double GetMatId(std::string path)
{
    std::string name = cFileUtil::GetFilename(path);

    int st = name.find("-") + 1;
    int ed = name.find(".mat");
    auto sub = name.substr(st, ed - st + 1);
    if (sub.size() == 0)
    {
        SIM_WARN("failed to get mat id from {}", path);
        return -1;
    }
    double idx = std::stof(sub.c_str());
    // printf("name %s float %f\n", name.c_str(),
    //        idx);
    return idx;
}

bool CompareMat(std::string img_path0, std::string img_path1)
{
    double angle0 = GetMatId(img_path0);
    double angle1 = GetMatId(img_path1);
    return angle0 < angle1;
}
typedef std::map<int, tBendingDataClothPtr> tBendingClothArray;

tBendingClothArray BuildBendingClothArray(std::string root_dir)
{
    std::vector<std::string> subdirs = cFileUtil::ListDir(root_dir);
    tBendingClothArray mBendingDataClothArray;
    for (auto subdir : subdirs)
    {
        // std::cout << subdir << std::endl;
        if (cFileUtil::IsDir(subdir) == true)
        {
            // 1. get all mat in this directory
            std::vector<std::string> front_paths, back_paths;
            if (true == GetFrontAndBackMat(subdir, front_paths, back_paths))
            {
                std::cout << "failed in dir " << subdir << std::endl;
                continue;
            }

            // 2. split them into front and backs
            std::sort(front_paths.begin(), front_paths.end(), CompareMat);
            std::sort(back_paths.begin(), back_paths.end(), CompareMat);
            SIM_ASSERT(front_paths.size() == back_paths.size());

            std::string subdir_single = cFileUtil::GetFilename(subdir);
            auto res = cRegexUtil::FindAllMatch(subdir_single, "^[0-9]+");
            SIM_ASSERT(res.size() == 1);
            int data_id = std::stoi(res[0]);
            // std::cout << subdir_single << " " << data_id << std::endl;
            // for (int i = 0; i < front_paths.size(); i++)
            // {
            //     std::cout << front_paths[i] << std::endl;
            //     std::cout << back_paths[i] << std::endl;
            // }
            SIM_ASSERT(mBendingDataClothArray.find(data_id) == mBendingDataClothArray.end());

            {
                // 1. build front & back data list
                auto cloth = std::make_shared<tBendingDataCloth>();
                cloth->mId = data_id;
                cloth->mDataDir = subdir;

                for (int i = 0; i < front_paths.size(); i++)
                {
                    auto data = std::make_shared<tBendingData>();
                    data->mMatPath = front_paths[i];
                    cloth->mFrontData.push_back(data);
                }

                for (int i = 0; i < back_paths.size(); i++)
                {
                    auto data = std::make_shared<tBendingData>();
                    data->mMatPath = back_paths[i];
                    cloth->mBackData.push_back(data);
                }

                // 2. assemble to the cloth data
                mBendingDataClothArray[data_id] = cloth;
                // 3. push
            }
            // 3. create datas, and push them all together
        }
    }
    return mBendingDataClothArray;
}
