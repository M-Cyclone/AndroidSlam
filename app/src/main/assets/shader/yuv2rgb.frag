#version 320 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float;

in vec2 v_texCoords;

out vec4 frag_color;

uniform samplerExternalOES sensor_image;

void main()
{
    vec2 uv = vec2(v_texCoords.x, 1.0f - v_texCoords.y);
    frag_color = texture(sensor_image, uv);
}