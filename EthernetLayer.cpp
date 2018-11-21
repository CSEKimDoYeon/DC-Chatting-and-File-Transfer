// EthernetLayer.cpp: implementation of the CEthernetLayer class.


#include "stdafx.h"
#include "ipc.h"
#include "EthernetLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CEthernetLayer::CEthernetLayer( char* pName )
: CBaseLayer( pName )
{
	// EthernetLayer를 사용하기 전 Header를 초기화.
	ResetHeader( ) ;
}

CEthernetLayer::~CEthernetLayer()
{
	// 소멸자 사용한 Ethernet memory를 제거.
}

void CEthernetLayer::ResetHeader()
{
	memset( m_sHeader.enet_dstaddr.addrs, 0, 6 ) ; // 수신 주소 memory set
	memset( m_sHeader.enet_srcaddr.addrs, 0, 6 ) ; // 송신 주소 memory set
	memset( m_sHeader.enet_data, 0, ETHER_MAX_DATA_SIZE ) ; // data정보 memory set
	m_sHeader.enet_type = 0x3412 ; // 0x0800
}

//getter
unsigned char* CEthernetLayer::GetEnetDstAddress() 
{
	return m_sHeader.enet_srcaddr.addrs;
}

//getter
unsigned char* CEthernetLayer::GetEnetSrcAddress()
{
	return m_sHeader.enet_dstaddr.addrs;
}

//setter
void CEthernetLayer::SetEnetSrcAddress(unsigned char *pAddress)
{
	// pAddress를 인자로 받아 수신 addrs에 memcpy를 6bytes만큼 해준다. addrs
	memcpy( &m_sHeader.enet_srcaddr.addrs, pAddress, 6 ) ;
}

//setter
void CEthernetLayer::SetEnetDstAddress(unsigned char *pAddress)
{
	// pAddress를 인자로 받아 수신 addrs에 memcpy를 6bytes만큼 해준다. addrs
	memcpy( &m_sHeader.enet_dstaddr.addrs, pAddress, 6 ) ;
}

BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength)
{
	memcpy( m_sHeader.enet_data, ppayload, nlength ) ;

	BOOL bSuccess = FALSE ; // FALSE로 초기화
	// Send함수에 대한 성공여부를 확인한다.
    // header를 붙여서
	bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader,nlength+ETHER_HEADER_SIZE);

	return bSuccess ;
}
	
BOOL CEthernetLayer::Receive( unsigned char* ppayload )
{
	PETHERNET_HEADER pFrame = (PETHERNET_HEADER) ppayload ; // 인자로 받은 ppayload를 pFrame에 저장

	BOOL bSuccess = FALSE ; // FALSE로 초기화

	/* int memcmp (const void * ptr1, const void * ptr2, size_t num);
	   두 개의 메모리 블록 비교.
	   0은 두 메모리가 같을 때, 0이 아닐 때는 두 메모리가 다른 값*/
    if((memcmp((char *)pFrame->enet_dstaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6)==0 &&
		memcmp((char *)pFrame->enet_srcaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6)!=0))
	{
		if(ntohs(pFrame->enet_type) == 0x1234){
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*) pFrame->enet_data);
		}
	}
	return bSuccess ;
}
