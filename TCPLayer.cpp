// TCPLayer.cpp: implementation of the CTCPLayer class.

#include "stdafx.h"
#include "ipc.h"
#include "TCPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CTCPLayer::CTCPLayer( char* pName )
: CBaseLayer( pName )
{
	ResetHeader( ) ;
}

CTCPLayer::~CTCPLayer()
{
}

void CTCPLayer::ResetHeader()       //�ʱ�ȭ
{
	m_sHeader.tcp_sport = 0x0000;   //tcp source port (2bytes)
	m_sHeader.tcp_dport = 0x0000;   //tcp destination port (2bytes)
	m_sHeader.tcp_seq = 0x00000000; //sequence number (4bytes)
	m_sHeader.tcp_ack = 0x00000000; //acknowledge sequence (4bytes)
	m_sHeader.tcp_offset = 0x00;    //no use	 (1byte)
	m_sHeader.tcp_flag = 0x00;		//control flag   (1byte)
	m_sHeader.tcp_window = 0x0000;  //no use 	 (2bytes)
	m_sHeader.tcp_cksum = 0x0000;   //check sum	 (2bytes)
	m_sHeader.tcp_urgptr = 0x0000;  //no use	 (2bytes)
	memset(m_sHeader.tcp_data,0,TCP_DATA_SIZE);
}
// ���� �󿡼� TCP Layer�� �����ڰ� ����� �� ResetHeader()�Լ��� ����Ǿ� header��.
// ������ �ʱ�ȭ (header�� ��� �ʵ��� ũ�⸦ ���ϸ� 20Bytes�� �Ǿ� header�� ũ��� ����).

void CTCPLayer::SetSourcePort(unsigned int src_port) 
{
	m_sHeader.tcp_sport = src_port; //source port
}

void CTCPLayer::SetDestinPort(unsigned int dst_port)
{
	m_sHeader.tcp_dport = dst_port;
}

BOOL CTCPLayer::Send(unsigned char* ppayload, int nlength)
{
	memcpy( m_sHeader.tcp_data, ppayload, nlength ) ;
	// ���� layer���� �Ѿ�� ppayload�� header�� tcp_data�� �־��ش�. 
	
	BOOL bSuccess = FALSE ;
	//nlength���� ����� 20 bytes�� �������� �Ѱ���
	bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader,nlength+TCP_HEADER_SIZE);	//nlength���� ����� 20 bytes�� �������� �Ѱ���
	// ����������ip���̾�� �ѱ��.
 	return bSuccess;
}

BOOL CTCPLayer::Receive(unsigned char* ppayload)
{
	PTCPLayer_HEADER pFrame = (PTCPLayer_HEADER) ppayload ; //destination port number�� Ȯ���Ͽ� demultiflexing�� ����
	
	BOOL bSuccess = FALSE;


	if(pFrame->tcp_dport == TCP_PORT_CHAT){
		bSuccess = mp_aUpperLayer[1]->Receive((unsigned char*)pFrame->tcp_data);
	}
	else if(pFrame->tcp_dport == TCP_PORT_FILE){
		bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->tcp_data);
	}
	//m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *IP ( *TCP ( *FileApp ( *ChatDlg ) *ChatApp ( *ChatDlg ) ) ) ) )");.
	// TCP Layer������ FileApp�� [0], ChatDlg�� [1]�� �ν�.

	return bSuccess ;
}