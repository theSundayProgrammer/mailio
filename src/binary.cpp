/*

binary.cpp
----------

Copyright (C) 2016, Tomislav Karastojkovic (http://www.alepho.com).

Distributed under the FreeBSD license, see the accompanying file LICENSE or
copy at http://www.freebsd.org/copyright/freebsd-license.html.

*/


#include <string>
#include <vector>
#include <mailio/binary.hpp>


using std::vector;


namespace mailio
{


binary::binary(std::string::size_type line1_policy, std::string::size_type lines_policy) :
    codec(line1_policy, lines_policy)
{
}


vector<std::string> binary::encode(const std::string& text) const
{
    vector<std::string> enc_text;
    enc_text.push_back(text);
    return enc_text;
}


std::string binary::decode(const vector<std::string>& text) const
{
    std::string dec_text;
    for (const auto& line : text)
        dec_text += line + END_OF_LINE;
    return dec_text;
}


} // namespace mailio
