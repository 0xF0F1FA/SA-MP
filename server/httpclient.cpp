
#include "main.h"

CHttpClient::CHttpClient(char* szBindAddress)
{
	memset(&m_Request, 0, sizeof(m_Request));
	memset(&m_Response, 0, sizeof(m_Response));
	memset(m_szBindAddress, 0, sizeof(m_szBindAddress));
	
	m_bHasBindAddress = false;

	if (szBindAddress)
	{
		strcpy(m_szBindAddress, szBindAddress);
		m_bHasBindAddress = true;
	}

	m_iError = 0;
	m_iSocket = -1;

#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(0x202, &wsaData);
#endif
}

CHttpClient::~CHttpClient()
{
	CloseConnection();

	if (m_Response.header)
		free(m_Response.header);
	if (m_Response.response)
		free(m_Response.response);

#ifdef WIN32
	WSACleanup();
#endif
}

bool CHttpClient::GetHeaderValue(char* szHeaderName, char* szReturnBuffer, int iBufSize)
{
	char* szHeaderStart;
	char* szHeaderEnd;
	int iLengthSearchHeader = strlen(szHeaderName);
	int iCopyLength;

	szHeaderStart = Util_stristr(m_Response.header, szHeaderName);
	if (!szHeaderStart) {
		return false;
	}
	szHeaderStart += iLengthSearchHeader + 1;

	szHeaderEnd = strchr(szHeaderStart, '\n');
	if (!szHeaderEnd) {
		szHeaderEnd = m_Response.header + strlen(m_Response.header);
	}

	iCopyLength = szHeaderEnd - szHeaderStart;
	if (iBufSize < iCopyLength) {
		return false;
	}

	memcpy(szReturnBuffer, szHeaderStart, iCopyLength);
	szReturnBuffer[iCopyLength] = '\0';
	return true;
}

void CHttpClient::CloseConnection()
{
#ifdef WIN32
	closesocket(m_iSocket);
#else
	close(m_iSocket);
#endif
}

bool CHttpClient::Send(char* szData)
{
	if (send(m_iSocket, szData, strlen(szData), 0) < 0) {
		m_iError = HTTP_ERROR_CANT_WRITE;
		return false;
	}
	return true;
}

int CHttpClient::Recv(char* szBuffer, int iBufferSize)
{
	return recv(m_iSocket, szBuffer, iBufferSize, 0);
}

void CHttpClient::InitRequest(int iType, char* szURL, char* szPostData, char* szReferer)
{
	char szUseURL[2048];
	char* slash_ptr;
	unsigned int slash_pos;
	char* port_char;
	char port[128];

	memset(szUseURL, 0, sizeof(szUseURL));
	strncpy(szUseURL, szURL, sizeof(szUseURL));
	
	m_Request.rtype = iType;
	
	strncpy(m_Request.referer, szReferer, sizeof(m_Request.referer));
	
	if (iType == HTTP_POST) {
		strncpy(m_Request.data, szPostData, sizeof(m_Request.data));
	}

	slash_ptr = strchr(szUseURL, '/');
	if (!slash_ptr) {
		strcat(szUseURL, "/");
		slash_ptr = strchr(szUseURL, '/');
	}
	
	slash_pos = (slash_ptr - szUseURL);
	if (slash_pos > 256)
		slash_pos = 256;
	memcpy(m_Request.host, szUseURL, slash_pos);
	m_Request.host[slash_pos] = '\0';
	
	strncpy(m_Request.file, strchr(szUseURL, '/'), sizeof(m_Request.file));

	port_char = strchr(m_Request.host, ':');
	if (port_char)
	{
		memset(port, 0, sizeof(port));
		strncpy(port, port_char + 1, sizeof(port));
		*port_char = '\0';
		m_Request.port = atoi(port);
	}
	else
		m_Request.port = 80;
}

