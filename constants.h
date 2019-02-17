/*  constants.h  */

#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <d2d1.h>
#include <string>

 enum WinType{ 
	MAIN_WIN,
	GL_WIN,
	DW_WIN,
	FAKE_WIN 
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
	HELP,  //11
	START_PAGE,  //12
	LEFT_PAGE,  //13
	RIGHT_PAGE,  //14
	SET_QUESTION_MARK, //15
	SET_COLOR_BOX, // 16
	PALETTE = 99
};

	 const int dW_gL = 40001;
	 const int dW_dW = 40002;
	 const int gL_gL = 40003;

    // Main window title bar text
	const wchar_t appName[30] = L"3DBook reader";
	
	// Window dimension constants.
	const int widthDWwindow(800);
	const int widthGLwindow(1000);
	const int windowHeigth(920);

// Page constants
	const float pageMargin(50.0f);
	const float pageTop(20.0f);
	const float bookFontSize(20.0f);
	const float bookTabStop(30.0f);

// Menu constants
	const int menuCells(13);
	const float menuHeight(20.0f);
	const float menuFontSize(12.0f);

	// Used to scale font to window width
	const float menuFontScaleFactor(0.02f);


	// Brush colors
	const D2D1::ColorF paperColor(0x0);
	const D2D1::ColorF menuTextColor(0xa06020);
	const D2D1::ColorF bookTextColor(0xa06020);
	const D2D1::ColorF menuTextHighlightColor(0xF4B480);
}
#endif