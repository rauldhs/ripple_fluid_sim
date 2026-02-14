#version 460 core

out vec4 color;
in vec3 Pos;

void main(){
    vec3 rainbow = 0.5 + 0.5 * cos(Pos * 2.0 + vec3(0.0, 2.0, 4.0));
    color = vec4(rainbow, 1.0);
}
