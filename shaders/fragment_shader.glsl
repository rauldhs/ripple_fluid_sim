#version 460 core
in vec3 velocity;
out vec4 fragColor;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float speed = length(velocity);
    float hue = mix(0.66, 0.0, clamp(speed * 2.0, 0.0, 1.0));
    float saturation = mix(0.3, 1.0, clamp(speed * 0.5, 0.0, 1.0));
    float brightness = mix(0.6, 1.0, clamp(speed * 0.3, 0.0, 1.0));

    vec3 color = hsv2rgb(vec3(hue, saturation, brightness));

    fragColor = vec4(color, 1.0);
}
