///////////////////////////////////////////////////////////////////////////////
// ViewDW.cpp
// ==============
// View component of OpenGL dialog window
//
///////////////////////////////////////////////////////////////////////////////

#include "Log.h"
#include "ViewDW.h"
#include <iostream>
#include <sstream>
#include <streambuf>
#include "constants.h"

#define DEBUG_GB // Activate the log file

// SafeRelease inline function .		
template <class T> inline void SafeRelease(T **ppT)
{
    if (*ppT) {  (*ppT)->Release();  *ppT = NULL; }
}

using namespace std;// used for string functions
using namespace Win;
 
ViewDW::ViewDW(ViewGL* viewGL):

// Pointer to ViewGL methods.
	viewGL(viewGL),

// Page State
	indexPage(0),
	tableOfContents(0),
	pageNumber(0),
	numberOfPages(0),

// Menu Logic
	bDrawMenu(TRUE),
	bMainMenu(TRUE),
	bFileMenu(FALSE),
	bPageMenu(FALSE),
	bColorMenu(FALSE),


// D2D and DirectWrite
    pD2DFactory_(NULL),
	pDWriteFactory_(NULL),
    pRT_(NULL),

// rushes
    pBookTextBrush_(NULL),
	pPaperBrush_(NULL),
	pMenuBrush_(NULL),

// DirectWrite
    pTextFormat_(NULL),
	pMenuLayout_(NULL),
	pLeftLayout_(NULL),
	pRightLayout_(NULL)

{
	ZeroMemory( &paperRect, sizeof(paperRect));
	ZeroMemory( &rtSize, sizeof(rtSize));
	ZeroMemory( &leftPageOrigin, sizeof(leftPageOrigin));
	ZeroMemory( &rightPageOrigin, sizeof(rightPageOrigin));
	ZeroMemory( &pnumLeftRect, sizeof(pnumLeftRect));
	ZeroMemory( &pnumRightRect, sizeof(pnumRightRect));
	pnumLeftRect.top = 0; pnumLeftRect.bottom = 20.0f;
	pnumLeftRect.left = 0; 
	pnumRightRect.top = 0; pnumRightRect.bottom = 20.0f;
	leftPageOrigin.x = constants::pageMargin;
	leftPageOrigin.y = constants::pageTop;
	rightPageOrigin.y = constants::pageTop;
	menuText.resize(constants::menuCells);
//	setMenuText(0);
}

///////////////////////////////////////////////////////////////////////////////
// default dtor
///////////////////////////////////////////////////////////////////////////////
ViewDW::~ViewDW()
{  
	SafeRelease(&pTextFormat_);
	SafeRelease(&pMenuFormat_);
    SafeRelease(&pD2DFactory_);
    SafeRelease(&pDWriteFactory_);
	DiscardDeviceResources();
}

void ViewDW::DiscardDeviceResources(){
#ifdef DEBUG_GB
	log(L"ViewDW::DiscardDeviceResources()");
#endif

    SafeRelease(&pBookTextBrush_);
    SafeRelease(&pPaperBrush_);
	SafeRelease(&pMenuBrush_);
	SafeRelease(&pLeftLayout_);
	SafeRelease(&pRightLayout_);
	SafeRelease(&pMenuLayout_);
    SafeRelease(&pRT_);
//		if (pRT_)
//		pRT_->Release();
}

// ControllerDW will call this function when it recieves
// WM_CREATE.  ViewDW stores the handel to its' window
// in dwHandle. 
void ViewDW::create(HWND hwnd)
{
#ifdef DEBUG_GB
	log(L" ViewDW::create(HWND hwnd)");
#endif

	dwHandle = hwnd;
	mainHandle = GetParent( dwHandle );
	initDW();
	openDW_file(constants::START_PAGE);
}

HRESULT ViewDW::initDW(){		
#ifdef DEBUG_GB
	log(L"ViewDW::initDW()");
#endif
	UINT DpI(0);
	HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory_ );

	if (SUCCEEDED(hr)){
		pD2DFactory_->GetDesktopDpi(&dpiScaleX_, &dpiScaleY_);	
		DpI = GetDpiForWindow(mainHandle);
		log(L"ViewDW::initDW() GetDpiForWindow(dwHandle) = %u; ", DpI);
		log(L"ViewDW::initDW() dpiScaleX_= %f", dpiScaleX_);
		dpiScaleX_ = 96/dpiScaleX_;
		dpiScaleY_ = 96/dpiScaleY_;
	}

	if (SUCCEEDED(hr)){
		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_) );
	}
	
	if (SUCCEEDED(hr)){ 
		hr = pDWriteFactory_->CreateTextFormat( 
			L"Arial",
			NULL,  
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			constants::bookFontSize,
			L"en-us",
			&pTextFormat_);
	
		pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
		pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		pTextFormat_->SetIncrementalTabStop(constants::bookTabStop);
	}

	if (SUCCEEDED(hr)){ 
		hr = pDWriteFactory_->CreateTextFormat( 
			L"Arial",
			NULL,  
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_SEMI_CONDENSED,
			constants::menuFontSize,
			L"en-us",
			&pMenuFormat_);

		pMenuFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
   }
	return hr;
}

