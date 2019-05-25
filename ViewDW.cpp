///////////////////////////////////////////////////////////////////////////////
// ViewDW.cpp
// ==============
// View component of 3dBook-Reader directWrite window.
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

// Static color menu menbers
bool ViewDW::bColorMenu = false;
bool  ViewDW::bMainMenu = true;
bool  ViewDW::bPageMenu = false;
bool  ViewDW::bFileMenu = false;
glm::vec4 ViewDW::colorFromGL = {0.0f, 0.0f,0.0f,0.0f};
std::vector<std::wstring> ViewDW::menuText;

// Constuctor for ViewDW* mySelf out Callback pointer
ViewDW::ViewDW():instanceID(2){ log(L"In the constructor 2"); }
 
// Constructor for viewDW
ViewDW::ViewDW(ViewGL* viewGL):

	instanceID(1),

// Pointer to ViewGL methods.
	viewGL(viewGL),

// Page State
	indexPage(0),
	tableOfContents(0),
	currentPage(-1),
	pageNumber(0),
	numberOfPages(0),
	linkIndex(-1),
//	qlinkIndex(-1),

// Menu Logic
	bDrawPage(TRUE),
	bDrawMenu(TRUE),
	//bMainMenu(TRUE),
	//bFileMenu(FALSE),
	//bPageMenu(FALSE),
	bNumberMenu(FALSE),
		userInt(0),
		userFloat(0),

// Hyperlink & Qlink flags
	bLeftButtonDown(FALSE),
	bHitflag(FALSE),

// D2D and DirectWrite
    pD2DFactory_(NULL),
	pDWriteFactory_(NULL),
	m_pWICFactory(NULL),
	m_pBitmap(NULL),
    pRT_(NULL),

// Brushes
    pBookTextBrush_(NULL),
	pPaperBrush_(NULL),
	pRedBrush_(NULL),
	pMenuBrush_(NULL),

// DirectWrite
    pTextFormat_(NULL),
	pMenuLayout_(NULL),
	pLeftLayout_(NULL),
	pRightLayout_(NULL),
	pHyperLinkBrush_(NULL),
	pQlinkBrush_(NULL)
{
	log(L"In the constructor 1");
	ZeroMemory( &paperRect, sizeof(paperRect));
	ZeroMemory( &menuRect, sizeof(paperRect));
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
	menuFontSize = 12.0f;
	fontSizeFactor = 1.25f;
	imageScale.width = 1.0f;
	imageScale.height = 1.0f;
	dummy = L"dumb";
}

///////////////////////////////////////////////////////////////////////////////
// default dtor
///////////////////////////////////////////////////////////////////////////////
ViewDW::~ViewDW()
{  
	log(L"In the destructor instanceID = %i", instanceID);
	// Is it viewDW: instanceID == 1 or 
	// the callback pointer ViewDW* mySelf: instanceID == 2
	//
	if(instanceID == 1){  // don't do all this if it's not viewDW
	SafeRelease(&pTextFormat_);
	SafeRelease(&pMenuFormat_);
    SafeRelease(&pD2DFactory_);
    SafeRelease(&pDWriteFactory_);
	SafeRelease(&m_pWICFactory);
	DiscardDeviceResources();}
}

void ViewDW::DiscardDeviceResources(){
#ifdef DEBUG_GB
	log(L"ViewDW::DiscardDeviceResources()");
#endif

    SafeRelease(&pBookTextBrush_);
    SafeRelease(&pPaperBrush_);
	SafeRelease(&pRedBrush_);
	SafeRelease(&pMenuBrush_);
	SafeRelease(&pHyperLinkBrush_);
	SafeRelease(&pQlinkBrush_);
	SafeRelease(&pLeftLayout_);
	SafeRelease(&pRightLayout_);
	SafeRelease(&pMenuLayout_);
	SafeRelease(&m_pBitmap);
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

	TCHAR exeDir[MAX_PATH];
	GetModuleFileName( NULL, exeDir, MAX_PATH); 
	log(L" ViewDW::create(HWND hwnd) executabe directory ...");
	log(exeDir);

	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path); 
	workDir = path;
	log(L" ViewDW::create(HWND hwnd) current directory ...");
	log(workDir);

	initDW();
	openDW_file(constants::START_PAGE);
	viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);

}

HRESULT ViewDW::initDW(){		
#ifdef DEBUG_GB
	log(L"ViewDW::initDW()");
#endif

	// maxClientArea is used to scale text on resize of window
	maxClientSize.width = static_cast<float>( GetSystemMetrics(SM_CXFULLSCREEN));
	maxClientSize.height = static_cast<float>(GetSystemMetrics(SM_CYFULLSCREEN));
	maxClientArea = maxClientSize.width * maxClientSize.height;

	log(L"ViewDW::initDW() maxClientSize.height = %f",maxClientSize.height); 
	log(L"ViewDW::initDW() maxClientSize.width = %f",maxClientSize.width); 

	HMONITOR monitor = MonitorFromWindow(dwHandle, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;

	log(L"ViewDW::initDW() monitor_width = %i", monitor_width); 
	log(L"ViewDW::initDW() monitor_height = %i",monitor_height); 

	resFactor = 1.25f*monitor_width/1920.0f;
	imageScaleH = monitor_width/1920.0f;
	imageScaleV = monitor_height/1080.0f;

	HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory_ );

	if (SUCCEEDED(hr)){
		pD2DFactory_->GetDesktopDpi(&dpiScaleX_, &dpiScaleY_);	
		dpiScaleX_ = 96/dpiScaleX_;
		dpiScaleY_ = 96/dpiScaleY_;
	}

	log(L"ViewDW::initDW() dpiScaleX_ = %f", dpiScaleX_);

	if (SUCCEEDED(hr)){
		hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_) );
	}

	// Create WIC factory for LoadBitmapFromFile
	if (SUCCEEDED(hr)) {        
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_pWICFactory)
		);
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
			//constants::menuFontSize,
			menuFontSize,
			L"en-us",
			&pMenuFormat_);

		pMenuFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
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

void ViewDW::ReadUTF16(const wstring &filename){
#ifdef DEBUG_GB
	log(L"void ViewDW::ReadUTF16(const wstring & filename)");
#endif
	book.clear();

    ifstream file(filename.c_str(), std::ios::binary);    // std::ios::binary for Windows
	if(!file)
	{
		MessageBox(NULL, TEXT("Oops: Could not open this file."), filename.c_str(), MB_ICONINFORMATION);
		book = L"Could not open this file.";
		return;
	}

    std::stringstream ss;
    ss << file.rdbuf() << '\0';
	int iUniTest = IS_TEXT_UNICODE_SIGNATURE; // 0xFEFF at beginning of file.
	int cTextLength_(0);						//  byte order mask

	cTextLength_ = (UINT32)strlen(ss.str().c_str());
	if(IsTextUnicode(ss.str().c_str(), cTextLength_, &iUniTest))
	{
		book = wstring((wchar_t *)ss.str().c_str());
		return;
	}

	else 
	{
		MessageBox(NULL, TEXT("Not UNICODE"), filename.c_str(), MB_ICONINFORMATION);
		log(L"This file is not unicode.");
	}

// Closing of streams:  Done inplicitly by the streams dtor.  So an explicit call to
//	close() /is needed only if the file must be closed before reaching the end of the
//	scope  in  which it was declared. 
} 
void ViewDW::loadRecoverText(){

	book = L"\\tdr1{\\fonttbl{\\f0\\fcharset0 Arial;}{\\endfonttbl;}0123456789xx\\f0\\$32 "
		L"\n"
		L"TDR was unable to find or open the start.tdr file\n"
		L"\n"
		L"start.tdr may not be in the doc directory.\n"
		L"\n"
		L"or start.tdr may be corrupted.\n"
		L"\n"
		L"Find or fix start.tdr or load another file.\n"
		L"\n"
		L"\\Mode_dW_gL04\\f0\\$32  ";
}

