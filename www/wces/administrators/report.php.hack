<%
 require_once("widgets.inc");
 require_once("page.inc");
 require_once("login.inc");
 require_once("reporting.inc");
 login_protect(login_administrator);

%>
<style type="text/css">
<!--

body                    { font-family: Arial, Helvetica, sans-serif; }
p                       { font-family: Arial, Helvetica, sans-serif; }
a                       { text-decoration:none; }
a:hover                 { text-decoration:underline; color:#FF4033; }
h3                      { font-family: Arial, Helvetica, sans-serif; }
h4                      { font-family: Arial, Helvetica, sans-serif; background-color:#CCCCFF; padding: 2px; font-weight:bold; }

.pagebody               { padding:10px; font-family: Arial, Helvetica, sans-serif; }

.treeheading            { background-color:#0033E5; color:#FFFB18; cursor:hand; font-weight:bold; font-family:Arial, Helvetica, sans-serif; font-size:12pt; }
.treeheadinghover       { background-color:#00BE18; color:#FFFB18; cursor:hand; font-weight:bold; font-family:Arial, Helvetica, sans-serif; font-size:12pt; }
.treeheadinghover:hover { text-decoration:underline; color:#FFFB18;  }

.treebody               { padding:3px; background-color:#000000; color:#FFFFFF; font-family:Arial, Helvetica, sans-serif; font-size:9pt; }

.treelink               { color:#FFFFFF; text-decoration:none; font-weight:bold; font-family: Arial, Helvetica, sans-serif; font-size:9pt; }
.treelink:visited       { color:#DDDDDD; text-decoration:none; font-weight:bold; font-family: Arial, Helvetica, sans-serif; font-size:9pt; }                    
.treelink:hover         { color:#FFFFFF; text-decoration:underline; }

-->
</style>
<%


$wiz = new ReportOptions("wizard","wiz",WIDGET_POST);
$wiz->loadvalues();

%>
<form name="wiz" method="post">
<input type=hidden name=revivewizard value=0>
<% 
 if (!$wiz->isfinished() || $revivewizard)
   $wiz->display(false);
 else
 {
   print("<p><b><a href=\"javascript:document.forms['wiz']['revivewizard'].value = 1; void(document.forms['wiz'].submit())\">Back to wizard</a></b></p>");
   $wiz->display(true);
   $wiz->makereport();
 }
%></form><%
%>