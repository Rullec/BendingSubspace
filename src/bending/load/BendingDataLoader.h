#pragma once
#include <map>
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
#include "bending/load/BendingData.h"
#include "bending/load/BendingDataCloth.h"

int LoadClothData(std::string data_dir,
                  std::vector<std::string> &img_list,
                  std::vector<std::string> &mat_list);

typedef std::vector<tBendingDataClothPtr> tBendingClothArray;

tBendingClothArray BuildBendingClothArray(std::string root_dir);

std::vector<std::string> BuildClothName(const tBendingClothArray &array);
std::vector<std::string> BuildAngleName(const tBendingClothArray &array);

