// FileAppLayer.cpp: implementation of the CFileAppLayer class.

#include "stdafx.h"
#include "ipc.h"
#include "FileAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFileAppLayer::CFileAppLayer( char* pName )
   : CBaseLayer( pName )
{
   bSEND = TRUE;
   ResetHeader( );
}

CFileAppLayer::~CFileAppLayer()
{
}

void CFileAppLayer::ResetHeader()
{
   m_sHeader.fapp_totlen  = 0x00000000 ; // 파일의 총 크기(길이)
   m_sHeader.fapp_type    = 0x0000 ; // 부분
   m_sHeader.fapp_msg_type = 0x00 ; // 종류
   m_sHeader.ed = 0x00; // unused
   m_sHeader.fapp_seq_num = 0x00000000 ; // 현재 구간

   memset( m_sHeader.fapp_data, 0, APP_DATA_SIZE) ; // 파일이 들어갈 부분 초기화
}

BOOL CFileAppLayer::Send(unsigned char* filePath)
{
   m_FilePath = filePath; // 파일이 저장될 공간
   bFILE = TRUE;
   bSEND = TRUE;

   ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE);// send & search 버튼 비활성화
   AfxBeginThread(FileThread,this); // 파일쓰레드로 쓰레드 시작
   return TRUE;
}

