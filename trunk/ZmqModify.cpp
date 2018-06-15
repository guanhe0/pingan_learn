#include "stdafx.h"
#include "ZmqModify.h"
#include <assert.h>

char ZmqModify::m_socketConnect[IP_SIZE];

ZmqModify::ZmqModify()
: m_zmqContext(NULL)
, m_zmqSocket(NULL)
{
	//����zmqͨ�Ż���--context--����--�����ģ�zmq��socket ��Ҫ��context�Ͻ��д��� 
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

//������socketǰ������context��������
int ZmqModify::ZmqSetContext(DWORD threadNumbers, DWORD maxSockets)
{
	if (maxSockets > 1023)//��������
		maxSockets = 1023;
	//�����̳߳�����
	zmq_ctx_set(m_zmqContext, ZMQ_IO_THREADS, threadNumbers);
	if (zmq_ctx_get(m_zmqContext, ZMQ_IO_THREADS) != threadNumbers)
		return ZMQ_ERR_SET_THREAD_NUMBER;
	//�������socket��������
	zmq_ctx_set(m_zmqContext, ZMQ_MAX_SOCKETS, maxSockets);
	if (zmq_ctx_get(m_zmqContext, ZMQ_MAX_SOCKETS) != maxSockets)
		return ZMQ_ERR_SET_MAX_SOCKET;
	return ZMQ_NO_ERR;
}

//����ͨ��ģʽ��Ӧ�𡢶��ĵ�
int ZmqModify::ZmqSetSocket(DWORD socketType, string IP, DWORD port, DWORD connectType, DWORD timeOut)
{
	/*
	socketType��
	����/Ӧ��ģʽ��ZMQ_REQ��ZMQ_REP��ZMQ_DEALER��ZMQ_ROUTER
	����/����ģʽ��ZMQ_PUB��ZMQ_SUB��ZMQ_XPUB��ZMQ_XSUB
	�ܵ�ģʽ��ZMQ_PUSH��ZMQ_PULL
	���ģʽ��ZMQ_PAIR
	����ģʽ��ZMQ_STREAM
	*/
	//����socket
	int res = ZMQ_NO_ERR;
	m_zmqSocket = zmq_socket(m_zmqContext, socketType);
	if (m_zmqSocket == NULL)
	{
		zmq_ctx_destroy(m_zmqContext);
		m_zmqContext = NULL;
		return ZMQ_ERR_SOCKET_CREATE;
	}
	//���ó�ʱʱ�䣬��ʱ�Ͽ�
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
		res = zmq_connect(m_zmqSocket, m_socketConnect);	//�󶨽��ն˵�IP
	}
	else if (SERVER_TYPE == connectType)
	{
		res = zmq_bind(m_zmqSocket, m_socketConnect);		//�󶨷��Ͷ˵�IP
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

//�������ݣ���ǰ��ɫ�ǿͻ���
int ZmqModify::ZmqSendMsg(string & msg)
{
	int res = ZMQ_NO_ERR;
	char msgSend[1024] = { 0 };
#if 1
	//��֯��Ϣ
	zmq_msg_t send_msg;
	strncpy(msgSend, msg.c_str(), msg.size());
	msgSend[msg.size() + 1] = '\0';

	//������Ϣ
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

//�������ݣ���ǰ��ɫ�Ƿ�����
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
	res = zmq_msg_recv(&recv_msg, m_zmqSocket, 0);                    //0��ʾ������
	if (res < 0)
	{
		if (errno == EBADF)
		{
			AfxMessageBox(L"���ӳ�ʱ");
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
	//�ȴ����յ�������Ϣ��������5��û�нӵ���Ϣʱ��zmq_recv�������ش�����Ϣ ����ʹ��zmq_strerror�������д���λ 
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