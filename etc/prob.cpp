#pragma warning( push )
#pragma warning( disable : 4786 )

#include "tokenizer.h"

#define CORPUS "tolstoy.txt"
#define UNSMOOTH_NONZEROS "UnsmoothNonzeros.txt"
#define SMOOTH_NONZEROS "SmoothNonzeros.txt"
#define SMOOTH_ZEROS "SmoothZeros.txt"
#define WORD_COUNTS "WordCounts.txt"

#include <map>

#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include <float.h>
#include <math.h>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::ifstream;
using std::ofstream;
using std::string;
using std::pair;
using std::binary_function;

// return true if one second has elapsed since the last time this function returned true
inline bool oneSecond(int mask = 0x7FFF)
{
  static int i = -1;
  static int lastTime = 0;
  ++i;
  if ((i & mask) == 0)
  {
    int t = time(NULL);
    if (t - lastTime > 1)
    {
      lastTime = t;
      return true;
    }
  }
  return false;
}

// information about a bigram
struct Bigram
{
  // number of times this bigram occurs in the text
  int count; 
  
  // unsmoothed probability for this bigram
  double prob; 
    
  // smoothed probability of this bigram
  double smoothProb; 

  Bigram() : count(0), prob(0), smoothProb(0) {}
};

// forward declaration (see below for definition)
struct Word;

// paired string object and Word Object
typedef pair<string const, Word> StringWord;

// functor for keeping StringWord objects in alphabetical order
struct StringWordComparator : public binary_function<StringWord*, StringWord*, bool> 
{
  bool operator()(StringWord * x, StringWord * y) const;
};

typedef map<StringWord *, Bigram, StringWordComparator> BigramMap;
typedef BigramMap::iterator BigramIterator;

// information about a word
struct Word
{
  // number of times this word occurs
  int count; 
   
  // number of distinct bigrams that begin with this word
  int countBigrams; 
    
  // smoothed probability for zero bigrams that begin with this word
  double zeroProb; 

  // list of bigrams that begin with this word
  // indexed by pointer to second word
  BigramMap bigramList;

  Word() : count(0), countBigrams(0), zeroProb(0) {} 
};

bool StringWordComparator::operator()(StringWord * x, StringWord * y) const
{
  return x->first < y->first;
};

typedef map<string, Word> WordMap;
typedef WordMap::iterator WordIterator;

class Dictionary
{
public:

  int countWords; // number of distinct words
  int countBigrams; // number of distinct bigrams

  WordMap wordList;

  Dictionary() : countWords(0), countBigrams(0) {}
  
  StringWord * addWord(string const & str)
  {
    typedef pair<WordIterator, bool> rt;
    rt r = wordList.insert(StringWord(str,Word()));
    ++r.first->second.count;
    if (r.second) // if new insertion
      ++countWords;
    return &(*(r.first));
  };

  void addBigram(StringWord * first, StringWord * second)
  {
    typedef pair<BigramIterator, bool> rt;
    Word & w = first->second;
    rt r = w.bigramList.insert(BigramMap::value_type(second,Bigram()));
    ++r.first->second.count;
    if (r.second) // if new insertion
    {
      ++countBigrams;
      ++w.countBigrams;
    }
  }

  void findProbs()
  {
    // outer loop on all the words in the dictionary
    for (WordIterator i = wordList.begin(); i != wordList.end(); ++i)
    {
      Word & w = i->second;

      // inner loop on all bigrams beginning with the current word
      for (BigramIterator j = w.bigramList.begin(); j != w.bigramList.end(); ++j)
      {
        Bigram & b = j->second;
        b.prob = (double)b.count / (double)w.count;
      }
    }
  }

  void smoothProbs()
  {
    // outer loop on all the words in the dictionary
    for (WordIterator i = wordList.begin(); i != wordList.end(); ++i)
    {
      Word & w = i->second;
      
      // inner loop on all bigrams beginning with the current word
      for (BigramIterator j = w.bigramList.begin(); j != w.bigramList.end(); ++j)
      {
        Bigram & b = j->second;
        b.smoothProb = (double)b.count / (double)(w.count + w.countBigrams);
      }

      // number of zero bigrams beginning with current word, w
      int Z = countWords - w.countBigrams;

      w.zeroProb = (double)w.countBigrams / (double)(w.count + w.countBigrams)
        / (double)Z;
    }
  }
};

int main()
{
  ifstream f(CORPUS);

  if (!f.is_open())
  {
    cerr << "Error: Failed to open " CORPUS << endl;
    return 1;
  }

  Tokenizer t(f);
  Dictionary d;

  StringWord * lastWord = 0;
  while(t.next())
  {
    StringWord * word = d.addWord(t.token);
    if (lastWord)
      d.addBigram(lastWord, word);
    lastWord = word;
    if (oneSecond())
    {
      cout << "Processing line " << t.lineno << "...\n"
        << d.countWords << " distinct tokens found\n"
        << d.countBigrams << " distinct bigrams found\n" << endl;
    }
  }

  cout << "Finding probabilities..." << endl;
  d.findProbs();

  cout << "Finding smoothed probabilities..." << endl;
  d.smoothProbs();


  cout << "Saving results..." << endl;
  
  ofstream unz(UNSMOOTH_NONZEROS);
  if (!unz.is_open())
  {
    cerr << "Error: Failed to open " UNSMOOTH_NONZEROS << endl;
    return 1;
  }

  ofstream snz(SMOOTH_NONZEROS);
  if (!snz.is_open())
  {
    cerr << "Error: Failed to open " SMOOTH_NONZEROS << endl;
    return 1;
  }
  
  ofstream sz(SMOOTH_ZEROS);
  if (!sz.is_open())
  {
    cerr << "Error: Failed to open " SMOOTH_ZEROS << endl;
    return 1;
  }

  ofstream wc(WORD_COUNTS);
  if (!wc.is_open())
  {
    cerr << "Error: Failed to open " WORD_COUNTS << endl;
    return 1;
  }

  int soFar = 1;
  // outer loop on all the words in the dictionary
  for (WordIterator i = d.wordList.begin(); i != d.wordList.end(); ++i)
  {
    if (oneSecond(0xF))
      cout << "Saving token [" << soFar << " / " << d.countWords << "]" << endl;
    
    Word & w = i->second;
    sz << i->first << " " << w.zeroProb << "\n";
    wc << i->first << " !! " << w.count << "\n";
    // inner loop on all bigrams beginning with the current word
    for (BigramIterator j = w.bigramList.begin(); j != w.bigramList.end(); ++j)
    {
      Bigram & b = j->second;
      unz << i->first << " " << j->first->first << " " << b.prob << "\n";
      snz << i->first << " " << j->first->first << " " << b.smoothProb << "\n";
      wc  << i->first << " " << j->first->first << " " << b.count << "\n";
      b.prob = (double)b.count / (double)w.count;
    }
    ++soFar;
  }

  return 0;
}

#pragma warning( pop )