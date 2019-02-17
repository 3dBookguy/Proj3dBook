#version 400

smooth in vec4 theColor;
out vec4 outputColor;

//We switch on this.
uniform int routineNumber;

void main()
{
	switch( routineNumber )
	{
		case 0:{outputColor = vec4(0.1f, 0.2f, 0.9f, 1.0f); break;}
		case 1:{outputColor = vec4(0.5f, 0.2f, 0.3f, 1.0f); break;}
		case 2:{outputColor = vec4(0.2f, 0.6f, 0.3f, 1.0f); break;}
		case 3:{outputColor = theColor; break;}
		case 4:{outputColor = theColor; break;}
		case 5:{outputColor = theColor; break;}
		case 12:{outputColor = theColor; break;}

		default:{outputColor = theColor; break;}

	}// End switch( routineNumber )
}



