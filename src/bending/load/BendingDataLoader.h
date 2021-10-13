#pragma once
#include <map>
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
#include "bending/load/BendingData.h"

int LoadClothData(std::string data_dir,
                  std::vector<std::string> &img_list,
                  std::vector<std::string> &mat_list);

typedef std::map<int, tBendingDataClothPtr> tBendingClothArray;

tBendingClothArray BuildBendingClothArray(std::string root_dir);