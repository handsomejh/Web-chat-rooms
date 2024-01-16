#include<iostream>
#include<string>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<map>


const int MAX_USER = 512;

struct Client
{
	int sockfd;
	std::string name;
};

int main()
{


	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("socket error\n");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9999);

	int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		printf("bind error\n");
		return -1;
	}

	ret = listen(sockfd, 1024);
	if (ret < 0)
	{
		printf("listen error\n");
		return -1;
	}

	int epld = epoll_create1(0);
	if (epld < 0)
	{
		printf("epoll error\n");
		return -1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	ret = epoll_ctl(epld, EPOLL_CTL_ADD, sockfd, &ev);
	if (ret < 0)
	{
		printf("epoll_ctl error\n");
		return -1;
	}

	std::map<int, Client>clients;


	while (1)
	{
		struct epoll_event evs[MAX_USER];
		int n = epoll_wait(epld, evs, MAX_USER, -1);
		if (n < 0)
		{
			printf("epoll_wait error\n");
			break;
		}

		for (int i = 0; i < n; i++)
		{
			int fd = evs[i].data.fd;
			if (fd == sockfd)
			{
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
				if (client_sockfd < 0)
				{
					printf("connect error\n");
					continue;
				}

				struct epoll_event ev_client;
				ev_client.events = EPOLLIN;
				ev_client.data.fd = client_sockfd;
				ret = epoll_ctl(epld, EPOLL_CTL_ADD, client_sockfd, &ev_client);
				if (ret < 0)
				{
					printf("epoll_ctl error\n");
					break;
				}
				printf("%s正在连接。。。\n", client_addr.sin_addr.s_addr);

				Client client;
				client.sockfd = client_sockfd;
				client.name = "";

				clients[client_sockfd] = client;

			}
			else
			{
				char buffer[1024];
				int n = read(fd, buffer, 1024);
				if (n < 0)
				{
					//TODO 处理错误
					break;
				}
				else if (n == 0)
				{
					close(fd);
					epoll_ctl(epld, EPOLL_CTL_DEL, fd, 0);
					clients.erase(fd);
				}
				else
				{
					std::string msg(buffer, n);

					if (clients[fd].name == "")
					{
						clients[fd].name = msg;
					}
					else
					{
						std::string name = clients[fd].name;

						for (auto& c : clients)
						{
							if (c.first != fd)
							{
								write(c.first, ('[' + name + ']' + ": " + msg).c_str(), msg.size() + name.size() + 4);
							}
						}
					}

				}
			}
		}
	}

	close(epld);
	close(sockfd);
	return 0;
}