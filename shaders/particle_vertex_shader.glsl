#version 460 core

layout (location=0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;
layout (location = 2) in vec3 aVelocity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 velocity;

out vec3 frag_normal;
out vec3 frag_pos;

void main(){
    vec3 scaledVertex = vec3(model * vec4(aPos, 1.0));
    vec3 worldPos = scaledVertex + aOffset;
    gl_Position = proj * view * vec4(worldPos, 1.0);


    velocity = aVelocity;

    frag_normal = normalize(scaledVertex); 
    frag_pos = worldPos;
}
