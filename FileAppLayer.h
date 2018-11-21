// FileAppLayer.h: interface for the CEthernetLayer class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "IPLayer.h"
#include "TCPLayer.h"
#include "IPCAppDlg.h"

static BOOL bSEND;// SEND switch
static BOOL bACK; // ACK switch
static BOOL bNAK; // NAK switch
static BOOL bNAK_SEND;
static BOOL bFILE;
static int send_fileTotlen; // 보낸 파일의 전체 크기
static int nak_num; // NAK 숫자

class CFileAppLayer 
: public CBaseLayer
{
private:
	inline void		ResetHeader( );
	CObject* mp_Dlg ;

public:
	unsigned char	*m_ppayload;
	int				m_length;
	unsigned char	*m_FilePath;
	int				receive_fileTotlen;

	BOOL			Receive( unsigned char* ppayload );
	BOOL			Send( unsigned char* filePath );
	
	static UINT		FileThread( LPVOID pParam );	// FileApp 쓰레드

	CFileAppLayer( char* pName );
	virtual ~CFileAppLayer();

	typedef struct _FILE_APP_HEADER {
		unsigned long	fapp_totlen ;// 데이터의 총 길이 (4byte)
		unsigned short	fapp_type ;	// 데이터 타입 (2byte)
		unsigned char	fapp_msg_type ;	// type of message (데이터 or 응답) (1byte)
		unsigned char	ed;	// no use // 사용 안함 (1byte)
		unsigned long	fapp_seq_num ;	// Fragmentation 순서 (4byte)
		unsigned char	fapp_data[APP_DATA_SIZE]; // application data
	} FileAppLayer_HEADER, *PFILE_APP_HEADER ;

protected:
	FileAppLayer_HEADER	m_sHeader ;

	enum {			DATA_TYPE_BEGIN = 0x00,	// 전송 시작
					DATA_TYPE_CONT = 0x01,	// 전송 중간
					DATA_TYPE_END =  0x02 };// 전송 끝

	enum {			MSG_TYPE_FRAG = 0x00,	// 단편화된 메시지
					MSG_TYPE_ACK = 0x01,	// 수신받은 데이터에 대한 확인 메시지
					MSG_TYPE_NAK = 0x02 };	// 잘못된 데이터를 받았을 때 보내는 메시지
};




