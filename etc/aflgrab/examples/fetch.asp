<%@ Language=Javascript %>

<% 
  var uni = Request.Form("uni");
  var password = Request.Form("password");
  uni = uni.Count == 1 ? uni : "";
  password = password.Count == 1 ? password : "";
%>  

<HTML>
<HEAD>
<META NAME="GENERATOR" Content="Microsoft Visual Studio 6.0">
</HEAD>
<BODY>

<p>Enter a CUNIX username and password below to retrieve AcIS affiliations</p>
<form name=default method=post action=fetch.asp>
<TABLE>
  <TR>
    <TD>UNI</TD>
    <TD><INPUT type=text name=uni value="<%=uni%>"></TD>
  </TR>
  <TR>
    <TD>Password</TD>
    <TD><INPUT type=password name=password value="<%=password%>"></TD>
  </TR>
  <tr><td colspan=2 align=middle><INPUT type="submit" value="Get My Affiliations"></td></tr> 
</TABLE>
</FORM>

<hr>

<%
  if (uni && password)
  {
    var grabber = Server.CreateObject("Oracle.AffilGrabber");
    if (!grabber.Validate(uni,password,"https://www1.columbia.edu/~rey4/info.html"))
      Response.Write("<p><strong><font color=red>Invalid User Name or Password</font></strong></p>");
    else
      Response.Write("<p><strong><font color=blue>Valid Logon</font></strong></p>");
    Response.Write('<pre>' + Server.HTMLEncode(grabber.rawoutput) + '</pre><hr>');
  }
%>



</BODY>
</HTML>
