
aflgrabps.dll: dlldata.obj aflgrab_p.obj aflgrab_i.obj
	link /dll /out:aflgrabps.dll /def:aflgrabps.def /entry:DllMain dlldata.obj aflgrab_p.obj aflgrab_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del aflgrabps.dll
	@del aflgrabps.lib
	@del aflgrabps.exp
	@del dlldata.obj
	@del aflgrab_p.obj
	@del aflgrab_i.obj
