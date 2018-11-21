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
	//CNILayer ������
}

CNILayer::~CNILayer()
{
}

void CNILayer::PacketStartDriver()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	//������ ������ �迭.

	if(m_iNumAdapter == -1){
		//������ �ƹ��͵� �ȵ��� ��, �ƹ� ��ġ�� ã�� ������ ��.
		AfxMessageBox("Not exist NICard");
		return;
	}
	
	m_AdapterObject = pcap_open_live(m_pAdapterList[m_iNumAdapter]->name,1500,PCAP_OPENFLAG_PROMISCUOUS,2000,errbuf);
	// ����̽� open , m_pAdapterList���� m_iNumAdapter��°�� ����� ���
	// pcap_open_live( ��ġ�� �̸�, ĸ���� ũ��(1500�� �̴����� �ִ� ����), Promiscuous mode�� ����� ��, 
	//��Ŷ�� �������� �� ��Ŷ�� ��� ��ٸ��� �ð�, errbuf ������ �߻����� �� ���� ������ ����/ �����ϸ� Null�� ����)

	if(!m_AdapterObject){
		//����̽� �� ����
		AfxMessageBox(errbuf);
		//���� �޽��� ��� �� ����
		return;
	}

	AfxBeginThread(ReadingThread, this);
	//ù��° ���ڴ� ������ thread, �ι�° ���ڴ� �̸� ����,������ Ŭ������ ������
	//���� �ι� ° ���ڴ� ù��° ������ �Ķ���ͷ� ��
	//NILayer ��ü.
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
	// ���� ������ ������ �迭

	for(int j=0;j<NI_COUNT_NIC;j++)
	{
		m_pAdapterList[j] = NULL;
		// �ִ� ���� 10���� �̹� �Ǿ� ����
		// 10��(0���� 9����) Null�� �ʱ�ȭ.
	}

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		// ���״�� ��� ����̽� ã�°�
		// (����̽� ����Ʈ ������, ��������)
		// 0: ����, -1: ����
		AfxMessageBox("Not exist NICard");
		return;
	}
	if(!alldevs)
	{
		//����̽��� ���� ��
		AfxMessageBox("Not exist NICard");
		return;
	}

	for(d=alldevs; d; d=d->next)
	{
		//����Ʈ�� ã�� ����̽� ����
		m_pAdapterList[i++] = d;
	}
}

BOOL CNILayer::Send(unsigned char *ppayload, int nlength)
{
	if(pcap_sendpacket(m_AdapterObject,ppayload,nlength))
	{
		// (,����,������)
		// 0: ����, 1: ����
		AfxMessageBox("��Ŷ ���� ����");
		return FALSE;
	}
	return TRUE;
}

BOOL CNILayer::Receive( unsigned char* ppayload )
{
	BOOL bSuccess = FALSE;
	//�ʱⰪ�� �ϴ� false
	bSuccess = mp_aUpperLayer[0]->Receive(ppayload);
	//Receive �����ϸ� true, �ƴϸ� false.
	return bSuccess;
}

UINT CNILayer::ReadingThread(LPVOID pParam)
{
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	int result;

	AfxBeginThread(FileTransferThread, (LPVOID)pParam);
	//FileTransferThread ����, nParam�� ������ this.

	CNILayer *pNI = (CNILayer *)pParam;

	while(pNI->m_thrdSwitch){
		result = pcap_next_ex(pNI->m_AdapterObject, &header, &pkt_data);
		// �������� ĸ�� �Ǵ� �������̽��� ���� ��Ŷ�� ����.
		// (��Ŷ�� ���� ������� ������, ��Ŷ ���, ��Ŷ ������)
		// 1: ����, 0: timeout, -1:����, -2:EOF

		if(result==0){
		}
		else if(result==1){
			pNI->Receive((u_char *)pkt_data);
			// ��Ŷ�� ������ ���� ���̾�� ���� 
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