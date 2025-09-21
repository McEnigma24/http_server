#pragma once

#include "common.h"

#include <cstring>
#include <unordered_map>
#include <thread>
#include <arpa/inet.h>
#include <cstdlib>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "file_handler.hpp"
#include "time.hpp"

extern "C" {
    #include "ssd1306.h"
    #include "font.h"
}

#define MAX_DISPLAY_LINE_LENGTH ( 19 )
#define MAX_DISPLAY_LINE_NUMER ( 8 )

class Display
{
    uint8_t font_size;
    string screen_buffer;
    std::mutex scbuff_mut;

    // #define debug_line { varr(i); varr(c); varr(word_length); varr(last_space_idx); var(chars_left_in_line); }
    #define debug_line 

    void prepare_string()
    {
        int word_length = 0;
        int last_space_idx = 0;
        int chars_left_in_line = MAX_DISPLAY_LINE_LENGTH;

        for(int i=0; i<screen_buffer.size(); i++)
        {
            char c = screen_buffer[i];

            if(' ' == c)
            {
                word_length = 0;
                last_space_idx = i;
                
                if(0 == chars_left_in_line)
                {
                    // screen_buffer[last_space_idx] = '\n';
                    chars_left_in_line = MAX_DISPLAY_LINE_LENGTH;
                    debug_line
                    continue;
                }
                debug_line
            }
            else if('\n' == c)
            {
                word_length = 0;
                chars_left_in_line = MAX_DISPLAY_LINE_LENGTH;
                debug_line
            }
            else
            {
                word_length++;

                // słowo jest dłuższe niż linijka
                if(word_length > MAX_DISPLAY_LINE_LENGTH)
                {
                    screen_buffer.resize(
                        screen_buffer.size()
                        + 2
                    );

                    memmove(&screen_buffer[i], &screen_buffer[i-2], screen_buffer.size() - i);

                    screen_buffer[i-2] = '-';
                    screen_buffer[i-1] = '\n';

                    word_length = 0;
                    chars_left_in_line = MAX_DISPLAY_LINE_LENGTH;
                    debug_line

                    continue;
                }

                // ostatnie słowo się nie mieści
                if(not (0 < chars_left_in_line))
                {
                    screen_buffer[last_space_idx] = '\n';
                    chars_left_in_line = MAX_DISPLAY_LINE_LENGTH - word_length;
                    debug_line
                    continue;
                }
                debug_line
            }

            chars_left_in_line--;
        }
    }

    void refresh_screen()
    {
        if(not (0 < screen_buffer.size())) return;
        if(screen_buffer.size() > (MAX_DISPLAY_LINE_LENGTH * MAX_DISPLAY_LINE_NUMER))
        {
            ssd1306_oled_set_XY(0, 0);
            ssd1306_oled_write_string(font_size, (char*)"too much characters in screen_buffer");
            return;
        }

        ssd1306_oled_clear_screen();

        prepare_string();

        int global_y = 0;
        void* current_part = screen_buffer.data();
        int chars_left = screen_buffer.size();

        while(chars_left > 0 && global_y < MAX_DISPLAY_LINE_NUMER)
        {
            char buffer[MAX_DISPLAY_LINE_LENGTH + 1]{};
            memcpy(buffer, current_part, MAX_DISPLAY_LINE_LENGTH);
            buffer[MAX_DISPLAY_LINE_LENGTH] = '\0';

            if(' ' == buffer[0] || '\n' == buffer[0])
            {
                current_part += 1;
                chars_left -= 1;
                continue;
            }

            char* n_pos = strchr(buffer, '\n');
            if(n_pos != nullptr)
            {
                int dist = (char*)buffer - (char*)n_pos; if(dist < 0) dist = -dist;

                (*n_pos) = '\0';
                ssd1306_oled_set_XY(0, global_y++);
                ssd1306_oled_write_string(font_size, buffer);

                dist++;
                current_part += dist;
                chars_left -= dist;
                continue;
            }

            ssd1306_oled_set_XY(0, global_y++);
            ssd1306_oled_write_string(font_size, buffer);

            current_part += MAX_DISPLAY_LINE_LENGTH;
            chars_left -= MAX_DISPLAY_LINE_LENGTH;
        }
    }

