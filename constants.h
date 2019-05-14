/*  constants.h  */

#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <d2d1.h>
#include <string>

 enum WinType{ 
	MAIN_WIN,
	GL_WIN,
	DW_WIN,
};

namespace constants
{

	enum MenuItem{ 
	FILE = 0,	//0
	RELOAD = 0, //0
	PAGE = 1,	//1
	LOAD = 1,	//1
	PRIOR = 2,	//2
	CANCEL = 2,	//2
	NEXT,		//3
	COVER,	//4
	TOC,	//5
	INDEX,	//6
	END,	//7
	COLOR,	//8
	TBD_1,	//9
	TBD_2,	//10
	HELP = 10,  //11
	START_PAGE,  //12
	NUMBER = 11,
	LEFT_PAGE,  //13
	RIGHT_PAGE,  //14
	SET_QUESTION_MARK, //15
	SET_COLOR_BOX, // 16
	ENTER_NUMBER, //17
	SET_POUND_SIGN,
	HAVE_USER_INT,
	HAVE_USER_FLOAT,
	HAVE_USER_ENTER,
	PALETTE = 99
};

// Window modes
	 const int dW_gL = 40001;
	 const int dW_dW = 40002;
	 const int gL_gL = 40003;

// Page constants
	const float pageMargin(50.0f);
	const float pageTop(20.0f);
	const float bookFontSize(20.0f);
	const float bookTabStop(30.0f);

// Menu constants
	const int menuCells(13);
	const float menuHeight(30.0f);
	const float menuFontSize(12.0f);

// Used to scale font to window width
	const float menuFontScaleFactor(0.02f);

// Brush colors
	const D2D1::ColorF paperColor(0x0);
	const D2D1::ColorF menuTextColor(0xa06020);
	const D2D1::ColorF bookTextColor(0xa06020);
	const D2D1::ColorF menuTextHighlightColor(0xF4B480);
//	const D2D1::ColorF hyperLinkBlue(0x0645AD);
	const D2D1::ColorF hyperLinkBlue(0xAAAAEE);
	const D2D1::ColorF qLinkColor(0xEE3344);

}
#endif