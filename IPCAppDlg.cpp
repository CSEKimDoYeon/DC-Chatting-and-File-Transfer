#include "stdafx.h"
#include "ipc.h"
#include "IPCAppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



class CAboutDlg : public CDialog
{
public:
   CAboutDlg();

   enum { IDD = IDD_ABOUTBOX };
   
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    

protected:
   DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
   
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  
END_MESSAGE_MAP()


CIPCAppDlg::CIPCAppDlg(CWnd* pParent)
   : CDialog(CIPCAppDlg::IDD, pParent), 
   CBaseLayer( "ChatDlg" ),
   m_bSendReady(FALSE)
{
   m_unSrcEnetAddr = "1";   // 초기 Source   Ethernet addr
   m_unDstEnetAddr = "2";   // 초기 Dest      Ethernet addr

   m_stMessage = _T("");
   m_filePath = _T("");

   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

   /*----------------------계층 생성------------------------*/
   m_LayerMgr.AddLayer( this ) ;   
   m_LayerMgr.AddLayer( new CNILayer( "NI" ) ) ;
   m_LayerMgr.AddLayer( new CEthernetLayer( "Ethernet" ) ) ;
   m_LayerMgr.AddLayer( new CIPLayer( "IP" ) );
   m_LayerMgr.AddLayer( new CTCPLayer( "TCP" ) );
   m_LayerMgr.AddLayer( new CChatAppLayer( "ChatApp" ) ) ;
   m_LayerMgr.AddLayer( new CFileAppLayer( "FileApp" ) ) ;

   /*----------------------계층 연결------------------------*/
   m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *IP ( *TCP ( *FileApp ( *ChatDlg ) *ChatApp ( *ChatDlg ) ) ) ) )");

   /*----------------------각 계층의 객체 할당--------------------*/
   m_ChatApp = (CChatAppLayer*)mp_UnderLayer ;
   m_TCP = (CTCPLayer *)m_LayerMgr.GetLayer("TCP");
   m_IP = (CIPLayer *)m_LayerMgr.GetLayer("IP");
   m_ETH = (CEthernetLayer *)m_LayerMgr.GetLayer("Ethernet");
   m_NI = (CNILayer *)m_LayerMgr.GetLayer("NI");//NILayer
}

void CIPCAppDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
 
   DDX_Control(pDX, IDC_LIST_CHAT, m_ListChat);
   DDX_Text(pDX, IDC_EDIT_DST, m_unDstEnetAddr);
   DDX_Text(pDX, IDC_EDIT_SRC, m_unSrcEnetAddr);
   DDX_Text(pDX, IDC_EDIT_MSG, m_stMessage);

   DDX_Control(pDX, IDC_EDIT_SRCIP, m_unSrcIPAddr);
   DDX_Control(pDX, IDC_EDIT_DSTIP, m_unDstIPAddr);
   DDX_Text(pDX, IDC_EDIT_FilePath, m_filePath);

   DDX_Control(pDX,IDC_PROGRESS, m_ProgressCtrl);
   DDX_Control(pDX, IDC_COMBO_ENETADDR, m_ComboEnetName);
}

BEGIN_MESSAGE_MAP(CIPCAppDlg, CDialog)
   ON_WM_SYSCOMMAND()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_BN_CLICKED(IDC_BUTTON_SEND, OnSendMessage)
   ON_BN_CLICKED(IDC_BUTTON_ADDR, OnButtonAddrSet)
   ON_WM_TIMER()

   ON_BN_CLICKED(IDC_BUTTON_FILE, OnAddFile)
   ON_BN_CLICKED(IDC_BUTTON_FILESEND, OnSendFile)

   ON_CBN_SELCHANGE(IDC_COMBO_ENETADDR, OnComboEnetAddr)
END_MESSAGE_MAP()


