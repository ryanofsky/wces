<?
  // old globals predating config file, should gradually phase these out
  $wces_path = $WCES_CONFIG_PATH;
  $oracle_root = $ORACLE_PATH;
  $server_isproduction = !$WBES_DEBUG;
  $server_media = $WCES_CONFIG_MEDIA_PATH;
  $server_tempdir = "";
  $server_feedback = $WBES_FEEDBACK;
  $server_dev = $WBES_DEVELOPER;

  require_once("wbes/general_url.inc");

  function server_mysqlinit()
  {
    return mysql_connect("localhost","root","------");
  };

  function server_pginit($dbname)
  {
    return pg_Connect("dbname=$dbname user=postgres");
  }

  if ($WBES_SERVER == "apache")
  {
    $server_win32 = false;
    $server_cheapredirects = false;

    // on apache, SERVER_NAME is canonical name determined from apache 
    // configuration. HTTP_HOST is the actual host used in the HTTP
    // request
    
    $server_url = new URL
    (
      isset($_SERVER["HTTPS"]) && $_SERVER["HTTPS"] == "on",
      $_SERVER["HTTP_HOST"],
      $_SERVER["SERVER_PORT"],
      $_SERVER["SCRIPT_NAME"],
      isset($_SERVER["PATH_INFO"]) ? $_SERVER["PATH_INFO"] : "",
      $_SERVER["QUERY_STRING"]
    );
  }
  else if ($WBES_SERVER == "iis-cgi")
  {
    $server_win32 = true;
    $server_cheapredirects = true;
    $server_url = new URL
    (
      $_SERVER["HTTPS"] == "on",
      $_SERVER["SERVER_NAME"],
      $_SERVER["SERVER_PORT"],
      $_SERVER["SCRIPT_NAME"],
      substr($_SERVER["PATH_INFO"], strlen($_SERVER["SCRIPT_NAME"])),
      isset($_SERVER["QUERY_STRING"]) ? $_SERVER["QUERY_STRING"] : ""
    );
  }
  else if ($WBES_SERVER == "iis-isapi")
  {
    $server_isproduction = false;
    $server_win32 = true;
    $server_cheapredirects = false;
    $server_url = new URL
    (
      $_SERVER["HTTPS"] == "on",
      $_SERVER["SERVER_NAME"],
      $_SERVER["SERVER_PORT"],
      $_SERVER["SCRIPT_NAME"],
      substr($_SERVER["PATH_INFO"], strlen($_SERVER["SCRIPT_NAME"])),
      isset($_SERVER["QUERY_STRING"]) ? $_SERVER["QUERY_STRING"] : ""
    );
  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Type '$WBES_SERVER'");

  if (!$server_isproduction)
  {
    ini_set ('display_startup_errors', 1);
    ini_set ('display_errors', 1);
    error_reporting (E_ALL);
  }

if (!isset($PHP_SELF)) $PHP_SELF = $_SERVER['PHP_SELF'];

?>
