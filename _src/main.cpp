#include "common.h"
#include "threadpool.hpp"
#include "display.hpp"
#include "handler.hpp"
#include "file_handler.hpp"


int main(int argc, char* argv[])
{
    Display disp;
    int server_fd = start_socket_http();

    ThreadPool<8> pool;
    for (;;)
    {
        fd_set rfds{};
        FD_ZERO(&rfds);
        FD_SET(server_fd, &rfds);
        timeval tv{};
        tv.tv_sec = 60 * 15; // 15 min
        // tv.tv_sec = 6; // 6 sec
        tv.tv_usec = 0;

        // line("Waiting for a client to connect..."); // this will spam log
        int ret = select(server_fd + 1, &rfds, nullptr, nullptr, &tv);
        if (ret < 0)
        {
            line("SELECT error");
            break;
        }
        else if (ret == 0)
        {
            disp.check_on_off();
            continue;
        }
        else
        {
            if (FD_ISSET(server_fd, &rfds))
            {
                struct sockaddr_in client_addr;
                int client_addr_len = sizeof(client_addr);

                int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
                if (client_socket < 0)
                {
                    line("accept failed");
                    continue;
                }

                // handle_client(client_socket, disp);                          // single thread
                pool.addTask([&]() { handle_client(client_socket, disp); });    // multi thread
            }
        }        
    }

    close(server_fd);
    return 0;
}