BOOL CIPCAppDlg::OnInitDialog()   //Dialog 초기화
{
   CDialog::OnInitDialog();

   ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ASSERT(IDM_ABOUTBOX < 0xF000);

   CMenu* pSysMenu = GetSystemMenu(FALSE);
   if (pSysMenu != NULL)
   {
      CString strAboutMenu;
      strAboutMenu.LoadString(IDS_ABOUTBOX);
      if (!strAboutMenu.IsEmpty())
      {
         pSysMenu->AppendMenu(MF_SEPARATOR);
         pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
   }
   SetIcon(m_hIcon, TRUE);         
   SetIcon(m_hIcon, FALSE);      

   SetRegstryMessage( ) ;
   SetDlgState(IPC_INITIALIZING);   //Dialog 초기 설정
   SetDlgState(CFT_COMBO_SET);      //COMBO_SET 설정

   return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPCAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
   if ( nID == SC_CLOSE )
   {
      if ( MessageBox( "Are you sure ?", 
         "Question", 
         MB_YESNO | MB_ICONQUESTION ) 
         == IDNO )
         return ;
      else EndofProcess( ) ;
   }

   if ((nID & 0xFFF0) == IDM_ABOUTBOX)
   {
      CAboutDlg dlgAbout;
      dlgAbout.DoModal();
   }
   else
   {
      CDialog::OnSysCommand(nID, lParam);
   }
}

void CIPCAppDlg::OnPaint() 
{
   if (IsIconic())
   {
      CPaintDC dc(this); 

      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}

HCURSOR CIPCAppDlg::OnQueryDragIcon()
{
   return (HCURSOR) m_hIcon;
}

void CIPCAppDlg::OnSendMessage()  
{
   UpdateData( TRUE ) ;

   if ( !m_stMessage.IsEmpty() )
   {
      SetTimer(1,3000,NULL);

      SendData( ) ;

      m_stMessage = "" ;

      (CEdit*) GetDlgItem( IDC_EDIT_MSG )->SetFocus( ) ;
   }
   UpdateData( FALSE ) ;
}

void CIPCAppDlg::OnButtonAddrSet()   //주소 설정 버튼 클릭
{
   UpdateData( TRUE ) ;
   unsigned char src_ip[4];
   unsigned char dst_ip[4];
   unsigned char src_mac[12];
   unsigned char dst_mac[12];


   if ( !m_unDstEnetAddr ||      //오류검출
      !m_unSrcEnetAddr  )
   {
      MessageBox( "주소를 설정 오류발생", 
         "경고", 
         MB_OK | MB_ICONSTOP ) ;

      return ;
   }

   if ( m_bSendReady ){   // 재설정
      SetDlgState( IPC_ADDR_RESET ) ;
      SetDlgState( IPC_INITIALIZING ) ;
   }
   else{      // 초기 설정 == m_bSendReady 초기값 : False ==

      /*------------  Source와 Destination의 IP 주소 받아와 설정  -----------------*/
      m_unSrcIPAddr.GetAddress(src_ip[0],src_ip[1],src_ip[2],src_ip[3]);   
      m_unDstIPAddr.GetAddress(dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);

      m_IP->SetSrcIPAddress(src_ip);
      m_IP->SetDstIPAddress(dst_ip);
      /*---------------------------------------------------------------------------*/

      /*------------  Source와 Destination의 Ethernet 주소 받아와 설정  -----------------*/
      sscanf(m_unSrcEnetAddr, "%02x%02x%02x%02x%02x%02x", &src_mac[0],&src_mac[1],&src_mac[2],&src_mac[3],&src_mac[4],&src_mac[5]);
      sscanf(m_unDstEnetAddr, "%02x%02x%02x%02x%02x%02x", &dst_mac[0],&dst_mac[1],&dst_mac[2],&dst_mac[3],&dst_mac[4],&dst_mac[5]);

      m_ETH->SetEnetSrcAddress(src_mac);
      m_ETH->SetEnetDstAddress(dst_mac);
      /*-------------------------------------------------------------------------------*/

      int nIndex = m_ComboEnetName.GetCurSel();
      m_NI->SetAdapterNumber(nIndex);      //Combo box의 index에 따라 adapter 정보 설정

      m_NI->PacketStartDriver();         //NI Layer의 Thread 동작 시작

      SetDlgState( IPC_ADDR_SET ) ;
      SetDlgState( IPC_READYTOSEND ) ;      
   }

   m_bSendReady = !m_bSendReady ;   // 초기 False -> True
}

void CIPCAppDlg::SetRegstryMessage()
{

}

void CIPCAppDlg::SendData()
{
   CString MsgHeader ; 

   MsgHeader.Format( "[%s:%s] ", m_unSrcEnetAddr, m_unDstEnetAddr ) ;   //Dialog 주소 출력 형식

   m_ListChat.AddString( MsgHeader + m_stMessage ) ;               //주소 + Message 내용 출력

   int nlength = m_stMessage.GetLength();
   unsigned char* ppayload = new unsigned char[nlength+1];            //data를 담을 ppayload 객체 생성 (크기에서 +1은 Null 문자)
   memcpy(ppayload,(unsigned char*)(LPCTSTR)m_stMessage,nlength);      //ppayload에 data 탑재
   ppayload[nlength] = '\0';

   m_ChatApp->Send(ppayload,m_stMessage.GetLength());               //ChatApp으로 Send
}

BOOL CIPCAppDlg::Receive(unsigned char *ppayload)
{
   CString Msg;
   int len_ppayload = strlen((char *)ppayload);                  //전달 받은 ppayload의 길이

   unsigned char *GetBuff = (unsigned char *)malloc(len_ppayload);      //Dialog에 출력할 Buffer 객체
   memset(GetBuff,0,len_ppayload);         //Buffer 초기화
   memcpy(GetBuff,ppayload,len_ppayload);   //Buffer에 Message 내용 입력
   GetBuff[len_ppayload] = '\0';

   Msg.Format("[%s:%s] %s",m_unDstEnetAddr,m_unSrcEnetAddr,(char *)GetBuff);   //주소 형식 + 내용

   KillTimer(1);
   m_ListChat.AddString( (char*) Msg.GetBuffer(0) ) ;   //출력
   return TRUE ;
}

BOOL CIPCAppDlg::PreTranslateMessage(MSG* pMsg) 
{
   switch( pMsg->message )
   {
   case WM_KEYDOWN :
      switch( pMsg->wParam )
      {
      case VK_RETURN : 
         if ( ::GetDlgCtrlID( ::GetFocus( ) ) == IDC_EDIT_MSG ) 
            OnSendMessage( ) ;               return FALSE ;
      case VK_ESCAPE : return FALSE ;
      }
      break ;
   }

   return CDialog::PreTranslateMessage(pMsg);
}

void CIPCAppDlg::SetDlgState(int state) // Dialog 상태에 따라 설정
{
   UpdateData( TRUE ) ;
   int i;
   CString device_description;

   CButton*   pSendButton = (CButton*) GetDlgItem( IDC_BUTTON_SEND ) ;
   CButton*   pSetAddrButton = (CButton*) GetDlgItem( IDC_BUTTON_ADDR ) ;
   CButton*   pFileSearchButton = (CButton*) GetDlgItem( IDC_BUTTON_FILE ) ;
   CButton*   pFileSendButton = (CButton*) GetDlgItem( IDC_BUTTON_FILESEND ) ;

   CEdit*      pMsgEdit = (CEdit*) GetDlgItem( IDC_EDIT_MSG ) ;
   CEdit*      pSrcEdit = (CEdit*) GetDlgItem( IDC_EDIT_SRC ) ;
   CEdit*      pDstEdit = (CEdit*) GetDlgItem( IDC_EDIT_DST ) ;
   CEdit*      pSrcIPEdit = (CEdit*) GetDlgItem( IDC_EDIT_SRCIP ) ;
   CEdit*      pDstIPEdit = (CEdit*) GetDlgItem( IDC_EDIT_DSTIP ) ;
   CEdit*      pFilePathEdit = (CEdit*) GetDlgItem( IDC_EDIT_FilePath ) ;

   CComboBox*   pEnetNameCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ENETADDR);

   switch( state )   //Dialog 상태
   {
   case IPC_INITIALIZING : // 첫 화면 세팅
      pSendButton->EnableWindow( FALSE ) ;
      pMsgEdit->EnableWindow( FALSE ) ;
      m_ListChat.EnableWindow( FALSE ) ;
      pFilePathEdit->EnableWindow( FALSE );
      pFileSearchButton->EnableWindow( FALSE );
      pFileSendButton->EnableWindow( FALSE );
      break ;
   case IPC_READYTOSEND : // Send(S)버튼을 눌렀을 때 세팅
      pSendButton->EnableWindow( TRUE ) ;
      pMsgEdit->EnableWindow( TRUE ) ;
      m_ListChat.EnableWindow( TRUE ) ;
      pFilePathEdit->EnableWindow( TRUE );
      pFileSearchButton->EnableWindow( TRUE );
      pFileSendButton->EnableWindow( TRUE );
      break ;
   case IPC_WAITFORACK :   break ;
   case IPC_ERROR :      break ;
   case IPC_UNICASTMODE :
      m_unDstEnetAddr.Format("%.2x%.2x%.2x%.2x%.2x%.2x",0x00,0x00,0x00,0x00,0x00,0x01) ;
      pDstEdit->EnableWindow( TRUE ) ;
      break ;
   case IPC_ADDR_SET :   // 설정(&O)버튼을 눌렀을 때
      pSetAddrButton->SetWindowText( "재설정(&R)" ) ; 
      pSrcEdit->EnableWindow( FALSE ) ;
      pDstEdit->EnableWindow( FALSE ) ;
      pSrcIPEdit->EnableWindow( FALSE );
      pDstIPEdit->EnableWindow( FALSE );
      pEnetNameCombo->EnableWindow( FALSE );
      m_NI->m_thrdSwitch = TRUE;
      break ;
   case IPC_ADDR_RESET : // 재설정(&R)버튼을 눌렀을 때
      pSetAddrButton->SetWindowText( "설정(&O)" ) ; 
      pSrcEdit->EnableWindow( TRUE ) ;
      pDstEdit->EnableWindow( TRUE ) ;
      pSrcIPEdit->EnableWindow( TRUE );
      pDstIPEdit->EnableWindow( TRUE );
      pEnetNameCombo->EnableWindow( TRUE );
      m_NI->m_thrdSwitch = FALSE;
      break ;
   case CFT_COMBO_SET :   //COMBO_SET
      for(i=0;i<NI_COUNT_NIC;i++){
         if(!m_NI->GetAdapterObject(i))
            break;
         device_description = m_NI->GetAdapterObject(i)->description;   //네트워크 adapter device들의 이름을 받아옴
         device_description.Trim();                     //문자열 양 쪽에 존재하는 공백을 제거
         pEnetNameCombo->AddString(device_description);      //device 이름을 Combo box에 넣어주기 위해 AddString
         pEnetNameCombo->SetCurSel(0);                  //SetCurSel을 이용해 Combo box에 넣어줌

         PPACKET_OID_DATA OidData;   //네트워크 카드에 대한 MAC 주소의 정보를 담기 위한 객체
         OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));   //메모리 할당
         OidData->Oid = 0x01010101;   
         OidData->Length = 6;

         LPADAPTER adapter = PacketOpenAdapter(m_NI->GetAdapterObject(0)->name);      //adapter의 정보를 받아와 설정.
         PacketRequest(adapter, FALSE, OidData);      //MAC 주소의 정보를 OIDData에 넣어줌.

         m_unSrcEnetAddr.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;
         //MAC 주소를 문자열 형식에 맞추어 Ethernet Source 주소를 설정.
      }
      break;
   }

   UpdateData( FALSE ) ;
}

