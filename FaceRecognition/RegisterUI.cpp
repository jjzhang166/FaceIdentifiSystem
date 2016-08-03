#include "stdafx.h"
#include "RegisterUI.h"
#include "CaptureNotification.h"

#include "Camera.h"

RegisterUI::RegisterUI()
:m_photo_agin(false), r(new Camera)
{
	addObserver(*this);
}


RegisterUI::~RegisterUI()
{
	removeObserver(*this);
}

DUI_BEGIN_MESSAGE_MAP(RegisterUI, WindowImplBase)
DUI_ON_CLICK_CTRNAME(BT_CLOSERWND, OnCloseRWnd)
DUI_ON_CLICK_CTRNAME(BT_GETPHOTO, OnGetPhoto)
DUI_ON_CLICK_CTRNAME(BT_OK_REGISTER, OnFilishi)
DUI_END_MESSAGE_MAP()

LPCTSTR RegisterUI::GetWindowClassName() const
{
	return _T("RegisterUI");
}

CDuiString RegisterUI::GetSkinFolder()
{
	return _T("skin");
}

CDuiString RegisterUI::GetSkinFile()
{
	return _T("xml\\RegisterUI.xml");
}

void RegisterUI::OnFinalMessage(HWND hWnd)
{
	r.stop();
	WindowImplBase::OnFinalMessage(hWnd);
}

void RegisterUI::Notify(TNotifyUI& msg)
{
	WindowImplBase::Notify(msg);
}

void RegisterUI::OnCloseRWnd(TNotifyUI& msg)
{
	Close();
}

void RegisterUI::InitWindow()
{	
	r.start();
}

void RegisterUI::OnFilishi(TNotifyUI& msg)
{
	CVerticalLayoutUI* vLyt = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("lab_lyt")));
	CLabelUI* lab_Prompt = dynamic_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_Prompt")));
	bool bRet = SaveRegisterInfo();
	if (!bRet)
	{
		CDialogBuilder builder;
		CLabelUI* lab = (CLabelUI*)(builder.Create(_T("xml//labUI1.xml"), (UINT)0, NULL, &m_PaintManager));
		vLyt->Remove(lab_Prompt);
		vLyt->Add(lab);
		return;
	}
	if (!m_photo_agin)
	{
		CDialogBuilder builder;
		CLabelUI* lab = (CLabelUI*)(builder.Create(_T("xml//labUI2.xml"), (UINT)0, NULL, &m_PaintManager));
		vLyt->Remove(lab_Prompt);
		vLyt->Add(lab);
		return;
	}
	Close();
}

void RegisterUI::OnGetPhoto(TNotifyUI& msg)
{
	CVerticalLayoutUI* vLyt = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("vLyt")));
	CButtonUI* bt_photo = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("photo")));
	if (m_photo_agin)
	{
		CDialogBuilder builder;
		CLabelUI* btn = (CLabelUI*)(builder.Create(_T("xml//labUI3.xml"), (UINT)0, NULL, &m_PaintManager));
		vLyt->Remove(bt_photo);
		vLyt->Add(btn);
		m_photo_agin = false;
	}
	else
	{
		CDialogBuilder builder;
		CLabelUI* btn = (CLabelUI*)(builder.Create(_T("xml//labUI4.xml"), (UINT)0, NULL, &m_PaintManager));
		vLyt->Remove(bt_photo);
		vLyt->Add(btn);
		m_photo_agin = true;
	}
}

bool RegisterUI::SaveRegisterInfo()
{
	CVerticalLayoutUI* photo_lyt = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("photo_wnd")));
	CEditUI* edit_name = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_Name")));
	CEditUI* edit_age = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_Age")));
	CComboUI* combo_sex = dynamic_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo_sex")));
	CEditUI* edit_birth = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_Birth")));
	CEditUI* edit_address = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_Address")));
	CEditUI* edit_phone = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_Phone")));
	CEditUI* edit_CertID = dynamic_cast<CEditUI*>(m_PaintManager.FindControl(_T("Edit_IDnumber")));

	IdentityInfo* Item = new IdentityInfo;
	Item->strName = edit_name->GetText();
	Item->strAge = edit_age->GetText();
	Item->strSex = combo_sex->GetText();
	Item->strBirth = edit_birth->GetText();
	Item->strAdress = edit_address->GetText();
	Item->strPhone = edit_phone->GetText();
	Item->strCertID = edit_CertID->GetText();
	Item->strPhotoInfo = photo_lyt->GetBkImage();

	if (Item->strName == _T("") || Item->strAge == _T("") || Item->strSex == _T("") || Item->strBirth == _T("")
		|| Item->strAdress == _T("") || Item->strPhone == _T("") || Item->strCertID == _T(""))
	{
		return false;
	}

	return true;
}

#include <Poco/AutoPtr.h>

using Poco::AutoPtr;

void RegisterUI::handle1(Poco::Notification* pNf)
{
	if (m_photo_agin) return;

	poco_check_ptr(pNf);
	//CaptureNotify::handle1(pNf);
	Notification::Ptr pf(pNf);
	poco_check_ptr(pf.get());
	CaptureNotification::Ptr nf = pf.cast<CaptureNotification>();
	poco_check_ptr(nf.get());
	Picture::Ptr pic(nf->data());
	poco_check_ptr(pic.get());

	//CVerticalLayoutUI* photo_Lyt = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("photo_wnd")));
	CVerticalLayoutUI* Image = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("photo_wnd")));
	poco_check_ptr(Image);
	COLORREF* data = (COLORREF*)pic->data();
	
	CDuiString name = "carmera";
	HDC PaintDC = ::GetDC(GetHWND());
	HDC hChildMemDC = ::CreateCompatibleDC(PaintDC);
	HBITMAP hBitmap = CRenderEngine::CreateARGB32Bitmap(hChildMemDC, pic->width(), pic->height(), &data);

	HDC hdcStill = ::GetDC(GetHWND());
	PAINTSTRUCT ps;
	::BeginPaint(GetHWND(), &ps);


	::SetStretchBltMode(hdcStill, COLORONCOLOR);

	BITMAPINFOHEADER bih;

	bih.biSize = 40; 						// header size
	bih.biWidth = pic->width();
	bih.biHeight = pic->height();
	bih.biPlanes = 1;
	bih.biBitCount = 24;					// RGB encoded, 24 bit
	bih.biCompression = BI_RGB;			// no compression
	bih.biSizeImage = bih.biWidth * bih.biHeight * 3;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	BITMAPINFO bi;
	memset(&bi, 0, sizeof(bi));
	memcpy(&(bi.bmiHeader), &bih, sizeof(BITMAPINFOHEADER));
	int iWidth = bih.biWidth;
	int iHeight = bih.biHeight;

	::StretchDIBits(hdcStill, Image->GetX(), Image->GetY(), Image->GetWidth(), Image->GetHeight(),
		0, 0, pic->width(), pic->height(), pic->data(), &bi,
		DIB_RGB_COLORS, SRCCOPY);


	::EndPaint(GetHWND(), &ps);
	::ReleaseDC(GetHWND(), hdcStill);
}