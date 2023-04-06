#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

using namespace std;

const int MAX_EVENTS = 10;
const int MAX_BUF_SIZE = 1024;

int main(int argc, char *argv[]) {
    // 创建 socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Error: could not create socket." << endl;
        return -1;
    }

    // 绑定地址和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Error: could not bind to address." << endl;
        return -1;
    }

    // 监听连接请求
    if (listen(server_fd, 5) == -1) {
        cerr << "Error: could not listen on socket." << endl;
        return -1;
    }

    // 创建 epoll 实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        cerr << "Error: could not create epoll instance." << endl;
        return -1;
    }

    // 添加监听套接字到 epoll 实例
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        cerr << "Error: could not add server socket to epoll instance." << endl;
        return -1;
    }

    struct epoll_event events[MAX_EVENTS];
    char buf[MAX_BUF_SIZE];

    while (true) {
        // 等待
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            cerr << "Error: epoll wait failed." << endl;
            break;
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                // 处理连接请求
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    cerr << "Error: could not accept connection." << endl;
                    break;
                }

                // 将连接套接字添加到 epoll 实例中
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    cerr << "Error: could not add client socket to epoll instance." << endl;
                    break;
                }

                cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << endl;
            } else {
                // 处理客户端请求
                int client_fd = events[i].data.fd;
                int num_bytes = recv(client_fd, buf, MAX_BUF_SIZE, 0);
                if (num_bytes == -1) {
                    cerr << "Error: could not receive data from client." << endl;
                    break;
                }

                if (num_bytes == 0) {
                    // 客户端关闭连接
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                    cout << "Client disconnected." << endl;
                } else {
                    // 向客户端发送响应
                    buf[num_bytes] = '\0';
                    cout << "Received data from client: " << buf << endl;
                    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 18\r\n\r\nHello, World!\r\n";
                    send(client_fd, response, strlen(response), 0);
                }
            }
        }
    }
    // 关闭监听套接字
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, server_fd, NULL);
    close(server_fd);

    // 关闭 epoll 实例
    close(epoll_fd);

    return 0;
}
