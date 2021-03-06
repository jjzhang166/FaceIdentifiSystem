#include "stdafx.h"
#include "Camera.h"
#include "TiCapture2.h"
#include <Poco/Buffer.h>
#include <Poco/NotificationCenter.h>
#include "CaptureNotification.h"

using Poco::NotificationCenter;
using Poco::Buffer;

static void MirrorDIB(const char* lpDIBBits, long lWidth, long lHeight, bool bDirection, int nImageBits);

#pragma warning(disable:4800)

Camera::Camera()
:_width(640), _height(480), isRunning(false)
{
	SCapParam cameraParam;
	memset(&cameraParam, 0, sizeof(SCapParam));

	cameraParam.szCap.cx = _width;
	cameraParam.szCap.cy = _height;
	cameraParam.eType = CAP_WDM;
	cameraParam.lIndex = 0;

	_camera = CCapture::Create(&cameraParam);
	poco_check_ptr(_camera);
	bool ret = _camera->InitCapture();
	poco_assert(ret);
}


Camera::~Camera()
{
	poco_check_ptr(_camera);
	delete _camera;
	_camera = 0;
}


void Camera::Open()
{
	poco_check_ptr(_camera);
	if (!isRunning)
	{
		isRunning = _camera->Play();
		poco_assert(isRunning);
	}
}

void Camera::Close()
{
	poco_check_ptr(_camera);
	if (isRunning)
	{
		_camera->Stop();
	}
}

void Camera::SetQuality(SCapQuality* pQuality)
{
	poco_check_ptr(_camera);
	bool ret = _camera->SetQuality(pQuality);
	poco_assert(ret);
}

size_t Camera::width() const
{
	return _width;
}

size_t Camera::height() const
{
	return _height;
}

#include <sstream>

void Camera::GetFrame()
{
	if (!isRunning) return;

	poco_check_ptr(_camera);

	long len = _width* _height * 3;
	char* data = new char[len];
	
	bool ret = _camera->GetFrame((BYTE*)data, len);
	if(!ret) return;
	MirrorDIB(data, _width, _height, true,24);

	Picture::Ptr pic(new Picture(data, len));
	poco_check_ptr(pic);
	pic->SetHeight(_height);
	pic->SetWidth(_width);
	NotificationCenter::defaultCenter().postNotification(new CaptureNotification(pic));

	delete data;
}

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
void MirrorDIB(const char* lpDIBBits, long lWidth, long lHeight, bool bDirection, int nImageBits)
{
	LPSTR	lpSrc;
	LPSTR	lpDst;
	LPSTR	lpBits;
	HLOCAL	hBits;
	LONG	i;
	LONG	j;
	int nBits;
	LONG lLineBytes;
	lLineBytes = WIDTHBYTES(lWidth *nImageBits);
	hBits = LocalAlloc(LHND, lLineBytes);
	if (hBits == NULL) return;

	lpBits = (char *)LocalLock(hBits);
	int nStep = nImageBits / 8;
	long lCenter = lWidth / 2 * nStep;

	if (bDirection)
	{
		for (i = 0; i < lHeight; i++)
		{
			for (j = 0; j < lCenter; j += nStep)
			{
				for (nBits = 0; nBits<nStep; nBits++)
				{
					lpSrc = (char *)lpDIBBits + lLineBytes * i + lCenter - j + nBits;
					lpDst = (char *)lpDIBBits + lLineBytes * i + lCenter + j + nBits;
					*lpBits = *lpDst;
					*lpDst = *lpSrc;
					*lpSrc = *lpBits;
				}

			}

		}
	}
	else
	{
		for (i = 0; i < lHeight / 2; i++)
		{
			lpSrc = (char *)lpDIBBits + lLineBytes * i;
			lpDst = (char *)lpDIBBits + lLineBytes * (lHeight - i - 1);
			memcpy(lpBits, lpDst, lLineBytes);
			memcpy(lpDst, lpSrc, lLineBytes);
			memcpy(lpSrc, lpBits, lLineBytes);

		}
	}
	LocalUnlock(hBits);
	LocalFree(hBits);
}