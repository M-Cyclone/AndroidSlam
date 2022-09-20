#version 320 es
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texCoords;

out vec2 v_texCoords;

void main()
{
    gl_Position = vec4(a_position, 0.0f, 1.0f);
    v_texCoords = a_texCoords;
}