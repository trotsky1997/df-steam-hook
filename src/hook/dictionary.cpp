#include "dictionary.h"

#include "korean_josa.hpp"
#include "ttf_manager.h"
#include "utils.hpp"

void Dictionary::ReplaceAll(std::string &subject, const std::string &search, const std::string &replace)
{
   size_t pos = 0;
   while ((pos = subject.find(search, pos)) != std::string::npos) {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }
}

std::string Dictionary::EraseFrontBackBlank(std::string &str)
{
   int index;
   index = str.find_first_not_of(" ");
   str.erase(0, index);
   index = str.find_last_not_of(" ");
   str.erase(index + 1);
   return str;
}

std::string Dictionary::EraseStringComma(std::string &str)
{
   // ReplaceAll(str, ",", "");
   char charToCount = ' ';
   int count = std::count(str.begin(), str.end(), charToCount);
   if (count >= 1 && str.find('.') != std::string::npos) ReplaceAll(str, ".", "");

   ReplaceAll(str, "  ", " ");
   return str;
}

std::string Dictionary::Sanitize(std::string &str)
{
   ReplaceAll(str, R"(\\)", "\\");
   ReplaceAll(str, R"(\t)", "\t");
   ReplaceAll(str, R"(\r)", "\r");
   ReplaceAll(str, R"(\n)", "\n");
   ReplaceAll(str, R"(")", "\"");

   std::regex r("\\[[^\\]]+\\]");
   str = std::regex_replace(str, r, "");

   // spdlog::debug("Dictionary :{}", str);
   return str;
}

void Dictionary::SplitRegex(const std::string &str)
{
   const std::regex re("(.*)=(.*)");

   std::smatch match;
   if (std::regex_search(str, match, re)) {
      std::string key = match[1];
      std::string arg = match[2];
      this->Add(key, arg);

      RegexRplace(key, true);
      this->regex_vector.emplace_back(key);
   }
}

void Dictionary::RegexRplace(std::string &str, bool on)
{
   if (on) {
      ReplaceAll(str, "{s}", "([^\\.^\\,]*)[,.]");
      ReplaceAll(str, "{s,}", "(.*)");
      ReplaceAll(str, "{d}", "([\\d]*)");
   } else {
      ReplaceAll(str, "([^\\.^\\,]*)[,.]", "{s}");
      ReplaceAll(str, "(.*)", "{s,}");
      ReplaceAll(str, "([\\d]*)", "{d}");
   }
}

std::pair<std::string, std::string> Dictionary::Split(const std::string &str)
{
   std::string delimiter = "\",\"";
   auto delimiter_pos = str.find(delimiter);
   auto key = str.substr(1, delimiter_pos - 1);
   // value length = str-key-1(for fisrt ")-1(for last ")-3(for delimiter)
   auto value = str.substr(delimiter_pos + 3, str.length() - key.length() - 5);

   Sanitize(key);
   EraseStringComma(key);
   return std::make_pair(EraseFrontBackBlank(key), Sanitize(value));
}

// C code would be faster here, but we need to load just once
void Dictionary::LoadCsv(const std::string &filename, const std::string &regex_filename)
{
   logger::info("trying to load dictionary from csv file {}", filename);
   std::ifstream file(filename);
   if (!file.is_open()) {
      logger::critical("unable to open csv file {}", filename);
      MessageBoxA(nullptr, "unable to find csv dictionary file", "dfint hook error", MB_ICONERROR);
      exit(2);
      // do we need exit(2) here?
      return;
   }

   std::string line;
   int i = 0;
   while (std::getline(file, line)) {
      auto pair = Split(line);
      this->Add(pair);
   }
   file.close();
   logger::info("csv dictionary loaded, total lines {}", this->Size());

   std::ifstream regex_file;
   regex_file.open(regex_filename);
   if (!regex_file.is_open()) {
      logger::critical("unable to open regex file {}", regex_filename);
      MessageBoxA(nullptr, "unable to open regex file", "dfint hook error", MB_ICONERROR);
      exit(2);
      // do we need exit(2) here?
      return;
   }

   while (std::getline(regex_file, line)) {
      SplitRegex(line);
   }
   regex_file.close();
   logger::info("add regex in csv dictionary, total lines {}", this->Size());
}

