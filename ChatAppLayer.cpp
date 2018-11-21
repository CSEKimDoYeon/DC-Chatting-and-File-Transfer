// ChatAppLayer.cpp: implementation of the CChatAppLayer class.

#include "stdafx.h"
#include "ipc.h"
#include "ChatAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CChatAppLayer::CChatAppLayer( char* pName ) 
   : CBaseLayer( pName ), 
   mp_Dlg( NULL )
{
   ResetHeader( ) ;
}

CChatAppLayer::~CChatAppLayer()
{
}

void CChatAppLayer::ResetHeader() // ��� ���� �κ� 
{ 
   m_sHeader.capp_totlen  = 0x0000 ; // totlen 2 bytes
   m_sHeader.capp_type    = 0x00 ; // type 1 byte

   memset( m_sHeader.capp_data, 0, APP_DATA_SIZE ) ;
}

BOOL CChatAppLayer::Send(unsigned char *ppayload, int nlength) //���� ��
{
   m_ppayload = ppayload; // ��ü ������
   m_length = nlength; // ��ü �������� ����

   if(nlength <= APP_DATA_SIZE){ // �������� ũ�⺸�� ���̰� ���� ��
      m_sHeader.capp_totlen = nlength; // ���̸� ����� ������ �� ���� ������ �Է�
      ((CTCPLayer*)(GetUnderLayer()))->SetDestinPort(TCP_PORT_CHAT); // ���� ���̾��� tcp���̾ ���� ����
      memcpy(m_sHeader.capp_data, ppayload, nlength); // ppayload�� ����� �����Ϳ� nlength ũ�⸸ŭ ����
      mp_UnderLayer->Send((unsigned char*) &m_sHeader, nlength + APP_HEADER_SIZE);
	  // (���)�� (���� + ���ũ��)�� �������̾�� ����
   }
   else{
      AfxBeginThread(ChatThread, this); // ũ�� �����带 ����
   }
   return TRUE;
}

BOOL CChatAppLayer::Receive( unsigned char* ppayload ) // ���� ��
{
   PCHAT_APP_HEADER capp_hdr = (PCHAT_APP_HEADER) ppayload ; // ppayload(��ü����)�� header�� ���� 
   static unsigned char *GetBuff;
   
   if(capp_hdr->capp_totlen <= APP_DATA_SIZE){ // APP_DATA_SIZE���� ���޹��� head�� totlend�� �۰ų� ���� ��
      GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen); // capp_totlen��ŭ ũ�⸦ �Ҵ��ؼ� GetBuff�� �Ҵ�
      memset(GetBuff,0,capp_hdr->capp_totlen); // GetBuff �����͸� capp_totlen���̸�ŭ 0���� �ʱ�ȭ �Ѵ�  
      memcpy(GetBuff,capp_hdr->capp_data,capp_hdr->capp_totlen); // ����� ������ ������ ����� ��ü���̸�ŭ GetBuff�� �����Ѵ�
      GetBuff[capp_hdr->capp_totlen] = '\0'; // GetBuff�� ���� ������ ���Ҹ� null������ ����

      mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff); // �������̾�� ������.
      return TRUE;
   }

   if(capp_hdr->capp_type == DATA_TYPE_BEGIN) // �޽��� Ÿ���� ó�� ������ �޽��� �� ��
   {      
      GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen); // ��ü ���� ��ŭ ũ�⸦ GetBuff�� �Ҵ�
      memset(GetBuff,0,capp_hdr->capp_totlen); // GetBuff �����͸� capp_totlen���̸�ŭ 0���� �ʱ�ȭ�Ѵ�. 
   }
   else if(capp_hdr->capp_type == DATA_TYPE_CONT) // �޽��� Ÿ���� �߰��� ������ �޽��� �� �� 
   {
      strncat((char *)GetBuff,(char *)capp_hdr->capp_data,strlen((char *)capp_hdr->capp_data)); // head�� �����͸� getbuff�� data�� ���̸�ŭ �����δ�.
      GetBuff[strlen((char *)GetBuff)] = '\0'; // ������ ���Ҹ� NULL�� �����Ѵ�.
   }
   else if(capp_hdr->capp_type == DATA_TYPE_END)  // �޽��� Ÿ���� �������� ������ �޽��� �� ��
   {
      memcpy(GetBuff,GetBuff,capp_hdr->capp_totlen); // �ռ� ���پ��� ����ǿ��� GetBuff�� ���ο� GetBuff�� ������ ���� ũ�⸸ŭ �����Ѵ�.
      GetBuff[capp_hdr->capp_totlen] = '\0'; // ������ ���Ҹ� NULL�� ���� 

      mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff); // �������̾��� IPCAppDlg�� ������.
      free(GetBuff); // GetBuff�� ��ȯ�Ѵ�.
   }
   else // 3���� Ÿ���� �ƴ� ���� FALSE�� ��ȯ.
      return FALSE;

   return TRUE ;
}

