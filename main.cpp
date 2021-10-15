#include <iostream>
// #include "bending/load/BendingDataLoader.h"
#include "bending/BendingGui.h"
#include "render/render.h"
#include "utils/TimeUtil.hpp"
// #include "render/RenderImGui.h"
unsigned int gWindowWidth = 0;
unsigned int gWindowHeight = 0;
int gStartX = 100;
int gStartY = 100;
std::string gWindowName = "";

SIM_DECLARE_PTR(cRender);
cRenderPtr render = nullptr;
#include <matplot/matplot.h>
int main()
{
    // {
    //     using namespace matplot;
    //     std::vector<double> x = linspace(0, 2 * pi);
    //     std::vector<double> y = transform(x, [](auto x)
    //                                       { return sin(x); });

    //     plot(x, y, "-o");
    //     hold(on);
    //     plot(x, transform(y, [](auto y)
    //                       { return -y; }),
    //          "--xr");
    //     plot(x, transform(x, [](auto x)
    //                       { return x / pi - 1.; }),
    //          "-:gs");
    //     plot({1.0, 0.7, 0.4, 0.0, -0.4, -0.7, -1}, "k");

    //     save("hello.png");
    //     exit(1);
    // }
    //std::cout << "--begin error code--\n";
    //std::vector<int> lst;
    //lst[4] = 1;
    //std::cout << lst[4] << std::endl;
    //std::cout << "--end error code--\n";

    std::string root_dir = "D:\\RealMeasureData\\BendingMeasureData";
    auto gui = std::make_shared<cBendingGui>();
    gui->Init(root_dir);
    // exit(1);
    gWindowHeight = 600;
    gWindowWidth = 800;

    render = std::make_shared<cRender>(gWindowHeight, gWindowWidth);
    render->Init();
    render->SetRenderablePtr(std::dynamic_pointer_cast<cRenderable>(gui));
    GLFWwindow *gWindow = render->GetWindow();

    while (!glfwWindowShouldClose(gWindow))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render->UseProgram();
        // render->UpdateTextureFromRenderResource(gui->GetDepthImageNew());
        // render->UpdateTextureFromRenderResource(manager->GetColorImageNew());
        render->UpdateTextureFromRenderResourceVec(gui->GetRenderingResource());
        render->PostUpdate();
    }
    glfwTerminate();
    // auto render = new cRenderImGui();
    // render->Init()
}