#pragma once


std::string toUtf8(const std::wstring& str);
std::u16string utf8_to_utf16(const std::string& s);
std::string utf16_to_utf8(const std::u16string& s);
std::u32string utf8_to_utf32(const std::string& s);
std::u32string utf16_to_utf32(const std::u16string& s);
std::u16string utf32_to_utf16(const std::u32string& s);