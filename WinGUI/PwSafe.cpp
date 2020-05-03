/*
  KeePass Password Safe - The Open-Source Password Manager
  Copyright (C) 2003-2006 Dominik Reichl <dominik.reichl@t-online.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "StdAfx.h"
#include "PwSafe.h"
#include "PwSafeDlg.h"

#include "../KeePassLibCpp/Util/TranslateEx.h"
#include "../KeePassLibCpp/Util/MemUtil.h"
#include "Util/CmdLine/CmdArgs.h"
#include "Util/CmdLine/Executable.h"
#include "Util/PrivateConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT g_uThreadACP = 0;
static BOOL g_bForceSimpleAsterisks = FALSE;
static TCHAR g_pFontNameNormal[12];
static TCHAR g_pFontNameSymbol[8];

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPwSafeApp, CWinApp)
	//{{AFX_MSG_MAP(CPwSafeApp)
	//}}AFX_MSG

	// ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CPwSafeApp::CPwSafeApp()
{
	_tcscpy_s(g_pFontNameNormal, _countof(g_pFontNameNormal), _T("MS Serif"));
	_tcscpy_s(g_pFontNameSymbol, _countof(g_pFontNameSymbol), _T("Symbol"));
}

/////////////////////////////////////////////////////////////////////////////

CPwSafeApp theApp;

/////////////////////////////////////////////////////////////////////////////

BOOL CPwSafeApp::InitInstance()
{
#if (_MFC_VER < 0x0500)
#ifdef _AFXDLL
	Enable3dControls();
#else
	Enable3dControlsStatic();
#endif
#endif

	// Create application's mutex object to make our presence public
	m_pAppMutex = new CMutex(FALSE, _T("KeePassApplicationMutex"), NULL);
	if(m_pAppMutex == NULL) { ASSERT(FALSE); }

	VERIFY(AfxOleInit());
	AfxEnableControlContainer();

#ifndef _UNICODE
	AfxInitRichEdit();
#else
	AfxInitRichEditEx();
#endif	

	InitCommonControls();

	// SetDialogBkColor(NewGUI_GetBgColor(), CR_FRONT); // Setup the "new" dialog look

	ASSERT(TRUE == 1); ASSERT(FALSE == 0);

	g_uThreadACP = GetACP();

	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	if((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) // Windows 2000
		g_bForceSimpleAsterisks = TRUE;

	CPwSafeDlg dlg;
	m_pMainWnd = &dlg;

	CPrivateConfig *pc = new CPrivateConfig(FALSE);
	if(pc != NULL)
	{
		dlg.m_bCheckForInstance = pc->GetBool(PWMKEY_SINGLEINSTANCE, FALSE);
		delete pc; pc = NULL;
	}

	if(dlg.m_bCheckForInstance == TRUE)
	{
		dlg.m_instanceChecker.ActivateChecker();

		if(dlg.m_instanceChecker.PreviousInstanceRunning())
		{
			/* CString strFile;
			LPCTSTR lpPassword = NULL;
			LPCTSTR lpKeyFile = NULL;
			LPCTSTR lpPreSelectPath = NULL;
			DWORD dwData = 0;

			ParseCurrentCommandLine(&strFile, &lpPassword, &lpKeyFile, &lpPreSelectPath);

			if(strFile.GetLength() != 0)
			{
				if(lpPassword != NULL)
				{
					dwData |= (DWORD)_tcslen(lpPassword) << 16;
					strFile = CString(lpPassword) + strFile;
				}

				if(lpKeyFile != NULL)
				{
					dwData |= (DWORD)_tcslen(lpKeyFile);
					strFile = CString(lpKeyFile) + strFile;
				}

				dlg.m_instanceChecker.ActivatePreviousInstance((LPCTSTR)strFile, dwData);
			}
			else dlg.m_instanceChecker.ActivatePreviousInstance(_T(""), 0xF0FFFFF0); */

			const FullPathName& database = CmdArgs::instance().getDatabase();
			if(database.getState()==FullPathName::PATH_AND_FILENAME)			
			{
				std_string string(database.getFullPathName());
				DWORD dwData = 0;
				if(!CmdArgs::instance().getPassword().empty())
				{
					dwData |= (DWORD)CmdArgs::instance().getPassword().length() << 16;
					string = CmdArgs::instance().getPassword() + string;
				}


				const FullPathName& keyfile = CmdArgs::instance().getKeyfile();
				enum {PATH_EXISTS = FullPathName::PATH_ONLY | FullPathName::PATH_AND_FILENAME};
				if(keyfile.getState() & PATH_EXISTS && !CmdArgs::instance().preselectIsInEffect())                
				{
					dwData |= (DWORD)keyfile.getFullPathName().length();
					string = keyfile.getFullPathName() + string;
				}

				dlg.m_instanceChecker.ActivatePreviousInstance(string.c_str(), dwData);
			}
			else dlg.m_instanceChecker.ActivatePreviousInstance(_T(""), 0xF0FFFFF0);

			m_pMainWnd = NULL;
			return FALSE;
		}
	}

	int nResponse = dlg.DoModal();

	if(nResponse == IDOK) { }
	else if(nResponse == IDCANCEL) { }

	return FALSE;
}