UINT CChatAppLayer::ChatThread( LPVOID pParam )
{
   
   BOOL bSuccess = FALSE;
   CChatAppLayer *pChat = (CChatAppLayer *)pParam;
   int data_length = APP_DATA_SIZE;
   int seq_tot_num;
   int data_index;   
   int temp = 0;

   if( pChat->m_length < APP_DATA_SIZE ) // APP_DATA_SIZE ���� CHAT�� ���̰� ���� ��
      seq_tot_num = 1; // ���� �������� ���� 1�� ����
   else  // APP_DATA_SIZE ���� CHAT�� ���̰� �� Ŭ ��
      seq_tot_num = (pChat->m_length/APP_DATA_SIZE) + 1; // CHAT�� ���̸� APP_DATA_SIZE�� ���� �򺸴� 1 ū ���� seq_tot_num�� ���� 

   for(int i = 0; i <= seq_tot_num + 1; i++) // thread �ݺ���
   {
      if(seq_tot_num == 1){ // chat�� ������ ũ�Ⱑ APP_DATA_SIZE���� ���� ��
         data_length = pChat->m_length; // pChat�� ���̸� data_length�� ����
      }
      else{ // APP_DATA_SIZE���� chat�� ������ ũ�Ⱑ �� Ŭ ��
         if(i == seq_tot_num) 
            data_length = pChat->m_length % APP_DATA_SIZE; // APP_DATA_SIZE�� ���� ������ ���� data_length�� �Է�
         else
            data_length = APP_DATA_SIZE; // APP_DATA_SIZE�� data_length�� �Է�
      }

      memset(pChat->m_sHeader.capp_data, 0, data_length); // pChat�� ����� �����ͺκ��� data_length ���� ��ŭ 0���� �ʱ�ȭ
      if(i == 0) 
      {
         pChat->m_sHeader.capp_totlen = pChat->m_length; // pChat�� length �κ��� �� totlen�� �Է�
         pChat->m_sHeader.capp_type = DATA_TYPE_BEGIN; // pChat�� Ÿ���� BEGIN���� ����
         memset(pChat->m_sHeader.capp_data,0,data_length); // pChat ����� data�κ��� data_length���̸�ŭ 0���� �ʱ�ȭ
         data_length = 0; // �������� ���̸� 0���� �ʱ�ȭ
      }
      else if(i!=0 && i<=seq_tot_num)  // i�� 0�� �ƴϰ� ������ �� ������ ���� ��
      {
         data_index = data_length; // data_length ���� data_index���� ����
         pChat->m_sHeader.capp_type = DATA_TYPE_CONT; // data_type�� CONT�� ����
         pChat->m_sHeader.capp_seq_num = i-1; // ����� ������ �� ������ ���� �ϳ� ����

         CString str = pChat->m_ppayload; // pChat�� ��ü ������ str�� ����
         str = str.Mid(temp,temp+data_index); // str.mid() = �ش� ��ġ���� ���۵Ǵ� ���ڿ��� �����ϴ� ��� ���ڿ��� ��ȯ 

         memcpy(pChat->m_sHeader.capp_data,str,data_length); // str�� data_length��ŭ ����� �����ͺκп� ����
         temp += data_index; // temp�� data_index��ŭ ����
      }
      else // i�� seq_tot_num�� ���� ��
      {
         pChat->m_sHeader.capp_type = DATA_TYPE_END; // ������ Ÿ���� end�� ����
         memset(pChat->m_ppayload,0,data_length); // ��ü�� data_length��ŭ 0���� �ʱ�ȭ
         data_length = 0;
      }
      ((CTCPLayer*)(pChat->GetUnderLayer()))->SetDestinPort(TCP_PORT_CHAT);
	  // pChat�� TCP ���̾�� ���� 
       bSuccess = pChat->mp_UnderLayer->Send((unsigned char*) &pChat->m_sHeader,data_length+APP_HEADER_SIZE);
	   // (��� ����)�� (data_length + APP_HEADER_SIZE)�� ���� ���̾�� ���� 
   }

   return bSuccess;
}