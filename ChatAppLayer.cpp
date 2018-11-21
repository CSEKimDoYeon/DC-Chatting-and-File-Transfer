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

void CChatAppLayer::ResetHeader() // 헤더 리셋 부분 
{ 
   m_sHeader.capp_totlen  = 0x0000 ; // totlen 2 bytes
   m_sHeader.capp_type    = 0x00 ; // type 1 byte

   memset( m_sHeader.capp_data, 0, APP_DATA_SIZE ) ;
}

BOOL CChatAppLayer::Send(unsigned char *ppayload, int nlength) //보낼 때
{
   m_ppayload = ppayload; // 전체 데이터
   m_length = nlength; // 전체 데이터의 길이

   if(nlength <= APP_DATA_SIZE){ // 데이터의 크기보다 길이가 적을 때
      m_sHeader.capp_totlen = nlength; // 길이를 헤더의 데이터 총 길이 정보에 입력
      ((CTCPLayer*)(GetUnderLayer()))->SetDestinPort(TCP_PORT_CHAT); // 하위 레이어인 tcp레이어에 정보 전달
      memcpy(m_sHeader.capp_data, ppayload, nlength); // ppayload를 헤더의 데이터에 nlength 크기만큼 저장
      mp_UnderLayer->Send((unsigned char*) &m_sHeader, nlength + APP_HEADER_SIZE);
	  // (헤더)와 (길이 + 헤더크기)를 하위레이어로 전달
   }
   else{
      AfxBeginThread(ChatThread, this); // 크면 쓰레드를 시작
   }
   return TRUE;
}

