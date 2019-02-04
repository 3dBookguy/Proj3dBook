/*					modelDW.cpp

*/

#define DEBUG_GB

#include <iostream>
#include <sstream>
#include <streambuf>
#include <locale>
#include "Log.h"
#include "modelDW.h"
#include "constants.h"
using namespace Win;

ModelDW::ModelDW():pageCount(0){
//	ZeroMemory( &Pages , sizeof( Pages ));
	ZeroMemory( &ofn , sizeof( ofn ));
	ofn.lStructSize = sizeof ( ofn );
}

bool ModelDW::openDW_file( HWND handle, int reloadFlag ){  
#ifdef DEBUG_GB		
	log(L"ModelDW::openDW_file(HWND handle, int reloadFlag )");
#endif

	if( reloadFlag == 0 ){  //reloadFlag == 0 means its not a reload need the Open File dlg.  
		TCHAR szFile[140];
		//ZeroMemory( &ofn , sizeof( ofn));
		//ofn.lStructSize = sizeof ( ofn );
		//ofn.hwndOwner = NULL;
		ofn.hwndOwner = handle; 
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof( szFile );
//		ofn.lpstrFilter = TEXT("All\0*.*\03dBook\0*.tdr\0");
//		ofn.lpstrFilter = TEXT("All files\0\*.*\0\0");
		ofn.lpstrFilter = TEXT("All\0*.*\0Text\0*.TXT\0");
//		ofn.lpstrFilter = TEXT("Text\0*.TXT\0\All\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0 ;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

		// Open a file dialog box or bail of it fails.
		if (GetOpenFileName(&ofn)  == FALSE)
			{
				log(L"GetOpenFileName(&ofn)  == FALSE");
				return FALSE;
		}
		// Save file name
		reload_Filename.clear();
	    reload_Filename = ofn.lpstrFile;
	}  

	log(L"std::ifstream file(ofn.lpstrFile, std::ios::binary); )");
	std::ifstream file(ofn.lpstrFile, std::ios::binary);
//	std::ifstream file(filename.c_str(), std::ios::binary);
	if(!file){
		MessageBox(NULL, TEXT("Could not open"), ofn.lpstrFile, MB_ICONINFORMATION);
		return FALSE;
	}
	log(L"ModelDW::openDW_file seems like we did open a file");
    std::stringstream ss;
    ss << file.rdbuf() << '\0';
//	 log(wstring((wchar_t *)ss.str().c_str()));
	int iUniTest(IS_TEXT_UNICODE_SIGNATURE); // 0xFEFF at beginning of file.
	int cTextLength;
	cTextLength = (UINT32)strlen(ss.str().c_str());
	if(IsTextUnicode(ss.str().c_str(), cTextLength, &iUniTest)){
		MessageBox(NULL, TEXT("Have Unicode"), reload_Filename.c_str(),	MB_ICONINFORMATION);

		SetWindowText( handle, reload_Filename.c_str());
		wsText = ((wchar_t *)ss.str().c_str());
		check_TDR_header();
		readFontTable();
		countPages();
		paginate();
		return TRUE;
	}
	else{
		MessageBox(NULL, TEXT("Not UNICODE"), reload_Filename.c_str(), MB_ICONINFORMATION);
		return FALSE;
	}
// Closing of streams:  Done inplicitly by the streams dtor.  So an explicit call to
//	close() /is needed only if the file must be closed before reaching the end of the
//	scope  in  which it was declared. 

}

int ModelDW::readFontTable(){
#ifdef DEBUG_GB
	log(L"void ViewDW::readFontTable()");
#endif
// First find the end of the fonttbl
// First append header into wstring temp and read that instead of whole file

	std::wstring temp;
	int endIndex = 0;
	int startIndex = 0;
	startIndex = wsText.find( L"\\fonttbl", startIndex);
	endIndex = wsText.find( L";}}", endIndex);
	temp.append(wsText, startIndex, endIndex - startIndex);
	//	log(temp);

	// Find number of fonts fontCount
	endIndex = 0;
	int fontCount = 0;		// position of last $ token
	while( endIndex  > -1 )
	{
		endIndex = temp.find( L"charset", endIndex);
		if( endIndex < 0 ) break;
		fontCount++;	
		endIndex++;
	}
	log(L"fontCount = %i", fontCount);

	// Read font names into vector<wstring> fontNames;
	fontNames.clear();
	fontNames.resize(fontCount);
	startIndex = 0;
	endIndex = 0;
	for( int i = 0; i < fontCount; i++ ){
		startIndex = temp.find(L"fcharset", startIndex) + 10;
		endIndex = startIndex;
		endIndex = temp.find(L";}", endIndex);
		fontNames[i].clear();
		fontNames[i].append( temp, startIndex, endIndex -  startIndex);
		log(fontNames[i]);
		startIndex++;
		endIndex++;
	}
	return 0;
}

