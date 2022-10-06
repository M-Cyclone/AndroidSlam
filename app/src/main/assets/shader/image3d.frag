#version 320 es
precision mediump float;

in vec2 v_texCoords;

out vec4 frag_color;

uniform sampler2D u_image;

void main()
{
    frag_color = texture(u_image, v_texCoords);
}
