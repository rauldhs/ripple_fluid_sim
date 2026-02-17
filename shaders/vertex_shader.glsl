#version 460 core

layout (location=0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 Pos;

void main(){
  gl_Position = proj*view*model*vec4(aPos+aOffset,1);
  Pos = aPos;
}