bool ViewDW::openDW_file( int reloadFlag ){  
#ifdef DEBUG_GB	
	log(L"ViewDW::openDW_file( int reloadFlag )");
#endif

	if( reloadFlag == constants::CANCEL ) return FALSE;

	bDrawPage =  TRUE;

	if (reloadFlag == constants::HELP)
	{
		pageNumber = 0;
//		// Note relative path is not the same for running in VS and from taskbar
//		reload_Filename = L".\\src\\doc\\manual.tdr";
//		reload_Filename = L"C:\\Users\\pstan\\source\\repos\\Proj3dBook\\src\\doc\\manual.tdr";
		reload_Filename = workDir;
		reload_Filename.append( L"\\doc\\manual.tdr");
		ReadUTF16(reload_Filename.c_str());
	}
	else if( reloadFlag == constants::START_PAGE )
	{	
		pageNumber = 0;
		reload_Filename = workDir;
		reload_Filename.append( L"\\doc\\start.tdr");

//		reload_Filename = L"C:\\Users\\pstan\\source\\repos\\Proj3dBook\\src\\doc\\start.tdr";

		ReadUTF16( reload_Filename.c_str()); 

		// Let user know TDR Failed to load start.tdr - how to recover, etc.
		if (book.find(L"Could not open this file.") != wstring::npos) 
		{		
			loadRecoverText();
		}
	}
	// Loading a new file -- Put up the Open File Dialog
	else if( reloadFlag == constants::LOAD ){ 

		// Dont try to draw a pageNumber > numberOfPages;
		pageNumber = 0;
		bDrawPage = TRUE;
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
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;;
		
		if (GetOpenFileName(&ofn) == TRUE) {
			// Save the file name for RELOAD call
			reload_Filename.clear();
			reload_Filename = ofn.lpstrFile;
			ReadUTF16(reload_Filename.c_str());
		}
		else return FALSE;
	}

	else if( reloadFlag == constants::RELOAD ){ 
		ReadUTF16( reload_Filename.c_str() );  
	}

// Do some file format checks.
	if( book.find( L"tdr1") == wstring::npos ){
		MessageBox( NULL, reload_Filename.c_str(), 
		TEXT("File is not a 3dBook reader file!"), MB_OK );
		return FALSE;
	}

	if (book.find(L"0123456789xx") == wstring::npos) {
		MessageBox(NULL, reload_Filename.c_str(),
			TEXT("This version .tdr file is no longer supported!"), MB_OK);
		return FALSE;
	}
// Put the file name up on the Title bar and proceed to parse the text.
	else { 
		SetWindowText( mainHandle, reload_Filename.c_str()); 
		parseText();
		return TRUE; 
	} 
}
void ViewDW::reportFormatError(int pageNo, int index, std::wstring temp){

	// Correct the format error: add a trailing space.
	Pages[pageNo].text.insert(index + 3, L" ");

	std::wstring page = L"Format error was found on page ";
	std::wstring blurb = temp;
	
	page.append(to_wstring(pageNo));
	page.append(L" index " );
	page.append(to_wstring(index));

	blurb.append(L" format error found on page \n"
			"check for missing space after the format block.\n"
			"See log.txt for details.  TDR will add the missing space\n"
			"but the .tdr file needs to be corrected.");

	MessageBox(NULL, blurb.c_str(), page.c_str(), MB_ICONWARNING);
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
			if (Pages[i].text[index + 2] == L'0' )
			{
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
				// Check for format errors
				if( Pages[i].text[index + 3] != L' ' && Pages[i].text[index + 3] != L'\\' ){
					temp = L"Bold";
					reportFormatError(i, index, temp);
				}		
				++Pages[i].bolds;
			}
			// Check for format errors
			else if (Pages[i].text[index + 2] != L' ' && Pages[i].text[index + 2] != L'\\') {
				temp = L"Bold";
				reportFormatError(i, index, temp);
			}
			++index;
		}
		Pages[i].boldRange.clear();
		Pages[i].boldRange.resize(Pages[i].bolds);

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

// ---- Hyperlinks ----
		index = 0;
		Pages[i].links = 0;
		while (index > -1)
		{
			index = Pages[i].text.find(L"\\h", index);
			if (index < 0) break;
			if (Pages[i].text[index + 2] == L'0')
			{			
				++Pages[i].links;
			}
			++index;
		}

		//		log(L"Pages[%i].links; = %i", i, Pages[i].links);
		Pages[i].linkRange.clear();
		Pages[i].linkRange.resize(Pages[i].links);

// ---- qlinks ----
		index = 0;
		Pages[i].qlinks = 0;
		while (index > -1)
		{
			index = Pages[i].text.find(L"\\q", index);
			if (index < 0) break;
			if (Pages[i].text[index + 2] == L'0')
			{
				++Pages[i].qlinks;
			}
			++index;
		}

		//		log(L"Pages[%i].qlinks; = %i", i, Pages[i].qlinks);
		Pages[i].qlinkRange.clear();
		Pages[i].qlinkRange.resize(Pages[i].qlinks);

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
		while (index > -1)
		{
			index = Pages[i].text.find(L"\\f", index);
			if (index < 0) break;
			// Check for format errors
			if (Pages[i].text[index + 3] != L' ' && Pages[i].text[index + 3] != L'\\')
			{
				temp = L"Font";
				reportFormatError(i, index, temp);

				//std::wstring page = to_wstring(i);
				//page.append(L" = the page the error was found on." );
				//MessageBox(NULL,
				//	TEXT("Font name format error found on page \n"
				//	"check for missing space after the format block.\n"
				//	"See log.txt for details.  TDR will add the missing space\n"
				//	"but the .tdr file needs to be corrected."), page.c_str(), MB_ICONWARNING);

				//std::wstring space = L" ";
				//Pages[i].text.insert(index + 3, space);
				//log(L"Pages[%i] format error: index = %i", i, index);				
				//page.clear();
				//// need to do some range checking here
				//page.append(Pages[i].text, index , 15);
				//log(page);
			}
			++Pages[i].names;			
			++index;
		}
		Pages[i].fontNumbers.clear();
		Pages[i].fontNumbers.resize(Pages[i].names);
		Pages[i].fontNameRange.clear();
		Pages[i].fontNameRange.resize(Pages[i].names);