BOOL CFileAppLayer::Receive(unsigned char* ppayload) // ppayload : 받은 값
{
   static HANDLE hFile = NULL;
   DWORD dwWrite=0, dwState=0;

   int progress_value; // 진행 상태를 알려주는 변수

   BOOL bResult;
   BOOL bSuccess = FALSE; // 성공여부 확인 변수
   PFILE_APP_HEADER fapp_hdr = (PFILE_APP_HEADER) ppayload ; // 데이터에 ppayload 저장
   static unsigned char** GetBuff;

   static int tot_seq_num;               
   static unsigned long curr_chk_seq_num;   
   static int end_chk_num;               

   if(hFile == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE:유효한 값인지 판단
   {
      return FALSE;
   }
   if(fapp_hdr->fapp_msg_type == MSG_TYPE_FRAG) // 메세지의 종류가 단편화된 메세지일때
   {
      if(fapp_hdr->fapp_type == DATA_TYPE_BEGIN) // 시작부분인 경우
      {
         ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE); // 파일버튼 비활성화

         hFile = CreateFile((char *)fapp_hdr->fapp_data,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		 // 받은 데이터를 이용해 새로운 파일을 생성.
         
		 if(hFile == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE:유효한 값인지 판단
         {
            AfxMessageBox("파일 생성 오류");
            return FALSE;
         }

         GetBuff = (unsigned char**)malloc(sizeof(unsigned char*)*fapp_hdr->fapp_seq_num); //malloc을 이용한 공간할당
         memset(GetBuff,0,fapp_hdr->fapp_seq_num); // 초기화
         
         receive_fileTotlen = 0;
         send_fileTotlen = fapp_hdr->fapp_totlen;      
         tot_seq_num = fapp_hdr->fapp_seq_num;      
         curr_chk_seq_num = 0;
         end_chk_num = 0;

         bFILE = FALSE; // bFILE 초기화
		 bACK = TRUE; //bACK 초기화
 
		 m_sHeader.fapp_totlen = 0; // 전체 길이를 0으로 초기화
         m_sHeader.fapp_msg_type = MSG_TYPE_ACK; // 타입을 ACK MESSAGE로 저장
         ((CTCPLayer*)(GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE); // 하위레이어인 TCP Layer로 전달

         bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader,FILE_HEADER_SIZE); 
		 //헤더의 정보와 크기를 보내면서 TRUE로 bSuccess를 설정 

         bSEND = FALSE;
         bACK = FALSE;
      }
      else if(fapp_hdr->fapp_type == DATA_TYPE_CONT) // 파일의 중간 부분일 경우
      {
         receive_fileTotlen += fapp_hdr->fapp_totlen; // receive_fileTotlen이 받은 총 파일의 양
		
         GetBuff[fapp_hdr->fapp_seq_num] = (unsigned char*)malloc(sizeof(unsigned char*)*fapp_hdr-> fapp_totlen); // malloc을 이용한 공간 할당
         memset(GetBuff[fapp_hdr->fapp_seq_num],0,fapp_hdr->fapp_totlen); // 할당 된 공간을 초기화
         memcpy(GetBuff[fapp_hdr->fapp_seq_num],fapp_hdr->fapp_data,fapp_hdr->fapp_totlen); // 초기화 된 공간에 데이터 정보 복사
         GetBuff[fapp_hdr->fapp_seq_num][fapp_hdr->fapp_totlen] = '\0'; // 마지막 값을 null값으로 설정
		 


         if(fapp_hdr->fapp_seq_num < curr_chk_seq_num){
            end_chk_num++;
            bACK = TRUE; // 성공 시 bACK가 TRUE값으로 설정
         }

         else if(fapp_hdr->fapp_seq_num > curr_chk_seq_num)
         {
            if(GetBuff[curr_chk_seq_num]==NULL) // 받은 파일이 아무 것도 없을 때
            {
               nak_num = curr_chk_seq_num; // 다시 보내야 하므로 nak_num에 해당 파일 번호를 저장
               bNAK = TRUE; // 실패 했을 때 bNAK 값이 TRUE 값으로 설정
            }
         }
         else
            end_chk_num++;

         curr_chk_seq_num++;

         progress_value = 100 * ((float)receive_fileTotlen / send_fileTotlen); // 진행율 계산
		 ((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(progress_value); // 파일 진행 정도를 나타내주는 바의 칸을 채워감

      }
      else if(fapp_hdr->fapp_type == DATA_TYPE_END) // 파일의 마지막 부분일 때
      {
         if(send_fileTotlen == receive_fileTotlen) // 모두 받은 후 파일이 같으면
         {
            int length;
            for(int i=0;i<tot_seq_num;i++){
               if(i<tot_seq_num-1)
                  length = FILE_READ_SIZE;
               else
                  length = receive_fileTotlen%FILE_READ_SIZE;

               bResult = WriteFile(hFile,GetBuff[i],length,&dwWrite,NULL); //GetBuff에 들어있던 것 모두
            }
            bSuccess = TRUE; // TRUE 값 저장
         }
         
         CloseHandle(hFile);

         if(bSuccess==TRUE)
            AfxMessageBox("파일 수신 완료");
         else  //만약 send_fileTotlen과 receive_fileTotlen이 같지 않아서 파일을 저장안했다면 실패창이 뜸
            AfxMessageBox("파일 수신 실패");

         ((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0); // 진행 상태를 알려주는 바 초기화
         ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(TRUE); // 파일 버튼 재활성화
         
         bFILE = TRUE;
      }
   }
   else if(fapp_hdr->fapp_msg_type == MSG_TYPE_ACK) // ACK 신호 메시지 일 때
   {
      bSEND = TRUE;
   }
   else if(fapp_hdr->fapp_msg_type == MSG_TYPE_NAK) // NAK 신호 메시지 일 때
   {
      bNAK_SEND = TRUE;
   }
   return bSuccess ; // 파일 수신 성공 여부를 리턴
}

UINT CFileAppLayer::FileThread( LPVOID pParam )
{
   CFileAppLayer *fapp_hdr = (CFileAppLayer *)pParam;
   BOOL bSuccess = FALSE ; // bSuccess 초기화

   HANDLE hFile;
   DWORD dwRead=0, dwFileSize=0, dwState=0; // 변수 초기화
   char *pszBuf;
   BOOL bResult;

   int progress_value; 
   int i=0, tot_seq_num;
   CString fileName;
   
   static unsigned char** GetBuff;

   if(bFILE==TRUE)
   {
      //파일 생성
	   hFile = CreateFile((char *)fapp_hdr->m_FilePath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);
      
	   //INVALID_HANDLE_VALUE :유효한 값인지 판단 후 아닐 경우 바로 오류메세지 출력하고 종료
	  if(hFile == INVALID_HANDLE_VALUE) 
      {
         AfxMessageBox("올바른 파일 경로가 아닙니다.");
         ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE);
         return FALSE;
      }

      fileName = fapp_hdr->m_FilePath;
      fileName = fileName.Right(fileName.GetLength()-fileName.ReverseFind('\\')-1);

      dwFileSize = GetFileSize(hFile,0);

      pszBuf = (char *)malloc(dwFileSize); // malloc을 이용한 공간 할당

      if(dwFileSize < FILE_READ_SIZE) // FILE_READ_SIZE보다 파일 크기가 적을 경우
         tot_seq_num = 1; // 한 번에 전송
      else // FILE_READ_SIZE보다 크거나 같을 경우
         tot_seq_num = (dwFileSize/FILE_READ_SIZE) + 1; // 전체 파일 크기를 FILE_READ_SIZE로 나눈 값에 1을 더한 값으로 설정
     
   }
   else //bFILE이 false인 경우
   {
      if(send_fileTotlen < FILE_READ_SIZE) 
         tot_seq_num = 1;
      else 
         tot_seq_num = (send_fileTotlen/FILE_READ_SIZE) + 1;
   }

   while(i <= tot_seq_num + 1) 
   {
      if(bACK == TRUE) // 수신이 성공했을 때  
      {
         fapp_hdr->m_sHeader.fapp_totlen = 0;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_ACK;
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE); 

         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // 수신 완료 메시지 전송

         bSEND = FALSE;
         bACK = FALSE;
      }
      if(bNAK == TRUE) // 수신이 실패했다는 메시지를 받았을 때
      {
         fapp_hdr->m_sHeader.fapp_totlen = 0;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_NAK;
         fapp_hdr->m_sHeader.fapp_seq_num = nak_num;

         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

		 bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // 메시지 재 전송

         bSEND = FALSE;
         bNAK = FALSE;
      }
      if(bNAK_SEND == TRUE) // 수신 받은 데이터를 확인했을 때
      {
         fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_CONT;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; 

         int data_length = strlen((char *)GetBuff[fapp_hdr->m_sHeader.fapp_seq_num]);
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

         memcpy(fapp_hdr->m_sHeader.fapp_data,GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],data_length);
         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // 메시지를 재전송
         memset(fapp_hdr->m_sHeader.fapp_data,0,data_length);

         
         bNAK_SEND = FALSE;
      }
      if(bSEND == TRUE) // 수신 받은 데이터를 확인했을 때
      {
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);
         if(i==0) // 첫 부분 일 때
         {
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_BEGIN; // type을 첫 부분으로 지정
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // 파일 단편화가 이루어진 type이라고 저장
            fapp_hdr->m_sHeader.fapp_totlen = dwFileSize; // 전체 파일 크기를 저장
            fapp_hdr->m_sHeader.fapp_seq_num = tot_seq_num; // 단편화 순서를 저장 

            dwRead = fileName.GetLength(); // 전체 파일 크기를 저장
            memcpy(fapp_hdr->m_sHeader.fapp_data,fileName,dwRead); // 파일 이름을 데이터에 dwRead 크기만큼 저장
            fapp_hdr->m_sHeader.fapp_data[dwRead] = '\0'; // 마지막을 null값으로 저장

            GetBuff = (unsigned char**)malloc(sizeof(unsigned char*)*tot_seq_num); // malloc을 이용한 공간 할당
            memset(GetBuff,0,tot_seq_num); // 메모리 초기화 

            bSEND = FALSE;
         }
         else if(i!=0 && i<=tot_seq_num) // i가 tot_seq_num 까지 왔을 때 
         {
            bResult = ReadFile(hFile,pszBuf,FILE_READ_SIZE,&dwRead,NULL); //bResult에 불러온 파일을 저장
            dwState += dwRead;
            pszBuf[dwRead] = '\0';

            fapp_hdr->m_sHeader.fapp_totlen = dwRead;// 파일 크기를 저장 
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_CONT; // 파일 상태를 중간 부분이라고 저장
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // 파일 단편화가 된 type으로 저장

            fapp_hdr->m_sHeader.fapp_seq_num = i-1; // 실행을 한 후 이므로 하나 줄임
            ((CIPLayer*)(fapp_hdr->GetUnderLayer()->GetUnderLayer()))->SetFragOff(fapp_hdr->m_sHeader.fapp_seq_num); //ipfragoff를 fragoff로 변경

            memcpy(fapp_hdr->m_sHeader.fapp_data,pszBuf,dwRead);//pszBuf를 dwRead 길이만큼 헤더의 데이터에 저장

            GetBuff[fapp_hdr->m_sHeader.fapp_seq_num] = (unsigned char*)malloc(sizeof(unsigned char)*dwRead); //malloc을 이용한 공간 할당
            memset(GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],0,dwRead); // 메모리 초기화
            memcpy(GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],pszBuf,dwRead); // pszBuf를 복사

            memset(pszBuf,0,dwRead); // pszBuf 초기화

            progress_value = 100 * ((float)dwState / dwFileSize); // 진행율 표시
            ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(progress_value); // 진행상태를 나타내는 바 설정  
         }
         else 
         {
            CloseHandle(hFile);
            fapp_hdr->m_sHeader.fapp_totlen = 0;
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // 파일 단편화가 된 type으로 저장
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_END; // 마지막 부분을 알림
            memset(fapp_hdr->m_sHeader.fapp_data,0,APP_DATA_SIZE); // 메모리 초기화
            dwRead = 0;
         }

         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE+dwRead); // 송신이 성공했을 때
         memset(fapp_hdr->m_sHeader.fapp_data,0,dwRead); // 메모리 초기화
         i++; // index값 증가
      }
   }

   if(bSuccess==TRUE)
      AfxMessageBox("파일 전송 완료");
   else
      AfxMessageBox("파일 전송 실패");

   ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0); // 진행 상태 0으로 초기화
   ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE); // 파일 버튼 활성화

   for(int i = 0; i<tot_seq_num;i++) // 메모리 반환 부분
   {
      free(GetBuff[i]); 
   }
   free(GetBuff); 
   free(pszBuf); 
   return bSuccess ;
}