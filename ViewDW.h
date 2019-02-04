///////////////////////////////////////////////////////////////////////////////
// ViewDW.h
// ============
// Direct Write headers, Direct D2D  headers.
// Class ViewDW: methods for DW text and D2D graphics.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_FORM_GL_H
#define VIEW_FORM_GL_H
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commdlg.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <string>
#include <vector>

#include "ViewGL.h"

namespace Win{

	struct BookPage{ 
		int mode;
		int glRoutine;
		std::wstring text;
		int itals;
		std::vector<DWRITE_TEXT_RANGE> italRange;
		int bolds;
		std::vector<DWRITE_TEXT_RANGE> boldRange;
		int ulines;
		std::vector<DWRITE_TEXT_RANGE> ulineRange;
		int names;
		std::vector<int> fontNumbers;
		std::vector<DWRITE_TEXT_RANGE> fontNameRange;
		std::vector<float> fontSizes;
		int sizes;
		std::vector<DWRITE_TEXT_RANGE> fontSizeRange;
	};

    class ViewDW{
    public:
        ViewDW(ViewGL* viewGL);
        ~ViewDW();

		HRESULT initDW();
		void create(HWND handle);
		int size(int w, int h);
		void drawDW();
		void setPageSize(int pageNumber);
		void setLeftRightLayout(int pageNumber);
		bool openDW_file(int ReloadFlag );
		void parseText();
		void countFormatBlocks();
		void setTextRanges( int pageIndex,  int wordIndex );
		void setTextLayout(int pageIndex, int side);
		void createLayout(int pageIndex, int side);
		void drawPageNumber(int pageNumber, int side);
		std::wstring ReadUTF16(const std::wstring &filename);
//		void WriteUTF16();
		int mouseMove(int x, int y);
		int lButtonDown(int x, int y, int item);
		void keyDown(int key, LPARAM lParam);
		void getChar(WPARAM findChar);
		HRESULT CreateDeviceResources();
		void DiscardDeviceResources();
		int readFontTable();
		void drawMenu( float windowWidth );
		void setMenuText(int parameter);
		void setMenuCell(int x, int y);

    private:

	ViewGL* viewGL;
	HWND  dwHandle;
	HWND  mainHandle;  // Used in SendMessage to change window mode

// file stuff
 	OPENFILENAMEW ofn;
	std::wstring activeFile;

	IDWriteFactory* pDWriteFactory_;
	IDWriteTextFormat* pTextFormat_;
	IDWriteTextLayout* pLeftLayout_;
	IDWriteTextLayout* pRightLayout_;
	IDWriteTextFormat* pMenuFormat_;
	IDWriteTextLayout* pMenuLayout_;

    ID2D1Factory* pD2DFactory_;
    ID2D1HwndRenderTarget* pRT_;
    ID2D1SolidColorBrush* pPaperBrush_;
    ID2D1SolidColorBrush* pBookTextBrush_;
    ID2D1SolidColorBrush* pMenuBrush_;

	int indexPage;
	int tableOfContents;
	float fpageWidth;
	float fpageHeight;
	std::vector<BookPage> Pages;

	int fbCount;
	std::vector<std::wstring> fontNames;
	std::wstring reload_Filename;

// menu FLAGS
	bool bDrawMenu;		// Display Menu
	bool bMainMenu;  
	bool bFileMenu;
	bool bLoadFile;
	bool bPageMenu;
	bool bColorMenu;

	int iMenuCell;
	float menuCellWidth;


	std::vector<std::wstring> menuText;
	D2D1_RECT_F pnumRightRect;
	D2D1_RECT_F pnumLeftRect;
	D2D1_RECT_F paperRect;
	D2D1_SIZE_F rtSize;

	D2D1_POINT_2F leftPageOrigin;
	D2D1_POINT_2F rightPageOrigin;

	int numberOfPages;	// Number of pages in the book
	int pageNumber;	// Page to display
    float dpiScaleX_;
    float dpiScaleY_;
	int dwWidth;

	std::wstring font;
	std::wstring pnum;  //page number
	std::wstring book;
	std::wstring fbChange;

    };//  end class ViewDW
}

#endif //#ifndef VIEW_FORM_GL_H
