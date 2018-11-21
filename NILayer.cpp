// NILayer.cpp: implementation of the CNILayer class.


#include "stdafx.h"
#include "ipc.h"
#include "NILayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CNILayer::CNILayer( char *pName, LPADAPTER *pAdapterObject, int iNumAdapter )
	: CBaseLayer( pName )
{
	m_AdapterObject = NULL;
	m_iNumAdapter = iNumAdapter;
	m_thrdSwitch = TRUE;
	SetAdapterList(NULL);
	//CNILayer 생성자
}

CNILayer::~CNILayer()
{
}

void CNILayer::PacketStartDriver()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	//에러를 저장할 배열.

	if(m_iNumAdapter == -1){
		//선택이 아무것도 안됐을 때, 아무 장치도 찾지 못했을 때.
		AfxMessageBox("Not exist NICard");
		return;
	}
	
	m_AdapterObject = pcap_open_live(m_pAdapterList[m_iNumAdapter]->name,1500,PCAP_OPENFLAG_PROMISCUOUS,2000,errbuf);
	// 디바이스 open , m_pAdapterList에서 m_iNumAdapter번째의 어댑터 사용
	// pcap_open_live( 장치의 이름, 캡쳐할 크기(1500은 이더넷의 최대 단위), Promiscuous mode를 사용할 지, 
	//패킷이 도착했을 때 패킷이 잠시 기다리는 시간, errbuf 에러가 발생했을 때 에러 원인을 저장/ 성공하면 Null값 저장)

	if(!m_AdapterObject){
		//디바이스 못 열면
		AfxMessageBox(errbuf);
		//에러 메시지 출력 후 종료
		return;
	}

	AfxBeginThread(ReadingThread, this);
	//첫번째 인자는 시작할 thread, 두번째 인자는 미리 정의,선언한 클래스의 포인터
	//보통 두번 째 인자는 첫번째 인자의 파라미터로 들어감
	//NILayer 자체.
}

pcap_if_t *CNILayer::GetAdapterObject(int iIndex)
{
	return m_pAdapterList[iIndex];
	// getter
}

void CNILayer::SetAdapterNumber(int iNum)
{
	m_iNumAdapter = iNum;
	//setter
}

void CNILayer::SetAdapterList(LPADAPTER *plist)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;
	
	char errbuf[PCAP_ERRBUF_SIZE];
	// 에러 원인을 저장할 배열

	for(int j=0;j<NI_COUNT_NIC;j++)
	{
		m_pAdapterList[j] = NULL;
		// 최대 갯수 10개로 이미 되어 있음
		// 10개(0부터 9까지) Null로 초기화.
	}

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		// 말그대로 모든 디바이스 찾는거
		// (디바이스 리스트 포인터, 에러버퍼)
		// 0: 성공, -1: 실패
		AfxMessageBox("Not exist NICard");
		return;
	}
	if(!alldevs)
	{
		//디바이스가 없을 때
		AfxMessageBox("Not exist NICard");
		return;
	}

	for(d=alldevs; d; d=d->next)
	{
		//리스트에 찾은 디바이스 저장
		m_pAdapterList[i++] = d;
	}
}

BOOL CNILayer::Send(unsigned char *ppayload, int nlength)
{
	if(pcap_sendpacket(m_AdapterObject,ppayload,nlength))
	{
		// (,버퍼,사이즈)
		// 0: 성공, 1: 실패
		AfxMessageBox("패킷 전송 실패");
		return FALSE;
	}
	return TRUE;
}

BOOL CNILayer::Receive( unsigned char* ppayload )
{
	BOOL bSuccess = FALSE;
	//초기값을 일단 false
	bSuccess = mp_aUpperLayer[0]->Receive(ppayload);
	//Receive 성공하면 true, 아니면 false.
	return bSuccess;
}

UINT CNILayer::ReadingThread(LPVOID pParam)
{
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	int result;

	AfxBeginThread(FileTransferThread, (LPVOID)pParam);
	//FileTransferThread 생성, nParam은 이전의 this.

	CNILayer *pNI = (CNILayer *)pParam;

	while(pNI->m_thrdSwitch){
		result = pcap_next_ex(pNI->m_AdapterObject, &header, &pkt_data);
		// 오프라인 캡쳐 또는 인터페이스로 부터 패킷을 읽음.
		// (패킷을 가진 어댑터의 포인터, 패킷 헤더, 패킷 데이터)
		// 1: 성공, 0: timeout, -1:에러, -2:EOF

		if(result==0){
		}
		else if(result==1){
			pNI->Receive((u_char *)pkt_data);
			// 패킷을 받으면 위의 레이어로 전달 
		}
		else if(result<0){
		}
	}

	return 0;
}

UINT CNILayer::FileTransferThread(LPVOID pParam)
{
	CNILayer *pNI = (CNILayer *)pParam;

	return 0;
}