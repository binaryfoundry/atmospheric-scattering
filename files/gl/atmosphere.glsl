#version 300 es

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        vec2 pos = (position.xy - vec2(0.5)) * 2.0;
        gl_Position = vec4(pos.xy, -1.0, 1.0);
    }

#elif defined(COMPILING_FS)

    #ifdef GL_ES
    precision lowp float;
    #endif

    layout(std140) uniform camera{
        mat4 view;
        mat4 projection;
        vec4 viewport;
        vec4 position;
        vec4 padding_0;
    };

    layout(std140) uniform atmosphere{
        float rayleigh_brightness_uniform;
        float mie_brightness_uniform;
        float spot_brightness_uniform;
        float scatter_strength_uniform;
        float rayleigh_strength_uniform;
        float mie_strength_uniform;
        float rayleigh_collection_power_uniform;
        float mie_collection_power_uniform;
        float mie_distribution_uniform;
        float elevation_uniform;
        float atmosphere_padding_1;
        float atmosphere_padding_2;
        vec4 Kr;
    };

    float surface_height = 0.99;
    float range = 0.01;
    float intensity = 1.8;
    int step_count = 16;

    in vec2 v_texcoord;
    layout(location = 0) out vec4 out_color;

    vec3 ray_direction(vec2 coords) {
        float zoom = 0.4;
        float aspect = viewport.w / viewport.z;
        float size = 1.0 / zoom;

        vec4 h = vec4(size * 2.0, 0.0, 0.0, 1.0);
        vec4 v = vec4(0.0, size * 2.0 * aspect, 0.0, 1.0);
        vec4 c = vec4(-size, -size * aspect, -1.5, 1.0);

        h = h * view;
        v = v * view;
        c = c * view;

        vec3 direction = normalize(
            (c + coords.x * h + coords.y * v).xyz);
        return direction;
    }

    float phase(float alpha, float g) {
        float a = 3.0 * (1.0 - g * g);
        float b = 2.0 * (2.0 + g * g);
        float c = 1.0 + alpha * alpha;
        float d = pow(1.0 + g  *g - 2.0  * g * alpha, 1.5);
        return (a / b) * (c / d);
    }

    float horizon_extinction(vec3 position, vec3 dir, float radius) {
        float u = dot(dir, -position);
        if(u<0.0) {
            return 1.0;
        }
        vec3 near = position + u * dir;
        if(length(near) < radius) {
            return 0.0;
        }
        else {
            vec3 v2 = normalize(near) * radius - position;
            float diff = acos(dot(normalize(v2), dir));
            return smoothstep(0.0, 1.0, pow(diff * 2.0, 3.0));
        }
    }

    float atmospheric_depth(vec3 position, vec3 dir) {
        float a = dot(dir, dir);
        float b = 2.0 * dot(dir, position);
        float c = dot(position, position)-1.0;
        float det = b * b - 4.0 * a * c;
        float detSqrt = sqrt(det);
        float q = (-b - detSqrt) / 2.0;
        float t1 = c / q;
        return t1;
    }

    vec3 absorb(float dist, vec3 color, float factor) {
        return color - color * pow(Kr.xyz, vec3(factor / dist));
    }

    void main() {
        float rayleigh_brightness = rayleigh_brightness_uniform / 10.0;
        float mie_brightness = mie_brightness_uniform / 1000.0;
        float spot_brightness = spot_brightness_uniform;
        float scatter_strength = scatter_strength_uniform / 1000.0;
        float rayleigh_strength = rayleigh_strength_uniform / 1000.0;
        float mie_strength = mie_strength_uniform / 10000.0;
        float rayleigh_collection_power = rayleigh_collection_power_uniform / 100.0;
        float mie_collection_power = mie_collection_power_uniform / 100.0;
        float mie_distribution = mie_distribution_uniform / 100.0;

        vec4 light_direction = vec4(0.0, 1.0 * elevation_uniform, -1.0, 1.0);

        vec3 direction = normalize(-light_direction.xyz);

        vec3 eyedir = ray_direction(v_texcoord);

        float alpha = dot(eyedir, -direction);

        float rayleigh_factor = phase(alpha, -0.01) *
            rayleigh_brightness;

        float mie_factor = phase(alpha, mie_distribution) *
            mie_brightness;

        float spot = smoothstep(0.0, 25.0, phase(alpha, 0.995)) *
            spot_brightness;

        vec3 eye_position = vec3(0.0, surface_height, 0.0);

        float eye_depth = atmospheric_depth(eye_position, eyedir);

        float step_length = eye_depth / float(step_count);

        float eye_extinction = horizon_extinction(
            eye_position,
            eyedir,
            surface_height - 0.25);

        vec3 rayleigh_collected = vec3(0.0);
        vec3 mie_collected = vec3(0.0);

        for(int i = 0; i < step_count; i++) {
            float sample_distance = step_length * float(i);

            vec3 position = eye_position + eyedir * sample_distance;

            float extinction = horizon_extinction(
                position,
                -direction,
                surface_height);

            float sample_depth = atmospheric_depth(
                position,
                -direction);

            vec3 influx = absorb(
                sample_depth,
                vec3(intensity),
                scatter_strength) * extinction;

            rayleigh_collected += absorb(
                sample_distance,
                Kr.xyz * influx,
                rayleigh_strength);

            mie_collected += absorb(
                sample_distance,
                influx,
                mie_strength);
        }

        float rayleigh_power = pow(
            eye_depth, rayleigh_collection_power);

        float mie_power = pow(
            eye_depth, mie_collection_power);

        rayleigh_collected =
            rayleigh_collected *
            rayleigh_power *
            eye_extinction;

        mie_collected = mie_collected * mie_power * eye_extinction;

        rayleigh_collected /= float(step_count);
        mie_collected /= float(step_count);

        vec3 final_color = vec3(
            spot * mie_collected +
            mie_factor * mie_collected +
            rayleigh_factor * rayleigh_collected);

        out_color = vec4(final_color, 1.0);
    }

#endif
