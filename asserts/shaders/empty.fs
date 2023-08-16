#version 330 core
//Reference:https://learnopengl-cn.github.io/01%20Getting%20started/05%20Shaders/
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color,1.0f);
}
