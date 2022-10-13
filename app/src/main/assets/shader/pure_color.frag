#version 320 es
precision mediump float;

in vec2 v_texCoords;

out vec4 frag_color;

void main()
{
    frag_color = vec4(0.2f, 0.3f, 0.3f, 1.0f);
}
