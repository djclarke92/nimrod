#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <modbus/modbus.h>
#include "mb_mysql.h"
#include "mb_devices.h"
#include "mb_thread.h"
#include "mb_main.h"


extern CThreadMsg gThreadMsgFromWS;


#define NIMROD_RX_BUFFER_BYTES (100)
struct payload
{
	unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + NIMROD_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	size_t len;
} received_payload;


#define MAX_LWS_CLIENTS	10
class CLWSClient {
private:
	bool bRefreshMissed;
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
	int GetCount();
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
	bRefreshMissed = false;
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
	else if ( bRefreshMissed )
	{
		LogMessage( E_MSG_INFO, "ws missed refresh");
		bRefreshMissed = false;
		AddMessage( "Refresh" );
	}
}

int CLWSClient::GetCount()
{
	int iCount = 0;

	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		if ( m_Wsi[i] != NULL )
		{
			iCount += 1;
		}
	}

	return iCount;
}
void CLWSClient::AddMessage( const char* szMsg )
{
	bool bAdded = false;

	for ( int i = 0; i < MAX_LWS_CLIENTS; i++ )
	{
		if ( m_Wsi[i] != NULL )
		{
			snprintf( (char*)&m_Data[i][LWS_SEND_BUFFER_PRE_PADDING], sizeof(m_Data[i])-LWS_SEND_BUFFER_PRE_PADDING, "%s", szMsg );

			if ( lws_get_context( m_Wsi[i] ) != NULL )
			{
				lws_callback_on_writable_all_protocol( lws_get_context( m_Wsi[i] ), lws_get_protocol( m_Wsi[i] ) );
				bAdded = true;
			}
			else
			{
				ClearClient( i );
			}
		}
	}

	if ( strcmp( szMsg, "Refresh" ) == 0 )
	{
		if ( !bAdded )
		{
			bRefreshMissed = true;
			//LogMessage( E_MSG_INFO, "ws send refresh again");
		}
		else
			bRefreshMissed = false;
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
		received_payload.data[LWS_SEND_BUFFER_PRE_PADDING+len] = '\0';
		LogMessage( E_MSG_INFO, "ws receive: '%s'", &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING] );

		gThreadMsgFromWS.PutMessage( (const char*)&received_payload.data[LWS_SEND_BUFFER_PRE_PADDING] );
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

	case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
		LogMessage( E_MSG_INFO, "ws ClientCertValidation" );
		break;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		// do nothing
		break;

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		// do nothing
		break;

	case LWS_CALLBACK_CLOSED:
		idx = myWsi.FindClient( wsi );
		if ( idx >= 0 )
		{
			myWsi.ClearClient( idx );
		}
		break;

	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:	// 71
//		LogMessage( E_MSG_INFO, "ws EventWaitCancelled");
		break;

	case LWS_CALLBACK_HTTP_BIND_PROTOCOL:	// 49
		//LogMessage( E_MSG_INFO, "ws HttpBindProtcol");
		break;

	case LWS_CALLBACK_ADD_HEADERS:	// 53
		//LogMessage( E_MSG_INFO, "ws AddHeaders");
		break;

	case LWS_CALLBACK_VHOST_CERT_AGING:	// 72
		gbCertificateAging = true;
		LogMessage( E_MSG_INFO, "Cert is aging");
		break;

	default:
		LogMessage( E_MSG_INFO, "ws callback %d", reason );
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

void ws_log_message( int iLvl, const char* szMsg )
{
	char szBuf[4096];

	snprintf( szBuf, sizeof(szBuf), "%s", szMsg );
	szBuf[strlen(szBuf)-1] = '\0';

//	LogMessage( E_MSG_INFO, "ws %d: %s", iLvl, szBuf );
}

// if you are using LetsEncrypt certs run these commands
// cd /home/nimrod
// sudo setfacl -R -m u:nimrod:rX /etc/letsencrypt/{live,archive}/flatcatit.co.nz
// sudo setfacl -m u:nimrod:rX /etc/letsencrypt/{live,archive}
// sudo ln -s /etc/letsencrypt/live/flatcatit.co.nz/fullchain.pem nimrod-cert.pem
// sudo ln -s /etc/letsencrypt/live/flatcatit.co.nz/privkey.pem nimrod-cert.key
void CThread::websocket_init()
{
	const char* pszCertFile = "/home/nimrod/nimrod-cert.pem";
	const char* pszKeyFile = "/home/nimrod/nimrod-cert.key";

	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.ka_time = 2;
	info.ka_interval = 10;
	info.ka_probes = 2;
	if ( m_bSecureWebSocket )
	{
		info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		info.ssl_cert_filepath = pszCertFile;
		info.ssl_private_key_filepath = pszKeyFile;
	}

    lws_set_log_level( 0x0007, ws_log_message );

	m_WSContext = lws_create_context( &info );

	LogMessage( E_MSG_INFO, "websocket context created %p, secure=%d", m_WSContext, (int)m_bSecureWebSocket );
}

void CThread::websocket_addmessage( const char* szMsg )
{
	if ( strlen(szMsg) != 0 )
	{
//		LogMessage( E_MSG_INFO, "ws context  %p", m_WSContext);
		myWsi.AddMessage( szMsg );
	}
}

void CThread::websocket_cancel_service()
{
	lws_cancel_service(m_WSContext);
}

void CThread::websocket_process()
{
	// timeout is ignored and the lws_service() call can block for a few seconds
//	LogMessage( E_MSG_INFO, "lws_service() %d", myWsi.GetCount());
	lws_service( m_WSContext, /* timeout_ms = */ -1 );
}

void CThread::websocket_destroy()
{
	lws_context_destroy( m_WSContext );
}
