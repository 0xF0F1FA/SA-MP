
#ifndef SAMPSRV_HTTPCLIENT_H
#define SAMPSRV_HTTPCLIENT_H

#define HTTP_SUCCESS 0
#define HTTP_ERROR_BAD_HOST 1
#define HTTP_ERROR_NO_SOCKET 2
#define HTTP_ERROR_CANT_CONNECT 3
#define HTTP_ERROR_CANT_WRITE 4
#define HTTP_ERROR_CONTENT_TOO_BIG 5
#define HTTP_ERROR_MALFORMED_RESPONSE 6

#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_HEAD 3

#define CONTENT_TYPE_UNKNOWN 0
#define CONTENT_TYPE_TEXT 1
#define CONTENT_TYPE_HTML 2

#define USER_AGENT "SAMP/0.3"

#define GET_FORMAT "GET %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n"
#define POST_FORMAT "POST %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: %u\r\n\r\n%s"
#define HEAD_FORMAT "HEAD %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n"

typedef struct
{
	unsigned short port;
	int rtype;
	char host[256];
	char file[4096];
	char data[8192];
	char referer[256];
	char request_head[16384];
} HTTP_REQUEST;

typedef struct
{
	char* header;
	char* response;
	unsigned int header_len;
	unsigned long response_len;
	unsigned int response_code;
	unsigned int content_type;
} HTTP_RESPONSE;

class CHttpClient
{
private:
	int m_iSocket;
	int m_iError;
	HTTP_REQUEST m_Request;
	HTTP_RESPONSE m_Response;
	char m_szBindAddress[256];
	bool m_bHasBindAddress;

public:
	CHttpClient(char* szBindAddress);
	~CHttpClient();

	bool GetHeaderValue(char* szHeaderName, char* szReturnBuffer, int iBufSize);
	void CloseConnection();
	bool Send(char* szData);
	int Recv(char* szBuffer, int iBufferSize);
	void InitRequest(int iType, char* szURL, char* szPostData, char* szReferer);
	void HandleEntity();
	bool Connect(char* szHost, int iPort);
	void Process();
	int ProcessURL(int iType, char* szURL, char* szPostData, char* szReferer);

	char* GetDocument() { return m_Response.response; };
	int GetResponseCode() { return m_Response.response_code; };
	int GetErrorCode() { return m_iError; };
};

#endif // SAMPSRV_HTTPCLIENT_H