std::optional<std::string> Dictionary::RegexSearch(const std::string &key)
{
   std::string input(key);
   std::smatch match;
   for (const auto regex_string : this->regex_vector) {
      std::regex r(regex_string);
      if (std::regex_match(input, match, r) && match.size() > 1) {
         std::string find(regex_string);
         RegexRplace(find, false);
         std::string result;

         auto it = this->dict.find(find);
         if (it != this->dict.end()) {
            result = std::regex_replace(match[0].str(), r, it->second);
            std::string matched;
            for (int i = 1; i < match.size(); i++) {
               matched = match[i].str();
               auto it = Get(matched);
               if (it) {
                  if (result.find(matched) != std::string::npos) result.replace(result.find(matched), matched.length(), it.value());
                  else spdlog::debug("#Can't find :{}:{}:", matched, result);
               }
            }
            this->dict_log.emplace(input, result);
            // spdlog::debug("\n#TRANS:{}", result);
            return result;
         } else {
            return std::nullopt;
         }
      }
   }
   return std::nullopt;
}

std::optional<std::string> Dictionary::Get(const std::string &key)
{
   if (key.empty()) {
      return std::nullopt;
   }
   std::string input(key);
   EraseStringComma(input);
   EraseFrontBackBlank(input);

   if (this->dict_log.find(input) != this->dict_log.end()) {
      return this->dict_log.at(input);
   }
   if (this->dict.find(input) != this->dict.end()) {
      return this->dict.at(input);
   }
   auto ret = RegexSearch(input);
   if (ret) return ret;
   return std::nullopt;
}

void Dictionary::InitBuffer()
{
   this->key_vec.clear();
   // this->string_buffer = "";
   this->string_translation = "";
   this->start_y = -1;
}
// 문자열 위치를 정해서 버퍼에 저장
void Dictionary::StoreBuffer(const std::string &buffer, const std::string &key, int x, int y)
{
   // 처음 시작되는 첫 문자열의 위치를 결정하고 저장 다음 문자열은 시작 문자열 뒤로 붙는다
   if (this->start_y == -1) {
      // 앞에 줄과 같은 줄인 경우
      if (this->pre_line == y) {  //&& std::abs(x - this->pre_len_x) >= 1) {
         this->start_x = this->pre_len_x;
         this->start_y = y;
         // spdlog::debug("\n#1 x{}y{}, sx{}sy{},pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x, buffer);
      }  // 앞에 줄보다 위 아래인 경우 // && std::abs(x - this->pre_len_x) > 1
      else if (abs(this->pre_line - y) == 1 && this->string_buffer.length() > 15) {
         this->start_y = this->pre_line - y == 1 ? y + 1 : y;
         this->start_x = this->pre_line - y == 1 ? this->pre_len_x : this->first_line_x;
         // spdlog::debug("\n#2 x{}y{}, sx{}sy{},pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x, buffer);
      } else {  // 새롭게 시작하는 문자열
         this->start_x = x;
         this->first_line_x = x;
         this->start_y = y;
         // spdlog::debug("\n#3 x{}y{}, sx{}sy{}, pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x,
         // buffer);
      }
      this->key_vec.push_back(key);
      this->pre_line = y;
      this->pre_len_x = x + buffer.length();
      this->string_buffer = buffer;
      EraseFrontBackBlank(this->string_buffer);
   } else {  // 다음 문자열
      this->key_vec.push_back(key);
      if (this->pre_len_x - x == 0) this->string_buffer += buffer;
      else this->string_buffer += " " + buffer;
      if (this->string_buffer.size() > 500) InitBuffer();  // 이상하게 길게 붙었을 경우 초기화
      this->pre_line = y;
      this->pre_len_x = x + buffer.length();
   }
}
// 단어를 원본에 맞춰서 비교하고 출력될 위치를 조정하여 맵에 저장
void Dictionary::SaveToStringMap(int index, int &preX, int &preY, int &length, std::string &temp, bool isSrc)
{
   int x, y, tempLen;
   Utils::CoordExtract(temp, x, y);

   if (isSrc) {
      tempLen = Utils::GetStringLength(temp);
   } else {
      std::wstring input = Utils::s2ws(temp + " ");
      tempLen = TTFManager::GetSingleton()->CreateWSTexture(input);
   }

   if (tempLen == 0) {
      this->dict_multi.emplace(this->key_vec[index], this->SKIP);
      return;
   }

   length += tempLen;
   if (length > this->GetLineLimit()) {
      preX = this->first_line_x;
      preY++;
      this->pre_line = preY;
      length = 0;
      this->start_x = this->first_line_x;
   }

   int gap = 0;
   if (isSrc) {
      temp += "#" + std::to_string(preX) + "#" + std::to_string(preY) + "$SRC";
      gap = 1;
   } else temp += "#" + std::to_string(preX) + "#" + std::to_string(preY);
   preX += tempLen + gap;
   length += tempLen + gap;
   this->pre_len_x = preX;
   this->dict_multi.emplace(this->key_vec[index], temp);
}

bool isKorean(const char *src)
{
   return src[0] & 0x80 ? true : false;
}

