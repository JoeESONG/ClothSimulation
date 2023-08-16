#version 330 core
//Reference:https://learnopengl-cn.github.io/01%20Getting%20started/05%20Shaders/

layout (location = 0) in vec3 aPos;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}