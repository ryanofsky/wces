// AffilGrabber.h : Declaration of the CAffilGrabber

#ifndef __AFFILGRABBER_H_
#define __AFFILGRABBER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAffilGrabber
class ATL_NO_VTABLE CAffilGrabber : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAffilGrabber, &CLSID_AffilGrabber>,
	public IDispatchImpl<IAffilGrabber, &IID_IAffilGrabber, &LIBID_AFLGRABLib>
{
public:
	CAffilGrabber()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_AFFILGRABBER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAffilGrabber)
	COM_INTERFACE_ENTRY(IAffilGrabber)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IAffilGrabber
public:
	STDMETHOD(get_rawoutput)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(Validate)(/*[in]*/ BSTR username, /*[in]*/ BSTR password, /*[in]*/ BSTR authurl, /*[out, retval]*/ int * returnval);

protected:
  CComBSTR _rawoutput;
  static size_t curl_write(void * ptr, size_t size, size_t nmemb, FILE * stream);
  static curl_getpass(void * client, char * prompt, char * buffer, int buflen);
  bool goodrequest();

};

#endif //__AFFILGRABBER_H_
