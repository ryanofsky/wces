<?

require_once("wbes/general_url.inc");
require_once("wces/database.inc");

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
  global $server_url, $server_isproduction;

  $errorid = 0;
  $url = $server_url->toString(true,true,true);
  $uni = function_exists("login_getuni") ? login_getuni() : "";

  if ($server_isproduction && function_exists("wces_oldconnect"))
  {
    $db = wces_oldconnect();
    $errorid = db_addrow($db, "warninglog", array("url" => $url, "file" => $file, "line" => $line, "uni" => $uni, "dump" => $dump));
  }

  if ($uni) $uni = "'$uni' was logged in at the time\n";
  $message = "An error was encountered at $file:$line while loading $url.\n\nThe error id is $errorid, and the text of the error message is as follows:\n\n$dump\n\n$uni";

  if ($server_isproduction)
    mail("rey4@columbia.edu", "Warning from $file in line $line", wordwrap($message));
  else
    print("<pre>" . wordwrap($message) . "</pre>");
}

function sendfeedback($comments, $from, $subject = "WCES WEB FEEDBACK")
{
  global $HTTP_SERVER_VARS, $LOGIN_SESSION;
  if (!$from) $from = "Anonymous";
  mail("wces@columbia.edu",$subject,"Feedback from $from\n\n$comments");
  $comments .= "\n\n----------------------------------------\n";

  if (isset($HTTP_SERVER_VARS) && is_array($HTTP_SERVER_VARS))
  {
    $comments .= "SERVER VARIABLES\n\n";
    foreach($HTTP_SERVER_VARS as $key => $value)
    {
      $comments .= "$key = $value\n";
    }
  }

  if (isset($LOGIN_SESSION) && is_array($LOGIN_SESSION))
  {
    $comments .= "\nLOGIN SESSION VARIABLES\n\n";
    foreach($LOGIN_SESSION as $key => $value)
    {
      $comments .= "$key = $value\n";
    }
  }
  mail("rey4@columbia.edu","$subject+","Feedback from $from\n\n$comments");
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
function simplesplit($splitchar,$name,&$first,&$last)
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

/**
* Redirect the browser to the specified URL using HTTP headers if possible.
*
* @param string  Destination URL
*/
function redirect($url)
{
  global $server_cheapredirects, $server_isproduction, $server_url;

  if (headers_sent())
  {
    sendwarning("Redirect was attempted after headers had already been sent.", __FILE__, __LINE__);
    //if ($server_isproduction) 
      sloppyredirect($url);
  }
  else
  {
    if ($server_cheapredirects && $url[0] == '/')
    {
      $url2 = $server_url;
      $url2->path = $url;
      $url = $url2->toString(true);
    }
    //header ("Location: $url");
    print("redirect to <a href=\"$url\">$url</a>");
  }
  exit();
}

/**
* Redirect the browser in html.
* @param string  Destination URL
*/
function sloppyredirect($url)
{
  //print("<meta http-equiv=\"refresh\" content=\"0; URL=$url\">\n");
  //print("<script>\n<!-- \nwindow.location = '". addslashes($url) . "';\n// -->\n</script>\n");
  print("Click <a href=\"$url\">here</a> to proceed.");
  exit();
}

function array_clean($arr)
{
  foreach($arr as $key => $value)
    if (!$value) unset($arr[$key]);
  return $arr;
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
  $lasttime = array_shift($times);
  foreach($times as $key => $time)
  {
    print("  - Instruction $key: " . ($time - $lasttime) . " seconds<br>\n");
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
  printtimes($times);
*/

function printarray(&$arr,$name)
{
  taskwindow_cprint("<table border=2><tr><td colspan=2>Array: $name</td></tr>");
  foreach($arr as $key => $value)
  {
    $key   .= " (<i>" . gettype($key)   . "</i>)";
    $value .= " (<i>" . gettype($value) . "</i>)";
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

/**
* Initialize a variable with a default value if it doesn't exist.
*
* @param mixed  Variable to initialize
* @param mixed  Default value
*/
function param(&$var, $default = false)
{
  if (!isset($var)) $var = $default;
}

/**
* Same as param(), except that the variable is returned instead
* of modified by reference
*
* @param mixed   Variable to initialize
* @param mixed   Default value
* @return mixed  New value
*/
function rparam(&$var, $default = false)
{
  return isset($var) ? $var : $default;
}

function emailvalid($email)
{
  $email = trim($email);
  return
    eregi("^[_a-z0-9-]+(\.[_a-z0-9-]+)*@([0-9a-z][0-9a-z-]*[0-9a-z]\.)+[a-z]{2}[mtgvu]?$", $email)
    ||
    eregi("^[^;,]*<[_\.0-9a-z-]+@([0-9a-z][0-9a-z-]+\.)+[a-z]{2,3}>$", $email);
}

function quot($str)
{
  return "'" . addslashes($str) . "'";
}

function nullquot($str)
{
  return isset($str) ? ("'" . addslashes($str) . "'") : "NULL";
}

function mnullquot($str)
{
  return isset($str) && $str ? ("'" . addslashes($str) . "'") : "NULL";
}

function aarray_map($func, &$aa) // associative safe, in place array_map
{
  reset($aa);
  while(list($key) = each($aa))
    $aa[$key] = $func($aa[$key]);
}

function adarray_map($func, $aa) // associative safe
{
  $r = array();
  reset($aa);
  while(list($key) = each($aa))
    $r[$key] = $func($aa[$key]);
  return $r;  
}

function addurlparam($url, $param)
{
  if (strlen($param) == 0)
    return $url;
  else if (strpos($url,'?') === false)
    return "$url?$param";
  else
    return "$url&$param";
}

function array_bflip($a)
{
  $r = array();
  foreach($a as $v) $r[$v] = true;
  return $r;
}

function bprint($str, $mod = 10)
{
  static $i = 0;
  print($str);
  if (++$i % $mod == 0)
  {
    $i = 0;
    flush(); 
  }
}

?>