#ifndef __TCP_CLIENT_H_
#define __TCP_CLIENT_H_

#include <string>

using std::string;

class CTcpClient {
public:
    CTcpClient(string host, unsigned short port);
    ~CTcpClient();

    /*
	        发送完size大小的缓冲内容才返回
	        返回-1异常
	        返回0 对方关闭连接
	 */
    int Send(const char* buffer, int size);

	 /*
	        接收完size大小的缓冲内容才返回
	        返回-1异常
	        返回0 对方关闭连接
	 */
    int Recv(char* buffer, int size);

    /*
	        连接远端地址
	        返回 -1异常 
	        返回 0 连接成功 
	 */
    int Connect();

    int Disconneced();

	int SetSendTimeout(unsigned int sec);

	int SetRecvTimeout(unsigned int sec);

	int SetNonblock();

	int Reconnect();

private:
    int	socketfd_;
    string	host_;
    unsigned short port_;

    bool IsConnected;
private:
	int Socket();
    int Close();

};

#endif