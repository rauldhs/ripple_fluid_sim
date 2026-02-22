#version 460 core
in vec3 velocity;
in vec3 frag_normal;
in vec3 frag_pos;
uniform vec3 cam_pos;
out vec4 fragColor;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float speed = length(velocity);
    float hue = mix(0.66, 0.0, clamp(speed * 0.02, 0.0, 1.0));
    float saturation = mix(0.3, 1.0, clamp(speed * 0.005, 0.0, 1.0));
    float brightness = mix(0.6, 1.0, clamp(speed * 0.003, 0.0, 1.0));
    vec3 color = hsv2rgb(vec3(hue, saturation, brightness));

    vec3 view_dir = normalize(cam_pos - frag_pos);
    float rim = 1.0 - max(dot(frag_normal, view_dir), 0.0);
    rim = pow(rim, 3.0);
    vec3 rim_color = vec3(0.0, 0.0, 0.0);

    float edge = step(0.3, rim); 
    fragColor = vec4(mix(color, rim_color, edge), 1.0);
}