int ModelDW::parse_TDR_file(){
#ifdef DEBUG_GB
	log(L"int ModelDW::parse_RTF_file()");
#endif
	int x = 3;
	return x;
}

int ModelDW::countPages(){
#ifdef DEBUG_GB
	log(L"ModelDW::countPages()");
#endif

	int index(0);
	while( index  > -1 )
	{
		index = wsText.find( L"\\highlight1 mode1", index);
		if( index < 0 ) break;
		pageCount++;
		index++;
	}

	if( pageCount < 1 ){
		MessageBox(NULL, TEXT("There are no formatted pages in"),				reload_Filename.c_str(), MB_ICONINFORMATION);
		return FALSE;
	}
	log(L"pageCount = %i", pageCount);
	return pageCount;
}

int ModelDW::paginate(){
#ifdef DEBUG_GB
	log(L"int ModelDW::paginate()");
#endif
	//struct PageInfo{ 
	//	int mode;
	//	std::wstring pageText;
	//	std::vector<txRange> italics;
	//	std::vector<FontNameRanges> font;
	//	std::vector<FontSizes> fontSize;
	//};

		//	std::vector<txRange> italics;
		//std::vector<FontNameRanges> font;
		//std::vector<FontSizes> fontSize;


	std::vector<PageInfo> Pages(pageCount);
	Pages.clear();
	Pages.resize(pageCount);
//	pageImages[arrayIndex].name.append( fbChange, 2 , fbChange.size() - 14);
	//Pages[0].font.clear();
	//Pages[0].font.resize(100);
	//Pages[0].fontSize.resize(100);
	//Pages[0].mode = GL_WIN;
	//Pages[0].fontSize[3].points = 1.2f;
	//Pages[0].pageText.clear();
	//Pages[0].pageText.resize(1);
//	Pages[0].pageText.resize(100);
	Pages[0].pageText.append(wsText, 5, 15);
//	Pages[0].pageText = L"Hello World";
 	log(Pages[0].pageText);
	return TRUE;
 
//	Page Book(0);
//	pages.clear();
//	pages.resize( pageCount );
//	pageMode.clear();
//	pageMode.resize( pageCount );
//	formattedWords.resize( pageCount );
//
//// Paginate, ie;  read book into pages[pageCount] 
//// Also count the number of \mode2 i.e.; GL_MODE pages into glModeCount
//
//	int start = 0;
//	int glModeCount  = 0;
//	index = 0;
//	index = book.find( L"12345", index);
//	for( int i = 0; i < pageCount; i++ ){
//		start = index + 5 ;
////		index = book.find( L"\\mode1", index);
//		index = book.find( L"\\mode", index);
//		if( book[ index + 5 ] == L'1' ) pageMode[i] = DW_MODE;
//		else if( book[ index + 5 ] == L'2' ){
//			pageMode[i] = GL_MODE;
//			glModeCount++;  
//			}
////		log(L"pageMode[%i] =   %i", i, pageMode[i]);
//		pages[i].clear();
//		pages[i].append( book, start, index - start);
//		index++;
//	}

}

// Check if we have .tdr file format:  This is a .rtf file saved as a
// UNICODE text file. The first sic chars of the file should be "{\rtf1".
// fonttbl - if we don't have .tdr format issue error message.  
int ModelDW::check_TDR_header(){
#ifdef DEBUG_GB
	log(L"int ModelDW::check_TDR_header()");
#endif

	int index = 0;
	index  = wsText.find(L"{\\rtf1", index );
	if( index != std::wstring::npos){ 
		MessageBox(NULL, TEXT("Have valid file format"),
			TEXT("Have valid .tdr file format"), MB_OK);
		return TRUE;
	}
	else{
		MessageBox(NULL, TEXT("Invalid file format"),
			TEXT("Invalid file format"), MB_OK);
		return FALSE;
	}
}