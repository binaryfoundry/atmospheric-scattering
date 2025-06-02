#include "Application.hpp"

#include "Input.hpp"
#include "imgui/imgui.h"
#include "math/Random.hpp"

#include <array>

#if !defined(EMSCRIPTEN)
#include "sdl/Main.hpp"
#else
#include "sdl/MainWeb.hpp"
#endif

int main(int argc, char* argv[])
{
    std::unique_ptr<IApplication> app = std::make_unique<Application>();
    return sdl_init(app);
}

void Application::Init()
{
    framebuffer_width = 1024;
    framebuffer_height = 768;

    camera = std::make_unique<Camera>();
    camera->position = glm::vec3(0, 0, 0);

    pipeline.Init();

    gui.Init();

    pipeline.InitAtmosphere(
        framebuffer_width,
        framebuffer_height);

    fps_time = timer_start();

    prop.Animate(
        context->property_manager,
        0, 1.0f, 1.0f,
        Properties::EasingFunction::Linear,
        []() {
            std::cout << "Complete!\n";
        }
    );
}

void Application::Deinit()
{
    pipeline.DeinitAtmosphere();
    pipeline.Deinit();
    gui.Deinit();
}

void Application::Update()
{
    const float time_ms = timer_end(fps_time);
    fps_time = timer_start();
    fps_time_avg = fps_alpha * fps_time_avg + (1.0f - fps_alpha) * time_ms;

    const float fps_scale = std::max<float>(
        1.0f / std::min<float>(5.0f, fps_time_avg / 16.66666f), 0.1f);

    context->property_manager.Update(
        time_ms / 1000.0f);

    const bool reinit_pipeline = GuiUpdate();

    const float window_aspect_ratio =
        static_cast<float>(window_width) /
        window_height;

    camera->orientation.yaw +=
        static_cast<float>(captured_mouse_delta_x) /
        (mouse_speed * window_aspect_ratio);

    camera->orientation.pitch +=
        static_cast<float>(captured_mouse_delta_y) /
        (mouse_speed);

    camera->viewport = glm::vec4(
        0, 0,
        framebuffer_width,
        framebuffer_height);

    if (reinit_pipeline)
    {
        pipeline.DeinitAtmosphere();

        pipeline.InitAtmosphere(
            framebuffer_width,
            framebuffer_height);
    }

    pipeline.SetWindowSize(
        window_width,
        window_height);

    ViewScale();

    pipeline.Draw(
        camera,
        projection,
        view,
        false);

    gui.Draw(
        window_width,
        window_height);
}

void Application::ViewScale()
{
    const float window_aspect =
        static_cast<float>(window_width) /
        window_height;

    const float framebuffer_ratio =
        static_cast<float>(framebuffer_width) /
        static_cast<float>(framebuffer_height);

    const float aspect =
        window_aspect / framebuffer_ratio;

    const bool wide =
        window_width / framebuffer_ratio > window_height;

    const glm::vec2 h_scale = glm::vec2(
        std::floor(window_width / aspect), window_height);

    const glm::vec2 v_scale = glm::vec2(
        window_width, std::floor(window_height * aspect));

    glm::vec3 scale = wide ?
        glm::vec3(h_scale, 1) :
        glm::vec3(v_scale, 1);

    const float hpos =
        std::round((window_width / 2) - (scale.x / 2));

    const float vpos =
        std::round((window_height / 2) - (scale.y / 2));

    projection = glm::ortho<float>(
        0,
        static_cast<float>(window_width),
        static_cast<float>(window_height),
        0,
        -1.0f,
        1.0f);

    view = glm::mat4();

    view = glm::translate(
        view,
        glm::vec3(hpos, vpos, 0.0f));

    view = glm::scale(
        view,
        scale);
}

bool Application::GuiUpdate()
{
    ImGui::NewFrame();

    ImGui::Begin(
        "Menu",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::LabelText(
        "Controls",
        "Mouse look.");

    ImGui::SliderFloat(
        "Exposure",
        &camera->exposure,
        0,
        3);

    auto& uniforms = pipeline.uniforms();

    ImGui::SliderFloat(
        "Spot Elevatiom",
        &uniforms->object.elevation_uniform,
        0.0,
        1);

    ImGui::SliderFloat(
        "Rayleigh Brightness",
        &uniforms->object.rayleigh_brightness_uniform,
        1,
        100);

    ImGui::SliderFloat(
        "Mie Brightness",
        &uniforms->object.mie_brightness_uniform,
        1,
        1000);

    ImGui::SliderFloat(
        "Spot Brightness",
        &uniforms->object.spot_brightness_uniform,
        1,
        100);

    ImGui::SliderFloat(
        "Scatter Strength",
        &uniforms->object.scatter_strength_uniform,
        1,
        1000);

    ImGui::SliderFloat(
        "Rayleigh Strength",
        &uniforms->object.rayleigh_strength_uniform,
        1,
        1000);

    ImGui::SliderFloat(
        "Mie Strength",
        &uniforms->object.mie_strength_uniform,
        1,
        10000);

    ImGui::SliderFloat(
        "Rayleigh Collection Power",
        &uniforms->object.rayleigh_collection_power_uniform,
        1,
        100);

    ImGui::SliderFloat(
        "Mie Collection Power",
        &uniforms->object.mie_collection_power_uniform,
        1,
        100);

    ImGui::SliderFloat(
        "mie_Distribution",
        &uniforms->object.mie_distribution_uniform,
        1,
        100);

    ImGui::ColorEdit3("Kr", Kr);
    uniforms->object.kr.r = Kr[0];
    uniforms->object.kr.g = Kr[1];
    uniforms->object.kr.b = Kr[2];

    ImGui::Text(
        "Application average %.3f ms/frame (%.1f FPS)",
        fps_time_avg,
        1000.0f / fps_time_avg);

    ImGui::End();

    bool reinit_pipeline = false;

    if (window_height != framebuffer_height ||
        window_width != framebuffer_width)
    {
        framebuffer_width = window_width;
        framebuffer_height = window_height;
        reinit_pipeline = true;
    }


    return reinit_pipeline;
}
