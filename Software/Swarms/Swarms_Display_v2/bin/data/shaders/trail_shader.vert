#version 330

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
layout(location = 0) in vec4 vertexPosition_ModelSpace;
layout (location = 1) in float alpha;
out float v_alpha;

uniform vec4 Color;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

void main()
{
    v_alpha = alpha;
	gl_Position =  ProjectionMatrix * ViewMatrix * ModelMatrix * vertexPosition_ModelSpace;
}