// IPLayer.cpp: implementation of the CIPLayer class.

#include "stdafx.h"
#include "ipc.h"
#include "IPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



CIPLayer::CIPLayer( char* pName )
: CBaseLayer( pName )
{
	ResetHeader( );
}

CIPLayer::~CIPLayer()
{
}

void CIPLayer::ResetHeader()
{
	m_sHeader.ip_verlen = 0x00; // ip version		(1 byte)
	m_sHeader.ip_tos = 0x00;	// type of service	(1 byte)
	m_sHeader.ip_len = 0x0000;  // total packet length   (2 byte)
	m_sHeader.ip_id = 0x0000;   // datagram id		     (2 byte)
	m_sHeader.ip_fragoff = 0x0000; // fragment offset		   (2 bytes)
	m_sHeader.ip_ttl = 0x00;   // time to live in gateway hops		(1 byte)
	m_sHeader.ip_proto = 0x00; // IP protocol			 (1 byte)
	m_sHeader.ip_cksum = 0x00; // header checksum		 (2 bytes)
	memset( m_sHeader.ip_src, 0, 4); // IP address of source (4 bytes)
	memset( m_sHeader.ip_dst, 0, 4); // IP address of destination (4 bytes)
	memset( m_sHeader.ip_data, 0, IP_DATA_SIZE);  // variable length data
}

void CIPLayer::SetSrcIPAddress(unsigned char* src_ip) // source IP address�� ����� 4����Ʈ��ŭ ����
{ 
	memcpy( m_sHeader.ip_src, src_ip, 4);
}

void CIPLayer::SetDstIPAddress(unsigned char* dst_ip) // Destination IP address�� ����� 4����Ʈ��ŭ ����
{
	memcpy( m_sHeader.ip_dst, dst_ip, 4);
}

void CIPLayer::SetFragOff(unsigned short fragoff) //����ȭ�� �ϱ� ���� fragofff�� ����� �ʱ�ȭ
{
	m_sHeader.ip_fragoff = fragoff;
}

BOOL CIPLayer::Send(unsigned char* ppayload, int nlength)
{
	memcpy( m_sHeader.ip_data, ppayload, nlength ) ;  // ppayload�� ip_data�� nlength��ŭ ����
	
	BOOL bSuccess = FALSE ;
	bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader,nlength+IP_HEADER_SIZE); //���� ���̾�� IP�� ����� �����͸� ���� ��Ŷ�� ���� 

	return bSuccess;
}

BOOL CIPLayer::Receive(unsigned char* ppayload)
{
	PIPLayer_HEADER pFrame = (PIPLayer_HEADER) ppayload ;
	
	BOOL bSuccess = FALSE ;
	
	if(memcmp((char *)pFrame->ip_dst,(char *)m_sHeader.ip_src,4) ==0 &&		//����� dest �ּ� = �ڽ��� source �ּ�.
		memcmp((char *)pFrame->ip_src,(char *)m_sHeader.ip_src,4) !=0 &&	//����� source �ּ� != �ڽ��� source �ּ� .
		memcmp((char *)pFrame->ip_src,(char *)m_sHeader.ip_dst,4) ==0 )		//����� source �ּ� = �ڽ��� destination �ּ�.
	{
		bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->ip_data);
	}
	return bSuccess ;
}
