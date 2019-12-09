#version 430
#include "cnovr.glsl"

out vec4 colorOut;

layout(location = 8) uniform sampler2D textures[];

in vec2 texcoords;

void main()
{
	colorOut = vec4( texture( textures[0], texcoords.xy ).rgb, 1. );
}