void CIPCAppDlg::EndofProcess()
{
   m_LayerMgr.DeAllocLayer( ) ;
}

void CIPCAppDlg::OnTimer(UINT nIDEvent) 
{

   KillTimer( 1 ) ;

   CDialog::OnTimer(nIDEvent);
}

void CIPCAppDlg::OnAddFile()
{
   UpdateData(TRUE);

   CFileDialog dlg( true, "*.*", NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, "All Files(*.*)|*.*|", NULL );
   if( dlg.DoModal() == IDOK )
   {
      m_filePath = dlg.GetPathName();
   }
   UpdateData(FALSE);
}


void CIPCAppDlg::OnSendFile()
{

   ((CFileAppLayer*)GetUnderLayer())->Send( (unsigned char*)(LPCTSTR)m_filePath );
}

void CIPCAppDlg::OnComboEnetAddr()
{
   UpdateData(TRUE);

   int nIndex = m_ComboEnetName.GetCurSel();
   m_NI->GetAdapterObject(nIndex)->name;

   PPACKET_OID_DATA OidData;
   OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
   OidData->Oid = 0x01010101;
   OidData->Length = 6;

   LPADAPTER adapter = PacketOpenAdapter(m_NI->GetAdapterObject(nIndex)->name);
   PacketRequest(adapter, FALSE, OidData);

   m_unSrcEnetAddr.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;

   UpdateData(FALSE);
}

void CIPCAppDlg::OnOffFileButton(BOOL bBool)
{
   CButton* pFileSendButton = (CButton*) GetDlgItem( IDC_BUTTON_FILESEND ) ;
   CButton* pFileSearchButton = (CButton*) GetDlgItem( IDC_BUTTON_FILE ) ;
   pFileSendButton->EnableWindow(bBool);
   pFileSearchButton->EnableWindow(bBool);
}
