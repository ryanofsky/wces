<%
require_once("wces/login.inc");
require_once("wces/server.inc");

function page_top($title,$treestatus = "1000",$hidemenus = false)
{
  global $server_wcespath;
%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd"> 
<!-- This page was created by the WCES development team for Columbia University, 2000 -->
<html>
<head>
<title><%=$title%></title>

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

.tinybutton             { FONT-WEIGHT: bold; FONT-SIZE: x-small; CURSOR: hand;  FONT-FAMILY: Verdana, Geneva, Arial, Helvetica, sans-serif; TEXT-DECORATION: none; }

@page                   { margin-left: 0.5in; margin-right: 0.5in; margin-top: 0.5in; margin-bottom: 0.5in; }

span.up { background-color: #0030E7; cursor: hand; color: #FFFFFF; font-weight: bold; padding: 0 3; border: outset #0030E7 4px; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 9pt; }
span.down { background-color: #0030E7; cursor: hand; color: #FFFFFF; font-weight: bold; padding: 0 3; border: inset #0030E7 4px; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 9pt; }
span.on { background-color: #336699; cursor: hand; color: #FFFFFF; font-weight: bold; padding: 0 3; border: inset #99CCFF 4px; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 9pt; }
span.off { background-color: #0030E7; color: #FFFFFF; font-weight: bold; padding: 0 3; border: solid #0030E7 4px; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 9pt; }
a.blank { color: #FFFFFF; text-decoration: none; }
a.blank:hover { color: #FFFFFF; text-decoration: none; }

-->
</style>

<script language="javascript">
<!--

  var happyrollover = document.images && window.Image ? true : false;
  var happytree = happyrollover && RegExp && (document.all || document.getElementById) ? true : false;
  var detect = navigator.userAgent.toLowerCase();
  var refreshbug =  (detect.indexOf('compatible') < 0 && detect.charAt(8) == '5') ? true : false; // check for mozilla

  // fix for Netscape 3 string object
  if (happyrollover && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

  function getImage(oldimage,filename)
  {
    var p = oldimage.src.lastIndexOf("/");
    var q = oldimage.src.lastIndexOf("\\");
    var i = new Image();
    i.src = oldimage.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
    return i;
  }; 

  function AttachImage(img, filename)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      i.flipimage = getImage(i,filename);
    };
  };
  
  function image_swap(img)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      var temp = i.src;
      i.src = i.flipimage.src;
      i.flipimage.src = temp;
    };
  };


  function childhunt(target,element)
  {
    if (element.name == target)
      return element;
    else if (element.childNodes)
    {
      var l = element.childNodes.length;
      var t = null;
      for(var i = 0; i < l; ++i)
      {
        t = childhunt(target,element.childNodes.item(i));
        if (t) return t;
      };  
    }
    return null;
  };
  
  function setclass(element,classname)
  {
    if (element.childNodes)
    {
      if (element && element.style)
      {
        element.className = classname;
        var l = element.childNodes.length;
        for(var i = 0; i < l; ++i)
          setclass(element.childNodes.item(i),classname);  
      };  
    }
    else if(element.all)
    {
      element.className = classname;
      var l = element.all.length;
      for(var i = 0; i < l; ++i)
      {
        var o = element.all.item(i);
        if (o.style) o.className = classname;
      };  
    };
  };

  function treehead_init(treeheadid,treebodyid)
  {
    if (happytree)
    {
      var treehead = document.getElementById ? document.getElementById(treeheadid) : document.all(treeheadid);
      treehead.onmouseover = treehead_onmouseover;
      treehead.onmouseout = treehead_onmouseout;

      if (treebodyid)
      {
        var treecount = treebranches.length;
        treebranches[treecount] = treehead;
        treehead.onclick = treehead_onclick;
        treehead.isopen = (treestatus.length > treecount) && (treestatus.charAt(treecount) == "1");
        treehead.treebody = document.getElementById ? document.getElementById(treebodyid) : document.all(treebodyid);
        treehead.icon = treehead.childNodes ? childhunt("headimage",treehead) : treehead.all.item("headimage");
        treehead.plus = getImage(treehead.icon,"plus_m.gif");
        treehead.minus = getImage(treehead.icon,"minus_m.gif");
        if (!treehead.isopen)
        {
          treehead.treebody.style.display = "none";  
          treehead.icon.src = getImage(treehead.icon,"plus.gif").src;
        };
      };  
    };
  };  
  
  function treehead_onmouseover()
  {
    setclass(this,'treeheadinghover');
  };
 
  function treehead_onmouseout()
  {
    setclass(this,'treeheading');
  };
  
  function treehead_onclick()
  {
    if (this.treebody)
    {
      this.isopen = !this.isopen;
      var display = this.isopen ? "block" : "none";
      var iconsrc = this.isopen ? this.minus.src : this.plus.src;

      if (document.all)
      { 
        this.treebody.style.display = display;
        this.icon.src = iconsrc;
      }
      else if (document.getElementById)
      { 
        var table = document.getElementById("thetree");
        if (refreshbug) table.style.visibility = "hidden"; // workaround for mozilla refresh bug
        this.treebody.style.display = display;
        this.icon.src = iconsrc;
        if (refreshbug) table.style.visibility = "visible";
      }
      var ts = "";
      for(var i in treebranches)
        ts += (treebranches[i].isopen == true) ? "1" : treebranches[i].isopen == false ? "0" : "";
      document.cookie = 'treestatus=' + ts + '; path=/';
    };  
  };
      
  if (happytree)
  {
    //this doesn't fly on IE5 Mac -->  var re = /.*(?:^|;)[ ]*treestatus=([^;]*)(?:$|;).*/;
    var re = new RegExp(".*(^|;)[ ]*treestatus=([^;]*)($|;).*");
    window.treestatus = (re.test(document.cookie)) ? document.cookie.replace(re,"$2") : "<%=$treestatus%>";
    window.treebranches = new Array();
  };    

//-->
</script>
</head>

<body background="<%=$server_wcespath%><%=$hidemenus ? "media/paper2.jpg" : "media/0x0030E7.gif"%>" bgcolor="#e9e9fd" text="#000000" link="#000090" vlink="#000090" alink="#000000" leftmargin="0" topmargin="0" rightmargin="0" marginwidth="0" marginheight="0">
<% if (!$hidemenus) { %>
<table width="100%" height="100%" border=0 cellpadding=0 cellspacing=0>
<tr>
  <td height=65 align="middle"><A onmouseover="image_swap('i1')" onmouseout="image_swap('i1')" href="/index.html"><img name="i1" border="0" src="<%=$server_wcespath%>media/wces1.gif" width="115" height="65"></a></td>
  <td height=65><a href="<%=$server_wcespath%>" target="_top" onmouseover="image_swap('i2')" onmouseout="image_swap('i2')"><img name="i2" border="0" src="<%=$server_wcespath%>media/wcestitle.gif" width="600" height="57"></a>
    <script language="JavaScript">
    <!--
      AttachImage("i1","wces2.gif");
      AttachImage("i2","wcestitle2.gif");
    // -->
    </script>
  </td>
</tr>  
<tr>
  <td valign="top" width="130">  
    <table id="thetree" border=0 cellpadding=1 cellspacing=0>

<% if (login_isloggedin()) { %>
    <tr id="logout" class="treeheading">
      <td><img name="headimage" src="<%=$server_wcespath%>media/menu/solid.gif" width="15" height="15" align="middle"></td>
      <td class="treeheading" nowrap><a href="<%=$server_wcespath%>login/logout.php" class="treeheading">Log Off <%=login_getuni()%></a></td>
    </tr>
    <tr><td><script>treehead_init("logout")</script>&nbsp;</td></tr>     
<% } %> 

    <tr id="section1h" class="treeheading">
      <td><img name="headimage" src="<%=$server_wcespath%>media/menu/minus.gif" width="15" height="15" align="middle"></td>
      <td class="treeheading" nowrap>General</td>
    </tr>
    <tr id="section1">
      <td>&nbsp;</td>
       <td class="treebody" bgcolor="#000000">
        <p><a href="<%=$server_wcespath%>index.php" class="treelink">Home</a></p>
        <p><a href="<%=$server_wcespath%>about/confidentiality.php" class="treelink">Confidentiality</a></p>
        <p><a href="<%=$server_wcespath%>about/feedback.php" class="treelink">Feedback</a></p>
        <p><a href="<%=$server_wcespath%>about/credits.php" class="treelink">Credits</a></p>
      </td>
    </tr>
    <tr><td><script>treehead_init("section1h","section1")</script>&nbsp;</td></tr>

    <tr id="section2h" class="treeheading">
      <td><img name="headimage" src="<%=$server_wcespath%>media/menu/minus.gif" width="15" height="15" align="middle"></td>
      <td class="treeheading" nowrap>Students</td>
    </tr>
    <tr class="treeheading" id="section2">
      <td>&nbsp;</td>
      <td class="treebody">
        <p><a href="<%=$server_wcespath%>students/survey.php" class="treelink">Complete a Survey</a></p>
        <p><a href="/oracle/" class="treelink">View Survey Results</a></p>
      </td>
    </tr>    
    <tr><td><script>treehead_init("section2h","section2")</script>&nbsp;</td></tr>

    <tr id="section3h" class="treeheading">
      <td><img name="headimage" src="<%=$server_wcespath%>media/menu/minus.gif" width="15" height="15" align="middle"></td>
      <td class="treeheading" nowrap>Professors</td>
    </tr>
    <tr id="section3">
      <td>&nbsp;</td>
      <td class="treebody">
        <p><a href="<%=$server_wcespath%>professors/index.php" class="treelink">Home</a></p>
        <p><a href="<%=$server_wcespath%>professors/editsurveys.php" class="treelink">Upcoming Surveys</a></p>
        <p><a href="<%=$server_wcespath%>professors/seeresults.php" class="treelink">See Survey Results</a></p>
        <p><a href="<%=$server_wcespath%>login/profsearch.php" class="treelink">Update CUNIX Association</a></p>
      </td>
    </tr>
    <tr><td><script>treehead_init("section3h","section3")</script>&nbsp;</td></tr>

    <tr id="section4h" class="treeheading">
      <td><img name="headimage" src="<%=$server_wcespath%>media/menu/minus.gif" width="15" height="15" align="middle"></td>
      <td class="treeheading" nowrap>Administrators</td>
    </tr>
    <tr id="section4">
      <td>&nbsp;</td>
      <td class="treebody">
        <p><a href="<%=$server_wcespath%>administrators/semester.php" class="treelink">New Semester Initialiation</a></p>
        <p><a href="<%=$server_wcespath%>administrators/pusage.php" class="treelink">Professor Usage</a></p>
        <p><a href="<%=$server_wcespath%>administrators/susage.php" class="treelink">Student Usage</a></p>
        <p><a href="<%=$server_wcespath%>administrators/report.php" class="treelink">Reporting Wizard</a></p>                                
        <p><a href="<%=$server_wcespath%>administrators/import.php" class="treelink">Import Data</a></p>
        <p><a href="<%=$server_wcespath%>administrators/fakelogin.php" class="treelink">Fake Login</a></p>
        <p><a href="<%=$server_wcespath%>administrators/enrollment.php" class="treelink">Enrollment Viewer</a></p>
        <p><a href="<%=$server_wcespath%>administrators/pstats.php" class="treelink">Professor Listing</a></p>
      </td>
    </tr>
    <tr><td><script>treehead_init("section4h","section4")</script>&nbsp;</td></tr>    
    </table>
  </td>
  <td bgcolor="#e5e6fa" background="<%=$server_wcespath%>media/paper2.jpg" valign="top">
<% } %>  
  <div class="pagebody">
<%  
};

function page_bottom($hidemenus = false)
{
%>
  </div>
<% if (!$hidemenus) { %>
  </td>
</tr>
<tr><td colspan=2 height=10>&nbsp;</td></tr>
</table>
<% } %>
</body>
</html>
<%
};
%>