// called on WM_CLOSE
//void ViewDW::WriteUTF16(){  // save the page number to file DWOGL22{z}  22 = pg num
//#ifdef DEBUG_GB
//	log(L"void ViewDW::WriteUTF16()");
//#endif
//		ofstream file2(activeFile.c_str(), std::ios::out | std::ios::binary);    // 
//		if(!file2){
//	//		MessageBox(NULL, TEXT("Could not open for out put"), activeFile.c_str(), MB_OK);
//		 }
//		else {
//			MessageBox(NULL, TEXT("Write"), activeFile.c_str(), MB_OK);
//			int index = 0;
//			index = book.find(L"DWOGL", index ); 	// token for direct write openGL file
//			fbCount  = book.find(L"{z}", fbCount );
//			if( index != wstring::npos && fbCount != wstring::npos){ 
//				book.replace(index + 5 , fbCount - index - 5, to_wstring( pageNumber -1 ));
//			}
//			file2.write((char *) book.c_str(), book.length()*sizeof(wchar_t));
//		}
//}
wstring ViewDW::ReadUTF16(const wstring &filename){
#ifdef DEBUG_GB
	log(L"wstring ViewDW::ReadUTF16(const wstring & filename)");
#endif
	activeFile.clear();
	activeFile.append(filename);
	int iUniTest = IS_TEXT_UNICODE_SIGNATURE; // 0xFEFF at beginning of file.
	int cTextLength_;											//  byte order mask
	wstring error = L"Can not open file";
    ifstream file(filename.c_str(), std::ios::binary);    // std::ios::binary for Windows
	if(!file){
		MessageBox(NULL, TEXT("Could not open"), filename.c_str(), MB_ICONINFORMATION);
		error = L"Could not open this file..";
		log(error);
		bLoadFile = FALSE;
		return error; }
    std::stringstream ss;
    ss << file.rdbuf() << '\0';
//	 log(wstring((wchar_t *)ss.str().c_str()));
	cTextLength_ = (UINT32)strlen(ss.str().c_str());
	if(IsTextUnicode(ss.str().c_str(), cTextLength_, &iUniTest)){
		bLoadFile = TRUE;
		return wstring((wchar_t *)ss.str().c_str());}
	else{
		MessageBox(NULL, TEXT("Not UNICODE"), filename.c_str(), MB_ICONINFORMATION);
		error = L"This file is not unicode.";
		log(error);
		bLoadFile = FALSE;
		return error;}

// Closing of streams:  Done inplicitly by the streams dtor.  So an explicit call to
//	close() /is needed only if the file must be closed before reaching the end of the
//	scope  in  which it was declared. 
} 


bool ViewDW::openDW_file( int reloadFlag ){  
#ifdef DEBUG_GB	
	log(L"ViewDW::openDW_file( int reloadFlag )");
#endif
	if( reloadFlag == constants::CANCEL ) return FALSE;
	if (reloadFlag == constants::HELP)
	{
		pageNumber = 0;
		reload_Filename = L"..\\src\\manual.tdr";
		book.clear();
		book = ReadUTF16(reload_Filename.c_str());
	}
	if( reloadFlag == constants::START_PAGE )
	{	
		pageNumber = 0;
		reload_Filename = L"..\\src\\start.tdr";
		book.clear();
		book = ReadUTF16( reload_Filename.c_str() );  
	}
	// Loading a new file -- Put up the Open File Dialog
	else if( reloadFlag == constants::LOAD ){ 

		// Dont try to draw a pageNumber > numberOfPages;
		pageNumber = 0;

		TCHAR szFile[256];
		ZeroMemory( &ofn , sizeof( ofn));
		ofn.lStructSize = sizeof ( ofn );
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof( szFile );
//		ofn.lpstrFilter = TEXT("All\0*.*\03dBook File *.tdr\0*.tdr\0");
		ofn.lpstrFilter = TEXT("3dBook Files       *.tdr\0*.tdr\0All\0*.*\0");
//		ofn.lpstrFilter = TEXT("Text\0*.TXT\0\All\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0 ;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		
		GetOpenFileName( &ofn );
		// Save the file name for RELOAD call
		reload_Filename.clear();
	    reload_Filename = ofn.lpstrFile;

		book.clear();
		book = ReadUTF16( reload_Filename.c_str() );  
	}
	else if( reloadFlag == constants::RELOAD ){ 
		book.clear();
		book = ReadUTF16( reload_Filename.c_str() );  
	}

// Do some file format checks.
	if( book.find( L"tdr1") == wstring::npos ){
		MessageBox( NULL, reload_Filename.c_str(), 
		TEXT("File is not a 3dBook reader file!"), MB_OK );
		return FALSE;
	}

	if (book.find(L"0123456789xx") == wstring::npos) {
		MessageBox(NULL, reload_Filename.c_str(),
			TEXT("This version .tdr file  is no longer supported!"), MB_OK);
		return FALSE;
	}
	else { 
		SetWindowText( mainHandle, reload_Filename.c_str()); 
		parseText();
		return TRUE; 
	} 
}

// The text is now in std::wstring book and we start parsing.
// We need the count of formats to resize our Pages struct
// std::vectors. Just counting the format blocks here.
// If bold, ital, undeline type blocks do not have closing blocks
// they are not counted.
void ViewDW::countFormatBlocks(){
#ifdef DEBUG_GB
	log(L"ViewDW::countFormatBlocks()");
#endif

	std::wstring temp;

	int index(0);
	for( int i = 0; i < numberOfPages; i++ ){

// ---- Italics ----
		index = 0;
		Pages[i].itals = 0;
		while( index  > -1 )
		{
			index = Pages[i].text.find(L"\\i", index);
			if( index < 0 ) break;
			if (Pages[i].text[index + 2] == L'0')
			{
			//	log(L"ital format at = %i page = %i", index, i);					
				++Pages[i].itals;
			}
			++index;
		}
//		log(L"Pages[%i].itals; = %i", i, Pages[i].itals);
		Pages[i].italRange.clear();
		Pages[i].italRange.resize(Pages[i].itals);

// ---- Bolds ----
		index = 0;
		Pages[i].bolds = 0;
		while( index  > -1 )
		{
			index = Pages[i].text.find(L"\\b", index);
			if( index < 0 ) break;
			if(Pages[i].text[ index + 2 ] == L'0' )
			{
			//	log(L"bold format at = %i page = %i", index, i);					
				++Pages[i].bolds;
			}
			++index;
		}
//		log(temp);
//		log(L"Pages[%i].bolds; = %i", i, Pages[i].bolds);
		Pages[i].boldRange.clear();
		Pages[i].boldRange.resize(Pages[i].bolds);
		temp.clear();
// ---- Underlines ----
		index = 0;
		Pages[i].ulines = 0;
		while( index  > -1 )
		{
			index = Pages[i].text.find(L"\\u", index);
			if( index < 0 ) break;
			if (Pages[i].text[index + 2] == L'0')
			{
			//	log(L"uline format at = %i page = %i", index, i);					
				++Pages[i].ulines;
			}
			++index;
		}

//		log(L"Pages[%i].ulines; = %i", i, Pages[i].ulines);
		Pages[i].ulineRange.clear();
		Pages[i].ulineRange.resize(Pages[i].ulines);

// ---- Font Sizes ----
		index = 0;
		Pages[i].sizes = 0;
		while( index  > -1 )
		{
			index = Pages[i].text.find(L"\\$", index);
			if( index < 0 ) break;
			++Pages[i].sizes;
			++index;
		}
//		log(L"Pages[%i].fontSizes; = %i", i, Pages[i].sizes);
		Pages[i].fontSizes.clear();
		Pages[i].fontSizes.resize(Pages[i].sizes);
		Pages[i].fontSizeRange.clear();
		Pages[i].fontSizeRange.resize(Pages[i].sizes);

// ---- Font Names ----
		index = 0;
		Pages[i].names = 0;
		while( index  > -1 )
		{
			index = Pages[i].text.find(L"\\f", index);
			if( index < 0 ) break;
			++Pages[i].names;			
			++index;
		}
//		log(L"Pages[%i].fontNames; = %i", i, Pages[i].names);
		Pages[i].fontNumbers.clear();
		Pages[i].fontNumbers.resize(Pages[i].names);
		Pages[i].fontNameRange.clear();
		Pages[i].fontNameRange.resize(Pages[i].names);
	}
	return;
}// End countFormatBlocks

