#ifndef tokenizer_h
#define tokenizer_h

#include <iostream>
#include <string>

using std::cerr;
using std::endl;
using std::string;
using std::ostream;
using std::istream;

class Tokenizer
{
public:
  istream & file;
  int lineno;
  string token;
  Tokenizer(istream & file_) : file(file_), lineno(1) { }  

  bool next()
  {
    bool inAlphaNum = false;
    
    for(;;)
    {
      int c = file.peek();
      bool isEof = c == EOF;
      bool isAlphaNum = isdigit(c) != 0 || isalpha(c) != 0;
      bool isPunc = !isAlphaNum && c > 32;
      bool isLine = c == '\n';

      if (isLine) ++lineno;

      if (inAlphaNum)
      {
        if (isAlphaNum)
        {
          token += c;
          file.get();
        }
        else
          return true;
      }
      else if (isAlphaNum)
      {
        token = c;
        file.get();
        inAlphaNum = true;
      }
      else if (isPunc)
      {
        token = c;
        file.get(); 
        return true;
      }
      else if (isEof)
        return false;
      else // whitespace
        file.get();
    }
  }
};

#endif
