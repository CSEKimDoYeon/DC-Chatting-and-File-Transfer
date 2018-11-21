// IPCAppDlg.h : header file
//

#if !defined(AFX_IPCAPPDLG_H__FE9C37CB_4C90_4772_99AD_50A9B24BCE62__INCLUDED_)
#define AFX_IPCAPPDLG_H__FE9C37CB_4C90_4772_99AD_50A9B24BCE62__INCLUDED_

#include "LayerManager.h"	// Added by ClassView
#include "ChatAppLayer.h"	// Added by ClassView
#include "FileAppLayer.h"	// Added by ClassView
#include "EthernetLayer.h"	// Added by ClassView
#include "IPLayer.h"		// Added by ClassView
#include "TCPLayer.h"		// Added by ClassView
#include "NILayer.h"		// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIPCAppDlg dialog

class CIPCAppDlg 
: public CDialog, 
  public CBaseLayer
{
// Construction
public:
	CIPCAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIPCAppDlg)
	enum { IDD = IDD_IPC_DIALOG };
	CListBox		m_ListChat;		// Chatting Message View
	CString			m_unSrcEnetAddr;// Source Ethernet Address
	CString			m_unDstEnetAddr;// Destination Ethernet Address
	CString			m_stMessage;	// Chatting Message String

	CIPAddressCtrl	m_unSrcIPAddr;	// Source IP Address
	CIPAddressCtrl	m_unDstIPAddr;	// Destination IP Address
	CString			m_filePath;		// File Path
	CProgressCtrl	m_ProgressCtrl;	// Progress bar for File transfer

	CComboBox		m_ComboEnetName;// ComboBox for Ethernet Name List
	//}}AFX_DATA

	// 파일 send 버튼 Control
	afx_msg void OnOffFileButton(BOOL bBool);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIPCAppDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIPCAppDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnSendMessage();
	afx_msg void OnButtonAddrSet();
	//afx_msg void OnCheckBroadcast();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnAddFile();
	afx_msg void OnSendFile();
	afx_msg	void OnComboEnetAddr();

	

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL			Receive( unsigned char* ppayload );
	inline void		SendData( );

private:
	CLayerManager	m_LayerMgr;

	enum {			IPC_INITIALIZING, 
					IPC_READYTOSEND, 
					IPC_WAITFORACK,
					IPC_ERROR,
					//IPC_BROADCASTMODE,
					IPC_UNICASTMODE,
					IPC_ADDR_SET,
					IPC_ADDR_RESET,
					CFT_COMBO_SET } ;

	void			SetDlgState( int state );
	inline void		EndofProcess( );
	inline void		SetRegstryMessage( );

	BOOL			m_bSendReady;

// Object App
	CChatAppLayer*	m_ChatApp;
	CTCPLayer*		m_TCP;
	CIPLayer*		m_IP;
	CEthernetLayer*	m_ETH;
	CNILayer*		m_NI;

// Implementation
	UINT			m_wParam;
	DWORD			m_lParam;
public:
	afx_msg void OnEnChangeEditDst();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IPCAPPDLG_H__FE9C37CB_4C90_4772_99AD_50A9B24BCE62__INCLUDED_) //
