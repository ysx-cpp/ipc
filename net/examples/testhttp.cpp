#include <iostream>
#include "../httpserver.h"
#include "../httpclient.h"
#include "../logdefine.h"

using ipc::net::HttpClient;
using ipc::net::HttpServer;

int main(int argc, char *argv[]) 
{
    if (argc != 4)
    {
        NET_LOGERR("param error!");
        return __LINE__;
    }

    std::string host = argv[2];
    unsigned short port = std::stod(argv[3]);

    if (strcmp(argv[1], "client") == 0)
    {
        try {
            HttpClient client(host, "./");
            client.SendRequest();
            client.Run();
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }
    else if (strcmp(argv[1], "server") == 0)
    {
        try {
            HttpServer server(host, port);
            server.Start();
        } catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }
    else
    {
        NET_LOGERR("Param error! please input client or server." );
        return __LINE__;
    }

    return 0;
}