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
#include <wincodec.h> // needed for IWICImagingFactory* 
#include <d2d1.h>
//#include <d2d1_3.h>
#include <dwrite.h>
//#include <dwrite_3.h>
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
		int links;
		std::vector<DWRITE_TEXT_RANGE> linkRange;
		int qlinks;
		std::vector<DWRITE_TEXT_RANGE> qlinkRange;
		int names;
		std::vector<int> fontNumbers;
		std::vector<DWRITE_TEXT_RANGE> fontNameRange;
		int sizes;
		std::vector<float> fontSizes;
		std::vector<DWRITE_TEXT_RANGE> fontSizeRange;
		int images;
		std::vector<std::wstring> imageNames;
		std::vector<D2D1_RECT_F> imageRect;
	};

    class ViewDW{
    public:
		ViewDW();
		ViewDW(ViewGL* viewGL);
//		ViewDW(){} // Need this for our callback interface to work
		~ViewDW();

		// This is our callback (from ViewGL) interface 
//		void Display(float* color);
		void Display(glm::vec4* color);
//		static void Wrapper_To_Call_Display(void* pt2Object, float* color);
		static void Wrapper_To_Call_Display(void* pt2Object, glm::vec4* color);

		HRESULT initDW();
		void create(HWND handle);
		void loadRecoverText();
		void setMenuText(int parameter);
		bool openDW_file(int ReloadFlag);
		void ReadUTF16(const std::wstring &filename);
		void parseText();
		int readFontTable();
		void countFormatBlocks();
		void reportFormatError(int page, int index, std::wstring temp);
		void setTextRanges(int pageIndex, int wordIndex);
		void drawDW();
		HRESULT CreateDeviceResources();
		void drawMenu(float windowWidth);
		int size(int w, int h);
		void setPageSize(int pageNumber);
		void setLeftRightLayout(int pageNumber);
		void setTextLayout(int pageIndex, int side);
		void createLayout(int pageIndex, int side);
		void drawPageNumber(int pageNumber, int side);
		void drawPageImages(int pageNumber, int side);
		void DiscardDeviceResources();
		int mouseMove(int x, int y);
		int lButtonDown(int x, int y, int item);
		void checkForLink(UINT x, UINT y); // look for links in the page.
		void handleHitTest(int x, int y, int page);
		void moveTheCursor();
		void keyDown(int key, LPARAM lParam);
		void setMenuCell(int x, int y);
		void getPageNumber(WPARAM findChar);
		int getNumber(WPARAM findChar);
		void getUserNumber(WPARAM findChar);

		HRESULT LoadBitmapFromFile(
			ID2D1RenderTarget *pRenderTarget,
			IWICImagingFactory *pIWICFactory,
			PCWSTR uri,
			UINT destinationWidth,
			UINT destinationHeight,
			ID2D1Bitmap **ppBitmap);

    private:

//  instanceID = 1 for viewGL,  
//  instanceID = 2 for ViewDW* mySelf used in callback impl.
	int instanceID;

//  So we can call viewGL's methods
	ViewGL* viewGL;

	HWND  dwHandle;
	HWND  mainHandle;  // Used in SendMessage to change window mode

//  DirectWrite
	IDWriteFactory* pDWriteFactory_;
	IDWriteTextFormat* pTextFormat_;
	IDWriteTextLayout* pLeftLayout_;
	IDWriteTextLayout* pRightLayout_;
	IDWriteTextFormat* pMenuFormat_;
	IDWriteTextLayout* pMenuLayout_;

//  D2D
    ID2D1Factory* pD2DFactory_;
    ID2D1HwndRenderTarget* pRT_;
	ID2D1SolidColorBrush* pRedBrush_;
	ID2D1SolidColorBrush* pHyperLinkBrush_;
	ID2D1SolidColorBrush* pQlinkBrush_;
    ID2D1SolidColorBrush* pPaperBrush_;
    ID2D1SolidColorBrush* pBookTextBrush_;
    ID2D1SolidColorBrush* pMenuBrush_;

// File loading
	OPENFILENAMEW ofn;
	std::wstring workDir;
	std::wstring activeFile;
	std::wstring reload_Filename;
	std::wstring book;
	std::wstring dummy;

// Image loading
	IWICImagingFactory* m_pWICFactory;  // for image files
	ID2D1Bitmap* m_pBitmap;
	std::wstring imageFilePathName;

// Image Scaling
	D2D1_SIZE_F maxClientSize;
	D2D1_SIZE_F imageScale;
	float imageScaleH;
	float imageScaleV;

// Font parsing
	std::vector<std::wstring> fontNames;
	std::wstring font;

// Font Scaling
	float dpiScaleX_;
	float dpiScaleY_;
	float resFactor;
	float maxClientArea;
	float pageArea;
	float fontSizeFactor;

// Book size factors
	std::vector<BookPage> Pages;
	int currentPage; // Page being displayed.
	int numberOfPages;	// Number of pages in the book.
	int pageNumber;	// Page to display.

// Page size factors
	int dwWidth;
	float fpageWidth;
	float fpageHeight;

	D2D1_SIZE_F rtSize;
	D2D1_RECT_F paperRect;
	D2D1_RECT_F menuRect;
	D2D1_POINT_2F leftPageOrigin;
	D2D1_POINT_2F rightPageOrigin;

// Page numbering
	std::wstring pnum;  //page number
	D2D1_RECT_F pnumRightRect;
	D2D1_RECT_F pnumLeftRect;

// Format block stuff
	int fbCount;
	std::wstring fbChange;

// Menu FLAGS
	bool bDrawPage;
	bool bDrawMenu;
//	bool bCallGL;
	static bool bMainMenu;  
	static bool bFileMenu;
	static bool bPageMenu;
	bool bNumberMenu;
		int userInt;  // These are what the
		float userFloat; // User entered in the number menu.
	static bool bColorMenu;

// Hyperlink and Qlink flags
	int linkIndex;
	// int qlinkIndex;
	std::wstring link;
	bool bLeftButtonDown;
	bool bHitflag;

// Menu variables
	int iMenuCell;
	float menuCellWidth;
	float menuFontSize;
	int indexPage;
	int tableOfContents;
	static std::vector<std::wstring> menuText;

// Color Menu

	static glm::vec4 colorFromGL;

    };//  end class ViewDW
}// End namespace Win

#endif //#ifndef VIEW_FORM_GL_H