    // void eliminate_incorrect_letters(std::string& line)
    // {
    //     static const std::unordered_map<wchar_t, wchar_t> fixing {
    //         {'ę', 'e'}, {'ą', 'a'}, {'ć', 'c'}, 
    //         {'ś', 's'}, {'ó', 'o'}, {'ł', 'l'}
    //     };

    //     for(auto& c : line)
    //     {
    //         auto it = fixing.find(c);
    //         if (it != fixing.end())
    //         {
    //             exit(EXIT_FAILURE);
    //         }
    //     }
    // }

    void separate_nlines(std::string& line)
    {
        for(int i=0; i<line.size() - 1; i++)
        {
            if('\n' == line[i] && ' ' != line[i+1])
            {
                line.insert(i + 1, " ");
            }
        }
    }

    std::string escape(const std::string& s)
    {
        std::string out;
        for (char c : s) {
            switch (c) {
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                case '\\': out += "\\\\"; break;
                default:
                    if (std::isprint(static_cast<unsigned char>(c))) {
                        out += c;
                    } else {
                        // np. dla innych kontrolnych: wypisz w formie \xHH
                        char buf[10];
                        std::snprintf(buf, sizeof buf, "\\x%02X", static_cast<unsigned char>(c));
                        out += buf;
                    }
            }
        }
        return out;
    }

    void sending_on_socket() const
    {
        sending_on_socket(screen_buffer);
    }

    void sending_on_socket(const std::string& text) const
    {
        // Adres serwera / port — dopasuj do tego, co masz
        const char* server_ip = "127.0.0.1";  // albo IP Raspberry Pi / localhost
        const int server_port = 12345;

        // Tworzenie socketu
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Nie mozna utworzyc socketu\n";
            return;
        }

        // Ustawienie adresu serwera
        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);
        serv_addr.sin_addr.s_addr = inet_addr(server_ip);

        // Połączenie
        if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Blad polaczenia do " << server_ip << ":" << server_port << "\n";
            close(sockfd);
            return;
        }

        // Wysłanie wiadomości
        ssize_t bytes_sent = send(sockfd, text.c_str(), text.size(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Blad wyslania danych\n";
            close(sockfd);
            return;
        }

        linee("Wyslano: "); cout << text << "\n";

        // Zamknięcie połączenia
        close(sockfd);
    }

    void init_small_lcd()
    {
        // inicjalizacja: numer urządzenia i2c — 1 oznacza /dev/i2c-1
        if (ssd1306_init(1) != 0)
        {
            fprintf(stderr, "SSD1306 init failed\n");
            exit(EXIT_FAILURE);
        }

        ssd1306_oled_default_config(SSD1306_128_64_COLUMNS, SSD1306_128_64_LINES);
        ssd1306_oled_clear_screen();
        ssd1306_oled_onoff(1);

        // font_size = SSD1306_FONT_NORMAL;
        font_size = SSD1306_FONT_SMALL;

        // sleep(5);
    }
    
    void init_big_lcd()
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            std::cerr << "fork failed\n";
            exit(EXIT_FAILURE);
        }

        if (0 == pid)
        {
            const char* cmd = "python3 ../display.py";
            int ret = system(cmd);
            if (ret != 0) std::cerr << "Błąd uruchomienia skryptu, kod: " << ret << "\n";

            exit(EXIT_FAILURE);
        }
        
        sleep(3);
    }

public:
    
    Display()
    {
        init_big_lcd();

        screen_buffer = FileHandler::get_text();

        // refresh_screen();
        sending_on_socket();
        check_on_off();
    }

    void check_on_off() const
    {
        auto [hour, min, sec] = getTime();
        if(in_bound(
                        OFF_START_HOUR,
                        hour,
                        OFF_END_HOUR
                    ))
        { OFF(); }
        else { ON(); }
    }

    void ON() const
    {
        // sending unchanged screen_buffer //
        sending_on_socket();
    }

    void OFF() const
    {
        sending_on_socket(" ");
    }

    void write(std::string line, bool append = false)
    {
        // eliminate_incorrect_letters(line);

        // LOGS //
        var(escape(line));
        separate_nlines(line);

        std::unique_lock<std::mutex> lock(scbuff_mut);
        if(append) screen_buffer += " " + line;
        else screen_buffer = line;

        // refresh_screen();

        sending_on_socket();
    }
};
