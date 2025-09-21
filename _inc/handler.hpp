#pragma once

#include "common.h"

#include "display.hpp"
#include "http.hpp"
#include "my_json.hpp"
#include "file_handler.hpp"

#include <iomanip>

// funkcja do dumpowania hex
void hexdump(const std::string &data)
{
    std::cout << "Hex dump (" << data.size() << " bytes):\n";
    for (size_t i = 0; i < data.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(data[i]);
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(c) << " ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }
    std::cout << std::dec << "\n";
}

#define EXIT_SOCKET_TASK      line("EXIT_SOCKET_TASK"); close(client_socket_fd); return;

void handle_client(int client_socket_fd, Display& disp)
{
    line("Client connected");
    
    Request req_global;
    char client_request[SIZE];
    string full_msg;

    while ( true )
    {
        ssize_t n = recv(client_socket_fd, client_request, sizeof(client_request), 0);
        if (n > 0)
        {
            full_msg.append(client_request, n);

            Request req;
            HttpRequestParser parser;
            HttpRequestParser::ParseResult res = parser.parse(req, full_msg.data(), full_msg.data() + full_msg.size());

            if (res != HttpRequestParser::ParsingCompleted) { continue; }
            else { req_global = req; break; }
        }
        else { EXIT_SOCKET_TASK }
    }

    if ( req_global.uri != "/notes" ) { EXIT_SOCKET_TASK }

    // LOGS //
    var(req_global.inspect());
    for(auto&  c : req_global.content) { cout << c << " "; }
    cout << endl;

    json j;
    try
    {
        j = json::parse(req_global.content.begin(), req_global.content.end());
    }
    catch (...) { EXIT_SOCKET_TASK }

    // print_json(j);
    if (!j.is_object()) { EXIT_SOCKET_TASK }
    

    string text = j["text"];
    bool append = j["append"];

    if( "~~ON~~" == text)
    {
        // just bring back what was in the file //
        disp.ON();
    }
    else if ("~~OFF~~" == text)
    {
        // just send empty text to displey, no overwriting the file //
        disp.OFF();
    }
    else
    {
        disp.write(text, append);

        if(append) FileHandler::append(text);
        else FileHandler::overwrite(text);
    }

    // append : true
    // text : "test tes.    Zuza Lover\n\nysud\n\nhdjs"

    string resp = "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "\r\n";

    send(client_socket_fd, resp.c_str(), resp.size(), 0);
    close(client_socket_fd);
}
