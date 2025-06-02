#pragma once


#include "Timing.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"

#include "properties/Property.hpp"
#include "interfaces/IApplication.hpp"
#include "pipelines/Atmosphere.hpp"

#include "gl/ImGui.hpp"
#include "gl/OpenGL.hpp"
#include "gl/Pipeline.hpp"

class Application : public IApplication
{
private:
    const float fps_alpha = 0.9f;
    hrc::time_point fps_time;
    float fps_time_avg = 60;

    float mouse_speed = 75.0f;

    float Kr[4] = {
        0.18867780436772762,
        0.4978442963618773,
        0.6616065586417131,
        1.0
    };

    uint32_t framebuffer_width = 0;
    uint32_t framebuffer_height = 0;

    glm::mat4 projection;
    glm::mat4 view;

    GUI gui;
    Pipelines::Atmosphere pipeline;

    Properties::Property<float> prop;

    std::unique_ptr<Camera> camera;

    void ViewScale();
    bool GuiUpdate();

public:
    void Init();
    void Deinit();
    void Update();
};
