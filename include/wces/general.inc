<%

require_once("wces/login.inc");


/**
* Puts a warning into the database and sends an obnoxious email to rey4@columbia.edu
* It should be used like an assert, to ensure that unexpected conditions do
* not go unnoticed.
* 
* @param string  A text error message containing some details.
* @param string  Source file where error occurred. Should be set to __FILE__.
* @param string  Line number where error occured. Should be __LINE__.
*/	
function sendwarning($dump, $file, $line)
{
  $db = wces_connect();
  $uni = login_getuni();
  $errorid = db_addrow($db, "warninglog", array("file" => $file, "line" => $line, "uni" => $uni, "dump" => $dump));
  mail("rey4@columbia.edu", "Warning from $file in line $line", wordwrap("Dear Russell Yanofsky,\n\nThe SEAS Oracle would like you to know about an error it recently encountered in $file, line $line.\n\nThe error id is $errorid, and the text of the error message is as follows:\n\n$dump\n\n"));
}

/**
* Constructs an absolute URL. If any of the parameters is false, then the
* function uses information from the current page.
* 
* @param string  Protocol, normally "http://" or "https://" 
* @param string  Server name.
* @param string  Port, if 0 then the port is omitted.
* @param string  Request string such as "/folder/file?name1=value1"
* @return string An absolute URL constructed from the parameters
*/	

function absurl($protocol = false, $server = false, $port = false, $request = false)
{
  global $HTTP_ENV_VARS;
  
  if ($protocol === false)
  {
    if ($HTTP_ENV_VARS["HTTPS"] == "on")
    {
      $protocol = "https://";
      if ($port === false) $port = 0;
    }  
    else
    {
      $protocol = "http://";
      if ($port == false) $port = 0;
    }  
  }
  if ($server === false) $server = $HTTP_ENV_VARS["SERVER_NAME"];
  if ($port === false) $port = $HTTP_ENV_VARS["SERVER_PORT"];
  if ($request === false) $request = server_getrequest(); 
}

/**
* A wimpier explode() function. It splits a two parted string whose
* parts are separated by a single character into two strings. It 
* searches from the end of the original string.
* 
* @param string  The character to split by.
* @param string  The original string.
* @param string  A reference (&) variable that stores the first part
*                of the output.
* @param string  A reference (&) variable that stores the second part
*                of the output.
*/	

function simplesplit($splitchar,$name,$first,$last)
{
  $p = strrpos($name,$splitchar);
  if ($p === false)
  {
    $first = "";
    $last = $name;
  }
  else
  {
    $first = substr($name,0,$p);
    $last = substr($name,$p+1);
  }  
}

// redirect function. headers can only be sent before text
// calling exit() after this function will cause redirect
// to take place

function redirect($url)
{
  return false;
  if (headers_sent())
    return false;
  else
  {  
    //header ("HTTP/1.0 302 Object Moved");
    //header ("Request-URI: $url"); 
    //header ("Content-Location: $url"); 
    //header ("Location: $url");
    return true;
  }
}

function sloppyredirect($url)
{
  //print("<meta http-equiv=\"refresh\" content=\"0; URL=$url\">\n");
  //print("<script>\n<!-- \nwindow.location = '". addslashes($url) . "';\n// -->\n</script>\n");
  print("Click <a href=\"$url\">here</a> to proceed.");
}

function goup($path)
{
  return substr($path,0,max(strrpos($path,"\\"),strrpos($path,"/")));
}

function array_clean($arr)
{
  foreach($arr as $key => $value)
    if (!$value) unset($arr[$key]);
  return $arr;
}

function getAbsUrl($relurl)
{
  global $HTTP_ENV_VARS;
  $base = ($HTTP_ENV_VARS["SERVER_PORT_SECURE"] == "1" ? "https://" : "http://") .
           $HTTP_ENV_VARS["SERVER_NAME"] . ":" . $HTTP_ENV_VARS["SERVER_PORT"];
  $current = goup($HTTP_ENV_VARS["SCRIPT_NAME"]);
  $oldpos = 0;
  $pos = strpos($relurl,"/");
  while ($pos != 0)
  {
    if (substr($relurl,$oldpos,$pos-$oldpos) == "..")
    {
      $current = goup($current);
      $oldpos = $pos + 1;
      $pos = strpos($relurl, "/",$oldpos);  
    }  
    else
      $pos = false;
  }
  return ($pos === false) ? $base . $current . "/" .  substr($relurl,$oldpos) : $base . $relurl;
}

function gettime($microtime)
{ 
  $microtime = explode(" ",$microtime); 
  return $microtime[1] + $microtime[0]; 
}  

function printtimes($times)
{
  foreach($times as $key => $time)
    $times[$key] = gettime($time);
  $lasttime = array_shift($time);
  foreach($times as $key => $time)
  {
    print("  - Instruction $key: " . ($time - $lasttime) . "\n");
    $lasttime = $time;
  }
}
  
/*
  $times = array();
  $times["begin"] = microtime();
  interestingcommand(); 
  $times["interestingcommand"] = microtime();
  anotherone();
  $times["anotherone"] = microtime();
*/

function printarray(&$arr,$name)
{
  taskwindow_cprint("<table border=2><tr><td colspan=2>Array: $name</td></tr>");
  foreach($arr as $key => $value)
  {
    taskwindow_cprint("<tr><td>$key</td><td>$value</td></tr>");
  }
  taskwindow_print("</table>\n");  
}

function debugout($text,$label = "")
{
  taskwindow_print("\n\n\n<p><b><font color=red>$label</font></b>$text</p>\n\n\n");
}

function recursearray(&$arr,$name)
{
  taskwindow_cprint("<table border=2><tr><td colspan=2>Array: $name</td></tr>");
  foreach($arr as $key => $value)
  {
    taskwindow_cprint("<tr><td valign=top>$key</td><td>");
    if (is_array($value)) recursearray($value,$key); else taskwindow_cprint(htmlspecialchars($value));
    taskwindow_cprint("</td></tr>");
  }
  taskwindow_print("</table>\n");  
}

function ifassign($value,&$variable)
{
  if ($value) $variable = $value;
}

function param(&$var)
{
  if (!isset($var)) $var = false;
}

function emailvalid($email)
{
  $email = trim($email);
  return
    eregi("^[_a-z0-9-]+(\.[_a-z0-9-]+)*@([0-9a-z][0-9a-z-]*[0-9a-z]\.)+[a-z]{2}[mtgvu]?$", $email)
    ||
    eregi("^[^;,]*<[_\.0-9a-z-]+@([0-9a-z][0-9a-z-]+\.)+[a-z]{2,3}>$", $email);
}

%>
