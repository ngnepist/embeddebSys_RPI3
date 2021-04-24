#include "OSNet.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

typedef int SOCKET;
#define SD_BOTH SHUT_RDWR
#define closesocket(sock) close((sock))

using namespace std;


double Humidite = 0;
double Temperature = 0;
double Pression = 0;

void thread_web_server(){

    SOCKET s, s_cli;
    struct sockaddr_in sa;
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char hostname[NI_MAXHOST];
    char service[NI_MAXSERV];
    char buf[1024];
    char content[1024];
    int contentlength = 0;
    int ret = 0;
    int reuse = 1;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int));
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((unsigned short)atoi("32768"));
    sa.sin_addr.s_addr = inet_addr("0.0.0.0");
    ret = bind(s, (struct sockaddr*)&sa, sizeof(sa));

    if (ret != 0) printf("bind() error\n");
    ret = listen(s, 1);
    if (ret != 0) printf("listen() error\n");

    for (;;)
    {
        memset(&addr, 0, sizeof(addr));
        addrlen = sizeof(addr);
        s_cli = accept(s, (struct sockaddr*)&addr, &addrlen);
        if (s_cli == -1)
        {
            printf("accept() error\n");
            continue;
        }
        getnameinfo((struct sockaddr*)&addr, addrlen, hostname, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);

        printf("Accepted connection from %s:%s\n", hostname, service);


        fd_set sock_set;
        FD_ZERO(&sock_set);
        FD_SET(s_cli, &sock_set);
        struct timeval tv;
        tv.tv_sec = 0; tv.tv_usec = 500000; // Timeout of 500 ms. Firefox was immediately sending the GET request, while Chrome needed more time, Internet Explorer even more...
        ret = select(s_cli+1, &sock_set, NULL, NULL, &tv);
        if (ret > 0) // This means that select() detected that at least 1 socket of the fd_set in 2nd parameter started to send data, a ret of 0 would mean timeout, < 0 would mean error...
        {
            if ((recv(s_cli, buf, sizeof(buf), 0) >= 3)&&(buf[0] == 'G')) // Lazy detection of the beginning of the HTTP 1.1 GET request sent by web browsers...
            {
                printf("Web browser detected\n");

                // <meta http-equiv=refresh content=3> this balise refresh the web page after 10s
                sprintf(content,
                        "<html>"
                            "<head>"
                                "<title>STATION METEO</title>"
                                "<meta http-equiv=refresh content=10>"
                            "</head>"
                            "<body>To %s:%s :"
                                "<h1>STATION METEO : MAJ toutes les 10 secondes</h1>"
                                    "<h2>Temperature = %f Celsus</h2>"
                                    "<h2>Humidite = %f %</h2>"
                                    "<h2> Pression = %f Pa</h2>"
                            "</body>"
                        "</html>\r\n"
                        "\r\n"
                        "\r\n", hostname, service, Temperature, Humidite, Pression);

                contentlength = strlen(content); // Maybe we should not include \r\n\r\n...
                sprintf(buf, "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Server: Server/1.0\r\n"
                             "Connection: close\r\n" // This will make Firefox shutdown and close the socket after receiving the webpage, which will generate a recv() error on the server side if a recv() is attempted...
                        "Content-Length: %d\r\n"
                        "\r\n"
                        "\r\n", contentlength);
                strcat(buf, content);
                ret = sendall(s_cli, buf, strlen(buf));
                if (ret != EXIT_SUCCESS) printf("sendall() error\n");
                // Since we have finished with the web browser, disconnect...
                shutdown(s_cli, SD_BOTH);
                closesocket(s_cli);
                continue;
            }
            else
            {
                printf("Web browser not detected\n");
                //ret = sendall(s_cli, argv[1], strlen(argv[1]));
                sprintf(buf, "To %s:%s", hostname, service);
                ret = sendall(s_cli, buf, strlen(buf));
                if (ret != EXIT_SUCCESS)
                {
                    printf("sendall() error\n");
                    shutdown(s_cli, SD_BOTH);
                    closesocket(s_cli);
                    continue;
                }
            }
        }
    }
    shutdown(s, SD_BOTH);
    closesocket(s);
    getchar();
    return;

}


int main(int argc, char* argv[])
{
    for (;;)
    {
        thread_web_server();
    }

    return EXIT_SUCCESS;
}