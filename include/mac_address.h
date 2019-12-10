// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <array>
#include <iostream>
#include <iomanip>
#include <string>

namespace Network
{
    struct MacAddress
    {
        std::array<std::uint8_t, 6> address;
    };

    inline std::istream& operator>> (std::istream& input_stream, MacAddress& mac_address)
    {
        for (std::uint8_t i = 1; i <= mac_address.address.size(); ++i)
        {
            std::uint16_t read_byte = 0;
            input_stream >> std::setbase(16) >> read_byte >> std::setbase(10);
            mac_address.address[i - 1] = static_cast<std::uint8_t>(read_byte);
            if (i != mac_address.address.size())
            {
                char separator;
                input_stream >> separator;
            }
        }
        return input_stream;
    }

    inline std::string to_string(const MacAddress& mac_address)
    {
        std::stringstream result;
        for (std::uint8_t i = 1; i <= mac_address.address.size(); ++i)
        {
            result << std::setw(2) << std::fixed << std::setfill('0') << std::hex << std::uppercase << static_cast<std::uint16_t>(mac_address.address[i - 1]);
            if (i != mac_address.address.size())
            {
                result << "-";
            }
        }
        return result.str();
    }
}
