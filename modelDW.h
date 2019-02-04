/*					modelDW.h



*/

#ifndef MODEL_DW_H
#define MODEL_DW_H
	//#ifndef UNICODE
	//#define UNICODE
	//#endif


#include <windows.h>
#include <commdlg.h> // needed for OPENFILENAMEW ofn;
#include <string>
#include <vector>

namespace Win
{

	struct txRange{
		int size;
		int txStart;
		int txLength;
	};

	struct FontNameRanges{ 
		std::wstring name;
		txRange range;
	};

	struct FontSizes{ 
		float points;
		txRange range;
	};

	struct PageInfo{ 
		int mode;
		std::wstring pageText;
		std::vector<txRange> italics;
		std::vector<FontNameRanges> font;
		std::vector<FontSizes> fontSize;
	};



//	struct DWRITE_TEXT_RANGE
//{
//    /// <summary>
//    /// The start text position of the range.
//    /// </summary>
//    UINT32 startPosition;
//
//    /// <summary>
//    /// The number of text positions in the range.
//    /// </summary>
//    UINT32 length;
//};
//	DWRITE_TEXT_RANGE italics;


    class ModelDW{
    public:
		ModelDW();
        ~ModelDW(){};
		bool openDW_file( HWND handle, int reloadFlag );		
		int countPages();
		int check_TDR_header();
		int readFontTable();
		int parse_TDR_file();
		int paginate();
    private:
		int pageCount;
		std::wstring reload_Filename;
		std::wstring wsText;
		std::vector<std::wstring> pages;
		std::vector<std::wstring> fontNames;
		OPENFILENAMEW ofn;
		HWND  DWhandle;
		HWND  mainHandle; 

	};
}
#endif