// The text is now in std::wstring book and we start parsing.
void ViewDW::parseText(){
#ifdef DEBUG_GB
	log(L"ViewDW::parseText()");
#endif

//	get the number of fonts and read them into wstring array fontNames[number of fonts]
	readFontTable();
	font.clear();
	font = fontNames[0];
//	log(fontNames[fontNumber]);

// Find the number of pages; numberOfPages
	int index = 0;
	numberOfPages = 0;
	while( index  > -1 )
	{
		index = book.find( L"\\Mode_", index);
		if( index < 0 ) break;
		numberOfPages++;
		index++;
	}

	if( numberOfPages == 0 ){
		MessageBox( NULL, reload_Filename.c_str(), 
		TEXT("There are no formatted pages in this file"), MB_OK );
		return;
	}

	// We have pages so size our std::vector<bookPage> Pages;
	log(L"Pages.resize(%i);", numberOfPages);
	Pages.clear();
	Pages.resize(numberOfPages);

	std::wstring temp;


// Paginate the book.  Get all the text out of book
// and into Pages struct.
//
// Notice! It is no coincidence that the number of
// chars in 0123456789xx and Mode_dW_gL01 both = 12,
// and start = index + 12;
	index = 0;
	int start(0);
	index = book.find( L"0123456789xx", index);
	for( int i = 0; i < numberOfPages; i++ ){
//		start = index + 10;
		start = index + 12;
		index = book.find( L"\\Mode_", index);
		temp.clear();
		temp.append(book, index + 6,  5 );
		if (temp == L"dW_gL")
		{
			temp.clear();
			temp.append(book, index + 11, 2);
		//	log(temp);
			Pages[i].mode = constants::dW_gL;
			Pages[i].glRoutine = stoi(temp);
		//	log(L"Pages[%i].glRoutine = %i",i, Pages[i].glRoutine );
		}
		else if (temp == L"dW_dW")
		{
			Pages[i].mode = constants::dW_dW;
			Pages[i].glRoutine = -1;
		}
		else if (temp == L"gL_gL")
		{
			Pages[i].mode = constants::gL_gL;
			temp.clear();
			temp.append(book, index + 11, 2);
			Pages[i].glRoutine = stoi(temp);
		}

		Pages[i].text.clear();
		Pages[i].text.append( book, start, index - start);
		index++;
	}

	log(L"numberOfPages = %i", numberOfPages);

	countFormatBlocks();

// Here we strip off the format blocks and send them to
// setTextRanges
	int fbStart(0);
	int fbEnd(0);
	for( int i = 0; i < numberOfPages; i++ ){
		fbStart = 0;
		while( fbStart > -1 ){
			fbStart = Pages[i].text.find( L"\\", fbStart);	// find fb start
			if( fbStart == wstring::npos ) { break;}
			if (Pages[i].text[fbStart + 1] == L'\\')
			// Hack to let me have text backslahes in our Pages[i].text
			{
				Pages[i].text.erase(fbStart , 0);
				++fbStart;
			}

			fbEnd = Pages[i].text.find( L" ", fbStart);
			fbChange.clear();
			fbChange.append( Pages[i].text, fbStart, fbEnd - fbStart );
			log(fbChange);
			if( fbStart  >= 0 && fbEnd >= 1 )
				{
				if( Pages[i].text.size() >  fbEnd  + 1 )
					Pages[i].text.erase( fbStart, fbEnd - fbStart + 1 );
				}
			setTextRanges( i, fbStart );	
		} // end while(
	} // end for i loop
	return;
}// End void ViewDW::parseText(){