int CPwSafeApp::ExitInstance()
{
	// Release application's mutex object
	if(m_pAppMutex != NULL)
	{
		m_pAppMutex->Unlock();
		delete m_pAppMutex;
		m_pAppMutex = NULL;
	}

	return CWinApp::ExitInstance();
}

BOOL CPwSafeApp::RegisterShellAssociation()
{
	LONG l;
	HKEY hBase, hShell, hTemp, hTemp2;
	// TCHAR tszTemp[MAX_PATH * 2];
	// TCHAR tszMe[MAX_PATH * 2];
	DWORD dw;

	// VERIFY(GetModuleFileName(NULL, tszMe, MAX_PATH * 2 - 2) != 0);
	std::string strMe = Executable::instance().getFullPathName();

	// HKEY_CLASSES_ROOT/.kdb

	l = RegCreateKey(HKEY_CLASSES_ROOT, _T(".kdb"), &hBase);
	if(l != ERROR_SUCCESS) return FALSE;

	std::string strTemp = "kdbfile";
	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hBase, _T(""), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hBase); return FALSE; }

	RegCloseKey(hBase);

	// HKEY_CLASSES_ROOT/kdbfile

	l = RegCreateKey(HKEY_CLASSES_ROOT, _T("kdbfile"), &hBase);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) return FALSE;

	// _tcscpy_s(tszTemp, _countof(tszTemp), TRL("KeePass Password Database"));
	strTemp = TRL("KeePass Password Database");

	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hBase, _T(""), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hBase); return FALSE; }

	// _tcscpy_s(tszTemp, _countof(tszTemp), _T(""));
	strTemp = "";

	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hBase, _T("AlwaysShowExt"), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hBase); return FALSE; }

	l = RegCreateKey(hBase, _T("DefaultIcon"), &hTemp);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) return FALSE;

	// _tcscpy_s(tszTemp, _countof(tszTemp), tszMe);
	strTemp = strMe;
	// _tcscat_s(tszTemp, _countof(tszTemp), _T(",0"));
	strTemp += ",0";
	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hTemp, _T(""), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hTemp); RegCloseKey(hBase); return FALSE; }

	RegCloseKey(hTemp);

	// HKEY_CLASSES_ROOT/kdbfile/shell

	l = RegCreateKey(hBase, _T("shell"), &hShell);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) return FALSE;

	// HKEY_CLASSES_ROOT/kdbfile/shell/open

	l = RegCreateKey(hShell, _T("open"), &hTemp);

	// _tcscpy_s(tszTemp, _countof(tszTemp), TRL("&Open with KeePass"));
	strTemp = TRL("&Open with KeePass");
	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hTemp, _T(""), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hTemp); RegCloseKey(hShell); RegCloseKey(hBase); return FALSE; }

	l = RegCreateKey(hTemp, _T("command"), &hTemp2);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) return FALSE;

	// _tcscpy_s(tszTemp, _countof(tszTemp), _T("\""));
	strTemp = "\"";
	// _tcscat_s(tszTemp, _countof(tszTemp), tszMe);
	strTemp += strMe;
	// _tcscat_s(tszTemp, _countof(tszTemp), _T("\" \"%1\""));
	strTemp += "\" \"%1\"";
	dw = (strTemp.length() + 1) * sizeof(TCHAR);
	l = RegSetValueEx(hTemp2, _T(""), 0, REG_SZ, (CONST BYTE *)strTemp.c_str(), dw);
	ASSERT(l == ERROR_SUCCESS); if(l != ERROR_SUCCESS) { RegCloseKey(hTemp); RegCloseKey(hShell); RegCloseKey(hBase); return FALSE; }

	VERIFY(RegCloseKey(hTemp2) == ERROR_SUCCESS);
	VERIFY(RegCloseKey(hTemp) == ERROR_SUCCESS);

	VERIFY(RegCloseKey(hShell) == ERROR_SUCCESS);

	VERIFY(RegCloseKey(hBase) == ERROR_SUCCESS);

	return TRUE;
}

