<HTML>
<HEAD>
<META NAME="GENERATOR" Content="Microsoft Visual Studio 6.0">
</HEAD>


<CFPARAM NAME="FORM.uni" DEFAULT="">
<CFPARAM NAME="FORM.password" DEFAULT="">

<BODY>

<CFOUTPUT>
<p>Enter a CUNIX username and password below to retrieve AcIS affiliations</p>
<form name=default method=post action=fetch.cfm>
<TABLE>
  <TR>
    <TD>UNI</TD>
    <TD><INPUT type=text name=uni value="#FORM.uni#"></TD>
  </TR>
  <TR>
    <TD>Password</TD>
    <TD><INPUT type=password name=password value="#FORM.password#"></TD>
  </TR>
  <tr><td colspan=2 align=middle><INPUT type="submit" value="Get My Affiliations"></td></tr> 
</TABLE>
</FORM>
</CFOUTPUT>

<hr>

<cfobject TYPE="COM" ACTION="CREATE" NAME="grabber" CLASS="Oracle.AffilGrabber">
<CFSCRIPT>
  if (LEN(FORM.uni) GREATER THAN 0 AND LEN(FORM.password) GREATER THAN 0)
  {
    if (grabber.Validate(FORM.uni,FORM.password,"https://www1.columbia.edu/~rey4/info.html") IS 0)
      writeoutput("<p><strong><font color=red>Invalid User Name or Password</font></strong></p>");
    else
      writeoutput("<p><strong><font color=blue>Valid Logon</font></strong></p>");
    writeoutput('<pre>'); writeoutput(HTMLCodeFormat(grabber.rawoutput)); writeoutput('</pre><hr>');
  }
</CFSCRIPT>

</BODY>
</HTML>
