#define SMOOTH_NONZEROS "SmoothNonzeros.txt"
#define SMOOTH_ZEROS "SmoothZeros.txt"

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <math.h>
#include "tokenizer.h"

using std::ifstream;
using std::map;
using std::cout;
using std::endl;
using std::string;

struct Bigram
{
  string w1;
  string w2;
  
  // constructor
  Bigram(string const & w1_, string const & w2_)
  : w1(w1_), w2(w2_) { } 
  
  bool operator< (Bigram const & bg) const 
  {
    return w1 < bg.w1 && w2 < bg.w2; 
  }  
};

int main(int argc, char * argv[])
{
  if (argc != 2)
  {
    cerr << "Usage: " << argv[0] << " filename" << endl;
    return 1;
  }

  map<Bigram, double> bigramList;
  map<string, double> wordList;

  ifstream in(argv[1]);
  if (!in.is_open())
  {
    cerr << "Error: Failed to open " << argv[1] << endl;
    return 1;
  }

  ifstream snz(SMOOTH_NONZEROS);
  if (!snz.is_open())
  {
    cerr << "Error: Failed to open " SMOOTH_NONZEROS << endl;
    return 1;
  }
  
  ifstream sz(SMOOTH_ZEROS);
  if (!sz.is_open())
  {
    cerr << "Error: Failed to open " SMOOTH_NONZEROS << endl;
    return 1;
  }
  
  cout << "Loading zeros..." << endl;
  
  while(!sz.eof())
  {
    string s; double d;
    sz >> s >> d;
    wordList[s] = d;
  };
  
  cout << "Loading nonzeros..." << endl;  
  
  while(!snz.eof())
  {
    string s1; string s2; double d;
    snz >> s1 >> s2 >> d;
    bigramList[Bigram(s1,s2)] = d;
  };  
  
  Tokenizer t(in);
  string lastToken = ".";
  
  double prob(0.0);
  int n;
  
  const double log2 = log(2);
  
  while(t.next())
  {
    Bigram bg(lastToken, t.token);
    map<Bigram, double>::iterator bi = bigramList.find(bg);
    
    // probability for this bigram
    double p;
    
    if (bi == bigramList.end()) // bigram not found
    {
      map<string, double>::iterator wi = wordList.find(lastToken);
      if (wi == wordList.end()) // word not found
      {
        continue;
        // we don't have to handle this case 
        // https://www1.columbia.edu/sec/bboard/021/coms4705-001/msg00178.html
      }
      p = wi->second;
    }
    else
      p = bi->second;
      
    prob += log(p) / log2;
    ++n;

    lastToken = t.token;
  }
  if (n == 0)
    cout << "Could not calculate entropy because _NO_ tokens in the given file"
      "appeared in the corpus" << endl;
  else
    cout << "H(input) = " << (-1.0 / (double)n * prob) << endl;
  
}