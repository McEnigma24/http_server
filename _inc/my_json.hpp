#pragma once

#include "common.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void print_json(const json& j, const std::string& prefix = "")
{
    if (j.is_object())
    {
        for (auto& [key, val] : j.items())
        {
            line("its obj");
            std::string new_prefix = prefix.empty() ? key : (prefix + "." + key);
            print_json(val, new_prefix);
        }
    }
    else if (j.is_array())
    {
        for (size_t i = 0; i < j.size(); ++i)
        {
            std::string new_prefix = prefix + "[" + std::to_string(i) + "]";
            print_json(j[i], new_prefix);
        }
    }
    else
    {
        // j to typ prymitywny: liczba, string, bool albo null
        std::cout << prefix << " : " << j << "\n";
    }
}