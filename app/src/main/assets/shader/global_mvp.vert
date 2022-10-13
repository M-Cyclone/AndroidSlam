#version 320 es
layout (location = 0) in vec3 a_position;

uniform vec3 u_center;
uniform float u_scale;
uniform vec3 u_color;

out vec4 v_color;

void main()
{
    vec3 pos = vec3(a_position - u_center) * u_scale * 0.75f;
    gl_Position = vec4(pos.x, pos.z, 0.0f, 1.0f);
    v_color = vec4(u_color, 1.0f);
}