// ---- Images ----
		index = 0;
		Pages[i].images = 0;
		while (index > -1)
		{
			index = Pages[i].text.find(L"\\c", index);
			if (index < 0) break;
			++Pages[i].images;
			++index;
		}

		Pages[i].imageNames.clear();
		Pages[i].imageNames.resize(Pages[i].images);
		Pages[i].imageRect.clear();
		Pages[i].imageRect.resize(Pages[i].images);

	} // End for( int i = 0; i < numberOfPages; i++ ){
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
	int index(0);
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
//	bool flag = TRUE;
// Here we strip off the format blocks and send them to
// setTextRanges
	int fbStart(0);
	int fbEnd(0);
	for( int i = 0; i < numberOfPages; i++ ){
		fbStart = 0;
		while( fbStart > -1 ){
//			flag = TRUE;
			fbStart = Pages[i].text.find( L"\\", fbStart);	// find fb start
			if( fbStart == wstring::npos ) { break;}
//			 Hack to let us have text backslahes in our Pages[i].text
			if (Pages[i].text[fbStart + 1] == L'\\')
			{
			//	log(L"found two backslahes page %i", i);
				Pages[i].text.erase(fbStart , 0);
				++fbStart;
			}
			temp.clear();


		//	if (Pages[i].text[fbStart + 1 ] == L'\\')// &&
		//	{

		////		log(L"found two backslahes");
		////		Pages[i].text.replace(fbStart-1, 0);
		//		temp.append(Pages[i].text, fbStart, 10);
		//		log(temp);
		//		temp.clear();
		//		Pages[i].text.erase(fbStart  , 1);
		//		temp.append(Pages[i].text, fbStart, 10);
		//		log(temp);
		//		++fbStart;
		//		flag = FALSE;
		//
		//	}

//			if (flag) {
			fbEnd = Pages[i].text.find(L" ", fbStart);
			fbChange.clear();
			fbChange.append(Pages[i].text, fbStart, fbEnd - fbStart);
		//	log(fbChange);
			if (fbStart >= 0 && fbEnd >= 1)
			{
				if (Pages[i].text.size() > fbEnd + 1)
					Pages[i].text.erase(fbStart, fbEnd - fbStart + 1);
			}
			setTextRanges(i, fbStart);
//			}// End if (flag)
		} // End while(
	} // End for i loop


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

// --------------------- Hyperlinks  ----------------------
	static int linkCount(0);
	if (Pages[pageIndex].links > 0) {
		fbIndex = fbChange.find(L"\\h");
		if (fbIndex != wstring::npos) {
			if (fbChange[fbIndex + 2] != '0')
				Pages[pageIndex].linkRange[linkCount].startPosition = wordIndex;
			else{
				Pages[pageIndex].linkRange[linkCount].length = wordIndex -
					Pages[pageIndex].linkRange[linkCount].startPosition;
				++linkCount;
			}
			if (linkCount == Pages[pageIndex].links) linkCount = 0;
		}
	}
