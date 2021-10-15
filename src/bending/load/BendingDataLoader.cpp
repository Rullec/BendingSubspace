#include "BendingDataLoader.h"
#include <iostream>
#include "utils/FileUtil.h"
#include "utils/LogUtil.h"

#include "utils/RegexUtil.h"

tBendingClothArray BuildBendingClothArray(std::string root_dir)
{
    std::vector<std::string> subdirs = cFileUtil::ListDir(root_dir);
    tBendingClothArray mBendingDataClothArray(0);
    for (auto subdir : subdirs)
    {
        if (cFileUtil::IsDir(subdir) == true)
        {
            // create the bending data cloth
            auto cloth = std::make_shared<tBendingDataCloth>();
            if (true == cloth->Init(subdir))
            {
                mBendingDataClothArray.push_back(cloth);
            }
        }
    }

    std::sort(mBendingDataClothArray.begin(), mBendingDataClothArray.end(),
              [](tBendingDataClothPtr p0, tBendingDataClothPtr p1)
              {
                  return p0->GetId() < p1->GetId();
              });
    return mBendingDataClothArray;
}

std::vector<std::string> BuildClothName(const tBendingClothArray &array)
{
    std::vector<std::string> name_lst(0);
    for (auto &cloth : array)
    {
        std::string name = cFileUtil::GetFilename(cloth->GetDir());
        name_lst.push_back(name);
    }
    return name_lst;
}

std::vector<std::string> BuildAngleName(const tBendingClothArray &array)
{
    tVectorXf final_angle_lst(0);
    for (auto &cur_cloth : array)
    {
        // std::cout << cur_cloth->GetDir() << std::endl;
        tVectorXf cur_front_angle_lst = cur_cloth->GetFrontAngleList(), cur_back_angle_lst = cur_cloth->GetBackAngleList();
        if (final_angle_lst.size() == 0)
        {
            final_angle_lst = cur_front_angle_lst;
            final_angle_lst = cur_back_angle_lst;
        }
        else
        {
            if (
                (cur_front_angle_lst.size() != final_angle_lst.size()) ||
                (cur_back_angle_lst.size() != final_angle_lst.size()))
            {
                SIM_ERROR("build angle name inconsistent (size): cur front angle {} != ulti front angle {}, cur back angle {} != ulti back angle {}, ", cur_front_angle_lst.size(), final_angle_lst.size(), cur_back_angle_lst.size(), final_angle_lst.size());
            }
            tVectorXf front_diff = final_angle_lst - cur_front_angle_lst;
            tVectorXf back_diff = final_angle_lst - cur_back_angle_lst;
            if (front_diff.norm() > 1e-3 || back_diff.norm() > 1e-3)
            {
                SIM_ERROR("build angle name inconsistent (value): cur front angle {} != ulti front angle {}, cur back angle {} != ulti back angle {}, ", cur_front_angle_lst.transpose(), final_angle_lst.transpose(), cur_back_angle_lst.transpose(), final_angle_lst.transpose());
            }
        }
    }

    std::vector<std::string> string_lst(0);
    for (int i = 0; i < final_angle_lst.size(); i++)
    {
        string_lst.push_back(std::to_string(final_angle_lst[i]).substr(0, 5));
    }
    return string_lst;
}

std::vector<std::string> BuildFaceName()
{
    return gBendingDataFaceInfoStr;
}

tBendingDataPtr GetBendingData(tBendingDataClothPtr cloth, int face_mode, float angle)
{
    tBendingDataPtr data = nullptr;
    switch (face_mode)
    {
    case eBendingDataFaceInfo::FRONT_BENDING:
        data = cloth->GetFrontDataByAngle(angle);
        break;
    case eBendingDataFaceInfo::BACK_BENDING:
        data = cloth->GetBackDataByAngle(angle);
        break;

    default:
        SIM_ERROR("Unsupported bending data for face {} angle {}", face_mode, angle);
        break;
    }
    return data;
}