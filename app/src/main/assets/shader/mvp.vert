#version 320 es
layout (location = 0) in vec3 a_position;

uniform float u_point_size;
uniform vec3 u_color;
uniform mat4 u_mat_mvp;

out vec4 v_color;

void main()
{
    gl_PointSize = u_point_size;
    gl_Position = u_mat_mvp * vec4(a_position, 1.0f);
    v_color = vec4(u_color, 1.0f);
}