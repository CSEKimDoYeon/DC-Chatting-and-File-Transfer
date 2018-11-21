// TCPLayer.h: interface for the CEthernetLayer class.
//
///////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"

class CTCPLayer 
: public CBaseLayer  
{
private:
	inline void		ResetHeader();	

public:
	CTCPLayer( char* pName );
	virtual ~CTCPLayer();

	void SetSourcePort(unsigned int ipAddress);	
	void SetDestinPort(unsigned int ipAddress);

	BOOL Send(unsigned char* ppayload, int nlength);
	BOOL Receive(unsigned char* ppayload);

	typedef struct _TCPLayer_HEADER {
		unsigned short tcp_sport;	// source port (2byte)
		unsigned short tcp_dport;	// destination port (2byte)
		unsigned int tcp_seq;		// sequence number (4byte)
		unsigned int tcp_ack;		// acknowledged sequence (4byte)
		unsigned char tcp_offset;	// no use (1byte)
		unsigned char tcp_flag;		// control flag (1byte)
		unsigned short tcp_window;	// no use (2byte)
		unsigned short tcp_cksum;	// check sum (2byte)
		unsigned short tcp_urgptr;	// no use (2byte)
		unsigned char Padding[4]; // (4byte)
		unsigned char tcp_data[ TCP_DATA_SIZE ]; // data part
	}TCPLayer_HEADER, *PTCPLayer_HEADER ;
protected:
	TCPLayer_HEADER	m_sHeader ;
};
