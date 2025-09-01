#include "__preprocessor__.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <httpparser/httprequestparser.h>
#include <httpparser/httpresponseparser.h>

#include <nlohmann/json.hpp>

#define SIZE 1024

using namespace httpparser;

#ifdef BUILD_EXECUTABLE
int main(int argc, char* argv[])
{
    srand(time(NULL));

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        cerr << "listen failed\n";
        return 1;
    }

    for (;;)
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        cout << "Waiting for a client to connect...\n";

        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
        if (client_socket < 0)
        {
            cerr << "accept failed\n";
            continue;
        }
        cout << "Client connected\n\n\n";

        char client_request[SIZE];
        recv(client_socket, client_request, sizeof(client_request), 0);

        // string client_req(client_request);

        Request request;
        HttpRequestParser parser;

        HttpRequestParser::ParseResult res = parser.parse(request, client_request, client_request + strlen(client_request));

        if (res == HttpRequestParser::ParsingCompleted)
            std::cout << request.inspect() << std::endl;
        else
            std::cerr << "Parsing failed" << std::endl;

        string resp = "HTTP/1.1 200 OK\r\n"
                      "Connection: close\r\n"
                      "\r\n";

        send(client_socket, resp.c_str(), resp.size(), 0);
        close(client_socket);
    }

    close(server_fd);

    return 0;
}
#endif