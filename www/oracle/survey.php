<%
require_once("oracle/page.inc");
page_top("SEAS Oracle Homepage - Survey","survey.gif");
%>
      <FORM action=http://www.columbia.edu/cgi-bin/generic-inbox.pl 
      method=post>
      
      <p>&nbsp;</p>
      
      <FONT face="Verdana,Trebuchet MS" size=-1><INPUT type=hidden 
      value="wces@columbia.edu" name=mail_dest> <INPUT type=hidden 
      value="Oracle Feedback" name=subject> <INPUT type=hidden value=true 
      name=echo_data> <INPUT type=hidden value=true name=tab_delim> <INPUT 
      type=hidden value=true name=cvt_nl> <INPUT type=hidden 
      value=major,year,overall name=reqfields> 
      <P align=left>Thank You for visiting the SEAS Oracle and WCES. In order 
      for us to further improve this site, we would appreciate your feedback. 
      </P>
      <TABLE width="30%" border=0>
        <TBODY>
        <TR>
          <TD><B>Email*</B></TD>
          <TD><B>Major</B></TD>
          <TD><B>Year</B></TD></TR>
        <TR>
          <TD><INPUT size=10 name=email></TD>
          <TD><INPUT name=major></TD>
          <TD><INPUT size=4 name=year></TD></TR></TBODY></TABLE><I>*optional, if you 
      want us to respond</I> 
      <P align=left>How would you rate this site overall? With 5 being Excellent 
      and 1 being Awful<BR><INPUT type=radio value=1 name=overall> 1 <INPUT 
      type=radio value=2 name=overall> 2 <INPUT type=radio value=3 name=overall> 
      3 <INPUT type=radio value=4 name=overall> 4 <INPUT type=radio value=5 
      name=overall> 5 </P>
      <P align=left>Do you think having this type of site is a good idea? <INPUT 
      type=radio value=Yes name=goodidea> Yes <INPUT type=radio value=No 
      name=goodidea> No </P>
      <P align=left>Why or why not? </P><TEXTAREA name=whygoodornot rows=5 cols=50></TEXTAREA> <BR>
      <P align=left>How useful was this site to you on a scale of 1-5, 5 being 
      excellent<BR><INPUT type=radio value=1 name=useful> 1 <INPUT type=radio 
      value=2 name=useful> 2 <INPUT type=radio value=3 name=useful> 3 <INPUT 
      type=radio value=4 name=useful> 4 <INPUT type=radio value=5 name=useful> 5 
      </P>
      <P align=left>How could we improve this site in terms of content? What 
      would you suggest to make it more useful in terms of content?<BR><TEXTAREA name=howImprove rows=5 cols=50></TEXTAREA> <BR></P>
      <P align=left>How did you like the WCES? Comment on its functions, layout, 
      appearance, speed, ease of use, etc. How can we improve it?<BR><TEXTAREA name=howwces rows=5 cols=50></TEXTAREA> <BR></P>
      <P align=left>How did you like the Oracle? Comment on its functions, 
      layout, appearance, etc. <BR><TEXTAREA name=howPDF rows=5 cols=50></TEXTAREA> <BR></P>
      <P align=left>Rate the WCES overall on a scale of 1-5, 5 being 
      excellent<BR><INPUT type=radio value=1 name=applet> 1 <INPUT type=radio 
      value=2 name=applet> 2 <INPUT type=radio value=3 name=applet> 3 <INPUT 
      type=radio value=4 name=applet> 4 <INPUT type=radio value=5 name=applet> 5 
      </P>
      <P>Rate the Oracle overall on a scale of 1-5, 5 being excellent<BR><INPUT 
      type=radio value=1 name=pdf> 1 <INPUT type=radio value=2 name=pdf> 2 
      <INPUT type=radio value=3 name=pdf> 3 <INPUT type=radio value=4 name=pdf> 
      4 <INPUT type=radio value=5 name=pdf> 5 </P>
      <P align=left>Any other suggestions or comments? Please feel free to tell 
      us anything...<BR><TEXTAREA name=anything rows=5 cols=50></TEXTAREA> 
      <BR></P><INPUT type=reset value=Reset> <INPUT type=submit value=Submit> 
      </FONT>
<%
page_bottom();
%>