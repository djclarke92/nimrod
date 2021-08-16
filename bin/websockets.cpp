#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <modbus/modbus.h>
#include "mb_mysql.h"
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"



#define NIMROD_RX_BUFFER_BYTES (100)
struct payload
{
	unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + NIMROD_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	size_t len;
} received_payload;


#define MAX_LWS_CLIENTS	10
class CLWSClient {
private:
	struct lws *m_Wsi[MAX_LWS_CLIENTS];
	unsigned char m_Data[MAX_LWS_CLIENTS][LWS_SEND_BUFFER_PRE_PADDING + NIMROD_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];

public:
	CLWSClient();
	~CLWSClient();

	void Init();
	void AddClient( struct lws* wsi );
	void AddMessage( const char* szMsg );
	int FindClient( struct lws* wsi );
	unsigned char* GetMessage( const int idx, int& len );
	void ClearClient( const int idx );
	void ClearMessage( const int idx );
};

CLWSClient::CLWSClient()
{
	Init();
}

CLWSClient::~CLWSClient()
{

}

void CLWSClient::Init()
{
	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		m_Wsi[i] = NULL;
		memset( m_Data[i], 0, sizeof(m_Data[i]) );
	}
}

void CLWSClient::AddClient( struct lws* wsi )
{
	bool found = false;

	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		if ( m_Wsi[i] == wsi )
		{	// already in the list
			found = true;
			break;
		}
	}
	if ( !found )
	{	// add a new client
		for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
		{
			if ( m_Wsi[i] == NULL )
			{
				found = true;
				m_Wsi[i] = wsi;
				break;
			}
		}
	}

	if ( !found )
	{
		LogMessage( E_MSG_WARN, "WS failed to add client, no more slots" );
	}
}

void CLWSClient::AddMessage( const char* szMsg )
{
	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		if ( m_Wsi[i] != NULL )
		{
			snprintf( (char*)&m_Data[i][LWS_SEND_BUFFER_PRE_PADDING], sizeof(m_Data[i]), "%s", szMsg );

			lws_callback_on_writable_all_protocol( lws_get_context( m_Wsi[i] ), lws_get_protocol( m_Wsi[i] ) );
		}
	}
}

unsigned char* CLWSClient::GetMessage( const int idx, int& len )
{
	unsigned char* uzData = NULL;

	len = 0;
	if ( idx >= 0  && idx < MAX_LWS_CLIENTS )
	{
		len = strlen( (char*)&m_Data[idx][LWS_SEND_BUFFER_PRE_PADDING] );
		uzData = &m_Data[idx][LWS_SEND_BUFFER_PRE_PADDING];
	}

	return uzData;
}

int CLWSClient::FindClient( struct lws* wsi  )
{
	int idx = -1;

	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		if ( m_Wsi[i] == wsi )
		{
			idx = i;
			break;
		}
	}

	return idx;
}

void CLWSClient::ClearClient( const int idx )
{
	if ( idx >= 0  && idx < MAX_LWS_CLIENTS )
	{
		LogMessage( E_MSG_INFO, "WS clear client at idx %d", idx );
		m_Wsi[idx] = NULL;
		memset( m_Data[idx], 0, sizeof(m_Data[idx]) );
	}
}

void CLWSClient::ClearMessage( const int idx )
{
	if ( idx >= 0  && idx < MAX_LWS_CLIENTS )
	{
		memset( m_Data[idx], 0, sizeof(m_Data[idx]) );
	}
}

CLWSClient myWsi;


static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	return 0;
}



static int callback_nimrod( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	int send_len = 0;
	int idx;
	int bytes;
	unsigned char* uzData = NULL;

	switch( reason )
	{
		case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
			LogMessage( E_MSG_INFO, "ws established: len %d %p", len, wsi );
			myWsi.AddClient( wsi );
	        break;

		case LWS_CALLBACK_RECEIVE:
			LogMessage( E_MSG_INFO, "ws receive: len %d", len );
			memcpy( &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len );
			received_payload.len = len;
			//lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:

			idx = myWsi.FindClient( wsi );
			if ( idx >= 0 )
			{
				uzData = myWsi.GetMessage( idx, send_len );
				if ( uzData != NULL )
				{
					LogMessage( E_MSG_DEBUG, "ws lws_write: len %d", send_len );

					bytes = lws_write( wsi, uzData, send_len, LWS_WRITE_TEXT );
					if ( bytes < 0 )
					{
						myWsi.ClearClient(idx);
					}
					else
					{
						myWsi.ClearMessage( idx );
					}
				}
			}
			break;

		default:
			break;
	}

	return 0;
}

enum protocols
{
	PROTOCOL_HTTP = 0,
	PROTOCOL_NIMROD,
	PROTOCOL_COUNT
};

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"nimrod-protocol",
		callback_nimrod,
		0,
		NIMROD_RX_BUFFER_BYTES,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

void CThread::websocket_init()
{
	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.ka_time = 2;
	info.ka_interval = 10;
	info.ka_probes = 2;

	m_WSContext = lws_create_context( &info );

	LogMessage( E_MSG_INFO, "websocket context created %p", m_WSContext );
}

void CThread::websocket_process( const char* szMsg )
{
	if ( strlen(szMsg) != 0 )
	{
		myWsi.AddMessage( szMsg );
	}

	lws_service( m_WSContext, /* timeout_ms = */ 20 );
}

void CThread::websocket_destroy()
{
	lws_context_destroy( m_WSContext );
}
