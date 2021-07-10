
// launch3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "launch3.h"
#include "launch3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


PROCESS_INFORMATION ModProcInfo;
STARTUPINFOA ModStartupInfo;


typedef BOOL(WINAPI PFNCREATEPROCESSA)(
	_In_opt_ LPCSTR lpApplicationName,
	_Inout_opt_ LPSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles,
	_In_ DWORD dwCreationFlags,
	_In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCSTR lpCurrentDirectory,
	_In_ LPSTARTUPINFOA lpStartupInfo,
	_Out_ LPPROCESS_INFORMATION lpProcessInformation);


void LaunchMod(PCHAR szPath, PCHAR szCmdLine);
DWORD GetStartAddress();
BOOL CreateProcessAndInjectDll(PCHAR szGamePath, PCHAR szCmdLine, LPSECURITY_ATTRIBUTES pProcAttrs,
	LPSECURITY_ATTRIBUTES pThreadAttrs, BOOL bInheritHandles, DWORD dwFlags, LPVOID pEnv, PCHAR szDir,
	LPSTARTUPINFOA pStartupInfo, LPPROCESS_INFORMATION pProcInfo, PCHAR szDLLPath, PFNCREATEPROCESSA pfnCreateProcessA);
BOOL InjectDLL(HANDLE hProcess, HANDLE hThread,
	DWORD dwStartAddress, PCHAR szDllPath, DWORD dwSize);
BYTE* MovEAX(BYTE* pAtAddress, DWORD dwEAX);
BYTE* MovEBX(BYTE* pAtAddress, DWORD dwEBX);
BYTE* MovECX(BYTE* pAtAddress, DWORD dwECX);
BYTE* MovEDX(BYTE* pAtAddress, DWORD dwEDX);
BYTE* MovESI(BYTE* pAtAddress, DWORD dwESI);
BYTE* MovEDI(BYTE* pAtAddress, DWORD dwEDI);
BYTE* MovEBP(BYTE* pAtAddress, DWORD dwEBP);
BYTE* MovESP(BYTE* pAtAddress, DWORD dwESP);
BYTE* PushValue(BYTE* pAtAddress, DWORD dwValue);
BYTE* MovEIP(BYTE* pAtAddress, DWORD dwEIP, DWORD dwNewEIP);
BYTE* CallProcedure(BYTE* pAtAddress, DWORD dwAddress, DWORD dwNewAddress);


/////////////////////////////////////////////////////////////////////////////
// Claunch3Dlg dialog


Claunch3Dlg::Claunch3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LAUNCH3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Claunch3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Claunch3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, &Claunch3Dlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &Claunch3Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON2, &Claunch3Dlg::OnBnClickedButton2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void LaunchMod(PCHAR szPath, PCHAR szCmdLine)
{
	char szGamePath[MAX_PATH];
	char szDllPath[MAX_PATH];

	sprintf(szGamePath, "%s\\%s", szPath, "gta_sa.exe");
	sprintf(szDllPath, "%s\\%s", szPath, "samp.dll");

	memset(&ModProcInfo, 0, sizeof(PROCESS_INFORMATION));
	memset(&ModStartupInfo, 0, sizeof(STARTUPINFOA));
	ModStartupInfo.cb = sizeof(STARTUPINFOA);

	BOOL bResult = CreateProcessAndInjectDll(
					szGamePath,
					szCmdLine,
					NULL,
					NULL,
					FALSE,
					CREATE_DEFAULT_ERROR_MODE,
					NULL,
					szPath,
					&ModStartupInfo,
					&ModProcInfo,
					szDllPath,
					NULL);

	if (!bResult)
	{
		MessageBoxA(NULL, "Initialization failed.\r\nPlease reinstall.", "SA:MP", MB_OK | MB_ICONERROR | MB_HELP);
	}
}

/////////////////////////////////////////////////////////////////////////////

