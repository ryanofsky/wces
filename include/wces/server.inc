<%
  define("server_mode","oracle"); // "oracle" "yahweh"
  
  if (server_mode == "oracle")
  {
    $server_docroot = $HTTP_ENV_VARS["DOCUMENT_ROOT"] . "/";
    $server_cgiroot = dirname($HTTP_ENV_VARS["DOCUMENT_ROOT"]) . "/cgi-bin/";
    $server_wcespath = "/wces/";
    $server_win32 = false;
    $server_base = "http://oracle.seas.columbia.edu";
    $server_sbase = "https://oracle.seas.columbia.edu";
    $server_tempdir = "";
    $server_isproduction = true;
    $oracleroot = "/oracle/";
        
    function server_getrequest($absolute = false)
    {
      global $HTTP_ENV_VARS;
      return $absolute ? ($HTTP_ENV_VARS["HTTPS"] == "on" ? "https://" : "http://") . $HTTP_ENV_VARS["SERVER_NAME"] . ":" . $HTTP_ENV_VARS["SERVER_PORT"] . $HTTP_ENV_VARS["REQUEST_URI"] : $HTTP_ENV_VARS["REQUEST_URI"];
    };
    
    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    }
    
    function server_sqlinit()
    {
      return mysql_connect("localhost","root","------");
      //return mysql_connect("160.39.128.25","russ","insecure");
    };    
  }
  else if (server_mode == "yahweh")
  {
    $server_docroot = "L:/server/shares/oracle";
    $server_cgiroot = $server_docroot . "cgi-bin" . "/";
    $server_wcespath = "/wces/";
    $server_win32 = true;

    $server_isproduction = false;

    $server_base = "http://oracle.yahweh.dyndns.org";
    $server_sbase = "https://oracle.yahweh.dyndns.org:999";
    $server_tempdir = "M:/server/shares/temp";
    $oracleroot = "/oracle/";
       
    function server_getrequest($absolute = false)
    {
      global $HTTP_ENV_VARS, $PHP_SELF;
      $req = isset($HTTP_ENV_VARS["QUERY_STRING"]) ? "$PHP_SELF?" . $HTTP_ENV_VARS["QUERY_STRING"] : $PHP_SELF;
      return $absolute ? ($HTTP_ENV_VARS["HTTPS"] == "on" ? "https://" : "http://") . $HTTP_ENV_VARS["SERVER_NAME"] . ":" . $HTTP_ENV_VARS["SERVER_PORT"] . $req : $req;
    };
    
    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    }
    
    function server_sqlinit()
    {
      return mysql_connect("localhost","root","------");
      //return mysql_connect("oracle.seas.columbia.edu","russ","insecure");
    };    

  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Type");
%>