<?
  require_once("wbes/general_url.inc");
  
  define("server_mode","oracle"); // "oracle" "yahweh"  "yahwehisapi"

  $server_media = "/media";
  $server_session_name = "session";    
  ini_set ('display_startup_errors', 1);
  ini_set ('display_errors', 1);
  error_reporting (E_ALL);

  if (server_mode == "oracle")
  {
    $server_isproduction = false;
    $server_win32 = false;
    $server_base = "https://eval.thayer.dartmouth.edu/";
    $server_sbase = "https://eval.thayer.dartmouth.edu/";
    $server_tempdir = "";
    $server_cheapredirects = false;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      isset($HTTP_SERVER_VARS["PATH_INFO"]) ? $HTTP_SERVER_VARS["PATH_INFO"] : "",
      $HTTP_SERVER_VARS["QUERY_STRING"]
    );
        
    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("dbname=$dbname user=postgres");
    }
  }
  else if (server_mode == "yahweh")
  {                        
    $server_isproduction = false;
    $server_win32 = true;  
    $server_base = "http://dartmouth.esurveys.hn.org";
    $server_sbase = "http://dartmouth.esurveys.hn.org";
    $server_tempdir = "M:/server/shares/temp";
    $server_cheapredirects = true;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      substr($HTTP_SERVER_VARS["PATH_INFO"], strlen($HTTP_SERVER_VARS["SCRIPT_NAME"])),
      isset($HTTP_SERVER_VARS["QUERY_STRING"]) ? $HTTP_SERVER_VARS["QUERY_STRING"] : ""
    );

    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };    
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("host=localhost dbname=$dbname user=postgres password=lewrujop");
    }
  }
  else if (server_mode == "yahwehisapi")
  {
    $server_isproduction = false;
    $server_win32 = true;
    $server_base = "http://russ.esurveys.hn.org";
    $server_sbase = "http://russ.esurveys.hn.org";
    $server_tempdir = "M:/server/shares/temp";
    $server_cheapredirects = false;
    $server_url = new URL
    (
      $HTTP_SERVER_VARS["HTTPS"] == "on",
      $HTTP_SERVER_VARS["SERVER_NAME"],
      $HTTP_SERVER_VARS["SERVER_PORT"],
      $HTTP_SERVER_VARS["SCRIPT_NAME"],
      substr($HTTP_SERVER_VARS["PATH_INFO"], strlen($HTTP_SERVER_VARS["SCRIPT_NAME"])),
      isset($HTTP_SERVER_VARS["QUERY_STRING"]) ? $HTTP_SERVER_VARS["QUERY_STRING"] : ""
    );
            
    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };
    
    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("host=localhost dbname=$dbname user=postgres password=--------");
    }
  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Name");
?>