//  Set the text format ranges, DWRITE_TEXT_RANGE, for ital, bold, ... etc.
void ViewDW::setTextRanges(int pageIndex, int wordIndex){
#ifdef DEBUG_GB
//	log(L"ViewDW::setTextRanges(int pageIndex = %i, wordIndex = %i)", 
//	pageIndex, wordIndex);
#endif
	int fbIndex(0);

// --------------------- Italics  ----------------------
	static int italCount(0);
	if(Pages[pageIndex].itals > 0){
		fbIndex = fbChange.find(L"\\i");
		if( fbIndex != wstring::npos ){
			if( fbChange[ fbIndex + 2 ] != '0' )
			{
				Pages[pageIndex].italRange[italCount].startPosition = wordIndex;
//log(L"Pages[%i].italRange[%i].startPosition = %i", pageIndex, italCount,
//				Pages[pageIndex].italRange[italCount].startPosition	);
			}
			else
			{
				Pages[pageIndex].italRange[italCount].length = wordIndex - 
				Pages[pageIndex].italRange[italCount].startPosition;
//log(L"Pages[%i].italRange[%i].length = %i", pageIndex, italCount,
//				Pages[pageIndex].italRange[italCount].length);
				++italCount;  
			}
			if( italCount == Pages[pageIndex].itals  ) italCount = 0;				
		}
	}

// --------------------- Bolds  ----------------------
	static int boldCount(0);
	if(Pages[pageIndex].bolds > 0){
		fbIndex = fbChange.find(L"\\b");
		if( fbIndex != wstring::npos ){
			if( fbChange[ fbIndex + 2 ] != '0' )
			{
				Pages[pageIndex].boldRange[boldCount].startPosition = wordIndex;
//log(L"Pages[%i].boldRange[%i].startPosition = %i", pageIndex, boldCount,
//				Pages[pageIndex].boldRange[boldCount].startPosition	);
			}
			else
			{
				Pages[pageIndex].boldRange[boldCount].length = wordIndex - 
				Pages[pageIndex].boldRange[boldCount].startPosition;
//log(L"Pages[%i].boldRange[%i].length = %i", pageIndex, boldCount,
//				Pages[pageIndex].boldRange[boldCount].length);
				++boldCount;
			}
			if( boldCount == Pages[pageIndex].bolds  ) boldCount = 0;				
		}
	}

// --------------------- Underlines  ----------------------
	static int ulineCount(0);
	if(Pages[pageIndex].ulines > 0){
		fbIndex = fbChange.find(L"\\u");
		if( fbIndex != wstring::npos ){
			if( fbChange[ fbIndex + 2 ] != '0' )
			{
				Pages[pageIndex].ulineRange[ulineCount].startPosition = wordIndex;
//log(L"Pages[%i].ulineRange[%i].startPosition = %i", pageIndex, ulineCount,
//				Pages[pageIndex].ulineRange[ulineCount].startPosition	);
			}
			else
			{
				Pages[pageIndex].ulineRange[ulineCount].length = wordIndex - 
				Pages[pageIndex].ulineRange[ulineCount].startPosition;
//log(L"Pages[%i].ulineRange[%i].length = %i", pageIndex, ulineCount,
//				Pages[pageIndex].ulineRange[ulineCount].length);
				++ulineCount;
			}
			if( ulineCount == Pages[pageIndex].ulines  ) ulineCount = 0;				
		}
	}

// ---------------------  Names  ----------------------
	static int nameCount(0);
	std::wstring temp;
	if( Pages[pageIndex].names > 0 ){
		fbIndex = fbChange.find(L"\\f");
		if( fbIndex != wstring::npos ){
			if(  iswdigit(fbChange[fbIndex + 2 ] )){
				temp.clear();
				temp.append( fbChange, fbIndex + 2, 1  );
				Pages[pageIndex].fontNumbers[nameCount] = stoi(temp);
			//	log( L"Pages[%i].fontNumbers[%i] = %i",pageIndex,nameCount, Pages[pageIndex].fontNumbers[nameCount]);
				Pages[pageIndex].fontNameRange[nameCount].startPosition  =										wordIndex;
				
				if( nameCount > 0){
			//		log(L"nameCount > 0 = %i ", nameCount);
					Pages[pageIndex].fontNameRange[nameCount - 1].length = 
					Pages[pageIndex].fontNameRange[nameCount].startPosition - 
					Pages[pageIndex].fontNameRange[nameCount-1].startPosition;

				}
				if( nameCount == Pages[pageIndex].names - 1 ){ // at end of page
					Pages[pageIndex].fontNameRange[nameCount].length  =  
					Pages[pageIndex].text.size() - 
					Pages[pageIndex].fontNameRange[nameCount].startPosition;
			//		log( L"Pages[%i].text.size() = %i", pageIndex, Pages[pageIndex].text.size());
					nameCount = 0;
				}
				else ++nameCount;
			}
		}
	}

// ---------------------  Sizes  ----------------------
	static int sizeCount(0);
	if( Pages[pageIndex].sizes > 0 ){
		fbIndex = fbChange.find(L"\\$");
		if( fbIndex != wstring::npos ){
			if(  iswdigit(fbChange[fbIndex + 2 ] )){
				temp.clear();
				temp.append( fbChange, fbIndex + 2, 2  );
				Pages[pageIndex].fontSizes[sizeCount] = 
					static_cast<float>(stoi(temp))/2.0f;
			//	log( L"Pages[%i].fontSizes[%i] = %i",pageIndex, sizeCount, Pages							[pageIndex].fontSizes[sizeCount]);
				Pages[pageIndex].fontSizeRange[sizeCount].startPosition  =										wordIndex;
				
				if( sizeCount > 0){
			//		log(L"sizeeCount > 0 = %i ", sizeCount);
					Pages[pageIndex].fontSizeRange[sizeCount - 1].length = 
					Pages[pageIndex].fontSizeRange[sizeCount].startPosition - 
					Pages[pageIndex].fontSizeRange[sizeCount-1].startPosition;

				}
				if( sizeCount == Pages[pageIndex].sizes - 1 ){ // at end of page
					Pages[pageIndex].fontSizeRange[sizeCount].length  =  
					Pages[pageIndex].text.size() - 
					Pages[pageIndex].fontSizeRange[sizeCount].startPosition;
			//		log( L"Pages[%i].text.size() = %i", pageIndex, Pages										[pageIndex].text.size());
					sizeCount = 0;
				}
				else ++sizeCount;
			}
		}
	}
	return;
} // End setTextState


// Read the header and  write the font names into vector<wstring> fontNames[ number of fonts ]
int ViewDW::readFontTable()
{
#ifdef DEBUG_GB
	log(L"void ViewDW::readFontTable()");
#endif

// First find the end of the fonttbl
// First append header into wstring temp and read that instead of whole file
	wstring temp;
	int endIndex = 0;
	int startIndex = 0;
	startIndex = book.find( L"\\fonttbl", startIndex);
	endIndex = book.find( L";}}", endIndex);
	temp.append(book, startIndex, endIndex - startIndex);

// Find number of fonts fontCount
	endIndex = 0;
	int fontCount = 0;		// position of last $ token
	while( endIndex  > -1 )
	{
		endIndex = temp.find( L"fcharset", endIndex);
		if( endIndex < 0 ) break;
		fontCount++;	endIndex++;
	}

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
		startIndex++, endIndex++;
	}
 return fontCount;
}

void ViewDW::setTextLayout(int pageIndex, int side){
#ifdef DEBUG_GB
//	log(L"ViewDW::setTextLayout() pageIndex = %i", pageIndex);
#endif

// ---------------------  Italics  --------------------
	for( int i = 0; i < Pages[pageIndex].itals; i++){
		if( side == constants::LEFT_PAGE ) 
			pLeftLayout_->SetFontStyle(DWRITE_FONT_STYLE_ITALIC,
				Pages[pageIndex].italRange[i]);
		else if( side == constants::RIGHT_PAGE ) 
			pRightLayout_->SetFontStyle(DWRITE_FONT_STYLE_ITALIC,
				Pages[pageIndex].italRange[i]);
	}
// ---------------------  Bolds  --------------------
	for( int i = 0; i < Pages[pageIndex].bolds; i++){
		if( side == constants::LEFT_PAGE ) 
			pLeftLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD,
				Pages[pageIndex].boldRange[i]);
		else if( side == constants::RIGHT_PAGE ) 
			pRightLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD,
				Pages[pageIndex].boldRange[i]);

	}
