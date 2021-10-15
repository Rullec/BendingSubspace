#include "utils/MatlabFileUtil.h"
#include "utils/FileUtil.h"
#include <iostream>
#include <vector>

tMatFile *cMatlabFileUtil::OpenMatFileReadOnly(const std::string &file)
{
    // the file must exist
    tMatFile *mat_fp = nullptr;
    mat_fp = Mat_Open(file.c_str(), MAT_ACC_RDONLY); // raed only

    if (mat_fp == nullptr)
    {
        printf("[error] failed to open matlab .mat file %s\n", file.c_str());
    }

    return mat_fp;
}

void cMatlabFileUtil::CloseMatFile(tMatFile *file_handle)
{
    Mat_Close(file_handle);
}

tMatFile *cMatlabFileUtil::OpenNewMatFile4(const std::string &file) // version 4 mat files
{
    // the file is forbidden
    tMatFile *mat_fp = Mat_CreateVer(
        file.c_str(), nullptr, MAT_FT_MAT4);
    if (mat_fp == nullptr)
    {
        printf("[error] create version 4 matfile %s failed\n", file.c_str());
    }
    return mat_fp;
}
tMatFile *cMatlabFileUtil::OpenNewMatFile5(const std::string &file) // version 5 mat files with  variable compression (if built with zlib)
{
    tMatFile *mat_fp = Mat_CreateVer(
        file.c_str(), nullptr, MAT_FT_MAT5);
    if (mat_fp == nullptr)
    {
        printf("[error] create version 5 matfile %s failed\n", file.c_str());
    }
    return mat_fp;
}
tMatFile *cMatlabFileUtil::OpenNewMatFile73(const std::string &file) // version 7.3 hdf5 format
{
    tMatFile *mat_fp = Mat_CreateVer(
        file.c_str(), nullptr, MAT_FT_MAT73);
    if (mat_fp == nullptr)
    {
        printf("[error] create version 73 matfile %s failed\n", file.c_str());
    }
    return mat_fp;
}

tMatVar *cMatlabFileUtil::ParseValueInfo(tMatFile *file, std::string name)
{
    tMatVar *var = Mat_VarReadInfo(file, name.c_str());
    int var_rank = var->rank;
    std::vector<int> dims(var_rank, 0);
    for (int i = 0; i < var_rank; i++)
    {
        dims[i] = var->dims[i];
    }
    return var;
}
tMatVar *cMatlabFileUtil::ParseValue(tMatFile *file, std::string name)
{
    tMatVar *var = Mat_VarRead(file, name.c_str());
    return var;
}
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
tVectorXd cMatlabFileUtil::ParseAsVectorXd(tMatFile *file, std::string name)
{
    tMatVar *var = cMatlabFileUtil::ParseValue(file, name);
    SIM_ASSERT(var->rank == 2);
    SIM_ASSERT(var->dims[0] == 1);
    SIM_ASSERT(MAT_C_DOUBLE == var->class_type);
    size_t total_size = var->nbytes / var->data_size;
    tVectorXd vec = tVectorXd::Zero(total_size);

    const double *data = static_cast<const double *>(var->data);
    for (int i = 0; i < total_size; i++)
    {
        vec[i] = data[i];
    }
    Mat_VarFree(var);
    return vec;
}

double cMatlabFileUtil::ParseAsDouble(tMatFile *file, std::string name)
{
    tMatVar *var = cMatlabFileUtil::ParseValue(file, name);
    int size = 1;
    for (int i = 0; i < var->rank; i++)
    {
        size *= var->dims[i];
    }
    SIM_ASSERT(size == 1);
    SIM_ASSERT(MAT_C_DOUBLE == var->class_type);

    const double *data = static_cast<const double *>(var->data);
    double ret = data[0];
    Mat_VarFree(var);
    return ret;
}
void cMatlabFileUtil::IterValue(tMatFile *file)
{
    tMatVar *cur_var = nullptr;
    std::vector<std::string> name_lst(0);
    while (
        (cur_var = Mat_VarReadNextInfo(file)) != nullptr)
    {
        name_lst.push_back(std::string(cur_var->name));
        std::cout << "oh " << cur_var->name << std::endl;
    }
    Mat_VarFree(cur_var);
}

std::string cMatlabFileUtil::ParseAsString(tMatFile *file, std::string name)
{
    matvar_t *var = Mat_VarRead(file, name.c_str());
    size_t total_size = var->nbytes / var->data_size;
    const char *data = static_cast<const char *>(var->data);
    std::string ret = std::string(data);
    Mat_VarFree(var);
    return ret;
}