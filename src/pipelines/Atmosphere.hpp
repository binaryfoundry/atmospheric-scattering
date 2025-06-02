#pragma once

#include "../Platform.hpp"
#include "../Camera.hpp"
#include "../Graphics.hpp"
#include "../Geometry.hpp"

#include "../math/Math.hpp"

#include <memory>

namespace Pipelines
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewport;
        glm::vec4 position;
        glm::vec4 exposure;
    };

    struct AtmosphereUniforms
    {
        float rayleigh_brightness_uniform = 64.0;
        float mie_brightness_uniform = 200.0;
        float spot_brightness_uniform = 10.0;
        float scatter_strength_uniform = 28.0;
        float rayleigh_strength_uniform = 239.0;
        float mie_strength_uniform = 264.0;
        float rayleigh_collection_power_uniform = 81.0;
        float mie_collection_power_uniform = 39.0;
        float mie_distribution_uniform = 63.0;
        float elevation_uniform = 1.0;
        float padding_1 = 0.0;
        float padding_2 = 0.0;
        glm::vec4 kr = glm::vec4(
            0.18867780436772762,
            0.4978442963618773,
            0.6616065586417131,
            1.0);
    };

    class Atmosphere : public Pipeline
    {
    private:
        bool initialized = false;

        float exposure = 1.0f;

        glm::mat4 view;
        glm::mat4 projection;

        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> framebuffer;
        std::unique_ptr<UniformBuffer<CameraUniforms>> camera_uniforms;

        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> atmosphere;
        std::unique_ptr<UniformBuffer<AtmosphereUniforms>> atmosphere_uniforms;

        Shader frontbuffer_shader;
        Shader atmosphere_shader;

        Descriptor frontbuffer_set_0;
        Descriptor atmosphere_set_0;
    public:
        Atmosphere();

        void Init();

        void InitAtmosphere(
            const uint32_t framebuffer_width,
            const uint32_t framebuffer_height);

        void Deinit();

        void DeinitAtmosphere();

        void Update();

        void Draw(
            const std::unique_ptr<Camera>& camera,
            const glm::mat4 projection,
            const glm::mat4 view,
            const bool upscale);

        std::unique_ptr<UniformBuffer<AtmosphereUniforms>>& uniforms()
        {
            return atmosphere_uniforms;
        }
    };
}
