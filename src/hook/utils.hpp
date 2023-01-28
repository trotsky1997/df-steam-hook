#pragma once

namespace Utils {
   inline std::u16string cp437_to_unicode(const std::string& str)
   {
      std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> ucs2conv;
      try {
         std::u16string ucs2 = ucs2conv.from_bytes(str);
         return ucs2;
      } catch (const std::range_error& e) {
         logger::error("some error while encoding from utf8 to ucs2");
      }
   }

   inline std::u16string s2s16(const std::string& str)
   {
      std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> converter;
      return converter.from_bytes(str);
   }

   inline std::wstring s2ws(const std::string& str)
   {
      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
      return converter.from_bytes(str);
   }

   inline std::string ws2s(const std::wstring& wstr)
   {
      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
      return converter.to_bytes(wstr);
   }

   inline std::string now()
   {
      auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
      return std::format("{:%Y%m%d-%H-%M-%S}-{}", time, std::time(NULL));
   }

   inline bool CoordExtract(std::string& str, int& x, int& y)
   {
      int idx, idx2;
      idx = str.find_first_of("#");
      if(idx == std::string::npos) return false;
      idx2 = str.find_last_of("#");
      x = std::stoi(str.substr(idx + 1, idx2 - 1));
      y = std::stoi(str.substr(idx2 + 1, str.length()));
      str.erase(idx);
      return true;
   }

   inline int GetStringLength(std::string& str)
   {
      int count =0;
      for (int i = 0; i < str.size(); i++)
      {
         if(str[i] & 0x80) i++;
         count++;
      }

      return count;
   }

   inline void SimpleString(std::string& str)
   {
      for (int s = 0; s < str.size(); s++) {
         switch (str[s]) {
            case (char)129:
            case (char)150:
            case (char)151:
            case (char)163:
               str[s] = 'u';
               break;
            case (char)154:
               str[s] = 'U';
               break;
            case (char)152:
               str[s] = 'y';
               break;
            case (char)164:
               str[s] = 'n';
               break;
            case (char)165:
               str[s] = 'N';
               break;
            case (char)131:
            case (char)132:
            case (char)133:
            case (char)134:
            case (char)145:
            case (char)146:
            case (char)160:
               str[s] = 'a';
               break;
            case (char)142:
            case (char)143:
               str[s] = 'A';
               break;
            case (char)130:
            case (char)136:
            case (char)137:
            case (char)138:
               str[s] = 'e';
               break;
            case (char)144:
               str[s] = 'E';
               break;
            case (char)139:
            case (char)140:
            case (char)141:
            case (char)161:
               str[s] = 'i';
               break;
            case (char)147:
            case (char)148:
            case (char)149:
            case (char)162:
               str[s] = 'o';
               break;
            case (char)153:
               str[s] = 'O';
               break;
            case (char)128:
               str[s] = 'C';
               break;
            case (char)135:
               str[s] = 'c';
               break;
         }
      }
   }
}  // namespace Utils
