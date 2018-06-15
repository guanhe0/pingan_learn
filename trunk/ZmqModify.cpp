#include "stdafx.h"
#include "ZmqModify.h"
#include <assert.h>

char ZmqModify::m_socketConnect[IP_SIZE];

ZmqModify::ZmqModify()
: m_zmqContext(NULL)
, m_zmqSocket(NULL)
{
	//创建zmq通信基础--context--环境--上下文，zmq的socket 需要在context上进行创建 
	m_zmqContext = zmq_ctx_new();
	assert(NULL != m_zmqContext);
}


ZmqModify::~ZmqModify()
{
	if (m_zmqSocket != NULL)
	{
		zmq_close(m_zmqSocket);
		m_zmqSocket = NULL;
	}
	if (m_zmqContext != NULL)
	{
		zmq_ctx_destroy(m_zmqContext);
		m_zmqContext = NULL;
	}
}

//在连接socket前，设置context环境属性
int ZmqModify::ZmqSetContext(DWORD threadNumbers, DWORD maxSockets)
{
	if (maxSockets > 1023)//数量超限
		maxSockets = 1023;
	//设置线程池数量
	zmq_ctx_set(m_zmqContext, ZMQ_IO_THREADS, threadNumbers);
	if (zmq_ctx_get(m_zmqContext, ZMQ_IO_THREADS) != threadNumbers)
		return ZMQ_ERR_SET_THREAD_NUMBER;
	//设置最大socket连接数量
	zmq_ctx_set(m_zmqContext, ZMQ_MAX_SOCKETS, maxSockets);
	if (zmq_ctx_get(m_zmqContext, ZMQ_MAX_SOCKETS) != maxSockets)
		return ZMQ_ERR_SET_MAX_SOCKET;
	return ZMQ_NO_ERR;
}

//设置通信模式，应答、订阅等
int ZmqModify::ZmqSetSocket(DWORD socketType, string IP, DWORD port, DWORD connectType, DWORD timeOut)
{
	/*
	socketType：
	请求/应答模式：ZMQ_REQ、ZMQ_REP、ZMQ_DEALER、ZMQ_ROUTER
	发布/订阅模式：ZMQ_PUB、ZMQ_SUB、ZMQ_XPUB、ZMQ_XSUB
	管道模式：ZMQ_PUSH、ZMQ_PULL
	配对模式：ZMQ_PAIR
	本地模式：ZMQ_STREAM
	*/
	//创建socket
	int res = ZMQ_NO_ERR;
	m_zmqSocket = zmq_socket(m_zmqContext, socketType);
	if (m_zmqSocket == NULL)
	{
		zmq_ctx_destroy(m_zmqContext);
		m_zmqContext = NULL;
		return ZMQ_ERR_SOCKET_CREATE;
	}
	//设置超时时间，超时断开
	if (0 != timeOut)
	{
		res = zmq_setsockopt(m_zmqSocket, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
		if (res < 0)
		{
			zmq_close(m_zmqSocket);
			zmq_ctx_destroy(m_zmqContext);
			m_zmqSocket = NULL;
			m_zmqContext = NULL;
			return ZMQ_ERR_TIMEOUT_FAIL;
		}
	}

	memset(m_socketConnect, 0, IP_SIZE * sizeof(char));
	int length = sprintf(m_socketConnect, "tcp://%s:%d", IP.c_str(), port);
	m_socketConnect[length] = '\0';
	if (CLIENT_TYPE == connectType)
	{
		res = zmq_connect(m_zmqSocket, m_socketConnect);	//绑定接收端的IP
	}
	else if (SERVER_TYPE == connectType)
	{
		res = zmq_bind(m_zmqSocket, m_socketConnect);		//绑定发送端的IP
	}
	if (res < 0)
	{
		zmq_close(m_zmqSocket);
		zmq_ctx_destroy(m_zmqContext);
		m_zmqSocket = NULL;
		m_zmqContext = NULL;
		return ZMQ_ERR_CONNECT_OR_BIND;
	}
	AfxMessageBox(CString(m_socketConnect));
	return ZMQ_NO_ERR;
}

//发送数据，当前角色是客户端
int ZmqModify::ZmqSendMsg(string & msg)
{
	int res = ZMQ_NO_ERR;
	char msgSend[1024] = { 0 };
#if 1
	//组织消息
	zmq_msg_t send_msg;
	strncpy(msgSend, msg.c_str(), msg.size());
	msgSend[msg.size() + 1] = '\0';

	//发送消息
	res = zmq_msg_init_size(&send_msg, msg.size() + 1);
	if (res < 0)
	{
		AfxMessageBox(CString(zmq_strerror(res)));
		return ZMQ_ERR_MSG_INIT_SIZE;
	}
	memcpy(zmq_msg_data(&send_msg), msgSend, msg.size() + 1);
	res = zmq_msg_send(&send_msg, m_zmqSocket, ZMQ_DONTWAIT);
	if (res < 0)
	{
		AfxMessageBox(CString(zmq_strerror(errno)));
		return ZMQ_ERR_MSG_SEND;
	}
	zmq_msg_close(&send_msg);
#else
	sprintf(msgSend, "%s", msg.c_str());
	res = zmq_send(m_zmqSocket, msgSend, sizeof(msgSend), 0);
	if (res < 0)
	{
		AfxMessageBox(CString("send message faild\n"));
		//fprintf(stderr, "send message faild\n");
		return ZMQ_ERR_SEND;
	}
#endif
	return ZMQ_NO_ERR;
}

//接收数据，当前角色是服务器
int ZmqModify::ZmqRecvMsg(string & msgGet)
{
	int res = ZMQ_NO_ERR;
#if 1
	zmq_msg_t recv_msg;
	res = zmq_msg_init(&recv_msg);
	if (res < 0)
	{
		AfxMessageBox(CString("init message faild\n"));
		return ZMQ_ERR_MSG_INIT;
	}
	res = zmq_msg_recv(&recv_msg, m_zmqSocket, 0);                    //0表示非阻塞
	if (res < 0)
	{
		if (errno == EBADF)
		{
			AfxMessageBox(L"连接超时");
		}
		else
		{
			CString str;
			str.Format(L"%s, errno = %d\n", zmq_strerror(errno), errno);
			AfxMessageBox(str);
		}
		return ZMQ_ERR_MSG_RECV;
	}
	msgGet = (char*)zmq_msg_data(&recv_msg);
	zmq_msg_close(&recv_msg);
#else
	char recvMsg[1024] = { 0 };
	//等待接收到来的消息，当超过5秒没有接到消息时，zmq_recv函数返回错误信息 ，并使用zmq_strerror函数进行错误定位 
	res = zmq_recv(m_zmqSocket, recvMsg, sizeof(recvMsg), 0);
	if (res < 0)
	{
		AfxMessageBox(CString(zmq_strerror(errno)));
		//printf("error = %s\n", zmq_strerror(res));
		return ZMQ_ERR_RECV;
	}
	AfxMessageBox(CString(recvMsg));
	//printf("received message : %s\n", recvMsg);
	msgGet = recvMsg;
#endif
	return ZMQ_NO_ERR;
}