// AffilGrabber.cpp : Implementation of CAffilGrabber
#include "stdafx.h"
#include "Aflgrab.h"
#include "AffilGrabber.h"

/////////////////////////////////////////////////////////////////////////////
// CAffilGrabber

#include <string>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <iostream>

#define DIM(x) (sizeof(x)/sizeof(x[0]))

// TODO: Actually use error reporting facilities of curl, win32, and com instead of throwing zeroes.

class Curl
{
public:
  Curl() : _curl(::curl_easy_init()) { if (_curl == 0) throw 0; }
  ~Curl() { curl_easy_cleanup(_curl); }
  operator CURL*() { return _curl; }
private:
  CURL * _curl;
};

STDMETHODIMP CAffilGrabber::Validate(BSTR username, BSTR password, BSTR authurl, int * returnval)
{
  try
  {
    *returnval = false;
    
    Curl curl;
 
    USES_CONVERSION;
    curl_easy_setopt(curl, CURLOPT_URL, OLE2A(authurl));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CAffilGrabber::curl_write);
    curl_easy_setopt(curl, CURLOPT_FILE, (FILE*)this);
    curl_easy_setopt(curl, CURLOPT_HEADER, true);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);

    std::string authinfo(OLE2A(username)); authinfo += ":"; authinfo += OLE2A(password);
    curl_easy_setopt(curl, CURLOPT_USERPWD, authinfo.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWDFUNCTION, &curl_getpass);

    _rawoutput.Empty();
    curl_easy_perform(curl);
    
    if (goodrequest()) *returnval = true;
    return S_OK;
  }
  catch(...)
  {
     return E_UNEXPECTED;
  }
} 

STDMETHODIMP CAffilGrabber::get_rawoutput(BSTR *pVal)
{
  *pVal = this->_rawoutput;
	return S_OK;
}

size_t CAffilGrabber::curl_write(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  CAffilGrabber & afl = *((CAffilGrabber *)stream);
  afl._rawoutput.Append((const char*) ptr);
  return size * nmemb;
}

int CAffilGrabber::curl_getpass(void  *client, char  *prompt, char* buffer, int buflen )
{
  buffer[0] = NULL;
  return 0;
}

bool CAffilGrabber::goodrequest()
{
  const OLECHAR find[] = OLESTR("200");
  const OLECHAR quit = '\n';
  
  unsigned int len(_rawoutput.Length());
  unsigned int found(0);

  for(unsigned int i = 0; i<len && _rawoutput.m_str[i] != quit; ++i)
  if(_rawoutput.m_str[i] == find[found])
  {
    ++found; 
    if (found >= DIM(find) - 1) return true;
  }
  else
    found = 0;

  return false;
}