// --------------------- Qlinks  ----------------------
	static int qlinkCount(0);
	if (Pages[pageIndex].qlinks > 0) {
		fbIndex = fbChange.find(L"\\q");
		if (fbIndex != wstring::npos) {
			if (fbChange[fbIndex + 2] != '0')
				Pages[pageIndex].qlinkRange[qlinkCount].startPosition = wordIndex;
			else {
				Pages[pageIndex].qlinkRange[qlinkCount].length = wordIndex -
					Pages[pageIndex].qlinkRange[qlinkCount].startPosition;
				++qlinkCount;
			}
			if (qlinkCount == Pages[pageIndex].qlinks) qlinkCount = 0;
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

// --------------------- Images  ----------------------

	static int imageCount(0);
	if (Pages[pageIndex].images > 0) {
		fbIndex = fbChange.find(L"\\c");
		if (fbIndex != wstring::npos) {
			Pages[pageIndex].imageNames[imageCount].append(fbChange, 2, fbChange.size() - 14);
		//	log(Pages[pageIndex].imageNames[imageCount]);
			temp.clear();
			temp.append(fbChange, fbChange.size() - 12, 3);
			Pages[pageIndex].imageRect[imageCount].left = stof(temp);
	//		log(L"Pages[pageIndex].imageRect[imageCount].left = %f", Pages[pageIndex].imageRect[imageCount].left);
			temp.clear();
			temp.append(fbChange, fbChange.size() - 9, 3);
			Pages[pageIndex].imageRect[imageCount].top = stof(temp);
			temp.clear();
			temp.append(fbChange, fbChange.size() - 6, 3);
			Pages[pageIndex].imageRect[imageCount].right = stof(temp);
			temp.clear();
			temp.append(fbChange, fbChange.size() - 6, 3);
			Pages[pageIndex].imageRect[imageCount].bottom = stof(temp);
			++imageCount;
			if (imageCount == Pages[pageIndex].images) imageCount = 0;
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
		if (side == constants::LEFT_PAGE) {
			pLeftLayout_->SetFontStyle(DWRITE_FONT_STYLE_ITALIC,
				Pages[pageIndex].italRange[i]);
	//		pLeftLayout_->SetDrawingEffect(pRedBrush_, Pages[pageIndex].italRange[i]);
		}
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

// ---------------------  Hyperlinks  --------------------
	for (int i = 0; i < Pages[pageIndex].links; i++){
		if (side == constants::LEFT_PAGE){
			pLeftLayout_->SetDrawingEffect(pHyperLinkBrush_, Pages[pageIndex].linkRange[i]);
			if( i == linkIndex) pLeftLayout_->SetUnderline(TRUE, Pages[pageIndex].linkRange[i]);
			else  pLeftLayout_->SetUnderline( FALSE, Pages[pageIndex].linkRange[i]);
		}
		else if (side == constants::RIGHT_PAGE){
			pRightLayout_->SetDrawingEffect(pHyperLinkBrush_, Pages[pageIndex].linkRange[i]);
		   if (i == linkIndex) pRightLayout_->SetUnderline(TRUE, Pages[pageIndex].linkRange[i]);
		   else  pRightLayout_->SetUnderline(FALSE, Pages[pageIndex].linkRange[i]);
		}
	}

// ---------------------  Qlinks  --------------------
	for (int i = 0; i < Pages[pageIndex].qlinks; i++) {
		if (side == constants::LEFT_PAGE) {
			pLeftLayout_->SetDrawingEffect(pQlinkBrush_, Pages[pageIndex].qlinkRange[i]);
		}
		else if (side == constants::RIGHT_PAGE) {
			pRightLayout_->SetDrawingEffect(pQlinkBrush_, Pages[pageIndex].qlinkRange[i]);
		}
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
			pLeftLayout_->SetFontSize(fontSizeFactor*Pages[pageIndex].fontSizes[i],
				Pages[pageIndex].fontSizeRange[i]);
		if( side == constants::RIGHT_PAGE ) 
		pRightLayout_->SetFontSize(fontSizeFactor*Pages[pageIndex].fontSizes[i],
			Pages[pageIndex].fontSizeRange[i]);
	}
	return;
}

// Displays user KB input to Number Menu and returns 0 until
// user hits enter.
// On enter: returns code for Integer, Float, or Not a Number
// writes the entered string to userFloat = stof(number);
// or userInt = std::stol(number, nullptr, 10);
int ViewDW::getNumber(WPARAM charCode){
	//log(L"void ViewDW::getNumber( WPARAM charCode = %i", charCode);

	static std::wstring number;

// What about number pad input???

// User entered a number.	
	if (charCode > 47 && charCode < 58) {
		number.append(to_wstring(charCode - 48));
		menuText[1] = number;
	}

// User entered a backspace.
	else if (charCode == 8 && number.size() > 0) {
		number.erase(number.size() - 1);
		if (number.size() == 0) menuText[1] = L"#";
		else menuText[1] = number;
	}

// User entered a L".".
	else if( charCode == 190 && number.find(L".") == wstring::npos){
		number.append(L".");
		menuText[1] = number;		 
	 }

// User entered a L"-". 
	 else if( charCode == 189 && number.size() == 0){
			 number = L"-";
			 menuText[1] = number;
	 }

// User hit return.
	else if (charCode == 13) {
		if (number.size() > 0){
			if (number.size() == 1) {
				if (number == L"." || number == L"-")
				{ number = L"0";}
			}

// Might need to check some bounds here.
// If the value read is out of the range of
// representable values by an int, an out_of_range exception is thrown.
			if (number.find(L".") == wstring::npos){ 
				userInt = std::stol(number, nullptr, 10);
				//userInt = stoi(number);
				userFloat = 0.0f;
			//	log(L"int i_number = %i ", userInt);
				number.clear();
				return constants::HAVE_USER_INT;
			}
			else{ userFloat = std::stof(number, nullptr);
			//	log(L"int f_number = %f ", userFloat);
				userFloat = stof(number); 
				userInt = 0;
			//	log(L"int f_number = %f ", userFloat);
				number.clear();
				return constants::HAVE_USER_FLOAT;
			}
		 }
		number.clear();
		// User entered - but not a number.
		return constants::HAVE_USER_ENTER;
	}
	InvalidateRect(dwHandle, NULL, FALSE);
	return(0);
}


void ViewDW::getUserNumber( WPARAM findChar ){

	// User has entered - get out of Page Menu
	if( getNumber(findChar) != 0) {
		bDrawMenu = TRUE;
		bDrawPage = FALSE;
		bNumberMenu = FALSE;
		bMainMenu = TRUE;
		InvalidateRect( dwHandle, NULL, FALSE );
	}
}


void ViewDW::getPageNumber( WPARAM findChar ){

	int getNumResutlt = getNumber( findChar );

	// User has entered - get out of Page Menu
	if( getNumResutlt !=0 ){
		bDrawMenu = TRUE;
		bPageMenu = FALSE;
		bMainMenu = TRUE;
	}

	if( getNumResutlt == constants::HAVE_USER_INT )
	{
		if (userInt < 1) pageNumber = 0;
		else if( userInt >= numberOfPages) pageNumber = numberOfPages - 1;
		else pageNumber = userInt;
		viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
		bDrawPage = TRUE;
		userInt = 0;
	}

	InvalidateRect(dwHandle, NULL, FALSE);
	return;
}


void ViewDW::drawMenu(float windowWidth ) {
#ifdef DEBUG_GB	
// log(L"ViewDW::drawMenu(int windowWidth = %f)", windowWidth );
#endif


//	fontSizeFactor
	D2D1_POINT_2F origin = {0.0f, 0.0f};
	DWRITE_TEXT_RANGE tRange = {0, 0}; // tRange.startPosition; tRange.length
	menuCellWidth = ( windowWidth - constants::pageMargin )/(constants::menuCells + 0.2f );

	//if (windowWidth < maxClientSize.width/2.0f)
	//	menuFontSize = windowWidth * constants::menuFontScaleFactor;
	//else menuFontSize = resFactor*19.0f;

	menuFontSize = fontSizeFactor*16.0f;
	for( int i = 0; i < constants::menuCells; i++ ) 
	{
		SafeRelease(&pMenuLayout_);
		pDWriteFactory_->CreateTextLayout(
			(PWSTR)menuText[ i ].c_str(),
			(UINT32)lstrlen((PWSTR)menuText[ i ].c_str()),			
			pMenuFormat_,
			menuCellWidth + 20.0f,
			constants::menuHeight,  
			&pMenuLayout_ );

		if( i == iMenuCell ) pMenuBrush_->SetColor(constants::menuTextHighlightColor);
		else pMenuBrush_->SetColor(constants::menuTextColor);
		origin.x = constants::pageMargin +  i*menuCellWidth;
		tRange.length = menuText[ i ].size();
		pMenuLayout_->SetFontSize(menuFontSize, tRange);

		if (ViewDW::bColorMenu || bNumberMenu || bPageMenu )
			pMenuBrush_->SetColor(constants::menuTextHighlightColor);

		// Underline the leading letter of
		if(bMainMenu ){	
			tRange.length = 1;
			if( i == constants::PAGE || i == constants::HELP || 
				i == constants::COLOR || i == constants::NUMBER)
				pMenuLayout_->SetUnderline(TRUE, tRange);
		}

		if(bFileMenu) {
			tRange.length = 1;
			if (i == constants::LOAD || i == constants::RELOAD ||
				i == constants::CANCEL )
				pMenuLayout_->SetUnderline(TRUE, tRange);
		}

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

		RECT rc;
		GetClientRect(dwHandle, &rc);
		D2D1_SIZE_U Usize = D2D1::SizeU((rc.right - rc.left ),
										(rc.bottom - rc.top ));
		hr = pD2DFactory_->CreateHwndRenderTarget( 
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties( dwHandle, Usize ), &pRT_ );
		pRT_->CreateSolidColorBrush(D2D1::ColorF(0xFF0000), &pRedBrush_);
		pRT_->CreateSolidColorBrush(constants::paperColor, &pPaperBrush_ ); 
		pRT_->CreateSolidColorBrush(D2D1::ColorF(0xF4B480),  &pBookTextBrush_ );
		pRT_->CreateSolidColorBrush(constants::menuTextColor,  &pMenuBrush_ );
		pRT_->CreateSolidColorBrush(constants::hyperLinkBlue, &pHyperLinkBrush_);
		pRT_->CreateSolidColorBrush(constants::qLinkColor, &pQlinkBrush_);

	}

	if( FAILED( hr )) return hr;

    return S_OK;
}
void ViewDW::setPageSize(int pageNumber){
#ifdef DEBUG_GB
 //log(L"ViewDW::setPageSize(int pageNumber) = %i", pageNumber);
#endif
	
	// This seems to be a crash site so we add the following
	// check for a dangling pointer 12-12-18
	if( pRT_ ) rtSize = pRT_->GetSize();
	else{
	//	log(L"ViewDW::setPageSize if( pRT_ ) returned false");
		return;
	}

	pageArea = rtSize.width*rtSize.height;
	imageScale.height = imageScaleV*rtSize.height / maxClientSize.height;
//	imageScale.height = rtSize.height / maxClientSize.height;

	paperRect.right = rtSize.width; paperRect.bottom = rtSize.height;

	menuRect.left = constants::pageMargin;
	menuRect.right = paperRect.right - 25.0f; menuRect.bottom = constants::menuHeight;

	if (Pages[pageNumber].mode == constants::dW_gL)
	{
		// Scale font to window size
//		fontSizeFactor = 1.25f*sqrt(2.0f*(pageArea / maxClientArea));
		fontSizeFactor = resFactor*sqrt(2.0f*(pageArea / maxClientArea));

		imageScale.width = imageScaleH*2.0f*rtSize.width / maxClientSize.width;
//		imageScale.width = 2.0f*rtSize.width / maxClientSize.width;

		if (rtSize.width > 2.0f*constants::pageMargin){
			fpageWidth = rtSize.width - 2.0f*constants::pageMargin;
			menuRect.right = rtSize.width;}
		else  fpageWidth = rtSize.width;

		if (rtSize.height > 2 * constants::pageMargin)
			fpageHeight = rtSize.height - 2.0f*constants::pageMargin;
		else  fpageHeight = rtSize.height;
		return;
	}

	if( Pages[pageNumber].mode == constants::dW_dW )
	{   
		// Scale font to window size
//		fontSizeFactor = 1.25f*sqrt(pageArea/maxClientArea);
		fontSizeFactor = resFactor*sqrt(pageArea/maxClientArea);

		imageScale.width = imageScaleH*rtSize.width / maxClientSize.width;
//		imageScale.width = rtSize.width / maxClientSize.width;

		// Do not allow fpageWidth or fpageHeight to go negative  < 1  !!
		if( rtSize.width  > 3*constants::pageMargin )
			fpageWidth = rtSize.width/2.0f - 1.5f*constants::pageMargin;  
		else  fpageWidth = rtSize.width;  

		if( rtSize.height > 2*constants::pageMargin )
			fpageHeight = rtSize.height - 2.0f*constants::pageMargin;
		else  fpageHeight = rtSize.height;
		return;
	}

	if (Pages[pageNumber].mode == constants::gL_gL) {
		return;
	}

	return;
}

void ViewDW::setLeftRightLayout( int pageNumber){
#ifdef DEBUG_GB
// log(L"ViewDW::setLeftRightLayout()  , pageNumber = %i", pageNumber);
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
			// Don't go past the last page
			if( pageNumber + 1 < numberOfPages && 
				Pages[pageNumber + 1 ].mode != constants::gL_gL)
			{
		//		log(L"pageNumber + 1 < numberOfPages ");
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
 // log(L"ViewDW::createLayout(int pageNumber, int side) ");
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
//  log(L"ViewDW::drawPageNumber(int pageNumber, int side) ");
#endif

	pnum.clear();
	pnum = to_wstring( pageNumber);

	SafeRelease(&pMenuLayout_);
	pDWriteFactory_->CreateTextLayout(
		(PWSTR)pnum.c_str(),
		(UINT32)lstrlen((PWSTR)pnum.c_str()),
		pMenuFormat_,
		15.0f*static_cast<float>(pnum.size()),
		constants::menuHeight,
		&pMenuLayout_);

	// Scale the font size
	DWRITE_TEXT_RANGE tRange = { 0, 0 };
	D2D1_POINT_2F origin = { 0.0f, 0.0f };
	tRange.length = pnum.size();
	pMenuLayout_->SetFontSize(fontSizeFactor*14.0f, tRange);

	if( side == constants::RIGHT_PAGE )
		origin.x = paperRect.right - 15.0f*static_cast<float>(pnum.size());

	pRT_->DrawTextLayout(origin, pMenuLayout_, pBookTextBrush_);
	return;
}

// Pages with image formatting Ex: \cReneDescartes.jpg120320000000 will call
// drawPageImages.
// Decoding the format: 120320000000  ->  120    320    000    000
// will put the upper left corner of the image 120 pix from the left side of the
// page and 320 pix down from the top of the page.
// The last two, three digit blocks are read into Pages[pageNumber].imageRect[i] 
// Which set the width (imageRect[i].right) and height (imageRect[i].bottom)  
// of the image.  If both (imageRect[i].right) and height (imageRect[i].bottom) = 0,  
// LoadBitmapFromFile will return the width of the image given in the image file in m_pBitmap.
// If values are specified in the format block, Ex: 120 320 300 400 ;
// Then LoadBitmapFromFile will scale the file image to 300 wide x 400 high.
//
// Scaling the image to page size
// D2D1_SIZE_F imageScale is set in ViewDW::setPageSize 
//
void ViewDW::drawPageImages(int pageNumber, int side) {
#ifdef DEBUG_GB
//	log(L"ViewDW::drawPageImages(int pageNumber = %i, int side = %i) ", pageNumber, side);
#endif
	float width(0.0f);
	if (side == constants::RIGHT_PAGE) width = fpageWidth + constants::pageMargin;
	else width = 0.0f;

	D2D1_RECT_F imgRect = {0.0f, 0.0f};

	for (int i = 0; i < Pages[pageNumber].images; i++) 
	{
		imgRect.left = width + imageScale.width*Pages[pageNumber].imageRect[i].left;
		imgRect.top = imageScale.height*Pages[pageNumber].imageRect[i].top;

		imageFilePathName.clear();
		imageFilePathName = workDir;
		imageFilePathName.append(L"\\images\\");
		imageFilePathName = imageFilePathName.append(Pages[pageNumber].imageNames[i]);
		HRESULT hr =LoadBitmapFromFile(pRT_, m_pWICFactory,
			(PWSTR)imageFilePathName.c_str(),
			static_cast<int>(Pages[pageNumber].imageRect[i].right),
			static_cast<int>(Pages[pageNumber].imageRect[i].bottom),
			&m_pBitmap);

		if (SUCCEEDED(hr))
		{
		D2D1_SIZE_F bmSize = m_pBitmap->GetSize();
		pRT_->DrawBitmap(m_pBitmap, D2D1::RectF(imgRect.left, imgRect.top,
			imgRect.left + imageScale.width*bmSize.width,
			imgRect.top + imageScale.height*bmSize.height));
		}
		else
		{
			std::wstring help = L"Ooops:  Can't load \n";
			help.append(Pages[pageNumber].imageNames[i]);

			pRT_->DrawText(help.c_str(),
				(UINT32)lstrlen(help.c_str()),
				pTextFormat_, 
				D2D1::RectF(imgRect.left, imgRect.top, imgRect.left + 100.0f, imgRect.top + 50.0f ), pRedBrush_,
				D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
			help.clear();
		}
	}
}



// Invoke DW's DrawText and DrawTextLayout to put our text
// on the display.
void ViewDW::drawDW(){
#ifdef DEBUG_GB
// log(L"ViewDW::drawDW() ");
#endif
										//WPARAM wParam,   LPARAM lParam
	SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );

	if (SUCCEEDED(CreateDeviceResources()))
	{
		HRESULT hr = S_OK;

		setPageSize(pageNumber);

		SafeRelease(&m_pBitmap);

		pRT_->BeginDraw(); 

		if( bDrawPage ){

			setLeftRightLayout(pageNumber);
			pRT_->FillRectangle(&paperRect, pPaperBrush_);

			if( Pages[pageNumber].mode == constants::gL_gL ) return;
			if( Pages[pageNumber].mode == constants::dW_dW ||
				Pages[pageNumber].mode == constants::dW_gL )
			{
				drawPageNumber(pageNumber, constants::LEFT_PAGE);
				if (Pages[pageNumber].images) drawPageImages(pageNumber, constants::LEFT_PAGE);
				pRT_->DrawTextLayout( leftPageOrigin, pLeftLayout_, pBookTextBrush_ );
			}
			if( Pages[pageNumber].mode == constants::dW_dW  )
			{
				if( pageNumber + 1 < numberOfPages && 
					Pages[pageNumber + 1 ].mode != constants::gL_gL)
				{
					drawPageNumber(pageNumber + 1, constants::RIGHT_PAGE);
					rightPageOrigin.x = fpageWidth + 2.0f*constants::pageMargin;
					if (Pages[pageNumber + 1].images) drawPageImages(pageNumber + 1, constants::RIGHT_PAGE);
					pRT_->DrawTextLayout( rightPageOrigin, pRightLayout_, pBookTextBrush_ );
				}
			}
		}

		pRT_->FillRectangle(&menuRect, pPaperBrush_);
		if (bMainMenu) setMenuText(Pages[pageNumber].mode);
		if (bDrawMenu)
		{

			if (Pages[pageNumber].mode == constants::dW_dW) drawMenu(rtSize.width / 2.0f);
			else drawMenu(rtSize.width);
		}

//		drawEquation(); // under construction

		if (D2DERR_RECREATE_TARGET == pRT_->EndDraw()) DiscardDeviceResources();
	} //End if (SUCCEEDED( CreateDeviceResources())
	return;
} // End drawDW()

// Set iMenuCell = -1 if mouse is out of the menu rectangle.
// Set iMenuCell = menu item if mouse in the menu rectangle.
void ViewDW::setMenuCell(int x, int y){

	x = dpiScaleX_*x;
    y = dpiScaleY_*y;

	if (y > 2 && y < constants::menuHeight &&
		x < (dwWidth - 5) && x > 0)
	{
		if (x < 50) { iMenuCell = -1; return; }
		else { iMenuCell = ((x - 50 )/ menuCellWidth); return; }
	}

	else iMenuCell = -1; // mouse is out of menu
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
//		log(L"ViewDW::mouseMove iMenuCell > -1");
		bDrawMenu = TRUE;
 // 		if( bFileMenu == FALSE) bDrawPage = FALSE;
		InvalidateRect( dwHandle, NULL, FALSE );
		return 0;
	}

	// Mouse is out of main menu set flag
	// to remove menu in drawDW(). 
	if( bMainMenu == TRUE && bDrawMenu == TRUE)
	{
		bDrawMenu = FALSE;
		InvalidateRect(dwHandle, NULL, FALSE);
		return 0;
	}

	// if mouse is out of the menu check for links
	if (iMenuCell < 0)  // Mouse is out of the menu
	{
		bLeftButtonDown = FALSE;
		if (y > constants::pageTop && x > constants::pageMargin)
		{
			checkForLink(x, y);
		}
	}
	// Mouse is out of sub-menu; do nothing
	return 0;
	
} // end mouseMove(x,y)

void ViewDW::keyDown(int key, LPARAM lParam){
#ifdef DEBUG_GB 
//	log(L"ViewDW::lkeyDown(int key = %i, LPARAM lParam = %i, %i)",key , HIWORD(lParam), LOWORD(lParam));
#endif
	
	if( bMainMenu )
	{
		if( key == VK_DOWN || key == VK_NEXT)
		{ lButtonDown(-1, 0, constants::NEXT); return; }
		if( key == VK_UP || key == VK_PRIOR)
		{ lButtonDown(-1, 0, constants::PRIOR); return; }
		if( key == VK_END )
		{ lButtonDown(0, 0, constants::END); return; }
		if( key == VK_HOME )
		{ lButtonDown(0, 0, constants::COVER); return; }
		if( key == VK_F1 )
		{ lButtonDown(0, 0, constants::FILE); return; }
		if (key == 'C')
		{ lButtonDown(0, 0, constants::COLOR); return; }
		if (key == 'P' )
		{ lButtonDown(0, 0, constants::PAGE); return; }
		if (key == 'H')
		{ lButtonDown(0, 0, constants::HELP); return; }
		if (key == 'N') 
		{ lButtonDown(0, 0, constants::NUMBER); return; }
	}

	if( bNumberMenu ) // Check for page menu chars; 0 - 9, backspace, enter
	{
		getUserNumber( key );
		return;	
	}
	if( bPageMenu ) // Check for page menu chars; 0 - 9, backspace, enter
	{
		getPageNumber( key );
		return;
	}

	if( bFileMenu ) {
		if( key == 'R') { lButtonDown(0, 0, constants::RELOAD); return; }
		if( key == 'L') { lButtonDown(0, 0, constants::LOAD); return; }
		if( key == 'C') { lButtonDown(0, 0, constants::CANCEL); return; }

	}

	if( ViewDW::bColorMenu && key == 'C' )
		{ lButtonDown(0, 0, constants::COLOR); return; } 

	return;
}


// Display is called by ViewDW* mySelf a local instantiation of ViewDW 
// goes out of scope on return to Wrapper_To_Call_Display.  This is the
// reason for making bColorMenu, bFileMenu, bPageMenu, bMainMenu static.
// So both mySelf and viewDW know the state of menu logic.

void ViewDW::Display(glm::vec4* color) {
//	log(L"ViewDW::lButtonDown(int x = %i , int y = %i, item = %i)", x, y, item);
	//	Win::log(L"color.rgba =  %f  %f  %f  %f", color.r, color.g, color.b, color.a);
//	float colorFromGL = *color;

//	glm::vec4 colorFromGL = *color;
	colorFromGL = *color;
//	Win::log(L"ViewDW::Display colorFromGL =  %f  ", colorFromGL.r);

//  mySelf's data menbers will be used for this call
	lButtonDown(50, 50, 1);

}

//void ViewDW::Wrapper_To_Call_Display(void* pt2Object, float* color) {
void ViewDW::Wrapper_To_Call_Display(void* pt2Object, glm::vec4* color) {

	// Explicitly cast to a pointer to ViewDW
	// We are instantiating another ViewDW object here
	// it will have it's own ctor, dtor, and data members.
	// This is the reason for making ViewDW::bColorMenu and
	// ViewDW::menuText{}; static members
	ViewDW* mySelf = (ViewDW*)pt2Object; 

	// call member
	mySelf->Display(color);

}

// Checks for mouse hits on hyperlinks and Qlinks.
// If it is just a mousemove over the link it changes
// the cursor to a hand.  For hyperlinks it also underlines
// the link.
// If it is a LeftButtonDown:
//		it opens the hyperlink in a browser.
//      or, if a Qlink;  calls viewGL->hello_From_DW() 
void ViewDW::handleHitTest(int x, int y, int page){

	float fX = dpiScaleX_*static_cast<float>(x);
	float fY = dpiScaleY_*static_cast<float>(y);


	DWRITE_HIT_TEST_METRICS hitTestMetrics;
	ZeroMemory( &hitTestMetrics, sizeof( hitTestMetrics));
	BOOL isTrailingHit = false;
	BOOL isInside = false;
	DWRITE_TEXT_RANGE textRange = { 0, 0 };
	IUnknown* X = NULL;
	int checkPage(0);
	linkIndex = -1;

	if (page == constants::LEFT_PAGE) checkPage = pageNumber;
	else if (page == constants::RIGHT_PAGE) checkPage = pageNumber + 1;
 
	if (page == constants::LEFT_PAGE) {
		pLeftLayout_->HitTestPoint( fX - leftPageOrigin.x, fY - leftPageOrigin.y,
			&isTrailingHit, &isInside, &hitTestMetrics);
		pLeftLayout_->GetDrawingEffect(hitTestMetrics.textPosition, &X, &textRange);
	}
	else if (page == constants::RIGHT_PAGE) {
		pRightLayout_->HitTestPoint(fX - rightPageOrigin.x, fY - rightPageOrigin.y,
			&isTrailingHit, &isInside, &hitTestMetrics);
		pRightLayout_->GetDrawingEffect(hitTestMetrics.textPosition, &X, &textRange);
	}

// Check for Hyperlink hit.
	if (X == pHyperLinkBrush_){
		if( bLeftButtonDown ){
			link.clear();
			link.append(Pages[checkPage].text, textRange.startPosition,
			textRange.length);
			ShellExecute(0, 0, (LPWSTR)link.c_str(), 0, 0, SW_SHOWNA);
			bLeftButtonDown = FALSE; }
		else {  // this is a mouse move
			SetCursor(LoadCursor(NULL, IDC_HAND));
			for (int i = 0; i < Pages[checkPage].links; i++) {
				if (Pages[checkPage].linkRange[i].startPosition == textRange.startPosition) {
					linkIndex = i;

					// Set the underline
					bDrawPage = TRUE;
					InvalidateRect(dwHandle, NULL, FALSE); }}
		}
		bHitflag  = TRUE;
	}

// Check for Qlink hit.
	else if( X == pQlinkBrush_ ){
		SetCursor(LoadCursor(NULL, IDC_HAND));
		for (int i = 0; i < Pages[checkPage].qlinks; i++) {
			if (Pages[checkPage].qlinkRange[i].startPosition == textRange.startPosition) {
			//	qlinkIndex = i;
			}
		}
		if( bLeftButtonDown ){
		//	log(L"webhave a Qlink = %i", qlinkIndex);
			bLeftButtonDown = FALSE;
			link.clear();
			link.append( Pages[checkPage].text, textRange.startPosition,
				textRange.length );
		//	log(link);
		    viewGL->hello_From_DW( userInt, link, 1 );
		}
	}

	SafeRelease(&X);

// Remove the underline when mouse goes out of link
// but don't invalidate for every mousemove.  

	if ((linkIndex < 0 ) && (bHitflag == TRUE))
	{    
		bHitflag = FALSE;
	//	log(L"if ((linkIndex < 0 ) && (bHitflag == TRUE)) linkIndex = %i", linkIndex );
		bDrawPage = TRUE;
		InvalidateRect(dwHandle, NULL, FALSE);
	}
}

// If there are links on the page; call handleHitTest
void ViewDW::checkForLink(UINT x, UINT y) {

	float fX = dpiScaleX_*static_cast<float>(x);
	float fY = dpiScaleY_*static_cast<float>(y);


// Mouse is in left page
if (Pages[pageNumber].mode == constants::dW_gL || 
(Pages[pageNumber].mode == constants::dW_dW && fX < paperRect.right / 2.0f))
	{ 
		if (Pages[pageNumber].links > 0 ||
			Pages[pageNumber].qlinks > 0) handleHitTest(x, y, constants::LEFT_PAGE);
        return;
	}
// Mouse is in right page
	if (Pages[pageNumber].mode == constants::dW_dW && fX > rightPageOrigin.x)
	{
		// Don't go last page!
		if( pageNumber + 1 < numberOfPages ){
			if (Pages[pageNumber + 1].links > 0 ||
				Pages[pageNumber + 1].qlinks > 0 ) handleHitTest( x, y, constants::RIGHT_PAGE);
		}
	}
	return;
}



// We use this for mouse clicks and keyboard shortcuts for the 
// same menu items as the mouse menu. If item == -1 its a mouse click
// if its from the keys item will be the iMenuCell for the item
int ViewDW::lButtonDown(int x, int y, int item){
#ifdef DEBUG_GB 
//	log(L"ViewDW::lButtonDown(int x = %i , int y = %i, item = %i)", x, y, item);
#endif

	// Set iMenuCell to the cell the mouse is in
	// or return -1 if mouse is not in the menu.
	if( item == -1 ) setMenuCell(x,y);  // Mouseclick
	else iMenuCell = item;	// Keyboard input
	
	// mouse is out of menu: check for links hit.
	if( iMenuCell < 0 ){ 
		bLeftButtonDown = TRUE;
		checkForLink( x,  y);
		return 0;}  

//log(L"ViewDW::lButtonDown(iMenuCell = %i)", iMenuCell);
// Check for clicks in the main menu.	
	if( bMainMenu ){
// Handle Sub-Menu Requests
		if( iMenuCell == constants::FILE )
		{
			bFileMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			bDrawPage = FALSE;
			setMenuText( constants::FILE );
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if (iMenuCell == constants::NUMBER)
		{
		//	log(L"ViewDW::lButtonDown(iMenuCell = %i)", iMenuCell);
			bNumberMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			bDrawPage = FALSE;
			setMenuText(constants::SET_POUND_SIGN);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if (iMenuCell == constants::PAGE)
		{
			bPageMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			bDrawPage = FALSE;
			setMenuText(constants::SET_QUESTION_MARK);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::COLOR 
			&& Pages[pageNumber].mode == constants::dW_gL)
		{
			ViewDW::bColorMenu = TRUE;
			bMainMenu = FALSE;
			bDrawMenu = TRUE;
			bDrawPage = FALSE;
			setMenuText(constants::COLOR);
			viewGL->hello_From_DW(constants::PALETTE, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if (iMenuCell == constants::HELP)
		{
			bDrawMenu = FALSE;
			openDW_file(iMenuCell);
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}
		//  Handle Page Requests 
		if( iMenuCell == constants::PRIOR && pageNumber > 0)
		{
			--pageNumber;
			if (Pages[pageNumber + 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode == constants::gL_gL) {
				viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
				// We are going from a gL_gL mode window to a gL_gL mode window:
				// This is all openGL rendering no need to InvalidateRect 
				return 0;
			}

			// If we are in gL_gL mode; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size 
			// so windows will send WM_PAINT to it.

			if (Pages[pageNumber + 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode != constants::gL_gL) 
			{
				// get the cursor out of what will be the dwWin 
				moveTheCursor();
				SendMessage(mainHandle, WM_SIZE, Pages[pageNumber].mode, pageNumber);
			}
			// This is from the mouse, so the mouse is prob in the menu
			if (item == -1 ) bDrawMenu = TRUE;

			// This is from the KB so don't draw the menu
			else{ bDrawMenu = FALSE;
		//	log(L"ViewDW::lButtonDown(iMenuCell == constants::PRIOR)");
			}
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::NEXT && pageNumber < numberOfPages - 1)	   {
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send WM_PAINT to it.
			++pageNumber;
			if (Pages[pageNumber - 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode == constants::gL_gL) {
				viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
				bDrawMenu = FALSE;
				return 0;
			}

			if (Pages[pageNumber - 1].mode == constants::gL_gL &&
				Pages[pageNumber].mode != constants::gL_gL) {
				// get the cursor out of what will be the dwWin 
				moveTheCursor();
				SendMessage(mainHandle, WM_SIZE, Pages[pageNumber].mode, pageNumber);
			}

			// This is from the mouse, so the mouse is prob in the menu
			if (item == -1) bDrawMenu = TRUE;

			// This is from the KB so don't draw the menu
			else bDrawMenu = FALSE;
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::COVER && pageNumber != 0 )
		{   			
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send WM_PAINT to it.
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[0].mode != constants::gL_gL)
			{
				// get the cursor out of what will be the dwWin 
				moveTheCursor();
				pageNumber = 0;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = 0;
			bDrawMenu = FALSE;
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);	
			return 0;
		}

		if( iMenuCell == constants::TOC && pageNumber != tableOfContents)
		{
			// If we are in gL_gL,]; InvalidateRect won't send
			// WM_PAINT to dwWin, we need to give it some size
			// so windows will send WM_PAINT to it.
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[tableOfContents].mode != constants::gL_gL)
			{
				//  dwWin is zero size  the cursor out of what will be the dwWin on resize
				moveTheCursor();
				pageNumber = tableOfContents;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = tableOfContents;
			bDrawMenu = FALSE;
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}

		if( iMenuCell == constants::INDEX && pageNumber != indexPage)
		{
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[indexPage].mode != constants::gL_gL)
			{
				// get the cursor out of what will be the dwWin 
				moveTheCursor();
				pageNumber = indexPage;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = indexPage;
			bDrawMenu = FALSE;
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect(dwHandle, NULL, FALSE);
			return 0;
		}
		if( iMenuCell == constants::END 
			&& pageNumber != numberOfPages - 1 && numberOfPages > 0)
		{ 
			if(	Pages[pageNumber].mode == constants::gL_gL &&
				Pages[numberOfPages - 1].mode != constants::gL_gL)
			{
				// get the cursor out of what will be the dwWin 
				moveTheCursor();
				pageNumber = numberOfPages - 1;
				SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
			}
			pageNumber = numberOfPages - 1;
			bDrawMenu = FALSE;
			bDrawPage = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
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
			bDrawPage = TRUE;
			openDW_file( iMenuCell );
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
			InvalidateRect( dwHandle, NULL, FALSE );
			return 0;
		}
		return 0; // Clicked an inactive cell; do nothing.
	}

	if( ViewDW::bColorMenu )
	{		
		if (iMenuCell == constants::COLOR){
			ViewDW::bColorMenu = FALSE;
			bMainMenu = TRUE;
			viewGL->hello_From_DW(Pages[pageNumber].glRoutine, dummy, 0);
		}

		else if(iMenuCell == 1)
		{
			std::wstring temp;
			temp.clear();
			temp = L"R ";
			temp.append(std::to_wstring(colorFromGL.r), 0, 3);
			ViewDW::menuText[1] = temp;

			temp.clear();
			temp = L"G ";
			temp.append(std::to_wstring(colorFromGL.g), 0, 3);
			ViewDW::menuText[2] = temp;

			temp.clear();
			temp = L"B ";
			temp.append(std::to_wstring(colorFromGL.b), 0, 3);
			ViewDW::menuText[3] = temp;

			temp.clear();
			temp = L"A ";
			temp.append(std::to_wstring(colorFromGL.a), 0, 3);
			ViewDW::menuText[4] = temp;

		}
		bDrawPage = FALSE;
		bDrawMenu = TRUE;
		InvalidateRect(dwHandle, NULL, FALSE);		
	}
	return 0;
} // End lButtonDown(...


//  This is a hack to prevent a loss of ID2D1HwndRenderTarget* pRT_;
//  when transitioning out of gL_gL mode.  The dwWin has zero size in
//  gL_gL mode then does not like the mouse in the client area on resizing
//  to a finite size.
void ViewDW::moveTheCursor(){

	RECT clientRect= { 0, 0, 0, 0 };
	POINT pt = {0,0};
	::GetClientRect(mainHandle, &clientRect);
	pt.x = clientRect.right; pt.y = clientRect.bottom;
	ClientToScreen(mainHandle, &pt);
	SetCursorPos(pt.x, pt.x);
}

// This can fail when transitioning out of gL_gL mode if the mouse is in
// the client area.  void ViewDW::moveTheCursor(){ is used to prevent this. 
int ViewDW::size( int width, int height ){
#ifdef DEBUG_GB
//	log(L"ViewDW::size %ix%i", width, height );
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

	bDrawPage = TRUE;
	InvalidateRect( dwHandle, NULL, FALSE );	
	return 0;
}

// Sets std vector<wstring>menuText to display the main menu or 
// sub-menus depending on flags set in lButtonDown(), getChar().
void ViewDW::setMenuText( int parameter )
{
#ifdef DEBUG_GB
//	log(L"void ViewDW::setMenuText( parameter = %i)", parameter );
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
		menuText[10] = L"Help";
		menuText[11] = L"Number";
		menuText[12] = L" ";
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

	if (bNumberMenu) {
	//		log(L"void ViewDW::setMenuText( parameter = %i)", parameter );
		menuText[0] = L" ";
		if (parameter == constants::SET_POUND_SIGN) menuText[1] = L"#";
		else menuText[1] = to_wstring(parameter);
		menuText[2] = L" ";
		menuText[3] = L"  ";
		menuText[4] = L" ";
		menuText[5] = L" ";
		menuText[6] = L" ";
		menuText[7] = L" ";
		menuText[8] = L" ";
		menuText[9] = L" ";
		menuText[10] = L" ";
		menuText[11] = L" ";
		menuText[12] = L" ";
		return;
	}

	if( ViewDW::bColorMenu ){
//		log(L"void ViewDW::setMenuText(bColorMenu)");
		menuText[0] = L" ";
		menuText[1] = L"Red";
		menuText[2] = L"Green";
		menuText[3] = L"Blue";
		menuText[4] = L"Alpha ";
		menuText[5] = L" ";
		menuText[6] = L" ";
		menuText[7] = L" ";
		menuText[8] = L"Cancel";
		menuText[9] = L" ";
		menuText[10] = L" ";
		menuText[11] = L" ";
		menuText[12] = L" ";
		return;
	}
	return;
}


HRESULT ViewDW::LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap)
{
#ifdef DEBUG_GB
//	log(L"ViewDW::LoadBitmapFromFile( )");
#endif
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}


	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + 		D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT> (destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar *static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT> (destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar *static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
					);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
					);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}