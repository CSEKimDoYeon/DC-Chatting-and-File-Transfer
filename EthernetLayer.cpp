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
	// EthernetLayer�� ����ϱ� �� Header�� �ʱ�ȭ.
	ResetHeader( ) ;
}

CEthernetLayer::~CEthernetLayer()
{
	// �Ҹ��� ����� Ethernet memory�� ����.
}

void CEthernetLayer::ResetHeader()
{
	memset( m_sHeader.enet_dstaddr.addrs, 0, 6 ) ; // ���� �ּ� memory set
	memset( m_sHeader.enet_srcaddr.addrs, 0, 6 ) ; // �۽� �ּ� memory set
	memset( m_sHeader.enet_data, 0, ETHER_MAX_DATA_SIZE ) ; // data���� memory set
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
	// pAddress�� ���ڷ� �޾� ���� addrs�� memcpy�� 6bytes��ŭ ���ش�. addrs
	memcpy( &m_sHeader.enet_srcaddr.addrs, pAddress, 6 ) ;
}

//setter
void CEthernetLayer::SetEnetDstAddress(unsigned char *pAddress)
{
	// pAddress�� ���ڷ� �޾� ���� addrs�� memcpy�� 6bytes��ŭ ���ش�. addrs
	memcpy( &m_sHeader.enet_dstaddr.addrs, pAddress, 6 ) ;
}

BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength)
{
	memcpy( m_sHeader.enet_data, ppayload, nlength ) ;

	BOOL bSuccess = FALSE ; // FALSE�� �ʱ�ȭ
	// Send�Լ��� ���� �������θ� Ȯ���Ѵ�.
    // header�� �ٿ���
	bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader,nlength+ETHER_HEADER_SIZE);

	return bSuccess ;
}
	
BOOL CEthernetLayer::Receive( unsigned char* ppayload )
{
	PETHERNET_HEADER pFrame = (PETHERNET_HEADER) ppayload ; // ���ڷ� ���� ppayload�� pFrame�� ����

	BOOL bSuccess = FALSE ; // FALSE�� �ʱ�ȭ

	/* int memcmp (const void * ptr1, const void * ptr2, size_t num);
	   �� ���� �޸� ��� ��.
	   0�� �� �޸𸮰� ���� ��, 0�� �ƴ� ���� �� �޸𸮰� �ٸ� ��*/
    if((memcmp((char *)pFrame->enet_dstaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6)==0 &&
		memcmp((char *)pFrame->enet_srcaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6)!=0))
	{
		if(ntohs(pFrame->enet_type) == 0x1234){
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*) pFrame->enet_data);
		}
	}
	return bSuccess ;
}
