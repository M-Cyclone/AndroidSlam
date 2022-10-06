#version 320 es
precision mediump float;

in vec2 v_texCoords;

out vec4 frag_color;

uniform sampler2D screen_shot;

void main()
{
    vec2 uv = vec2(v_texCoords.x, v_texCoords.y);
    vec4 color = texture(screen_shot, uv);

    frag_color = color;
}
