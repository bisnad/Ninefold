#version 330

in float v_alpha;
uniform vec4 Color;
out vec4 outputColor;

void main()
{
    outputColor = Color;
    outputColor.a *= v_alpha;
}