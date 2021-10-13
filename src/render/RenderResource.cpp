#include "RenderResource.h"
#include "utils/LogUtil.h"
cRenderResource::cRenderResource()
{
    mHeight = 0;
    mWidth = 0;
    mChannels = -1;
    mPixelData.resize(0);
}
void cRenderResource::Reset()
{
    mHeight = 0;
    mWidth = 0;
    mChannels = -1;
    mPixelData.resize(0);
}
#define STB_IMAGE_WRITE_IMPLEMETATION
#include "utils/stb_image_write.h"
#include "utils/FileUtil.h"
std::vector<uint8_t> stb_export_buf;

/**
 * \brief       
 *      Input raw_buf, unit is [m]
 *      Output png, unit is [m * 255.99]
 * 
*/
void ExportDepthToPng(float *raw_buf, int height, int width, int buf_channels, std::string output_name)
{
    if (cFileUtil::ValidateFilePath(output_name) == false)
    {
        SIM_ERROR("invalida file path when export depth to png {}", output_name);
        exit(1);
    }
    stb_export_buf.resize(height * width);
    // row major
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)
        {
            int raw_buf_idx = (row * width + col) * buf_channels;
            int stb_buf_idx = ((height - 1 - row) * width + col);
            int val = raw_buf[raw_buf_idx] * 255.99f;
            if (val < 0)
                val = 0;
            if (val > 255)
                val = 0;
            stb_export_buf[stb_buf_idx] = uint8_t(val);
        }

    stbi_write_png(output_name.c_str(), width, height, 1, stb_export_buf.data(), width * sizeof(uint8_t));
}

/**
 * \brief   
 *      Output depth value as txt, unit is [m]
*/
#include <iomanip>
void ExportDepthToTxt(float *raw_buf, int height, int width, int buf_channels, std::string output_name)
{
    if (cFileUtil::ValidateFilePath(output_name) == false)
    {
        SIM_ERROR("invalida file path when export depth to png {}", output_name);
        exit(1);
    }
    std::ofstream fout(output_name, 'w');
    fout << "width " << width << " height " << height << " unit "
         << "meter\n";
    // row major
    fout << std::setprecision(6);
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int raw_buf_idx = ((height - 1 - row) * width + col) * buf_channels;
            fout << raw_buf[raw_buf_idx] << " ";
            // uint32_t val = raw_buf[raw_buf_idx] * 255.99f;
            // if (val > 255)
            //     val = 0;
            // stb_export_buf[stb_buf_idx] = uint8_t(val);
        }
        fout << std::endl;
    }
}

void ExportRGBColorToPng(float *buf, int height, int width, int buf_channels, std::string output_name)
{
    if (cFileUtil::ValidateFilePath(output_name) == false)
    {
        SIM_ERROR("invalida file path when export depth to png {}", output_name);
        exit(1);
    }
    stb_export_buf.resize(height * width * 3);
    // row major
    for (int row = 0; row < height; row++)
        for (int col = 0; col < width; col++)
        {
            int raw_buf_idx_st = (row * width + col) * buf_channels;
            int stb_buf_idx = ((height - 1 - row) * width + col) * 3;

            for (int j = 0; j < 3; j++)
            {
                stb_export_buf[stb_buf_idx + j] = uint8_t(buf[raw_buf_idx_st + j] * 255.99);
            }
        }

    stbi_write_png(output_name.c_str(), width, height, 3, stb_export_buf.data(), width * sizeof(uint8_t) * 3);
}
#include "utils/OpenCVUtil.h"
void cRenderResource::ConvertFromOpencv(const cv::Mat &image)
{
    // raw data fill
    mHeight = image.rows;
    mWidth = image.cols;
    mChannels = 3;
    if (image.type() != CV_8UC3)
    {
        SIM_ERROR("image type {} != CV_8UC3", cOpencvUtil::type2str(image.type()));
    }
    mPixelData.resize(mHeight * mWidth * mChannels);
    for (int row_id = 0; row_id < mHeight; row_id++)
        for (int col_id = 0; col_id < mWidth; col_id++)
        {
            auto pt = image.at<cv::Vec3b>(row_id, col_id);
            int output_idx = (mHeight - 1 - row_id) * mWidth + col_id;
            mPixelData[output_idx * 3 + 0] = float(pt[2]) / 255.0;
            mPixelData[output_idx * 3 + 1] = float(pt[1]) / 255.0;
            mPixelData[output_idx * 3 + 2] = float(pt[0]) / 255.0;
        }
}

void cRenderResource::DimmedByWindow(const tVector2i &st_pos_, const tVector2i &window_size_)
{
    // only dim the present data
    tVector2i st_pos = st_pos_;
    tVector2i window_size = window_size_;
    double dim_scale = 0.5;
    // if enable down sampling, window_size /= 2, st_pos /= 2
    int height_st = st_pos[0], height_ed = height_st + window_size[0];
    int width_st = st_pos[1], width_ed = width_st + window_size[1];
    for (int row_id = 0; row_id < mHeight; row_id++)
    {
        for (int col_id = 0; col_id < mWidth; col_id++)
        {
            if (
                (row_id < height_st) ||
                (row_id > height_ed) ||
                (col_id < width_st) ||
                (col_id > width_ed))
            {
                int output_idx = (mHeight - 1 - row_id) * mWidth + col_id;
                mPixelData[3 * output_idx + 0] *= dim_scale;
                mPixelData[3 * output_idx + 1] *= dim_scale;
                mPixelData[3 * output_idx + 2] *= dim_scale;
            }
        }
    }
}

void cRenderResource::ConvertFromAnotherResource(cRenderResourcePtr ptr)
{
    mHeight = ptr->mHeight, mWidth = ptr->mWidth;
    mHeight = ptr->mHeight, mWidth = ptr->mWidth;
    mChannels = ptr->mChannels;
    mPixelData = ptr->mPixelData;
}