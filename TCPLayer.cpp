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

void CTCPLayer::ResetHeader()       //초기화
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
// 구현 상에서 TCP Layer의 생성자가 수행될 때 ResetHeader()함수가 실행되어 header의.
// 값들을 초기화 (header의 모든 필드의 크기를 더하면 20Bytes가 되어 header의 크기와 같음).

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
	// 상위 layer에서 넘어온 ppayload를 header의 tcp_data에 넣어준다. 
	
	BOOL bSuccess = FALSE ;
	//nlength에는 헤더의 20 bytes가 더해져서 넘겨짐
	bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader,nlength+TCP_HEADER_SIZE);	//nlength에는 헤더의 20 bytes가 더해져서 넘겨짐
	// 하위계층인ip레이어로 넘긴다.
 	return bSuccess;
}

BOOL CTCPLayer::Receive(unsigned char* ppayload)
{
	PTCPLayer_HEADER pFrame = (PTCPLayer_HEADER) ppayload ; //destination port number를 확인하여 demultiflexing을 수행
	
	BOOL bSuccess = FALSE;


	if(pFrame->tcp_dport == TCP_PORT_CHAT){
		bSuccess = mp_aUpperLayer[1]->Receive((unsigned char*)pFrame->tcp_data);
	}
	else if(pFrame->tcp_dport == TCP_PORT_FILE){
		bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->tcp_data);
	}
	//m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *IP ( *TCP ( *FileApp ( *ChatDlg ) *ChatApp ( *ChatDlg ) ) ) ) )");.
	// TCP Layer에서는 FileApp이 [0], ChatDlg이 [1]로 인식.

	return bSuccess ;
}