#pragma once

#include "common.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <httpparser/httprequestparser.h>
#include <httpparser/httpresponseparser.h>

using namespace httpparser;

#define SIZE ( 1024 )
#define PORT ( 80 )

int start_socket_http()
{
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
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        cerr << "Failed to bind to port " << PORT << endl;
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        cerr << "listen failed\n";
        return 1;
    }

    return server_fd;
}