BOOL CPwSafeApp::UnregisterShellAssociation()
{
	HKEY hBase, hShell, hOpen, hCommand;
	LONG l;

	l = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T(".kdb"), 0, KEY_WRITE, &hBase);
	if(l != ERROR_SUCCESS) return FALSE;

	RegDeleteValue(hBase, _T(""));
	VERIFY(RegCloseKey(hBase) == ERROR_SUCCESS);

	VERIFY(RegDeleteKey(HKEY_CLASSES_ROOT, _T(".kdb")) == ERROR_SUCCESS);

	l = RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("kdbfile"), 0, KEY_WRITE, &hBase);
	if(l != ERROR_SUCCESS) return FALSE;

	l = RegOpenKeyEx(hBase, _T("shell"), 0, KEY_WRITE, &hShell);
	if(l != ERROR_SUCCESS) return FALSE;

	l = RegOpenKeyEx(hShell, _T("open"), 0, KEY_WRITE, &hOpen);
	if(l != ERROR_SUCCESS) return FALSE;

	l = RegOpenKeyEx(hOpen, _T("command"), 0, KEY_WRITE, &hCommand);
	if(l != ERROR_SUCCESS) return FALSE;

	RegDeleteValue(hCommand, _T(""));
	VERIFY(RegCloseKey(hCommand) == ERROR_SUCCESS);

	RegDeleteValue(hOpen, _T(""));
	VERIFY(RegCloseKey(hOpen) == ERROR_SUCCESS);

	RegDeleteValue(hShell, _T(""));
	VERIFY(RegCloseKey(hShell) == ERROR_SUCCESS);

	RegDeleteValue(hBase, _T(""));
	RegDeleteValue(hBase, _T("AlwaysShowExt"));
	VERIFY(RegCloseKey(hBase) == ERROR_SUCCESS);

	return TRUE;
}

BOOL CPwSafeApp::GetStartWithWindows()
{
	HKEY h = NULL;
	LONG l;
	TCHAR tszBuf[512];
	DWORD dwSize = 510;

	l = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &h);
	if(l != ERROR_SUCCESS) return FALSE;

	DWORD dwType = REG_SZ;
	if(RegQueryValueEx(h, _T("KeePass Password Safe"), NULL, &dwType, (LPBYTE)tszBuf, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(h); h = NULL;
		return FALSE;
	}

	RegCloseKey(h); h = NULL;

	if((_tcslen(tszBuf) > 0) && (tszBuf[0] != _T('-'))) return TRUE;
	return FALSE;
}

BOOL CPwSafeApp::SetStartWithWindows(BOOL bAutoStart)
{
	HKEY h = NULL;
	// TCHAR tszBuf[512];
	LONG l;

	l = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_WRITE, &h);
	if(l != ERROR_SUCCESS) return FALSE;

	if(bAutoStart == TRUE)
	{
		// GetModuleFileName(NULL, tszBuf, 510);
		std::string strBuf = Executable::instance().getFullPathName();

		DWORD dwSize = (strBuf.length() + 1) * sizeof(TCHAR);
		l = RegSetValueEx(h, _T("KeePass Password Safe"), 0, REG_SZ, (LPBYTE)strBuf.c_str(), dwSize);
		if(l != ERROR_SUCCESS)
		{
			RegCloseKey(h); h = NULL;
			return FALSE;
		}
	}
	else // bAutoStart == FALSE)
	{
		l = RegDeleteValue(h, _T("KeePass Password Safe"));
		if(l != ERROR_SUCCESS)
		{
			RegCloseKey(h); h = NULL;
			return FALSE;
		}
	}

	RegCloseKey(h); h = NULL;
	return TRUE;
}

