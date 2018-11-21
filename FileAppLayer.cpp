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
   m_sHeader.fapp_totlen  = 0x00000000 ; // ������ �� ũ��(����)
   m_sHeader.fapp_type    = 0x0000 ; // �κ�
   m_sHeader.fapp_msg_type = 0x00 ; // ����
   m_sHeader.ed = 0x00; // unused
   m_sHeader.fapp_seq_num = 0x00000000 ; // ���� ����

   memset( m_sHeader.fapp_data, 0, APP_DATA_SIZE) ; // ������ �� �κ� �ʱ�ȭ
}

BOOL CFileAppLayer::Send(unsigned char* filePath)
{
   m_FilePath = filePath; // ������ ����� ����
   bFILE = TRUE;
   bSEND = TRUE;

   ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE);// send & search ��ư ��Ȱ��ȭ
   AfxBeginThread(FileThread,this); // ���Ͼ������ ������ ����
   return TRUE;
}

BOOL CFileAppLayer::Receive(unsigned char* ppayload) // ppayload : ���� ��
{
   static HANDLE hFile = NULL;
   DWORD dwWrite=0, dwState=0;

   int progress_value; // ���� ���¸� �˷��ִ� ����

   BOOL bResult;
   BOOL bSuccess = FALSE; // �������� Ȯ�� ����
   PFILE_APP_HEADER fapp_hdr = (PFILE_APP_HEADER) ppayload ; // �����Ϳ� ppayload ����
   static unsigned char** GetBuff;

   static int tot_seq_num;               
   static unsigned long curr_chk_seq_num;   
   static int end_chk_num;               

   if(hFile == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE:��ȿ�� ������ �Ǵ�
   {
      return FALSE;
   }
   if(fapp_hdr->fapp_msg_type == MSG_TYPE_FRAG) // �޼����� ������ ����ȭ�� �޼����϶�
   {
      if(fapp_hdr->fapp_type == DATA_TYPE_BEGIN) // ���ۺκ��� ���
      {
         ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(FALSE); // ���Ϲ�ư ��Ȱ��ȭ

         hFile = CreateFile((char *)fapp_hdr->fapp_data,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		 // ���� �����͸� �̿��� ���ο� ������ ����.
         
		 if(hFile == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE:��ȿ�� ������ �Ǵ�
         {
            AfxMessageBox("���� ���� ����");
            return FALSE;
         }

         GetBuff = (unsigned char**)malloc(sizeof(unsigned char*)*fapp_hdr->fapp_seq_num); //malloc�� �̿��� �����Ҵ�
         memset(GetBuff,0,fapp_hdr->fapp_seq_num); // �ʱ�ȭ
         
         receive_fileTotlen = 0;
         send_fileTotlen = fapp_hdr->fapp_totlen;      
         tot_seq_num = fapp_hdr->fapp_seq_num;      
         curr_chk_seq_num = 0;
         end_chk_num = 0;

         bFILE = FALSE; // bFILE �ʱ�ȭ
		 bACK = TRUE; //bACK �ʱ�ȭ
 
		 m_sHeader.fapp_totlen = 0; // ��ü ���̸� 0���� �ʱ�ȭ
         m_sHeader.fapp_msg_type = MSG_TYPE_ACK; // Ÿ���� ACK MESSAGE�� ����
         ((CTCPLayer*)(GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE); // �������̾��� TCP Layer�� ����

         bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader,FILE_HEADER_SIZE); 
		 //����� ������ ũ�⸦ �����鼭 TRUE�� bSuccess�� ���� 

         bSEND = FALSE;
         bACK = FALSE;
      }
      else if(fapp_hdr->fapp_type == DATA_TYPE_CONT) // ������ �߰� �κ��� ���
      {
         receive_fileTotlen += fapp_hdr->fapp_totlen; // receive_fileTotlen�� ���� �� ������ ��
		
         GetBuff[fapp_hdr->fapp_seq_num] = (unsigned char*)malloc(sizeof(unsigned char*)*fapp_hdr-> fapp_totlen); // malloc�� �̿��� ���� �Ҵ�
         memset(GetBuff[fapp_hdr->fapp_seq_num],0,fapp_hdr->fapp_totlen); // �Ҵ� �� ������ �ʱ�ȭ
         memcpy(GetBuff[fapp_hdr->fapp_seq_num],fapp_hdr->fapp_data,fapp_hdr->fapp_totlen); // �ʱ�ȭ �� ������ ������ ���� ����
         GetBuff[fapp_hdr->fapp_seq_num][fapp_hdr->fapp_totlen] = '\0'; // ������ ���� null������ ����
		 


         if(fapp_hdr->fapp_seq_num < curr_chk_seq_num){
            end_chk_num++;
            bACK = TRUE; // ���� �� bACK�� TRUE������ ����
         }

         else if(fapp_hdr->fapp_seq_num > curr_chk_seq_num)
         {
            if(GetBuff[curr_chk_seq_num]==NULL) // ���� ������ �ƹ� �͵� ���� ��
            {
               nak_num = curr_chk_seq_num; // �ٽ� ������ �ϹǷ� nak_num�� �ش� ���� ��ȣ�� ����
               bNAK = TRUE; // ���� ���� �� bNAK ���� TRUE ������ ����
            }
         }
         else
            end_chk_num++;

         curr_chk_seq_num++;

         progress_value = 100 * ((float)receive_fileTotlen / send_fileTotlen); // ������ ���
		 ((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(progress_value); // ���� ���� ������ ��Ÿ���ִ� ���� ĭ�� ä����

      }
      else if(fapp_hdr->fapp_type == DATA_TYPE_END) // ������ ������ �κ��� ��
      {
         if(send_fileTotlen == receive_fileTotlen) // ��� ���� �� ������ ������
         {
            int length;
            for(int i=0;i<tot_seq_num;i++){
               if(i<tot_seq_num-1)
                  length = FILE_READ_SIZE;
               else
                  length = receive_fileTotlen%FILE_READ_SIZE;

               bResult = WriteFile(hFile,GetBuff[i],length,&dwWrite,NULL); //GetBuff�� ����ִ� �� ���
            }
            bSuccess = TRUE; // TRUE �� ����
         }
         
         CloseHandle(hFile);

         if(bSuccess==TRUE)
            AfxMessageBox("���� ���� �Ϸ�");
         else  //���� send_fileTotlen�� receive_fileTotlen�� ���� �ʾƼ� ������ ������ߴٸ� ����â�� ��
            AfxMessageBox("���� ���� ����");

         ((CIPCAppDlg *)mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0); // ���� ���¸� �˷��ִ� �� �ʱ�ȭ
         ((CIPCAppDlg *)mp_aUpperLayer[0])->OnOffFileButton(TRUE); // ���� ��ư ��Ȱ��ȭ
         
         bFILE = TRUE;
      }
   }
   else if(fapp_hdr->fapp_msg_type == MSG_TYPE_ACK) // ACK ��ȣ �޽��� �� ��
   {
      bSEND = TRUE;
   }
   else if(fapp_hdr->fapp_msg_type == MSG_TYPE_NAK) // NAK ��ȣ �޽��� �� ��
   {
      bNAK_SEND = TRUE;
   }
   return bSuccess ; // ���� ���� ���� ���θ� ����
}

UINT CFileAppLayer::FileThread( LPVOID pParam )
{
   CFileAppLayer *fapp_hdr = (CFileAppLayer *)pParam;
   BOOL bSuccess = FALSE ; // bSuccess �ʱ�ȭ

   HANDLE hFile;
   DWORD dwRead=0, dwFileSize=0, dwState=0; // ���� �ʱ�ȭ
   char *pszBuf;
   BOOL bResult;

   int progress_value; 
   int i=0, tot_seq_num;
   CString fileName;
   
   static unsigned char** GetBuff;

   if(bFILE==TRUE)
   {
      //���� ����
	   hFile = CreateFile((char *)fapp_hdr->m_FilePath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL);
      
	   //INVALID_HANDLE_VALUE :��ȿ�� ������ �Ǵ� �� �ƴ� ��� �ٷ� �����޼��� ����ϰ� ����
	  if(hFile == INVALID_HANDLE_VALUE) 
      {
         AfxMessageBox("�ùٸ� ���� ��ΰ� �ƴմϴ�.");
         ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE);
         return FALSE;
      }

      fileName = fapp_hdr->m_FilePath;
      fileName = fileName.Right(fileName.GetLength()-fileName.ReverseFind('\\')-1);

      dwFileSize = GetFileSize(hFile,0);

      pszBuf = (char *)malloc(dwFileSize); // malloc�� �̿��� ���� �Ҵ�

      if(dwFileSize < FILE_READ_SIZE) // FILE_READ_SIZE���� ���� ũ�Ⱑ ���� ���
         tot_seq_num = 1; // �� ���� ����
      else // FILE_READ_SIZE���� ũ�ų� ���� ���
         tot_seq_num = (dwFileSize/FILE_READ_SIZE) + 1; // ��ü ���� ũ�⸦ FILE_READ_SIZE�� ���� ���� 1�� ���� ������ ����
     
   }
   else //bFILE�� false�� ���
   {
      if(send_fileTotlen < FILE_READ_SIZE) 
         tot_seq_num = 1;
      else 
         tot_seq_num = (send_fileTotlen/FILE_READ_SIZE) + 1;
   }

   while(i <= tot_seq_num + 1) 
   {
      if(bACK == TRUE) // ������ �������� ��  
      {
         fapp_hdr->m_sHeader.fapp_totlen = 0;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_ACK;
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE); 

         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // ���� �Ϸ� �޽��� ����

         bSEND = FALSE;
         bACK = FALSE;
      }
      if(bNAK == TRUE) // ������ �����ߴٴ� �޽����� �޾��� ��
      {
         fapp_hdr->m_sHeader.fapp_totlen = 0;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_NAK;
         fapp_hdr->m_sHeader.fapp_seq_num = nak_num;

         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

		 bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // �޽��� �� ����

         bSEND = FALSE;
         bNAK = FALSE;
      }
      if(bNAK_SEND == TRUE) // ���� ���� �����͸� Ȯ������ ��
      {
         fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_CONT;
         fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; 

         int data_length = strlen((char *)GetBuff[fapp_hdr->m_sHeader.fapp_seq_num]);
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);

         memcpy(fapp_hdr->m_sHeader.fapp_data,GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],data_length);
         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE); // �޽����� ������
         memset(fapp_hdr->m_sHeader.fapp_data,0,data_length);

         
         bNAK_SEND = FALSE;
      }
      if(bSEND == TRUE) // ���� ���� �����͸� Ȯ������ ��
      {
         ((CTCPLayer*)(fapp_hdr->GetUnderLayer()))->SetDestinPort(TCP_PORT_FILE);
         if(i==0) // ù �κ� �� ��
         {
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_BEGIN; // type�� ù �κ����� ����
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // ���� ����ȭ�� �̷���� type�̶�� ����
            fapp_hdr->m_sHeader.fapp_totlen = dwFileSize; // ��ü ���� ũ�⸦ ����
            fapp_hdr->m_sHeader.fapp_seq_num = tot_seq_num; // ����ȭ ������ ���� 

            dwRead = fileName.GetLength(); // ��ü ���� ũ�⸦ ����
            memcpy(fapp_hdr->m_sHeader.fapp_data,fileName,dwRead); // ���� �̸��� �����Ϳ� dwRead ũ�⸸ŭ ����
            fapp_hdr->m_sHeader.fapp_data[dwRead] = '\0'; // �������� null������ ����

            GetBuff = (unsigned char**)malloc(sizeof(unsigned char*)*tot_seq_num); // malloc�� �̿��� ���� �Ҵ�
            memset(GetBuff,0,tot_seq_num); // �޸� �ʱ�ȭ 

            bSEND = FALSE;
         }
         else if(i!=0 && i<=tot_seq_num) // i�� tot_seq_num ���� ���� �� 
         {
            bResult = ReadFile(hFile,pszBuf,FILE_READ_SIZE,&dwRead,NULL); //bResult�� �ҷ��� ������ ����
            dwState += dwRead;
            pszBuf[dwRead] = '\0';

            fapp_hdr->m_sHeader.fapp_totlen = dwRead;// ���� ũ�⸦ ���� 
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_CONT; // ���� ���¸� �߰� �κ��̶�� ����
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // ���� ����ȭ�� �� type���� ����

            fapp_hdr->m_sHeader.fapp_seq_num = i-1; // ������ �� �� �̹Ƿ� �ϳ� ����
            ((CIPLayer*)(fapp_hdr->GetUnderLayer()->GetUnderLayer()))->SetFragOff(fapp_hdr->m_sHeader.fapp_seq_num); //ipfragoff�� fragoff�� ����

            memcpy(fapp_hdr->m_sHeader.fapp_data,pszBuf,dwRead);//pszBuf�� dwRead ���̸�ŭ ����� �����Ϳ� ����

            GetBuff[fapp_hdr->m_sHeader.fapp_seq_num] = (unsigned char*)malloc(sizeof(unsigned char)*dwRead); //malloc�� �̿��� ���� �Ҵ�
            memset(GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],0,dwRead); // �޸� �ʱ�ȭ
            memcpy(GetBuff[fapp_hdr->m_sHeader.fapp_seq_num],pszBuf,dwRead); // pszBuf�� ����

            memset(pszBuf,0,dwRead); // pszBuf �ʱ�ȭ

            progress_value = 100 * ((float)dwState / dwFileSize); // ������ ǥ��
            ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(progress_value); // ������¸� ��Ÿ���� �� ����  
         }
         else 
         {
            CloseHandle(hFile);
            fapp_hdr->m_sHeader.fapp_totlen = 0;
            fapp_hdr->m_sHeader.fapp_msg_type = MSG_TYPE_FRAG; // ���� ����ȭ�� �� type���� ����
            fapp_hdr->m_sHeader.fapp_type = DATA_TYPE_END; // ������ �κ��� �˸�
            memset(fapp_hdr->m_sHeader.fapp_data,0,APP_DATA_SIZE); // �޸� �ʱ�ȭ
            dwRead = 0;
         }

         bSuccess = fapp_hdr->mp_UnderLayer->Send((unsigned char*) &fapp_hdr->m_sHeader,FILE_HEADER_SIZE+dwRead); // �۽��� �������� ��
         memset(fapp_hdr->m_sHeader.fapp_data,0,dwRead); // �޸� �ʱ�ȭ
         i++; // index�� ����
      }
   }

   if(bSuccess==TRUE)
      AfxMessageBox("���� ���� �Ϸ�");
   else
      AfxMessageBox("���� ���� ����");

   ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->m_ProgressCtrl.SetPos(0); // ���� ���� 0���� �ʱ�ȭ
   ((CIPCAppDlg *)fapp_hdr->mp_aUpperLayer[0])->OnOffFileButton(TRUE); // ���� ��ư Ȱ��ȭ

   for(int i = 0; i<tot_seq_num;i++) // �޸� ��ȯ �κ�
   {
      free(GetBuff[i]); 
   }
   free(GetBuff); 
   free(pszBuf); 
   return bSuccess ;
}