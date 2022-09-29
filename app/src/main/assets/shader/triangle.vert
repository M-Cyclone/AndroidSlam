#version 320 es
layout (location = 0) in vec3 a_positon;
layout (location = 1) in vec3 a_color;

out vec4 v_color;

void main()
{
    gl_Position = vec4(a_positon, 1.0f);
    v_color = vec4(a_color, 1.0f);
}