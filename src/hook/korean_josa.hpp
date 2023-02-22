#pragma once

class Korean
{
  public:
   class Josa
   {
     public:
      Josa();
      void Replace(const std::wstring& srcText, std::wstring& outText);

     private:
      typedef std::pair<std::wstring, std::wstring> JosaPair;
      typedef std::map<std::wstring, JosaPair> JosaPatternPaird;
      void InsertJosaPatternPair(const std::wstring& pattern, const std::wstring& first, const std::wstring& second);
      const std::wstring& ChooseJosa(wchar_t prevChar, const std::wstring& preStr, const std::wstring& josaKey, const JosaPair& josaPair);

     private:
      std::wregex _josaRegex;
      JosaPatternPaird _josaPatternPaird;
   };  // end_of_class Josa

   static Josa& GetJosaInstance();
   static void ReplaceJosa(const std::wstring& srcText, std::wstring& outText);
   static std::wstring ReplaceJosa(const std::wstring& srcText);

};  // end_of_class Korean

Korean::Josa::Josa() : _josaRegex(L"\\(이\\)가|\\(와\\)과|\\(을\\)를|\\(은\\)는|\\(아\\)야|\\(이\\)여|\\(으\\)로|\\(이\\)라")
{
   InsertJosaPatternPair(L"(이)가", L"이", L"가");
   InsertJosaPatternPair(L"(와)과", L"과", L"와");
   InsertJosaPatternPair(L"(을)를", L"을", L"를");
   InsertJosaPatternPair(L"(은)는", L"은", L"는");
   InsertJosaPatternPair(L"(아)야", L"아", L"야");
   InsertJosaPatternPair(L"(으)로", L"으로", L"로");
   InsertJosaPatternPair(L"(이)여", L"이여", L"여");
   InsertJosaPatternPair(L"(이)라", L"이라", L"라");
}

void Korean::Josa::Replace(const std::wstring& srcText, std::wstring& outText)
{
   outText.clear();
   outText.reserve(srcText.size());

   std::wstring::const_iterator srcTextEnd = srcText.cend();
   std::wstring::const_iterator srcTextBegin = srcText.cbegin();
   std::wstring::const_iterator srcTextIter = srcTextBegin;

   std::regex_constants::match_flag_type flags = std::regex_constants::match_default | std::regex_constants::match_prev_avail;
   std::match_results<std::wstring::const_iterator> what;

   while (std::regex_search(srcTextIter, srcTextEnd, what, _josaRegex, flags)) {
      outText.append(srcTextIter, what[0].first);

      const std::wstring josaKey(what[0].first, what[0].second);
      if (what[0].first == srcTextBegin) {
         outText.append(josaKey);
      } else {
         auto f = _josaPatternPaird.find(josaKey);
         if (f == _josaPatternPaird.cend()) {
            outText.append(josaKey);
         } else {
            const JosaPair& josaPair = f->second;
            std::wstring::const_iterator srcTextPrev = what[0].first;
            --srcTextPrev;
            std::wstring preStr = what.prefix().str();
            preStr = preStr.substr(preStr.rfind(' ') + 1);
            wchar_t prevChar = *srcTextPrev;
            outText.append(ChooseJosa(prevChar, preStr, josaKey, josaPair));
         }
      }

      srcTextIter = what[0].second;
      flags |= std::regex_constants::match_prev_avail;
   }

   outText.append(srcTextIter, srcTextEnd);
}

void Korean::Josa::InsertJosaPatternPair(const std::wstring& pattern, const std::wstring& first, const std::wstring& second)
{
   _josaPatternPaird.insert(JosaPatternPaird::value_type(pattern, JosaPair(first, second)));
}

const std::wstring& Korean::Josa::ChooseJosa(wchar_t prevChar, const std::wstring& preStr, const std::wstring& josaKey, const JosaPair& josaPair)
{
   bool isWhat = false;
   if (prevChar >= 0xAC00 && prevChar <= 0xD7A3)  // Korean
   {
      int localCode = prevChar - 0xAC00;
      int jongCode = localCode % 28;
      isWhat |= jongCode > 0 && josaKey != L"(으)로" ? true : false;
      isWhat |= (jongCode == 0 || jongCode == 8) && josaKey != L"(으)로" ? false : true;
   } else if ((0x61 <= prevChar && prevChar <= 0x7A) || (0x41 <= prevChar && prevChar <= 0x5A))  // English
   {
      std::wsmatch match;
      std::wregex EngRegex3 = std::wregex(
          L"^[lr]|^\\Sr|\\Sle?|check|[hm]ook|limit|^[mn]|\\S[mn]e?|\\S(?:[aeiom]|lu)b|(?:u|\\S[aei]|[^o]o)p|(?:^i|[^auh]i|\\Su|[^ei][ae]|[^oi]o)t|(?:"
          L"\\S[iou]|[^e][ae])c?k|\\S[aeiou](?:c|ng)|foot|go+d|b[ai]g|private|^(?:app|kor)");
      isWhat = (std::regex_match(josaKey, match, EngRegex3) && josaKey.length() == match.position() + match.length()) ? true : false;
   } else if (prevChar == 0x0030 || prevChar == 0x0031 || prevChar == 0x0033 || prevChar == 0x0036 || prevChar == 0x0037 ||
              prevChar == 0x0038)  // 0, 1, 3, 6, 7, 8 Number
   {
      isWhat = true;
   }

   return isWhat ? josaPair.first : josaPair.second;
}

Korean::Josa& Korean::GetJosaInstance()
{
   static Korean::Josa josa;
   return josa;
}

void Korean::ReplaceJosa(const std::wstring& srcText, std::wstring& outText)
{
   Korean::Josa& josa = GetJosaInstance();
   josa.Replace(srcText, outText);
}

std::wstring Korean::ReplaceJosa(const std::wstring& srcText)
{
   std::wstring outText;
   Korean::Josa& josa = GetJosaInstance();
   josa.Replace(srcText, outText);
   return outText;
}