CString GetAppPath()
{
TCHAR app_path[_MAX_PATH];

GetModuleFileName((HMODULE)AfxGetApp()->m_hInstance, app_path, MAX_PATH);
CString app_str = app_path;
app_str = app_str.Left(app_str.ReverseFind('\\')+1);

return app_str;
}

/////////////////////////////////////////////////////////////////////////////
// Claunch3Dlg message handlers

BOOL Claunch3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Claunch3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Claunch3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/////////////////////////////////////////////////////////////////////////////
// Event for "Exit" button

void Claunch3Dlg::OnBnClickedButton3()
{
	CDialogEx::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
// Event for "Connect localhost" button

void Claunch3Dlg::OnBnClickedButton4()
{
	CString app_path = GetAppPath();
	LaunchMod((PCHAR)app_path.GetString(), "-c -h 127.0.0.1 -p 7777 -n Player");
}

/////////////////////////////////////////////////////////////////////////////
// Event for "Launch Debug" button

void Claunch3Dlg::OnBnClickedButton2()
{
	CString app_path = GetAppPath();
	LaunchMod((PCHAR)app_path.GetString(), "-d");
}

/////////////////////////////////////////////////////////////////////////////

#pragma warning(disable : 6387) // bla-bla, could be 0, blah...
DWORD GetStartAddress()
{
	return (DWORD)GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryA");
}

/////////////////////////////////////////////////////////////////////////////

BOOL CreateProcessAndInjectDll(PCHAR szGamePath, PCHAR szCmdLine, LPSECURITY_ATTRIBUTES pProcAttrs,
	LPSECURITY_ATTRIBUTES pThreadAttrs, BOOL bInheritHandles, DWORD dwFlags, LPVOID pEnv, PCHAR szDir,
	LPSTARTUPINFOA pStartupInfo, LPPROCESS_INFORMATION pProcInfo, PCHAR szDLLPath, PFNCREATEPROCESSA pfnCreateProcessA)
{
	PROCESS_INFORMATION pi;
	DWORD dwSize;
	DWORD dwStartAddr;
	DWORD dwCreationFlags;

	if (!pfnCreateProcessA)
		pfnCreateProcessA = CreateProcessA;

	dwCreationFlags = dwFlags | CREATE_SUSPENDED;

	if (pfnCreateProcessA(szGamePath, szCmdLine, pProcAttrs, pThreadAttrs,
		bInheritHandles, dwCreationFlags, pEnv, szDir, pStartupInfo, &pi))
	{
		if (szDLLPath)
			dwSize = strlen(szDLLPath) + 1;
		else
			dwSize = 0;

		dwStartAddr = GetStartAddress();

		if (InjectDLL(pi.hProcess, pi.hThread, dwStartAddr, szDLLPath, dwSize))
		{
			if (pProcInfo)
				memcpy(pProcInfo, &pi, sizeof(PROCESS_INFORMATION));

			if (!(dwFlags & CREATE_SUSPENDED))
				ResumeThread(pi.hThread);

			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL InjectDLL(HANDLE hProcess, HANDLE hThread,
	DWORD dwStartAddress, PCHAR szDllPath, DWORD dwSize)
{
	SIZE_T BytesWritten = 0;
	CONTEXT context;
	BYTE ByteBuffer[0x480];
	BYTE* pByteAt;
	DWORD dwOldProtect = 0;
	BYTE* pESPAddress;
	BOOL bSuccess = FALSE;

	SuspendThread(hThread);
	
	memset(&context, 0, sizeof(CONTEXT));
	context.ContextFlags = CONTEXT_FULL;

	if (GetThreadContext(hThread, &context))
	{
		pESPAddress = (BYTE*)((context.Esp - 0x480) & 0xFFFFFFE0);

		pByteAt = &ByteBuffer[0];

		if (szDllPath)
		{
			memcpy(ByteBuffer + 128, szDllPath, dwSize);

			pByteAt = PushValue(pByteAt, (DWORD)pESPAddress + 128);
			pByteAt = CallProcedure(pByteAt, dwStartAddress,
				(DWORD)pESPAddress + (DWORD)pByteAt - (DWORD)&ByteBuffer);

		}

		pByteAt = MovEAX(pByteAt, context.Eax);
		pByteAt = MovEBX(pByteAt, context.Ebx);
		pByteAt = MovECX(pByteAt, context.Ecx);
		pByteAt = MovEDX(pByteAt, context.Edx);
		pByteAt = MovESI(pByteAt, context.Esi);
		pByteAt = MovEDI(pByteAt, context.Edi);
		pByteAt = MovEBP(pByteAt, context.Ebp);
		pByteAt = MovESP(pByteAt, context.Esp);

		pByteAt = MovEIP(pByteAt, context.Eip,
			(DWORD)pESPAddress + (DWORD)pByteAt - (DWORD)&ByteBuffer);

		context.Esp = (DWORD)(pESPAddress - 4);
		context.Eip = (DWORD)pESPAddress;

		if (VirtualProtectEx(hProcess, pESPAddress, 0x480, PAGE_EXECUTE_READWRITE, &dwOldProtect) &&
			WriteProcessMemory(hProcess, pESPAddress, &ByteBuffer, 0x480, &BytesWritten) &&
			FlushInstructionCache(hProcess, pESPAddress, 0x480) &&
			SetThreadContext(hThread, &context))
		{
			bSuccess = TRUE;
		}
	}

	ResumeThread(hThread);
	
	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEAX(BYTE* pAtAddress, DWORD dwEAX)
{
	*pAtAddress = 0xB8;
	*(DWORD*)(pAtAddress + 1) = dwEAX;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEBX(BYTE* pAtAddress, DWORD dwEBX)
{
	*pAtAddress = 0xBB;
	*(DWORD*)(pAtAddress + 1) = dwEBX;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovECX(BYTE* pAtAddress, DWORD dwECX)
{
	*pAtAddress = 0xB9;
	*(DWORD*)(pAtAddress + 1) = dwECX;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEDX(BYTE* pAtAddress, DWORD dwEDX)
{
	*pAtAddress = 0xBA;
	*(DWORD*)(pAtAddress + 1) = dwEDX;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovESI(BYTE* pAtAddress, DWORD dwESI)
{
	*pAtAddress = 0xBE;
	*(DWORD*)(pAtAddress + 1) = dwESI;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEDI(BYTE* pAtAddress, DWORD dwEDI)
{
	*pAtAddress = 0xBF;
	*(DWORD*)(pAtAddress + 1) = dwEDI;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEBP(BYTE* pAtAddress, DWORD dwEBP)
{
	*pAtAddress = 0xBD;
	*(DWORD*)(pAtAddress + 1) = dwEBP;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovESP(BYTE* pAtAddress, DWORD dwESP)
{
	*pAtAddress = 0xBC;
	*(DWORD*)(pAtAddress + 1) = dwESP;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* PushValue(BYTE* pAtAddress, DWORD dwValue)
{
	*pAtAddress = 0x68;
	*(DWORD*)(pAtAddress + 1) = dwValue;
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* MovEIP(BYTE* pAtAddress, DWORD dwEIP, DWORD dwNewEIP)
{
	if (!dwNewEIP)
		dwNewEIP = (DWORD)pAtAddress;

	*pAtAddress = 0xE9;
	*(DWORD*)(pAtAddress + 1) = dwEIP - (dwNewEIP + 5);
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////

BYTE* CallProcedure(BYTE* pAtAddress, DWORD dwAddress, DWORD dwNewAddress)
{
	if (!dwNewAddress)
		dwNewAddress = (DWORD)pAtAddress;

	*pAtAddress = 0xE8;
	*(DWORD*)(pAtAddress + 1) = dwAddress - (dwNewAddress + 5);
	return pAtAddress + 5;
}

/////////////////////////////////////////////////////////////////////////////