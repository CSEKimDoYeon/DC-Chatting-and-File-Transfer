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
static int send_fileTotlen; // ���� ������ ��ü ũ��
static int nak_num; // NAK ����

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
	
	static UINT		FileThread( LPVOID pParam );	// FileApp ������

	CFileAppLayer( char* pName );
	virtual ~CFileAppLayer();

	typedef struct _FILE_APP_HEADER {
		unsigned long	fapp_totlen ;// �������� �� ���� (4byte)
		unsigned short	fapp_type ;	// ������ Ÿ�� (2byte)
		unsigned char	fapp_msg_type ;	// type of message (������ or ����) (1byte)
		unsigned char	ed;	// no use // ��� ���� (1byte)
		unsigned long	fapp_seq_num ;	// Fragmentation ���� (4byte)
		unsigned char	fapp_data[APP_DATA_SIZE]; // application data
	} FileAppLayer_HEADER, *PFILE_APP_HEADER ;

protected:
	FileAppLayer_HEADER	m_sHeader ;

	enum {			DATA_TYPE_BEGIN = 0x00,	// ���� ����
					DATA_TYPE_CONT = 0x01,	// ���� �߰�
					DATA_TYPE_END =  0x02 };// ���� ��

	enum {			MSG_TYPE_FRAG = 0x00,	// ����ȭ�� �޽���
					MSG_TYPE_ACK = 0x01,	// ���Ź��� �����Ϳ� ���� Ȯ�� �޽���
					MSG_TYPE_NAK = 0x02 };	// �߸��� �����͸� �޾��� �� ������ �޽���
};




