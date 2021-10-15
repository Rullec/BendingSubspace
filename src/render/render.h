#pragma once
#include <memory>
#include <string>
#include <vector>
#include "utils/MathUtil.h"
#include "utils/DefUtil.h"
#include "utils/GLUtil.h"
SIM_DECLARE_CLASS_AND_PTR(cRenderable);
SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cRender
{
public:
    cRender(int window_height, int window_width);
    virtual ~cRender();
    void Init();
    unsigned int BindVAO();
    GLFWwindow *GetWindow();
    void MouseMoveCallback(double xpos, double ypos);
    void UseProgram();
    void UpdateTextureFromRenderResourceVec(std::vector<cRenderResourcePtr> resource);
    void PostUpdate();
    void SetRenderablePtr(cRenderablePtr mana);

protected:
    unsigned int InitShader();
    int mHeight, mWidth;
    unsigned int mShaderProgram;
    GLFWwindow *mWindow;
    const int mStartX = 100, mStartY = 100;
    GLuint mTextureId, mFBO;
    std::vector<float> mTextureData;
    const std::string mWindowName = "SubspaceMaker";
    cRenderablePtr mRenderInterior;
    std::vector<cRenderResourcePtr> mCurRenderingResource;

    void InitGL();
    void InitTextureAndFBO();
    void CreateTexture(GLuint &texture, std::vector<float> &texture_data, int width, int height) const;
    void CreateFBOFromTexture(GLuint &fbo, GLuint texture);

    void UpdateTextureFromDepthImage(const tMatrixXi &depth_image);
    void UpdateTextureFromRenderResource(cRenderResourcePtr resource);
    void UpdateTextureData(GLuint texture, std::vector<float *> data_array,
                           std::vector<int> channels_array,
                           const tEigenArr<tVector2i> &shape_array, const tEigenArr<tVector2i> &st_array);
    void UpdateFBO(GLuint fbo);

    void Resize(int height, int width);
};
SIM_DECLARE_PTR(cRender);