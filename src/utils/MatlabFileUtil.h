#pragma once
#include <matio.h>
#include <string>
#include "utils/MathUtil.h"

typedef mat_t tMatFile;
typedef matvar_t tMatVar;

class cMatlabFileUtil
{
public:
    static tMatFile *OpenMatFileReadOnly(const std::string &file);
    static tMatFile *OpenNewMatFile4(const std::string &file);  // version 4 mat files
    static tMatFile *OpenNewMatFile5(const std::string &file);  // version 5 mat files with variable compression (if built with zlib)
    static tMatFile *OpenNewMatFile73(const std::string &file); // version 7.3 hdf5 format
    static void CloseMatFile(tMatFile *file_handle);
    static tMatVar *ParseValueInfo(tMatFile *file, std::string name);
    static tMatVar *ParseValue(tMatFile *file, std::string name);
    static tVectorXd ParseAsVectorXd(tMatFile *file, std::string name);
    static double ParseAsDouble(tMatFile *file, std::string name);
    static std::string ParseAsString(tMatFile *file, std::string name);
    static void IterValue(tMatFile *file);
};