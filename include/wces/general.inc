<%

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
  print("<meta http-equiv=\"refresh\" content=\"0; URL=$url\">\n");
  print("<script>\n<!-- \nwindow.location = '". addslashes($url) . "';\n// -->\n</script>\n");
  print("Click <a href=\"$url\">here</a> to proceed.");
}

function goup($path)
{
  return substr($path,0,max(strrpos($path,"\\"),strrpos($path,"/")));
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
%>