void CHttpClient::HandleEntity()
{
	int bytes_read;
	int bytes_total;
	char buffer[4096];
	bool header_got;
	char* head_end;
	char response_code_str[4];
	char szContentType[256];
	char* pcontent_buf;
	int content_len;
	char content_len_str[64];
	bool has_content_len;

	bytes_total = 0;
	header_got = false;
	has_content_len = false;
	content_len = 0;

	memset(content_len_str, 0, 64);
	memset(buffer, 0, sizeof(buffer));

	m_Response.header = NULL;
	m_Response.response = NULL;
	m_Response.header_len = 0;
	m_Response.response_len = 0;

	while ((bytes_read = Recv(buffer, sizeof(buffer))) > 0)
	{
		SLEEP(5);

		bytes_total += bytes_read;

		m_Response.response = (char*)realloc(m_Response.response, bytes_total + 1);
		if (!m_Response.response)
		{
			bytes_total = 0;
			break;
		}

		memcpy(m_Response.response + (bytes_total - bytes_read), buffer, bytes_read);
		m_Response.response[bytes_total] = '\0';

		if (!header_got)
		{
			if ((head_end = strstr(m_Response.response, "\r\n\r\n")) != NULL
				|| (head_end = strstr(m_Response.response, "\n\n")) != NULL)
			{
				header_got = true;

				m_Response.header_len = (head_end - m_Response.response);
				m_Response.header = (char*)calloc(1, m_Response.header_len + 1);
				memcpy(m_Response.header, m_Response.response, m_Response.header_len);
				m_Response.header[m_Response.header_len] = '\0';

				if ((*(m_Response.response + m_Response.header_len)) == '\n')
				{
					bytes_total -= (m_Response.header_len + 2);
					memmove(m_Response.response, (m_Response.response + (m_Response.header_len + 2)), bytes_total);
				}
				else
				{
					bytes_total -= (m_Response.header_len + 4);
					memmove(m_Response.response, (m_Response.response + (m_Response.header_len + 4)), bytes_total);
				}

				pcontent_buf = Util_stristr(m_Response.header, "CONTENT-LENGTH:");
				if (pcontent_buf)
				{
					// Seems like things this part fucked up in linux and windows version.
					// For some reason the content_len always 0, and content_len_str always empty.
					pcontent_buf += 16;
					while (*pcontent_buf != '\n' && *pcontent_buf)
					{
						pcontent_buf++;
						content_len++;
					}

					pcontent_buf -= content_len;
					memcpy(content_len_str, pcontent_buf, content_len);

					if (content_len_str[content_len - 1] == '\r')
						content_len_str[content_len - 1] = '\0';
					else
						content_len_str[content_len] = '\0';

					content_len = atoi(content_len_str);
					if (content_len > 0x100000)
					{
						CloseConnection();
						m_iError = HTTP_ERROR_CONTENT_TOO_BIG;
						return;
					}

					has_content_len = true;
				}
			}
		}

		if (header_got && has_content_len)
			if (bytes_total >= content_len)
				break;

		if (bytes_total > 0x100000)
		{
			CloseConnection();
			m_iError = HTTP_ERROR_CONTENT_TOO_BIG;
			return;
		}
	}

	CloseConnection();

	if(m_Response.response)
		m_Response.response[bytes_total] = '\0';

	if (bytes_total <= 0 || !header_got || Util_strnicmp(m_Response.header, "HTTP", 4) != 0)
	{
		m_iError = HTTP_ERROR_MALFORMED_RESPONSE;
		return;
	}

	strncpy(response_code_str, m_Response.header + 9, 3);
	response_code_str[3] = '\0';

	m_Response.response_code = atoi(response_code_str);
	m_Response.response_len = bytes_total;
	m_Response.content_type = CONTENT_TYPE_HTML;

	if (GetHeaderValue("CONTENT-TYPE:", szContentType, sizeof(szContentType)))
	{
		if (strstr(szContentType, "text/html") != NULL)
			m_Response.content_type = CONTENT_TYPE_HTML;
		else if (strstr(szContentType, "text/plain") != NULL)
			m_Response.content_type = CONTENT_TYPE_TEXT;
		else
			m_Response.content_type = CONTENT_TYPE_UNKNOWN;
	}
}

bool CHttpClient::Connect(char* szHost, int iPort)
{
	struct hostent* hp;
	struct sockaddr_in sa;
	int optval[2];

	hp = (struct hostent*)gethostbyname(szHost);
	if (hp == NULL) {
		m_iError = HTTP_ERROR_BAD_HOST;
		return false;
	}

	memset(&sa, 0, sizeof(sa));
	memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons((unsigned short)iPort);

	if (m_bHasBindAddress)
	{
		hp = (struct hostent*)gethostbyname(szHost);
		if (!hp)
		{
			m_iError = HTTP_ERROR_BAD_HOST;
			return 0;
		}
		memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
		sa.sin_family = hp->h_addrtype;
		sa.sin_port = 0;
	}

	m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_iSocket < 0) {
		m_iError = HTTP_ERROR_NO_SOCKET;
		return false;
	}

	if (m_bHasBindAddress && bind(m_iSocket, (struct sockaddr*)&sa, sizeof(sa)) < 0)
	{
		m_iError = HTTP_ERROR_CANT_CONNECT;
		return false;
	}

	optval[0] = 20000;
	optval[1] = 20000;
	setsockopt(m_iSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&optval, sizeof(optval));

	if (connect(m_iSocket, (struct sockaddr*)&sa, sizeof sa) < 0) {
		CloseConnection();
		m_iError = HTTP_ERROR_CANT_CONNECT;
		return false;
	}

	return true;
}

void CHttpClient::Process()
{
	int header_len;
	int data_len;

	if (!Connect(m_Request.host, m_Request.port)) {
		return;
	}

	switch (m_Request.rtype)
	{
	case HTTP_GET:
		header_len = strlen(m_Request.file) + strlen(m_Request.host) +
			strlen(m_Request.referer) + (strlen(GET_FORMAT) - 8) + strlen(USER_AGENT);
		if (header_len > sizeof(m_Request.request_head))
			return;
		sprintf(m_Request.request_head, GET_FORMAT, m_Request.file, USER_AGENT,
			m_Request.referer, m_Request.host);
		break;
	case HTTP_POST:
		data_len = strlen(m_Request.data);
		header_len = data_len + strlen(m_Request.file) +
			strlen(m_Request.host) + strlen(m_Request.referer) +
			strlen(POST_FORMAT) + strlen(USER_AGENT);
		if (header_len > sizeof(m_Request.request_head))
			return;
		sprintf(m_Request.request_head, POST_FORMAT, m_Request.file, USER_AGENT, m_Request.referer,
			m_Request.host, data_len, m_Request.data);
		break;

	case HTTP_HEAD:
		header_len = strlen(m_Request.file) + strlen(m_Request.host) +
			strlen(m_Request.referer) + (strlen(HEAD_FORMAT) - 8) + strlen(USER_AGENT);
		if (header_len > sizeof(m_Request.request_head))
			return;
		sprintf(m_Request.request_head, HEAD_FORMAT, m_Request.file, USER_AGENT,
			m_Request.referer, m_Request.host);
		break;
	}

	if (Send(m_Request.request_head))
		HandleEntity();
}

int CHttpClient::ProcessURL(int iType, char* szURL, char* szPostData, char* szReferer)
{
	InitRequest(iType, szURL, szPostData, szReferer);
	Process();
	return m_iError;
}
