<?
  $wbes_database = 'wces';
  $wces_path = '/wces/';
  $oracle_root = "/oracle/";

  require_once("wbes/general_url.inc");

  define("server_mode", "oracle"); // "oracle" "dartmouth" "russ"

  $server_session_name = "session";

  if (server_mode == "oracle")
  {
    $server_type = "apache";
    $server_isproduction = false;
    $server_media = "/wces/media";
    $server_base = "http://oracle.seas.columbia.edu";
    $server_sbase = "https://oracle.seas.columbia.edu";
    $server_tempdir = "";
    $server_massreply = "wces@columbia.edu";
    $server_feedback = "wces@columbia.edu";
    $server_admin = "wces@columbia.edu";
    $server_dev = "rey4@columbia.edu";
    $server_upload = "/home/httpd/wwwroot/wces/upload/";
    $server_www_upload = "/wces/upload/";

    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };

    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("dbname=$dbname user=postgres");
    }
  }
  else if (server_mode == "dartmouth")
  {
    $server_type = "apache";
    $server_isproduction = false;
    $server_media = "/media";
    $server_base = "https://eval.thayer.dartmouth.edu";
    $server_sbase = "https://eval.thayer.dartmouth.edu";
    $server_tempdir = "";
    $server_massreply = "registrar@thayer.dartmouth.edu";
    $server_feedback = "admin@thayer.dartmouth.edu";
    $server_admin = "websage@thayer.dartmouth.edu";
    $server_dev = "admin@thayer.dartmouth.edu";

    function server_mysqlinit()
    {
      return mysql_connect("localhost","root","------");
    };

    function server_pginit($dbname = "wbes")
    {
      return pg_Connect("dbname=$dbname user=postgres");
    }
  }
  else if (server_mode == "russ")
  {
    $server_type = "iis-cgi";
    $server_isproduction = false;
    $server_media = "/wces/media";
    $server_base = "http://russ.esurveys.hn.org";
    $server_sbase = "http://russ.esurveys.hn.org";
    $server_tempdir = "M:/server/shares/temp";
    $server_massreply = "rey4@columbia.edu";
    $server_feedback = "rey4@columbia.edu";
    $server_admin = "rey4@columbia.edu";
    $server_dev = "rey4@columbia.edu";
    $server_upload = "l:/server/shares/russ.esurveys.hn.org/www/wces/media/upload/";
    $server_www_upload = "/wces/media/upload/";

    function server_mysqlinit()
    {
      return mysql_connect("localhost","wces","--------");
    };

    function server_pginit($dbname = "wbes")
    {
      //return pg_Connect("host=oracle.seas.columbia.edu dbname=$dbname user=russ password=----------------");
      return pg_Connect("host=localhost dbname=$dbname user=postgres");
    }
  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Name");


  if ($server_type == "apache")
  {
    $server_win32 = false;
    $server_cheapredirects = false;
    $server_url = new URL
    (
      isset($_SERVER["HTTPS"]) && $_SERVER["HTTPS"] == "on",
      $_SERVER["SERVER_NAME"],
      $_SERVER["SERVER_PORT"],
      $_SERVER["SCRIPT_NAME"],
      isset($_SERVER["PATH_INFO"]) ? $_SERVER["PATH_INFO"] : "",
      $_SERVER["QUERY_STRING"]
    );
  }
  else if ($server_type == "iis-cgi")
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
  else if ($server_type == "iis-isapi")
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
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Type '$server_type'");

  if (!$server_isproduction)
  {
    ini_set ('display_startup_errors', 1);
    ini_set ('display_errors', 1);
    error_reporting (E_ALL);
  }

?>