/* BOOL CPwSafeApp::ParseCurrentCommandLine(CString *psFile, LPCTSTR *lpPassword, LPCTSTR *lpKeyFile, LPCTSTR *lpPreSelectPath)
{
	long i;
	BOOL bFirst = TRUE;

	ASSERT(psFile != NULL); if(psFile == NULL) return FALSE;
	psFile->Empty();

	ASSERT(lpPassword != NULL); if(lpPassword == NULL) return FALSE;
	*lpPassword = NULL;
	ASSERT(lpKeyFile != NULL); if(lpKeyFile == NULL) return FALSE;
	*lpKeyFile = NULL;
	ASSERT(lpPreSelectPath != NULL); if(lpPreSelectPath == NULL) return FALSE;
	*lpPreSelectPath = NULL;

	if(__argc <= 1) return FALSE;

	for(i = 1; i < (long)__argc; i++)
	{
		if((_tcsnicmp(__targv[i], _T("-pw:"), 4) == 0) && (_tcslen(__targv[i]) > 4))
			*lpPassword = &__targv[i][4];
		else if((_tcsnicmp(__targv[i], _T("/pw:"), 4) == 0) && (_tcslen(__targv[i]) > 4))
			*lpPassword = &__targv[i][4];
		else if((_tcsnicmp(__targv[i], _T("-keyfile:"), 9) == 0) && (_tcslen(__targv[i]) > 9))
			*lpKeyFile = &__targv[i][9];
		else if((_tcsnicmp(__targv[i], _T("/keyfile:"), 9) == 0) && (_tcslen(__targv[i]) > 9))
			*lpKeyFile = &__targv[i][9];
		else if((_tcsnicmp(__targv[i], _T("-preselect:"), 11) == 0) && (_tcslen(__targv[i]) > 11))
			*lpPreSelectPath = &__targv[i][11];
		else if((_tcsnicmp(__targv[i], _T("/preselect:"), 11) == 0) && (_tcslen(__targv[i]) > 11))
			*lpPreSelectPath = &__targv[i][11];
		else if((_tcsnicmp(__targv[i], _T("-ext:"), 5) == 0) && (_tcslen(__targv[i]) > 5))
		{ // Ignore this parameter
		}
		else if((_tcsnicmp(__targv[i], _T("/ext:"), 5) == 0) && (_tcslen(__targv[i]) > 5))
		{ // Ignore this parameter
		}
		else
		{
			if(bFirst != TRUE) *psFile += _T(" ");
			*psFile += __targv[i];
			bFirst = FALSE;
		}
	}

	psFile->TrimLeft(); psFile->TrimRight();

	if(psFile->GetLength() == 0) return FALSE;
	if(psFile->Left(1) == _T("\"")) *psFile = psFile->Right(psFile->GetLength() - 1);
	if(psFile->GetLength() == 0) return FALSE;
	psFile->TrimLeft(); psFile->TrimRight();
	if(psFile->GetLength() == 0) return FALSE;
	if(psFile->Right(1) == _T("\"")) *psFile = psFile->Left(psFile->GetLength() - 1);
	if(psFile->GetLength() == 0) return FALSE;
	psFile->TrimLeft(); psFile->TrimRight();
	if(psFile->GetLength() == 0) return FALSE;

	return TRUE;
} */

void CPwSafeApp::CreateHiColorImageList(CImageList *pImageList, WORD wResourceID, int czSize)
{
	ASSERT(pImageList != NULL); if(pImageList == NULL) return;

	CBitmap bmpImages;
	VERIFY(bmpImages.LoadBitmap(MAKEINTRESOURCE(wResourceID)));

	VERIFY(pImageList->Create(czSize, czSize, ILC_COLOR24 | ILC_MASK, bmpImages.GetBitmapDimension().cx / czSize, 0));
	pImageList->Add(&bmpImages, RGB(255,0,255));

	bmpImages.DeleteObject();
}

BOOL CPwSafeApp::IsMBThreadACP()
{
	if((g_uThreadACP == 932) || (g_uThreadACP == 936) || (g_uThreadACP == 950)) return TRUE;
	return FALSE;
}

#pragma warning(push)
#pragma warning(disable: 4310) // Type cast shortens constant value

TCHAR CPwSafeApp::GetPasswordCharacter()
{
	if((IsMBThreadACP() == TRUE) || (g_bForceSimpleAsterisks == TRUE))
		return _T('*');
	return (TCHAR)0xB7;
}

#pragma warning(pop)

LPCTSTR CPwSafeApp::GetPasswordFont()
{
	if((IsMBThreadACP() == TRUE) || (g_bForceSimpleAsterisks == TRUE))
		return g_pFontNameNormal;
	return (TCHAR *)g_pFontNameSymbol;
}