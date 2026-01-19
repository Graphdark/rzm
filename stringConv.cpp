
#include "stdafx.h"

//#include <codecvt>
//#include <locale>
//#include <string>


std::string toUtf8(const std::wstring& str)
{
  std::string ret;
  int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
  if (len > 0)
  {
    ret.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len, NULL, NULL);
  }
  return ret;
}
//
//bool isLittleEndianSystem() {
//  char16_t test = 0x0102;
//  return (reinterpret_cast<char*>(&test))[0] == 0x02;
//}
//
//std::u16string utf8_to_utf16(const std::string& s) {
//  static bool littleEndian = isLittleEndianSystem();
//
//  if (littleEndian) {
//    std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffffU, std::codecvt_mode::little_endian>, char16_t> convert_le;
//    return convert_le.from_bytes(s);
//  }
//  else {
//    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert_be;
//    return convert_be.from_bytes(s);
//  }
//}
//
//std::string utf16_to_utf8(const std::u16string& s) {
//  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
//  return convert.to_bytes(s);
//}
//
//
//std::u32string utf8_to_utf32(const std::string& s) {
//  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
//  return convert.from_bytes(s);
//}
//
//std::u32string utf16_to_utf32(const std::u16string& s) {
//  std::string bytes;
//  bytes.reserve(s.size() * 2);
//
//  for (const char16_t c : s) {
//    bytes.push_back(static_cast<char>(c / 256));
//    bytes.push_back(static_cast<char>(c % 256));
//  }
//
//  std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
//  return convert.from_bytes(bytes);
//}
//
//
//std::u16string utf32_to_utf16(const std::u32string& s) {
//  std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
//  std::string bytes = convert.to_bytes(s);
//
//  std::u16string result;
//  result.reserve(bytes.size() / 2);
//
//  for (size_t i = 0; i < bytes.size(); i += 2) {
//    result.push_back(static_cast<char16_t>(static_cast<unsigned char>(bytes[i]) * 256 + static_cast<unsigned char>(bytes[i + 1])));
//  }
//
//  return result;
//}
//
//
//std::u16string utf8_to_ucs2(const std::string& s) {
//  static bool littleEndian = isLittleEndianSystem();
//
//  if (littleEndian) {
//    std::wstring_convert<std::codecvt_utf8<char16_t, 0x10ffff, std::little_endian>, char16_t> convert_le;
//    return convert_le.from_bytes(s);
//  }
//  else {
//    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert_be;
//    return convert_be.from_bytes(s);
//  }
//}
//
//std::string ucs2_to_utf8(const std::u16string& s) {
//  std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
//  return convert.to_bytes(s);
//}
//
//std::u16string utf16_to_ucs2(const std::u16string& s) {
//  std::string bytes;
//  bytes.reserve(s.size() * 2);
//
//  for (const char16_t c : s) {
//    bytes.push_back(static_cast<char>(c / 256));
//    bytes.push_back(static_cast<char>(c % 256));
//  }
//
//  std::wstring_convert<std::codecvt_utf16<char16_t>, char16_t> convert;
//  return convert.from_bytes(bytes);
//}
//
//std::u16string ucs2_to_utf16(const std::u16string& s) {
//  std::wstring_convert<std::codecvt_utf16<char16_t>, char16_t> convert;
//  std::string bytes = convert.to_bytes(s);
//
//  std::u16string result;
//  result.reserve(bytes.size() / 2);
//
//  for (size_t i = 0; i < bytes.size(); i += 2) {
//    result.push_back(static_cast<char16_t>(static_cast<unsigned char>(bytes[i]) * 256 + static_cast<unsigned char>(bytes[i + 1])));
//  }
//
//  return result;
//}
