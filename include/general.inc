<%

// redirect function. headers can only be sent before text
// calling exit() after this function will cause redirect
// to take place

function redirect($url)
{
  sloppyredirect($url); // make it work on apache
  return true;
  if (headers_sent())
    return false;
  else
  {  
    //header ("HTTP/1.0 302 Moved Temporarily");
    //header ("Request-URI: $url"); 
    //header ("Content-Location: $url"); 
    header ("Location: $url");
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

%>