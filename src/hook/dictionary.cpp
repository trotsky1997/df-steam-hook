#include "dictionary.h"

#include <rapidfuzz/fuzz.hpp>

// #include "korean_josa.hpp"
#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <map>
#include <optional>

#include "regex"
#include "string"
#include "ttf_manager.h"
#include "utils.hpp"
#include "mutex"
std::mutex youdao_mutex;
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
   ReplaceAll(str, "\"", "");
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

/* online Machine Translation*/
#include <curl/curl.h>
#include <json/json.h>
#include <stdlib.h>
#include <string.h>

#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
using namespace std;
std::map<std::string, std::string> youdaoMap;
 int youdaoFlag = 0;

string GbkToUtf8(const char *src_str)
{
   int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
   wchar_t *wstr = new wchar_t[len + 1];
   memset(wstr, 0, len + 1);
   MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
   len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
   char *str = new char[len + 1];
   memset(str, 0, len + 1);
   WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
   string strTemp = str;
   if (wstr) delete[] wstr;
   if (str) delete[] str;
   return strTemp;
}

string Utf8ToGbk(const char *src_str)
{
   int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
   wchar_t *wszGBK = new wchar_t[len + 1];
   memset(wszGBK, 0, len * 2 + 2);
   MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
   len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
   char *szGBK = new char[len + 1];
   memset(szGBK, 0, len + 1);
   WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
   string strTemp(szGBK);
   if (wszGBK) delete[] wszGBK;
   if (szGBK) delete[] szGBK;
   return strTemp;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string *)userp)->append((char *)contents, size * nmemb);
   return size * nmemb;
}

string youdaoEscape(string text){
      CURL *curl;
   // std::string text;
   // getline(cin,text);

   curl = curl_easy_init();
   if (curl) {
      for (int i = 0; i < text.length(); i++) {
         if (text[i] == '\!' && text[i] == '?' && text[i] == '.' &&  text[i] == ','
            ) {  // 如果不是字母或数字
            text[i] = '.';         // 替换为一个空格
         }
         else if (!std::isalnum(text[i]) && text[i] != '\'' && text[i] != '"' 
            ) {  // 如果不是字母或数字
            text[i] = ' ';         // 替换为一个空格
         }
      }
      text = curl_easy_escape(curl, text.c_str(), text.length());
}
   return text;
}

void SendRequestToYouDaoAPI(string text)
{
   // if (youdaoMap.find(text) != youdaoMap.end()) {
   //    return youdaoMap[text];
   // }
   if (youdaoFlag == 0){
      youdaoFlag = 1;
      const std::string file_path = "./youdaoMap.json";
     

      std::ifstream input_file(file_path);  // 打开 JSON 文件

      if (input_file.good()) {   // 文件存在，加载到内存对象 youdaoMap
         Json::Value root;
         input_file >> root;  // 从文件读取数据

         for (const auto& key : root.getMemberNames()) {       
            youdaoMap.emplace(key, root[key].asString());  // 将 json 中的所有键值对都放入 map 当中
         }

         input_file.close();     // 关闭文件流
      } else {
         youdao_mutex.lock();
         // std::cout << "File not found." << std::endl;
         std::ofstream ofs("./youdaoMap.json");
         ofs.close();
         youdao_mutex.unlock();


      }
   }
   CURL *curl;
   CURLcode res;
   std::string readBuffer;
   // std::string text;
   // getline(cin,text);

   curl = curl_easy_init();
   if (curl) {

      // std::cout<<text<<std::endl;
      std::string url = "http://fanyi.youdao.com/translate?doctype=text&i=" + text;
      // std::cout<<url<<std::endl;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(
          curl, CURLOPT_USERAGENT,
          "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36 Edg/106.0.1370.42");
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);

      // std::string res = Utf8ToGbk(readBuffer.c_str());
      string res = readBuffer;
      std::size_t pos = res.find("result=");
      std::string ans = res.substr(pos + 7);
      spdlog::debug("{} translated to {} by youdao.", text, ans);

      youdaoMap.insert(std::pair<std::string, std::string>(text, ans));
   }
   
   // 将std::map<std::string, std::string>对象转换为json值
   Json::Value root(Json::objectValue);
   for (const auto &entry : youdaoMap) {
      root[entry.first] = entry.second;
   }

   // 将json值写入文件
   youdao_mutex.lock();
   std::ofstream ofs("./youdaoMap.json");
   ofs << root;
   ofs.close();
   youdao_mutex.unlock();
   // return "";
}

// Calculate Jaro-Winkler distance
double calc_similarity(const std::string &s1, const std::string &s2)
{
   return rapidfuzz::fuzz::ratio(s1, s2);
}

std::map<std::string, std::string> fuzzyMap;

// Find the key with highest similarity to the given input key
std::string find_similar_key(const std::unordered_map<std::string, std::string> &my_map, const std::string &input_key, double threshold)
{
   double max_similarity = 0.0;
   std::string result_key;
   if (fuzzyMap.find(input_key) != fuzzyMap.end()) {
      return fuzzyMap[input_key];
   }
#pragma omp parallel for
   for (const auto &pair : my_map) {
      double similarity = calc_similarity(pair.first, input_key);
      if (similarity > max_similarity) {
         max_similarity = similarity;
         result_key = pair.first;
      }
   }

   if (max_similarity < threshold) {
      result_key = "";
   }
   fuzzyMap.insert(std::pair<std::string, std::string>(input_key, result_key));
   return result_key;
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
   auto simil_key = find_similar_key(this->dict, key, 85);
   if (simil_key != "") {
      return this->dict.at(simil_key);
   }
   string text = youdaoEscape(key);
   if (youdaoMap.find(text) != youdaoMap.end()) {
      return youdaoMap[text];
   } else {
      if (key[0] <= '9' && key[0] >= '0') {
         return std::nullopt;
      }
      std::thread t(SendRequestToYouDaoAPI, text);
      t.detach();
   }

   return std::nullopt;
}