BOOL CChatAppLayer::Receive( unsigned char* ppayload ) // 받을 때
{
   PCHAT_APP_HEADER capp_hdr = (PCHAT_APP_HEADER) ppayload ; // ppayload(전체내용)을 header에 저장 
   static unsigned char *GetBuff;
   
   if(capp_hdr->capp_totlen <= APP_DATA_SIZE){ // APP_DATA_SIZE보다 전달받은 head의 totlend이 작거나 같을 때
      GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen); // capp_totlen만큼 크기를 할당해서 GetBuff에 할당
      memset(GetBuff,0,capp_hdr->capp_totlen); // GetBuff 포인터를 capp_totlen길이만큼 0으로 초기화 한다  
      memcpy(GetBuff,capp_hdr->capp_data,capp_hdr->capp_totlen); // 헤더의 데이터 정보를 헤더의 전체길이만큼 GetBuff에 복사한다
      GetBuff[capp_hdr->capp_totlen] = '\0'; // GetBuff의 제일 마지막 원소를 null값으로 지정

      mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff); // 상위레이어로 보낸다.
      return TRUE;
   }

   if(capp_hdr->capp_type == DATA_TYPE_BEGIN) // 메시지 타입이 처음 보내는 메시지 일 때
   {      
      GetBuff = (unsigned char *)malloc(capp_hdr->capp_totlen); // 전체 길이 만큼 크기를 GetBuff에 할당
      memset(GetBuff,0,capp_hdr->capp_totlen); // GetBuff 포인터를 capp_totlen길이만큼 0으로 초기화한다. 
   }
   else if(capp_hdr->capp_type == DATA_TYPE_CONT) // 메시지 타입이 중간에 보내는 메시지 일 때 
   {
      strncat((char *)GetBuff,(char *)capp_hdr->capp_data,strlen((char *)capp_hdr->capp_data)); // head의 데이터를 getbuff에 data의 길이만큼 덧붙인다.
      GetBuff[strlen((char *)GetBuff)] = '\0'; // 마지막 원소를 NULL로 지정한다.
   }
   else if(capp_hdr->capp_type == DATA_TYPE_END)  // 메시지 타입이 마지막에 보내는 메시지 일 때
   {
      memcpy(GetBuff,GetBuff,capp_hdr->capp_totlen); // 앞서 덧붙어져 저장되오던 GetBuff를 새로운 GetBuff에 데이터 길이 크기만큼 저장한다.
      GetBuff[capp_hdr->capp_totlen] = '\0'; // 마지막 원소를 NULL로 지정 

      mp_aUpperLayer[0]->Receive((unsigned char*) GetBuff); // 상위레이어인 IPCAppDlg로 보낸다.
      free(GetBuff); // GetBuff를 반환한다.
   }
   else // 3가지 타입이 아닐 때는 FALSE값 반환.
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

   if( pChat->m_length < APP_DATA_SIZE ) // APP_DATA_SIZE 보다 CHAT의 길이가 적을 때
      seq_tot_num = 1; // 보낼 데이터의 수를 1로 지정
   else  // APP_DATA_SIZE 보다 CHAT의 길이가 더 클 때
      seq_tot_num = (pChat->m_length/APP_DATA_SIZE) + 1; // CHAT의 길이를 APP_DATA_SIZE로 나눈 몫보다 1 큰 값을 seq_tot_num에 저장 

   for(int i = 0; i <= seq_tot_num + 1; i++) // thread 반복문
   {
      if(seq_tot_num == 1){ // chat의 데이터 크기가 APP_DATA_SIZE보다 작을 때
         data_length = pChat->m_length; // pChat의 길이를 data_length에 저장
      }
      else{ // APP_DATA_SIZE보다 chat의 데이터 크기가 더 클 때
         if(i == seq_tot_num) 
            data_length = pChat->m_length % APP_DATA_SIZE; // APP_DATA_SIZE로 나눈 나머지 값을 data_length에 입력
         else
            data_length = APP_DATA_SIZE; // APP_DATA_SIZE를 data_length에 입력
      }

      memset(pChat->m_sHeader.capp_data, 0, data_length); // pChat의 헤더의 데이터부분을 data_length 길이 만큼 0으로 초기화
      if(i == 0) 
      {
         pChat->m_sHeader.capp_totlen = pChat->m_length; // pChat의 length 부분을 총 totlen에 입력
         pChat->m_sHeader.capp_type = DATA_TYPE_BEGIN; // pChat의 타입을 BEGIN으로 설정
         memset(pChat->m_sHeader.capp_data,0,data_length); // pChat 헤더의 data부분을 data_length길이만큼 0으로 초기화
         data_length = 0; // 데이터의 길이를 0으로 초기화
      }
      else if(i!=0 && i<=seq_tot_num)  // i는 0이 아니고 보내야 할 수보다 작을 때
      {
         data_index = data_length; // data_length 값을 data_index값에 저장
         pChat->m_sHeader.capp_type = DATA_TYPE_CONT; // data_type을 CONT로 지정
         pChat->m_sHeader.capp_seq_num = i-1; // 헤더의 보내야 할 정보의 수를 하나 줄임

         CString str = pChat->m_ppayload; // pChat의 전체 정보를 str에 저장
         str = str.Mid(temp,temp+data_index); // str.mid() = 해당 위치부터 시작되는 문자열을 포함하는 모든 문자열을 반환 

         memcpy(pChat->m_sHeader.capp_data,str,data_length); // str을 data_length만큼 헤더의 데이터부분에 저장
         temp += data_index; // temp를 data_index만큼 저장
      }
      else // i가 seq_tot_num와 같을 때
      {
         pChat->m_sHeader.capp_type = DATA_TYPE_END; // 데이터 타입을 end로 지정
         memset(pChat->m_ppayload,0,data_length); // 전체를 data_length만큼 0으로 초기화
         data_length = 0;
      }
      ((CTCPLayer*)(pChat->GetUnderLayer()))->SetDestinPort(TCP_PORT_CHAT);
	  // pChat을 TCP 레이어로 보냄 
       bSuccess = pChat->mp_UnderLayer->Send((unsigned char*) &pChat->m_sHeader,data_length+APP_HEADER_SIZE);
	   // (헤더 정보)와 (data_length + APP_HEADER_SIZE)를 하위 레이어로 보냄 
   }

   return bSuccess;
}