// ---------------------  Underlines  --------------------
	for( int i = 0; i < Pages[pageIndex].ulines; i++){
		if( side == constants::LEFT_PAGE ) 
			pLeftLayout_->SetUnderline(TRUE, Pages[pageIndex].ulineRange[i]);
		else if( side == constants::RIGHT_PAGE ) 
			pRightLayout_->SetUnderline(TRUE, Pages[pageIndex].ulineRange[i]);
	}
// ---------------------  Names  --------------------
	for( int i = 0; i < Pages[pageIndex].names; i++){
		font = fontNames[ Pages[pageIndex].fontNumbers[i] ] ;
		if( side == constants::LEFT_PAGE ) 
			pLeftLayout_->SetFontFamilyName( (PWSTR)font.c_str(), 
				Pages[pageIndex].fontNameRange[i]);
		else if( side == constants::RIGHT_PAGE ) 
			pRightLayout_->SetFontFamilyName( (PWSTR)font.c_str(), 
				Pages[pageIndex].fontNameRange[i]);
	}
// ---------------------  Sizes  --------------------
	for( int i = 0; i < Pages[pageIndex].sizes; i++){
		if( side == constants::LEFT_PAGE ) 
			pLeftLayout_->SetFontSize(1.25*Pages[pageIndex].fontSizes[i],
				Pages[pageIndex].fontSizeRange[i]);
		if( side == constants::RIGHT_PAGE ) 
			pRightLayout_->SetFontSize(1.25*Pages[pageIndex].fontSizes[i],
				Pages[pageIndex].fontSizeRange[i]);
	}
	return;
}

// When user hits a key on the KB -> triggers a WM_CHAR from windows;
// called from int ControllerDW::getChar(  WPARAM message ).
void ViewDW::getChar( WPARAM charCode){
#ifdef DEBUG_GB
	log(L"void ViewDW::getChar( WPARAM charCode = %i",charCode);

#endif
	int digit;
	static int newPageNumber = -1;
	static int charCount = 0;
	static int pageDigit[3];

// The keys 0 - 9  send the values 48 thru 57 in WPARAM charCode.
// A backspace sends 8 in charCode; enter send 13 in charCode.
  
	// charCount is init to 0 then reset to 0 when user presses enter in page menu
	if( bPageMenu )
	{
		// User presses enter (charCode = 13 ) while page menu is active.
		// If we have a valid pageNumber draw the page, remove the page menu.
		if( charCode == 13 )
		{
			log(L"User entered %i", newPageNumber);
			charCount = 0;
			pageDigit[0] = pageDigit[1] = pageDigit[1] = 0;
			bDrawMenu = TRUE;
			bPageMenu = FALSE;
			bMainMenu = TRUE;
			if( newPageNumber > -1 )
			{
				pageNumber = newPageNumber;
				newPageNumber = -1;
			}
			setMenuText(newPageNumber);
			InvalidateRect( dwHandle, NULL, FALSE );
			return;
		}

		// Check for backspace
		if (charCode == 8)
		{
			if( charCount > 0) --charCount;
			else return; 
		}

		// Check for digit
		if(charCode > 47 && charCode < 58 ) digit = charCode - 48;
		else if (charCode != 8 ) return;

		if( charCount < 3 )
		{
			if( charCode == 8  ) pageDigit[charCount] = 0;
			else
			{ 
				pageDigit[charCount] = digit; 
				++charCount;
			} 
		}
		if( charCount == 0 ) newPageNumber = -1; // tell display menu to display "?"
		else if( charCount == 1 ) newPageNumber = pageDigit[0];
		else if( charCount == 2 ) newPageNumber = 10*pageDigit[0] + pageDigit[1];
		else if( charCount == 3 ) newPageNumber = 100*pageDigit[0] + 10*pageDigit[1] + pageDigit[2];
		if( newPageNumber >= numberOfPages) newPageNumber = numberOfPages - 1; // Catch attemps to go past end of book.
		setMenuText(newPageNumber);
		InvalidateRect( dwHandle, NULL, FALSE );
	} // End if( bPageMenu )
	return;
}

// Called from drawDW whenever bDrawMenu flag is TRUE
//void ViewDW::drawMenu(int windowWidth){
void ViewDW::drawMenu(float windowWidth ) {
#ifdef DEBUG_GB	
 log(L"ViewDW::drawMenu(int windowWidth = %f)", windowWidth );
#endif

	D2D1_POINT_2F origin = {0.0f};
	DWRITE_TEXT_RANGE tRange = {0};
	float fontSize = 0.0f;
	menuCellWidth = windowWidth/static_cast<float>(constants::menuCells + 0.2f );
	log(L"ViewDW::drawMenu(cellWidth = %f)", menuCellWidth);
	if (windowWidth < 954 )
		fontSize = windowWidth*constants::menuFontScaleFactor;
	else fontSize = 19.0f;
	for( int i = 0; i < constants::menuCells; i++ )
	{
		origin.x = static_cast<float>(i + 0.2)*menuCellWidth;
		tRange.length = menuText[ i ].size();

		SafeRelease(&pMenuLayout_);
		pDWriteFactory_->CreateTextLayout(
			(PWSTR)menuText[ i ].c_str(),
			(UINT32)lstrlen((PWSTR)menuText[ i ].c_str()),			
			pMenuFormat_,
			menuCellWidth,
			constants::menuHeight,  
			&pMenuLayout_ );
		if( i == iMenuCell ) pMenuBrush_->SetColor(constants::menuTextHighlightColor);
		else pMenuBrush_->SetColor(constants::menuTextColor);
		pMenuLayout_->SetFontSize( fontSize, tRange);
		pRT_->DrawTextLayout( origin , pMenuLayout_, pMenuBrush_ );
	}
}

