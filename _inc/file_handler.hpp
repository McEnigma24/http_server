#pragma once

#include "common.h"

#include <fstream>
#include <sstream>
#include <string>

struct FileHandler
{
    #define FILEPATH "../last_received_string"

    static string get_text()
    {
        std::ifstream fin(FILEPATH);
        if (!fin)
        {
            cout << "Nie można otworzyć pliku: " << FILEPATH << endl;;
            return {};
        }
        std::ostringstream ss;
        ss << fin.rdbuf();
        fin.close();

        return ss.str();
    }

    static bool overwrite(const std::string& new_text)
    {
        std::ofstream fout(FILEPATH, std::ios::out | std::ios::trunc);
        if (!fout)
        {
            std::cerr << "Nie można otworzyć pliku do nadpisania: " << FILEPATH << "\n";
            return false;
        }
        fout << new_text;

        if (!fout)
        {
            std::cerr << "Błąd podczas zapisu do pliku: " << FILEPATH << "\n";
            fout.close();
            return false;
        }
        fout.close();
        return true;
    }

    static bool append(const std::string& more_text)
    {
        std::ofstream fout(FILEPATH, std::ios::out | std::ios::app);
        if (!fout)
        {
            std::cerr << "Nie można otworzyć pliku do dopisania: " << FILEPATH << "\n";
            return false;
        }
        fout << more_text;
        
        if (!fout)
        {
            std::cerr << "Błąd podczas dopisywania do pliku: " << FILEPATH << "\n";
            fout.close();
            return false;
        }
        fout.close();
        return true;
    }
};
