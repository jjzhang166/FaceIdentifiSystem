#include "stdafx.h"
#include "MonitoringUI.h"

#include "Poco/Delegate.h"
#include "Picture.h"
#include "BitMapCompare.h"
#include "CaptureNotification.h"


CMonitoringUI::CMonitoringUI()
	:m_nBmp(0), r(new Camera)
{
	m_testID = 100001;
	m_pCompare = new BitMapCompare(this);
	Poco::ThreadPool::defaultPool().start(*m_pCompare);

	addObserver(*this);
	r.start();
}


CMonitoringUI::~CMonitoringUI()
{
	m_theEvent += Poco::delegate(m_pCompare, &BitMapCompare::onEvent);
	fireEvent(true);
	m_theEvent -= Poco::delegate(m_pCompare, &BitMapCompare::onEvent);
	//Poco::ThreadPool::defaultPool().joinAll();
	delete m_pCompare;

	removeObserver(*this);
	r.stop();
}

DUI_BEGIN_MESSAGE_MAP(CMonitoringUI, WindowImplBase)
DUI_ON_CLICK_CTRNAME(BT_CLOSE_MonWnd, OnCloseWnd)
DUI_END_MESSAGE_MAP()

LPCTSTR CMonitoringUI::GetWindowClassName() const
{
	return _T("CMonitoringUI");
}

CDuiString CMonitoringUI::GetSkinFolder()
{
	return _T("skin");
}

CDuiString CMonitoringUI::GetSkinFile()
{
	return _T("xml\\MonitoringUI.xml");
}

void CMonitoringUI::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
}

void CMonitoringUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK && msg.pSender->GetName() == _T("test"))
	{
	//	ShowMonitInfoList();
	}
	WindowImplBase::Notify(msg);
}

void CMonitoringUI::OnCloseWnd(TNotifyUI& msg)
{
	Close();
}

void CMonitoringUI::InitWindow()
{
	
}

void CMonitoringUI::ShowMonitInfoList()
{
	CDialogBuilder builder;
	CListContainerElementUI* SubList = (CListContainerElementUI*)(builder.Create(_T("xml//RegInfoList.xml"), (UINT)0, NULL, &m_PaintManager));
	CListUI* pList = dynamic_cast<CListUI*>(m_PaintManager.FindControl(_T("Refer_InfoList")));
	pList->Add(SubList);
	CControlUI* photoUI = dynamic_cast<CControlUI*>(m_PaintManager.FindSubControlByClass(SubList, DUI_CTR_CONTROL));
	CLabelUI* lab_ID = dynamic_cast<CLabelUI*>(m_PaintManager.FindSubControlByClass(SubList, DUI_CTR_LABEL, 1));
	CLabelUI* lab_similarity = dynamic_cast<CLabelUI*>(m_PaintManager.FindSubControlByClass(SubList, DUI_CTR_LABEL, 3));
	CLabelUI* lab_time = dynamic_cast<CLabelUI*>(m_PaintManager.FindSubControlByClass(SubList, DUI_CTR_LABEL, 5));

	std::string strName = std::string(_T("file = 'bmp/test")) + std::to_string(m_nBmp) + std::string(".bmp'");
	photoUI->SetBkImage(strName.c_str());
	lab_ID->SetText(std::to_string(m_testID).c_str());
	lab_similarity->SetText("76%");
	lab_time->SetText("2016-7-29 17:12:55");

	if (m_testID % 3 == 0)
	{
		SubList->SetBkColor(0xFFFFDDDD);
	}
	m_testID++;
}

std::queue<CapBitmapData>& CMonitoringUI::getCapDataQueue()
{
	return m_capdata;
}

void CMonitoringUI::handle1(Poco::Notification* pNf)
{
	poco_check_ptr(pNf);
	Picture *pImg = NULL;
	//CaptureNotify::handle1(pNf);
	//CaptureNotify::handle1(pNf, &pImg);
	Notification::Ptr pf(pNf);
	poco_check_ptr(pf.get());
	CaptureNotification::Ptr nf = pf.cast<CaptureNotification>();
	poco_check_ptr(nf.get());
	Picture::Ptr pic(nf->data());
	poco_check_ptr(pic.get());
	pImg = pic.get();
	CapBitmapData capdata((const BYTE *)pImg->data(), pImg->width() * pImg->height() * 3, pImg->width(), pImg->height());
	Camera::MirrorDIB((const char *)capdata.getData(), capdata.getWidth(), capdata.getHeigth(), false, 24);
	m_capdata.push(capdata);		
	//delete pImg;
}