HRESULT ViewDW::CreateDeviceResources()
{
#ifdef DEBUG_GB
//	log(L"ViewDW::CreateDeviceResources()");
#endif
    HRESULT hr = S_OK;
	if(!pRT_){
//		int mode;
		RECT rc;
		GetClientRect(dwHandle, &rc);
		D2D1_SIZE_U Usize = D2D1::SizeU((rc.right - rc.left ),
										(rc.bottom - rc.top ));
		hr = pD2DFactory_->CreateHwndRenderTarget( 
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties( dwHandle, Usize ), &pRT_ );
//		mode = pRT_->GetTextAntialiasMode();
//			log(L"ViewDW::CreateDeviceResources() mode = %i", mode);
//		pRT_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
//		pRT_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_FORCE_DWORD);

//		mode = pRT_->GetTextAntialiasMode();
//		log(L"ViewDW::CreateDeviceResources() mode = %i", mode);
		//typedef enum D2D1_TEXT_ANTIALIAS_MODE {
		//	D2D1_TEXT_ANTIALIAS_MODE_DEFAULT,
		//	D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE,
		//	D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE,
		//	D2D1_TEXT_ANTIALIAS_MODE_ALIASED,
		//	D2D1_TEXT_ANTIALIAS_MODE_FORCE_DWORD
		//};
		pRT_->CreateSolidColorBrush(constants::paperColor, &pPaperBrush_ ); 
		pRT_->CreateSolidColorBrush(D2D1::ColorF(0xF4B480),  &pBookTextBrush_ );
		pRT_->CreateSolidColorBrush(constants::menuTextColor,  &pMenuBrush_ );

	}

	if( FAILED( hr )) return hr;

    return S_OK;
}
void ViewDW::setPageSize(int pageNumber){
#ifdef DEBUG_GB
log(L"ViewDW::setPageSize(int pageNumber) = %i", pageNumber);
#endif
	
	// This seems to be a crash site so we add the following
	// check for a dangling pointer 12-12-18
	if( pRT_ ) rtSize = pRT_->GetSize();
	else{
//		log(L"ViewDW::setPageSize if( pRT_ ) returned false");
		return;
	}
 //   if (pRT_){		
	//	if(FAILED( pRT_->GetSize())){
	//		DiscardDeviceResources();
	//		log(L"FAILED( pRT_->Resize");
	//		return; 
	//	}
	//}

	paperRect.right = rtSize.width; paperRect.bottom = rtSize.height;

	if( Pages[pageNumber].mode == constants::gL_gL ){
//		log(L"Pages[pageNumber].mode = constants::gL_gL");
		viewGL->hello_From_DW(Pages[pageNumber].glRoutine);
		return;}

	if( Pages[pageNumber].mode == constants::dW_dW )
	{  
//		log(L"Pages[pageNumber].mode = constants::dW_dW");
		// Do not allow fpageWidth or fpageHeight to go negative  < 1  !!
		if( rtSize.width  > 3*constants::pageMargin )
			fpageWidth = rtSize.width/2.0f - 1.5f*constants::pageMargin;
		else  fpageWidth = rtSize.width;  

		if( rtSize.height > 2*constants::pageMargin )
			fpageHeight = rtSize.height - 2.0f*constants::pageMargin;
		else  fpageHeight = rtSize.height;
		viewGL->hello_From_DW(-1);
	}

	else if( Pages[pageNumber].mode == constants::dW_gL )
	{
//		log(L"Pages[pageNumber].mode = constants::dW_gL");
		// Tell viewGL to display the palette
		if (bColorMenu) viewGL->hello_From_DW(constants::PALETTE);
		else viewGL->hello_From_DW(Pages[pageNumber].glRoutine);
		if( rtSize.width > 2.0f*constants::pageMargin )
			fpageWidth = rtSize.width - 2.0f*constants::pageMargin;
		else  fpageWidth = rtSize.width;

		if( rtSize.height > 2*constants::pageMargin ) 
			fpageHeight = rtSize.height - 2.0f*constants::pageMargin;
		else  fpageHeight = rtSize.height; 	
	}
	return;
}

void ViewDW::setLeftRightLayout( int pageNumber){
#ifdef DEBUG_GB
log(L"ViewDW::setLeftRightLayout() ");
#endif

	if( pageNumber >= 0 )
	{
		if( Pages[pageNumber].mode == constants::gL_gL ) return;
		if( Pages[pageNumber].mode == constants::dW_dW ||
			Pages[pageNumber].mode == constants::dW_gL )
		{
			createLayout(pageNumber, constants::LEFT_PAGE);
			setTextLayout( pageNumber, constants::LEFT_PAGE );
		}

		if( Pages[pageNumber].mode == constants::dW_dW)
		{
			if( pageNumber + 1 < numberOfPages && 
				Pages[pageNumber + 1 ].mode != constants::gL_gL)
			{
				createLayout(pageNumber + 1, constants::RIGHT_PAGE);
				setTextLayout( pageNumber + 1, constants::RIGHT_PAGE);
				return;
			}
		}
	}
	return;
}

void ViewDW::createLayout(int pageNumber, int side){
#ifdef DEBUG_GB
log(L"ViewDW::createLayout(int pageNumber, int side) ");
#endif

	if( side == constants::LEFT_PAGE )
	{
		SafeRelease(&pLeftLayout_);
		pDWriteFactory_->CreateTextLayout(
		(PWSTR)Pages[pageNumber].text.c_str(),
		(UINT32)lstrlen((PWSTR)Pages[pageNumber].text.c_str()),
		pTextFormat_, fpageWidth, fpageHeight, &pLeftLayout_ );
		return;
	}
	if( side == constants::RIGHT_PAGE )
	{
		SafeRelease(&pRightLayout_);
		pDWriteFactory_->CreateTextLayout(
		(PWSTR)Pages[pageNumber].text.c_str(),
		(UINT32)lstrlen((PWSTR)Pages[pageNumber].text.c_str()),
		pTextFormat_, fpageWidth, fpageHeight, &pRightLayout_ );
		return;
	}
}
void ViewDW::drawPageNumber(int pageNumber, int side){
#ifdef DEBUG_GB
log(L"ViewDW::drawPageNumber(int pageNumber, int side) ");
#endif

	pnum.clear();
	pnum = to_wstring( pageNumber);
	if( side == constants::LEFT_PAGE )
	{
		pnumLeftRect.right = (UINT32)lstrlen((PWSTR)pnum.c_str())*9.0f;
		pRT_->DrawText((PWSTR)pnum.c_str(), 
			(UINT32)lstrlen((PWSTR)pnum.c_str()),
			pMenuFormat_, pnumLeftRect, pBookTextBrush_ ,
			D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL );
		return;
	}

	if( side == constants::RIGHT_PAGE )
	{
		pnumRightRect.right = paperRect.right;
		pnumRightRect.left =  paperRect.right - 
			(UINT32)lstrlen((PWSTR)pnum.c_str())*9.0f;
		pRT_->DrawText((PWSTR)pnum.c_str(), 
			(UINT32)lstrlen((PWSTR)pnum.c_str()),
			pMenuFormat_, pnumRightRect, pBookTextBrush_ ,
			D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL );
		return;
	}
	return;
}

