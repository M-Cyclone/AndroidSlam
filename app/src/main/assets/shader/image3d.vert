#version 320 es
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texCoords;

uniform mat4 u_mat_mvp;

out vec2 v_texCoords;

void main()
{
    gl_Position = u_mat_mvp * vec4(a_position, 0.0f, 1.0f);
    v_texCoords = a_texCoords;
}