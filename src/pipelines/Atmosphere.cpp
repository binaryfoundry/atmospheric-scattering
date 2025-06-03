#include "Atmosphere.hpp"

#include <sstream>
#include <algorithm>

namespace Pipelines
{
    Atmosphere::Atmosphere()
    {
    }

    void Atmosphere::Init()
    {
        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        atmosphere_shader.Load("files/gl/atmosphere.glsl");

        frontbuffer_shader.Link();
        atmosphere_shader.Link();

        camera_uniforms =
            std::make_unique<UniformBuffer<CameraUniforms>>();
    }

    void Atmosphere::Deinit()
    {
        atmosphere->Delete();

        frontbuffer_shader.Delete();
        atmosphere_shader.Delete();
    }

    void Atmosphere::InitAtmosphere(
        const uint32_t framebuffer_width,
        const uint32_t framebuffer_height)
    {
        atmosphere =
            std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

        atmosphere->Create(
            framebuffer_width,
            framebuffer_height,
            true);

        camera_uniforms =
            std::make_unique<UniformBuffer<CameraUniforms>>();

        atmosphere_uniforms =
            std::make_unique<UniformBuffer<AtmosphereUniforms>>();

        framebuffer =
            std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

        framebuffer->Create(
            framebuffer_width,
            framebuffer_height,
            true);

        frontbuffer_set_0.SetSampler2D(
            "tex",
            *framebuffer,
            Filter::NEAREST,
            Filter::NEAREST,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        frontbuffer_set_0.SetUniformMat4(
            "view",
            &view);

        frontbuffer_set_0.SetUniformMat4(
            "projection",
            &projection);

        frontbuffer_set_0.SetUniformFloat(
            "exposure",
            &exposure);

        frontbuffer_shader.Set(
            frontbuffer_set_0,
            0);

        atmosphere_set_0.SetUniformBlock(
            "camera",
            *camera_uniforms);

        atmosphere_set_0.SetUniformBlock(
            "atmosphere",
            *atmosphere_uniforms);

        atmosphere_shader.Set(
            atmosphere_set_0,
            0);
    }

    void Atmosphere::DeinitAtmosphere()
    {
        if (framebuffer == nullptr)
        {
            return;
        }

        framebuffer->Delete();
        camera_uniforms->Delete();
        atmosphere->Delete();
        atmosphere_uniforms->Delete();
    }

    inline size_t sampler_index(
        uint16_t x, uint16_t y, uint32_t w)
    {
        return x + (y * w);
    }

    void Atmosphere::Update()
    {
    }

    void Atmosphere::Draw(
        const std::unique_ptr<Camera>& camera,
        const glm::mat4 projection_,
        const glm::mat4 view_,
        const bool upscale)
    {
        projection = projection_;
        view = view_;

        // Draw to FBO

        framebuffer->Bind();

        camera->Validate();
        camera_uniforms->object.view =
            camera->View();
        camera_uniforms->object.projection =
            camera->Projection();
        camera_uniforms->object.viewport =
            camera->viewport;
        camera_uniforms->object.position = glm::vec4(
            camera->position, 1.0f);
        camera_uniforms->Update();

        atmosphere_uniforms->Update();

        DrawQuad(
            atmosphere_shader);

        // Render to front buffer

        exposure = camera->exposure;

        FrontBuffer();

        Clear();

        DrawQuad(
            frontbuffer_shader);
    }
}
