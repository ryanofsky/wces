<%@ Language=Javascript %>

<% 
  var uni = Request.Form("uni");
  var password = Request.Form("password");
  uni = uni.Count == 1 ? uni : "";
  password = password.Count == 1 ? password : "";
  
  var crlf = "\x0D\x0A";
  
%>  

<HTML>
<HEAD>

<META NAME="GENERATOR" Content="Microsoft Visual Studio 6.0">
</HEAD>
<BODY>

<P>
<form name=default method=post action=fetch.asp>
<TABLE>
  <TR>
    <TD>UNI</TD>
    <TD>
    
    <%='<INPUT type=text name=uni value="'+uni+'">'%>
    
    </TD></TR>
  <TR>
    <TD>Password</TD>
    <TD>
    
    <%='<INPUT type=password name=password value="'+password+'">'%>

  </TD></TR>
   <tr>
   <td colspan=2 align=middle><INPUT type="submit" value="Get My Affiliations"></td>
   </tr> 
</TABLE>
</P>
<hr>

<%

if (uni != "" || password != "")
{
  var grabber = Server.CreateObject("Oracle.AffilGrabber");
  var result = grabber.retrieve("www1.columbia.edu",443,"/~rey4/info.html",uni,password);
  Response.Write('<pre>' + Server.HTMLEncode(result) + '</pre><hr>');
}
%>

</FORM>

</BODY>
</HTML>
