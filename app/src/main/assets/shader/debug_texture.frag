#version 320 es
precision mediump float;

in vec2 v_texCoords;

out vec4 frag_color;

uniform sampler2D screen_shot;

void main()
{
    vec2 uv = vec2(v_texCoords.x, 1.0f - v_texCoords.y);
    vec4 color = texture(screen_shot, uv);

    if(gl_FragCoord.x < 320.0f)
    {
        frag_color = color;
    }
    else
    {
        frag_color = vec4(color.x, color.y, 1.0f, 1.0f);
    }
}