// Invoke DW's DrawText and DrawTextLayout to put our text
// on the display.
void ViewDW::drawDW(){
#ifdef DEBUG_GB
log(L"ViewDW::drawDW() ");
#endif
										//WPARAM wParam,   LPARAM lParam
	SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );

	if (SUCCEEDED( CreateDeviceResources()) )
	{ 	
		setPageSize(pageNumber);
		setLeftRightLayout(pageNumber);

		pRT_->BeginDraw(); 

		// Set background color to paperColor		
		pRT_->FillRectangle(&paperRect, pPaperBrush_);

		if( bMainMenu) setMenuText(Pages[pageNumber].mode);
		if (bDrawMenu)
		{
			if( Pages[pageNumber].mode == constants::dW_dW ) drawMenu( rtSize.width/2.0f );
			else drawMenu( rtSize.width );
		}

		if( pageNumber >= 0 && pageNumber < numberOfPages )
		{
			if( Pages[pageNumber].mode == constants::gL_gL ) return;
			if( Pages[pageNumber].mode == constants::dW_dW ||
				Pages[pageNumber].mode == constants::dW_gL )
			{
				drawPageNumber(pageNumber, constants::LEFT_PAGE);
				pRT_->DrawTextLayout( leftPageOrigin, pLeftLayout_, pBookTextBrush_ );
			}
			if( Pages[pageNumber].mode == constants::dW_dW)
			{
				if( pageNumber + 1 < numberOfPages && 
					Pages[pageNumber + 1 ].mode != constants::gL_gL)
				{
					drawPageNumber(pageNumber + 1, constants::RIGHT_PAGE);
					rightPageOrigin.x = fpageWidth  +  2.0f*constants::pageMargin;
					pRT_->DrawTextLayout( rightPageOrigin, pRightLayout_, pBookTextBrush_ );
				}
			}
		}
		if (D2DERR_RECREATE_TARGET == pRT_->EndDraw()) DiscardDeviceResources();
	} //End if (SUCCEEDED( CreateDeviceResources())
	return;
} // End drawDW()

// Set iMenuCell = -1 if mouse is out of the menu rectangle.
// Set iMenuCell = menu item if mouse in the menu rectangle.
void ViewDW::setMenuCell(int x, int y){

	//if( y > 2 && y < constants::menuHeight && 
	//	x < (dwWidth - 5) && x > 0 )
	//{
	//	float menuCellWidth = ((float)dwWidth/(float)constants::menuCells);  			
	//	if( x < menuCellWidth ) iMenuCell = -1;
	//	else iMenuCell = (x/menuCellWidth) -1;
	//}
	//else iMenuCell = -1;

//	menuCellWidth = ((float)dwWidth / (float)constants::menuCells);

	if (y > 2 && y < constants::menuHeight &&
		x < (dwWidth - 5) && x > 0)
	{
//		float menuCellWidth = ((float)dwWidth / (float)constants::menuCells);
		if (x < 5) { iMenuCell = -1; return; }
		else { iMenuCell = (x / menuCellWidth); return; }
	}
	else iMenuCell = -1;
}

// Set flag, bDrawMenu, to draw or remove the main
// or sub-menu.  Set iMenuCell
int ViewDW::mouseMove( int x, int y ){
#ifdef DEBUG_GB 
//	log(L"ViewDW::mouseMove(int x = %i , int y = %i)", x, y);
#endif

	// Set iMenuCell to the cell the mouse is in
	// or return -1 if mouse is not in the menu.
	setMenuCell(x,y);

	// Mouse is in main or sub-menu set flag
	// to call drawMenu() in drawDW(). 
	if( iMenuCell > -1  )
	{
		bDrawMenu = TRUE;
		InvalidateRect( dwHandle, NULL, FALSE );
		return 0;
	}

	// Mouse is out of main menu set flag
	// to remove menu in drawDW(). 
	if( bMainMenu == TRUE && bDrawMenu == TRUE)
	{
		bDrawMenu = FALSE;
		InvalidateRect( dwHandle, NULL, FALSE );
		return 0;
	}

	// Mouse is out of sub-menu; do nothing
	return 0;
	
} // end mouseMove(x,y)

