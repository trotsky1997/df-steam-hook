#pragma once

class Dictionary
{
  public:
   [[nodiscard]] static Dictionary *GetSingleton()
   {
      static Dictionary singleton;
      return &singleton;
   }

   enum StringType
   {
      Main,
      Top,
      Colored
   };

   int GetLineLimit() const
   {
      switch (str_type) {
         case Main:
            return 128 - start_x;
         case Top:
            return 74 - std::abs(first_line_x - start_x);
         case Colored:
            return 42 - std::abs(first_line_x - start_x);
      }
      return 128 - start_x;
   }

   void LoadCsv(const std::string &filename, const std::string &regex_filename);
   std::optional<std::string> GetMulti(const char *key, int x, int y, StringType type = StringType::Main, int justify = -1);
   std::optional<std::string> Get(const std::string &key);
   bool Exist(std::string &key);
   void Add(std::string &key, std::string &value);
   void Add(std::pair<std::string, std::string> &pair);
   size_t Size();
   void Clear();

  private:
   Dictionary() {}
   Dictionary(const Dictionary &) = delete;
   Dictionary(Dictionary &&) = delete;

   ~Dictionary() = default;

   std::string EraseFrontBackBlank(std::string &str);
   std::string EraseStringComma(std::string &str);
   std::pair<std::string, std::string> Split(const std::string &str);
   std::string Sanitize(std::string &str);
   void ReplaceAll(std::string &subject, const std::string &search, const std::string &replace);

   void SplitRegex(const std::string &str);
   void RegexRplace(std::string &str, bool on);
   std::optional<std::string> RegexSearch(const std::string &key);

   void InitBuffer();
   void FlushBuffer();
   void PrepareBufferOut();
   void TranslationBuffer();
   void StoreBuffer(const std::string &buffer, const std::string &key, int x, int y);
   void SaveToStringMap(int index, int &preX, int &preY, int &length, std::string &temp, bool isSrc);
   std::string GetTranslation(const std::string &tstr);
   std::optional<std::string> StringBufferControl(const std::string &buffer, int x, int y, StringType type, int justify);

   bool shouldInitBuffer(int y) const;
   bool shouldFlushBuffer(int y, int x, int length, const std::string &buffer);
   bool endsWithSpecialCharacter(const std::string &buffer);

   const std::string SKIP = "$SKIP";
   int line_limit = 40;
   int start_y = -1;
   int start_x = -1;
   int pre_line = -1;
   int pre_len_x = -1;
   int first_line_x = -1;
   StringType str_type = StringType::Main;

   std::unordered_map<std::string, std::string> dict;
   std::unordered_map<std::string, std::string> dict_log;
   std::unordered_map<std::string, std::string> dict_multi;
   std::vector<std::string> regex_vector;

   std::vector<std::string> key_vec;
   std::string string_buffer;
   std::string string_translation;
};
