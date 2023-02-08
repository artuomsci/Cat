#include "str_utils.h"

using namespace cat;

//-----------------------------------------------------------------------------------------
std::string cat::remove(const std::string& string_, std::string::value_type symbol_)
{
   std::string ret; ret.reserve(string_.length());
   for (const auto& it : string_)
   {
      if (it != symbol_)
         ret.push_back(it);
   }
   return ret;
}

//-----------------------------------------------------------------------------------------
std::string cat::remove_rep(const std::string& string_, std::string::value_type symbol_)
{
   std::string ret;
   ret.reserve(string_.size());

   for (const auto& it : string_)
   {
      if (it == symbol_ && !ret.empty() && ret.back() == symbol_)
         continue;

      ret.push_back(it);
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
StringVec cat::split(const std::string& string_, std::string::value_type symbol_, bool keep_empty_)
{
   std::string newstr = remove_rep(string_, symbol_);

   StringVec ret;

   std::string::iterator marker = newstr.begin();

   for (std::string::iterator it = marker; it != newstr.end(); ++it)
   {
      if (*it == symbol_)
      {
         std::string str(marker, it);

         marker = std::next(it);

         bool add = str.empty() && (keep_empty_ == false);
         if (!add)
            ret.push_back(str);
      }
   }

   if (marker != newstr.begin())
   {
      std::string str(marker, newstr.end());

      bool add = str.empty() && (keep_empty_ == false);
      if (!add)
         ret.push_back(str);
   }
   else
      ret.push_back(string_);

   return ret;
}

//-----------------------------------------------------------------------------------------
StringVec cat::split(const std::string& string_, const std::string& splitter_, bool keep_empty_)
{
   StringVec ret;

   if (splitter_.empty())
      return { string_ };

   std::string remain = string_;

   auto ind = remain.find(splitter_);
   if (ind == -1)
      return { string_ };

   while (ind != -1)
   {
      std::string chunk(std::string(remain.begin(), remain.begin() + ind));

      bool isCopy = !chunk.empty() || keep_empty_;
      if (isCopy)
         ret.push_back(chunk);

      remain = std::string(remain.begin() + ind + splitter_.length(), remain.end());

      ind = remain.find(splitter_);
      if (ind == -1)
      {
         bool isCopy = !remain.empty() || keep_empty_;
         if (isCopy)
            ret.push_back(remain);
      }
   }

   return ret;
}

//-----------------------------------------------------------------------------------------
std::string cat::trim_left(const std::string& string_, std::string::value_type symbol_)
{
   int start_ind {};
   while (true && start_ind < string_.size())
   {
      if (string_.at(start_ind) != symbol_)
         break;
      start_ind++;
   }

   return string_.substr(start_ind, string_.length());
}

//-----------------------------------------------------------------------------------------
std::string cat::trim_right(const std::string& string_, std::string::value_type symbol_)
{
   int end_ind = (int)string_.length() - 1;
   while (true && end_ind >= 0)
   {
      if (string_.at(end_ind) != symbol_)
         break;
      end_ind--;
   }

   return string_.substr(0, end_ind + 1);
}