void ViewDW::keyDown(int key, LPARAM lParam){
#ifdef DEBUG_GB 
	log(L"ViewDW::lkeyDown(int key = %i, LPARAM lParam = %i)",key ,lParam );
#endif
	
	if( bMainMenu )
	{
		if( key == VK_DOWN || key == VK_NEXT)
		{ lButtonDown(0, 0, constants::NEXT); return; }
		if( key == VK_UP || key == VK_PRIOR)
		{ lButtonDown(0, 0, constants::PRIOR); return; }
		if( key == VK_END )
		{ lButtonDown(0, 0, constants::END); return; }
		if( key == VK_HOME )
		{ lButtonDown(0, 0, constants::COVER); return; }
		if( key == VK_F1 )
		{ lButtonDown(0, 0, constants::FILE); return; }

	}
	if( bPageMenu ) // Check for page menu chars; 0 - 9, backspace, enter
	{
		//  enter		backspace
		if( key == 13 || key == 8){ getChar(key); return; }
		// digit 0 thro 9
		if( key > 47 && key < 58 ){ getChar(key); return; }
		return;
	}
	if( bFileMenu )
	{ 
		if( key == VK_F1 ){ lButtonDown(0, 0, constants::LOAD); return; }
	}
	return;
}
// We use this for mouse clicks and keyboard shortcuts for the 
// same menu items as the mouse menu. If item == -1 its a mouse click
// if its from the keys item will be the iMenuCell for the item
int ViewDW::lButtonDown(int x, int y, int item){
#ifdef DEBUG_GB 
	log(L"ViewDW::lButtonDown(int x = %i , int y = %i, item = %i)", x, y, item);
#endif

	// Set iMenuCell to the cell the mouse is in
	// or return -1 if mouse is not in the menu.
	if( item == -1 ) setMenuCell(x,y);  // Mouseclick
	else iMenuCell = item;	// Keyboard input
	
	if( iMenuCell < 0 ) return 0;

// Check for clicks in the main menu.	
	if( bMainMenu )
	{
		log(L"ViewDW::lButtonDownif( bMainMenu ) iMenuCell = %i", iMenuCell);

// Handle Sub-Menu Requests
		if( iMenuCell == constants::FILE )
		{
			bFileMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			setMenuText( constants::FILE );
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if (iMenuCell == constants::PAGE)
		{
			bPageMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			setMenuText(constants::SET_QUESTION_MARK);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::COLOR 
			&& Pages[pageNumber].mode == constants::dW_gL)
		{
			// 	bColorMenu == TRUE will flag this call - 
			// viewGL->hello_From_DW(99);  in ViewDW::setPageSize
			bColorMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			setMenuText(constants::COLOR);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if (iMenuCell == constants::HELP)
		{
			bDrawMenu = FALSE;
			openDW_file(iMenuCell);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}
		//  Handle Page Requests 
		if( iMenuCell == constants::PRIOR && pageNumber > 0)
		{
			// If we are in gL_gL mode; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size 
			// so windows will send this msg to it.
			--pageNumber;
			if(	Pages[pageNumber + 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode != constants::gL_gL)
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			if (item == -1) bDrawMenu = TRUE;
			else bDrawMenu = FALSE;
			//log(L"ViewDW::lButtonDown(pageNumber = %i )", pageNumber);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::NEXT && pageNumber < numberOfPages - 1)	   {
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send this msg to it.
			++pageNumber;
			if(	Pages[pageNumber - 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode != constants::gL_gL)
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			if( item == -1 ) bDrawMenu = TRUE;
			else bDrawMenu = FALSE;
			//log(L"ViewDW::lButtonDown(pageNumber = %i )", pageNumber);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::COVER && pageNumber != 0 )
		{   			
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send this msg to it.
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[0].mode != constants::gL_gL)
			{
				pageNumber = 0;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = 0;
			bDrawMenu = FALSE;
			InvalidateRect(dwHandle, NULL, FALSE);	
			return 0;
		}

		if( iMenuCell == constants::TOC && pageNumber != tableOfContents)
		{
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send this msg to it.
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[tableOfContents].mode != constants::gL_gL)
			{
				pageNumber = tableOfContents;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = tableOfContents;
			bDrawMenu = FALSE;
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::INDEX && pageNumber != indexPage)
		{
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[indexPage].mode != constants::gL_gL)
			{
				pageNumber = indexPage;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = indexPage;
			bDrawMenu = FALSE;
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}
		if( iMenuCell == constants::END 
			&& pageNumber != numberOfPages - 1 && numberOfPages > 0)
		{ 
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[numberOfPages - 1].mode != constants::gL_gL)
			{
				pageNumber = numberOfPages - 1;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = numberOfPages - 1;
			bDrawMenu = FALSE;
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}
	} // end if(bMainMenu)

// Check for clicks in a sub-menu item that requires mouse input

	if( bFileMenu )
	{
		if( iMenuCell == constants::RELOAD ||
			iMenuCell == constants::LOAD ||
			iMenuCell == constants::CANCEL )
		{
			bFileMenu = FALSE;
 			bMainMenu = TRUE;
			openDW_file( iMenuCell );
			InvalidateRect( dwHandle, NULL, FALSE );
			return 0;
		}
		return 0; // Clicked an inactive cell; do nothing.
	}

	if (bColorMenu)
	{
		bColorMenu = FALSE;
		bMainMenu = TRUE;
		InvalidateRect(dwHandle, NULL, FALSE);
		return 0;
	}

	return 0;
} // End lButtonDown(...


int ViewDW::size( int width, int height ){
#ifdef DEBUG_GB
	log(L"ViewDW::size %ix%i", width, height );
#endif

	dwWidth = width;
	// width and height is in pixels
    if (pRT_){		
		if(FAILED( pRT_->Resize(D2D1::SizeU(width, height)))){
			DiscardDeviceResources();
			log(L"FAILED( pRT_->Resize");
			return 0; 
		}
	}

	InvalidateRect( dwHandle, NULL, FALSE );	
	return 0;
}

// Sets std vector<wstring>menuText to display the main menu or 
// sub-menus depending on flags set in lButtonDown(), getChar().
void ViewDW::setMenuText( int parameter )
{
#ifdef DEBUG_GB
	log(L"void ViewDW::setMenuText( parameter = %i)", parameter );
#endif

	if(bMainMenu){
		menuText[0]  = L"File";
		menuText[1]  = L"Page";
		menuText[2]  = L" < ";
		menuText[3]  = L" > ";
		menuText[4]  = L"Cover";
		menuText[5]  = L"TOC";
		menuText[6]  = L"Index";
		menuText[7]  = L"End";
        // Only "Color" menu item in dW_gL mode.
		if (Pages[pageNumber].mode == constants::dW_gL) menuText[8] = L"Color";
		else menuText[8]  = L"  ";
		menuText[9]  = L" ";
		menuText[10]  = L" ";
		menuText[11]  = L"Help";
		menuText[12]  = L"Find";
		return;
	}
	if(bFileMenu){
		menuText[0]  = L"Reload";
		menuText[1]  = L"Load";
		menuText[2]  = L"Cancel";
		menuText[3]  = L"  ";
		menuText[4]  = L" ";
		menuText[5]  = L" ";
		menuText[6]  = L" ";
		menuText[7]  = L" ";
		menuText[8]  = L" ";
		menuText[9]  = L" ";
		menuText[10]  = L" ";
		menuText[11]  = L" ";
		menuText[12]  = L" ";
		return;
	}

	if(bPageMenu){
		menuText[0]  = L" ";
		if( parameter == constants::SET_QUESTION_MARK) menuText[1]  = L"?";
		else menuText[1] =  to_wstring( parameter );
		menuText[2]  = L" ";
		menuText[3]  = L"  ";
		menuText[4]  = L" ";
		menuText[5]  = L" ";
		menuText[6]  = L" ";
		menuText[7]  = L" ";
		menuText[8]  = L" ";
		menuText[9]  = L" ";
		menuText[10]  = L" ";
		menuText[11]  = L" ";
		menuText[12]  = L" ";
		return;
	}

	if (bColorMenu) {
		log(L"void ViewDW::setMenuText(bColorMenu)");
		menuText[0] = L" ";
		menuText[1] = L" ";
		menuText[2] = L" ";
		menuText[3] = L"  ";
		menuText[4] = L" ";
		menuText[5] = L" ";
		menuText[6] = L" ";
		menuText[7] = L" ";
		menuText[8] = L"Exit Color Menu";
		menuText[9] = L" ";
		menuText[10] = L" ";
		menuText[11] = L" ";
		menuText[12] = L" ";
		return;
	}
	return;
}