// 번역된 문자열 출력되기전 준비
void Dictionary::PrepareBufferOut()
{
   std::vector<std::string> valueVec;
   int len = this->GetLineLimit();

   std::wstring changeJosa = Korean::ReplaceJosa(Utils::s2ws(this->string_translation));
   this->string_translation = Utils::ws2s(changeJosa);

   std::stringstream ss(this->string_translation);
   std::string valueLine, word;
   // 단어가 아닌 문장으로 된 문자열이 들어와서 단어 단위로 잘라서 제한 길이에 맞춰 저장
   if (this->str_type == StringType::Colored) {
      while (getline(ss, word, ' ')) {
         if (word.empty()) continue;
         std::string temp = valueLine + word;
         if (Utils::GetStringLength(temp) + 1 > len && !valueLine.empty()) {
            valueVec.push_back(valueLine);
            valueLine = "";
         }
         valueLine += word + " ";
      }
      if (!valueLine.empty()) {
         valueVec.push_back(valueLine);
      }
   } else {  // 단어 단위로 잘라서 저장
      std::string josa = "";
      while (getline(ss, word, ' ')) {
         if (word.empty()) continue;
         valueVec.push_back(word);
      }
   }

   std::unordered_map<std::string, int> position_map;
   for (int i = 0; i < this->key_vec.size(); i++) {
      std::string temp = this->key_vec[i].substr(0, this->key_vec[i].find("#"));
      position_map[temp] = i;
      // spdlog::debug("position {}", temp);
   }

   int preX, preY, length = 0;
   std::string temp = this->key_vec[0];
   Utils::CoordExtract(temp, preX, preY);
   preX = this->start_x;
   preY = this->start_y;
   // 원본의 순서대로 위치에 맞게 번역 저장
   for (int index = 0; index < this->key_vec.size(); index++) {
      for (int i = index; i < valueVec.size(); i++) {
         if (position_map.count(valueVec[i])) {
            int position = position_map[valueVec[i]];
            std::string key = this->key_vec[position];
            SaveToStringMap(index, preX, preY, length, key, true);
            break;
         } else {
            SaveToStringMap(index, preX, preY, length, valueVec[i], false);
            break;
         }
      }
      if (index >= valueVec.size()) this->dict_multi.emplace(this->key_vec[index], this->SKIP);
   }
}

// 사전에서 번역 문자열로 교체
void Dictionary::TranslationBuffer()
{
   // spdlog::debug("TranslationBuffer");
   std::string delimiter = (this->str_type == StringType::Top) ? ".:!?" : ".,:!?";
   std::string buffer = this->string_buffer;
   int delimiterPos = 0;
   while (delimiterPos != std::string::npos) {
      delimiterPos = buffer.find_first_of(delimiter);
      if (delimiterPos != std::string::npos) {
         // int gap = 1;
         // if (buffer[delimiterPos] == ',') gap = 0;
         std::string tstr = buffer.substr(0, delimiterPos);
         std::string translation = GetTranslation(tstr);
         // if (buffer[delimiterPos] == '.') this->string_translation += " " + translation + buffer[delimiterPos];
         // else
         this->string_translation += " " + translation;
         buffer.erase(0, delimiterPos + 2);
         // spdlog::debug("TRANS:{}", this->string_translation);
      }
   }

   if (!buffer.empty()) {
      std::string translation = GetTranslation(buffer);
      this->string_translation += " " + translation;
      // spdlog::debug("TRANS:{}", this->string_translation);
   }
}

std::string Dictionary::GetTranslation(const std::string &tstr)
{
   auto translation = Get(tstr);
   if (translation) return translation.value();
   else {
      spdlog::debug("{}", tstr);
      return tstr;
   }
}