void Dictionary::InitBuffer()
{
   this->key_vec.clear();
   // this->string_buffer = "";
   this->string_translation = "";
   this->start_y = -1;
}
// 确定字符串位置并保存到缓冲器
void Dictionary::StoreBuffer(const std::string &buffer, const std::string &key, int x, int y)
{
   // 确定第一个字符串开始的位置，保存之后字符串紧跟在开始字符串之后
   if (this->start_y == -1) {
      // 前面的一行
      if (this->pre_line == y) {  //&& std::abs(x - this->pre_len_x) >= 1) {
         this->start_x = this->pre_len_x;
         this->start_y = y;
         spdlog::debug("\n#1 x{}y{}, sx{}sy{},pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x, buffer);
      }  // 如果是在前排下方 // && std::abs(x - this->pre_len_x) > 1
      else if (abs(this->pre_line - y) == 1 && this->string_buffer.length() > 15) {
         this->start_y = this->pre_line - y == 1 ? y + 1 : y;
         this->start_x = this->pre_line - y == 1 ? this->pre_len_x : this->first_line_x;
         spdlog::debug("\n#2 x{}y{}, sx{}sy{},pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x, buffer);
      } else {  // 重新开始的字符串
         this->start_x = x;
         this->first_line_x = x;
         this->start_y = y;
         spdlog::debug("\n#3 x{}y{}, sx{}sy{}, pre{},first{}, {}", x, y, this->start_x, this->start_y, this->pre_len_x, this->first_line_x, buffer);
      }
      this->key_vec.push_back(key);
      this->pre_line = y;
      this->pre_len_x = x + buffer.length();
      this->string_buffer = buffer;
      EraseFrontBackBlank(this->string_buffer);
   } else {  // 下一个字符串
      this->key_vec.push_back(key);
      if (this->pre_len_x - x == 0) this->string_buffer += buffer;
      else this->string_buffer += " " + buffer;
      if (this->string_buffer.size() > MAX_BUFFER) InitBuffer();  // 异常加长时初始化
      this->pre_line = y;
      this->pre_len_x = x + buffer.length();
   }
}
// 将单词与原本进行比较，并调整输出位置，保存到映射中
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

bool isKorean(const char *src)  // 也可用于检测中文字符
{
   return src[0] & 0x80 ? true : false;
}

// 翻译字符串输出前的准备
void Dictionary::PrepareBufferOut()
{
   std::vector<std::string> valueVec;
   int len = this->GetLineLimit();

   std::stringstream ss(this->string_translation);
   std::string valueLine, word;
   // 出现不是单词的句子的字符串，以单词为单位切下来，按照限制长度储存
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
   } else {  // 以单词为单位切下来储存
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
   // 按照原本的顺序，按照位置保存翻译
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

// 从字典替换成翻译字符串
void Dictionary::TranslationBuffer()
{
   // spdlog::debug("TranslationBuffer");
   std::string delimiter = (this->str_type == StringType::Top) ? ".:!?" : ".,:!?";
   std::string buffer = this->string_buffer;
   int delimiterPos = 0;
   while (delimiterPos != std::string::npos) {
      delimiterPos = buffer.find_first_of(delimiter);
      if (delimiterPos != std::string::npos) {
         std::string tstr = buffer.substr(0, delimiterPos);
         std::string translation = GetTranslation(tstr);
         this->string_translation += " " + translation;
         buffer.erase(0, delimiterPos + 2);
      }
   }

   if (!buffer.empty()) {
      std::string translation = GetTranslation(buffer);
      this->string_translation += " " + translation;
   }

   spdlog::debug("TRANS:{}", this->string_translation);
}

std::string Dictionary::GetTranslation(const std::string &tstr)
{
   auto translation = Get(tstr);
   if (translation) return translation.value();
   else {
      // spdlog::debug("{}", tstr);
      return tstr;
   }
}

// 清空缓存区
void Dictionary::FlushBuffer()
{
   if (this->key_vec.size() == 1 || this->string_buffer.length() < 15) {
      auto ret = Get(this->string_buffer);
      if (this->str_type == StringType::Main) spdlog::debug("1 size {}", this->string_buffer);
      if (ret) {
         // spdlog::debug("1 size:{}:{}", ret.value(), this->string_buffer);
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
                                               this->str_type != StringType::Colored &&
                                               (buffer.find(" ") != std::string::npos || (is_uppercase(buffer.front()) && this->key_vec.size() < 2));

   // spdlog::debug("CHECK {} {} {} {} {}
   // {}",isNotInitialValue,isLessThanStartingValue,isXDiffGreaterThanOne,endsWithSpecial,isLineDiffGreaterThanOne,isLineDiffOneAndStringLengthLessThan);
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
   // spdlog::debug(":{}", buffer);

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
      // 多行组合词、一行句子混合后进入，仅以空白标准过滤句子
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
   auto len = strnlen_s(key, MAX_BUFFER);
   if (!key || len <= 0 || len >= MAX_BUFFER) {
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