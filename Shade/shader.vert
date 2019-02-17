#version 400
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

smooth out vec4 theColor;

// Uniforms are shared between all shaders and must be declared as global variables.

//We switch on this.
uniform int routineNumber;
uniform mat4 projectionMatrix;
uniform mat4 PVM;
uniform mat4 rotate;
// Moving triangle vert offsets.
uniform vec2 offset;
uniform vec3 offset2;

void main()
{
	switch( routineNumber )
	{
		case 0:
		{
			gl_Position = position;
			break;
		}
		case 1:
		{	
			gl_Position = position;
			break;
		}
		case 2:
		{
			vec4 totalOffset = vec4(offset.x, offset.y, 0.0, 0.0);
			gl_Position = position + totalOffset;
			break;
		}
		case 3:
		{
			vec4 totalOffset = vec4(offset.x, offset.y, 0.0, 0.0);
			gl_Position = position + totalOffset;
			theColor = color;
			break;
		}
		case 4:
		{
			vec4 cameraPos = position + vec4(offset.x, offset.y, 0.0, 0.0);
			gl_Position = projectionMatrix * cameraPos;
			theColor = color;
			break;
		}
		case 5:
		{
			vec4 cameraPos = position + vec4(offset2.x, offset2.y, offset2.z, 0.0);
			gl_Position = projectionMatrix * cameraPos;
			theColor = color;
			break;
		}
		case 10:
		{	
			gl_Position = PVM*position;
			theColor = color;
			break;
		} 
		case 11:
		{	
			vec4 cameraPos = position + vec4(offset.x, offset.y, 0.0, 0.0);
			gl_Position = projectionMatrix * cameraPos;
			theColor = color;
			break;
		} 
		case 12:
		{	
			gl_Position = rotate*position;
			theColor = color;
			break;
		}
		case 14:
		{	
			gl_Position = position;
		    theColor = color;
			break;
		}

		default:
		{
			gl_Position = position;
			theColor = color;
			break;
		}

	}// End switch( routineNumber )
}