// 버퍼 문자열 비우기
void Dictionary::FlushBuffer()
{
   if (this->key_vec.size() == 1) {  // || this->string_buffer.length() < 15) {
      auto ret = Get(this->string_buffer);
      if (this->str_type == StringType::Main) spdlog::debug("1 size {}", this->string_buffer);
      if (ret) {
         // spdlog::debug("TRANS:{}:{}", ret.value(), this->string_buffer);
         for (int i = 0; i < this->key_vec.size(); i++) {
            if (i == 0) this->dict_multi.emplace(this->key_vec[i], ret.value());
            else this->dict_multi.emplace(this->key_vec[i], this->SKIP);
         }
      } else {
         // spdlog::debug("{}", this->string_buffer);
         for (int i = 0; i < this->key_vec.size(); i++) {
            std::string temp = this->key_vec[i] + "$SRC";
            this->dict_multi.emplace(this->key_vec[i], temp);
         }
      }
      InitBuffer();
   } else {
      TranslationBuffer();
      PrepareBufferOut();
      InitBuffer();
   }
}
// 버퍼 초기화 확인
bool Dictionary::shouldInitBuffer(int y) const
{
   // bool isSame = false;
   // if (!this->key_vec.empty()) isSame = this->dict_multi.count(this->key_vec) > 0;
   return this->start_y == -1;  // || y < this->start_y;
}
// 영문 대문자 확인
bool is_uppercase(char c)
{
   return c >= 'A' && c <= 'Z';
}
// 문자열 버퍼를 번역 해야 되는지 더 저장 해야 되는지 판단
bool Dictionary::shouldFlushBuffer(int y, int x, int length, const std::string &buffer)
{
   bool isNotInitialValue = this->start_y != -1;                                 // 초기값이 아닐때
   bool isLessThanStartingValue = y < this->pre_line;                            // 이전줄보다 앞에 줄일때
   bool isXDiffGreaterThanOne = y == this->pre_line && x - this->pre_len_x > 1;  // 이전줄과 같고 간격이 1칸 넘을때
   bool endsWithSpecial = endsWithSpecialCharacter(this->string_buffer);         // 문자열 끝이 특수문자일때
   bool isLineDiffGreaterThanOne = y - this->pre_line > 1;                       // 이전줄과의 간격이 1줄 이상일때
   bool isLineDiffOneAndStringLengthLessThan = y - this->pre_line == 1 && this->string_buffer.length() < 43 &&
                                               (buffer.find(" ") != std::string::npos || (is_uppercase(buffer.front()) && this->key_vec.size() < 2));

   return (isNotInitialValue &&
           (isLessThanStartingValue || isXDiffGreaterThanOne || endsWithSpecial || isLineDiffGreaterThanOne || isLineDiffOneAndStringLengthLessThan));
}
// 문자열이 특수문자로 끝나는지 확인
bool Dictionary::endsWithSpecialCharacter(const std::string &buffer)
{
   return buffer.length() > 0 && (buffer[buffer.length() - 1] == '.' || buffer[buffer.length() - 1] == ':' || buffer[buffer.length() - 1] == '?' ||
                                  buffer[buffer.length() - 1] == '!' || buffer[buffer.length() - 1] == ')');
}
// 문자열 버퍼 저장 컨트롤
std::optional<std::string> Dictionary::StringBufferControl(const std::string &buffer, int x, int y, StringType type, int justify)
{
   const std::string keyWithCoord = buffer + "#" + std::to_string(x) + "#" + std::to_string(y);
   if (shouldFlushBuffer(y, x, buffer.length(), buffer)) {
      spdlog::debug("Flush SRC({}){}:{}", this->key_vec.size(), this->str_type, this->string_buffer);
      FlushBuffer();
   }

   if (this->dict_multi.count(keyWithCoord) > 0) {
      return this->dict_multi.at(keyWithCoord);
   }

   this->str_type = type;
   if (shouldInitBuffer(y)) {
      InitBuffer();
      // 여러줄 조합 단어, 1줄 문장 혼합해서 들어와서 여기서 공백기준으로 문장만 걸러냄
      if (this->str_type == StringType::Main) {
         char charToCount = ' ';
         int count = std::count(buffer.begin(), buffer.end(), charToCount);
         if (count >= 1) {
            this->key_vec.push_back(keyWithCoord);
            this->start_x = x;
            this->first_line_x = x;
            this->start_y = y;
            this->pre_line = y;
            this->pre_len_x = x + buffer.length();
            this->string_buffer = buffer;
            EraseFrontBackBlank(this->string_buffer);
            FlushBuffer();
            return std::nullopt;
         }
      }
   }

   StoreBuffer(buffer, keyWithCoord, x, y);
   return std::nullopt;
}

std::optional<std::string> Dictionary::GetMulti(const char *key, int x, int y, StringType type, int justify)
{
   auto len = strnlen_s(key, 500);
   if (!key || len <= 0 || len >= 500) {
      return std::nullopt;
   }

   auto ret = StringBufferControl(key, x, y, type, justify);
   if (ret) return ret.value();
   return std::nullopt;
}

bool Dictionary::Exist(std::string &key)
{
   return this->dict.find(key) != this->dict.end();
}

void Dictionary::Add(std::string &key, std::string &value)
{
   this->dict.emplace(std::make_pair(key, value));
}

void Dictionary::Add(std::pair<std::string, std::string> &pair)
{
   this->dict.emplace(pair);
}

size_t Dictionary::Size()
{
   return this->dict.size();
}

void Dictionary::Clear()
{
   this->dict.clear();
   this->dict_log.clear();
   this->dict_multi.clear();
   this->regex_vector.clear();

   void InitBuffer();
}