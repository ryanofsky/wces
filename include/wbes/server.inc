<?
  require_once("wbes/general_url.inc");
  
  define("server_mode","yahweh"); // "oracle" "yahweh"  "yahwehisapi"

  $server_media = "/wces/media";

  if (server_mode == "oracle")
  {
    $server_isproduction = true;
    $server_win32 = false;
    $server_base = "http://oracle.seas.columbia.edu";
    $server_sbase = "https://oracle.seas.columbia.edu";
    $server_tempdir = "";
    $server_cheapredirects = false;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      $HTTP_SERVER_VARS["PATH_INFO"],
      "?" . $HTTP_SERVER_VARS["QUERY_STRING"]
    );
        
    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("host=localhost dbname=$dbname user=www password=------");
    }
  }
  else if (server_mode == "yahweh")
  {                        
    $server_isproduction = false;
    $server_win32 = true;  
    $server_base = "http://oracle.yahweh.dyndns.org";
    $server_sbase = "https://oracle.yahweh.dyndns.org:999";
    $server_tempdir = "M:/server/shares/temp";
    $server_cheapredirects = true;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      substr($HTTP_SERVER_VARS["PATH_INFO"], strlen($HTTP_SERVER_VARS["SCRIPT_NAME"])),
      isset($HTTP_SERVER_VARS["QUERY_STRING"]) ? "?" . $HTTP_SERVER_VARS["QUERY_STRING"] : ""
    );

    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };    
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("host=localhost dbname=$dbname user=Admin password=------");
    }
  }
  else if (server_mode == "yahwehisapi")
  {
    $server_isproduction = false;
    $server_win32 = true;
    $server_base = "http://oracle.yahweh.dyndns.org";
    $server_sbase = "https://oracle.yahweh.dyndns.org:999";
    $server_tempdir = "M:/server/shares/temp";
    $server_cheapredirects = false;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      substr($HTTP_SERVER_VARS["PATH_INFO"], len($HTTP_SERVER_VARS["SCRIPT_NAME"])),
      "?" . $HTTP_SERVER_VARS["QUERY_STRING"]
    );
            
    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("host=localhost dbname=$dbname user=Admin password=------");
    }
  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Name");
  
  $server_session_name = "session";  
?>