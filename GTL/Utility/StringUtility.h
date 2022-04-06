// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.04.06

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#StringUtility

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

namespace gtl
{
    inline std::wstring ConvertNarrowToWide(std::string const& input)
    {
        std::wstring output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](int32_t c)
            {
                std::wint_t converted = std::btowc(c);
                return (converted != WEOF ? static_cast<wchar_t>(converted) : L' ');
            });
        return output;
    }

    inline std::string ConvertWideToNarrow(std::wstring const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](std::wint_t c)
            {
                int32_t converted = std::wctob(c);
                return (converted != EOF ? static_cast<char>(converted) : ' ');
            });
        return output;
    }

    inline std::string ToLower(std::string const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](int32_t c) { return static_cast<char>(std::tolower(c)); });
        return output;
    }

    inline std::string ToUpper(std::string const& input)
    {
        std::string output;
        std::transform(input.begin(), input.end(), std::back_inserter(output),
            [](int32_t c) { return static_cast<char>(std::toupper(c)); });
        return output;
    }

    // In the default locale for C++ strings, the whitespace characters are
    // space (0x20, ' '), form feed (0x0C, '\f'), line feed (0x0A, '\n'),
    // carriage return (0x0D, '\r'), horizontal tab (0x09, '\t') and
    // vertical tab (0x0B, '\v'). See
    // https://en.cppreference.com/w/cpp/string/byte/isspace
    // for a table of ASCII values and related is* and isw* functions (with
    // 'int32_t ch' input) that return 0 or !0.
    inline void GetTokens(std::string const& input, std::string const& whiteSpace,
        std::vector<std::string>& tokens)
    {
        std::string tokenString(input);
        tokens.clear();
        while (tokenString.length() > 0)
        {
            // Find the beginning of a token.
            auto begin = tokenString.find_first_not_of(whiteSpace);
            if (begin == std::string::npos)
            {
                // All tokens have been found.
                break;
            }

            // Strip off the white space.
            if (begin > 0)
            {
                tokenString = tokenString.substr(begin);
            }

            // Find the end of the token.
            auto end = tokenString.find_first_of(whiteSpace);
            if (end != std::string::npos)
            {
                std::string token = tokenString.substr(0, end);
                tokens.push_back(token);
                tokenString = tokenString.substr(end);
            }
            else
            {
                // This is the last token.
                tokens.push_back(tokenString);
                break;
            }
        }
    }

    // For basic text extraction, choose 'whiteSpace' to be ASCII values
    // 0x00-0x20,0x7F-0xFF in GetTokens(...).
    inline void GetTextTokens(std::string const& input,
        std::vector<std::string>& tokens)
    {
        static std::string const whiteSpace = []
        {
            std::string temp;
            for (size_t i = 0; i <= 32; ++i)
            {
                temp += static_cast<char>(i);
            }
            for (size_t i = 127; i < 255; ++i)
            {
                temp += static_cast<char>(i);
            }
            return temp;
        }
        ();

        GetTokens(input, whiteSpace, tokens);
    }

    // For advanced text extraction, choose 'whiteSpace' to be ASCII values
    // 0x00-0x20,0x7F in GetTokens(...). Any special characters for ASCII
    // values 0x80 or larger are retained as text.
    inline void GetAdvancedTextTokens(std::string const& input,
        std::vector<std::string>& tokens)
    {
        static std::string const whiteSpace = []
        {
            std::string temp;
            for (size_t i = 0; i <= 32; ++i)
            {
                temp += static_cast<char>(i);
            }
            temp += static_cast<char>(127);
            return temp;
        }
        ();

        GetTokens(input, whiteSpace, tokens);
    }
}
