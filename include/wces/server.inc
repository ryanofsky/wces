<%
  define("server_mode","yahweh"); // "oracle" "yahweh" "yahwehisapi" "james" or "athlon"
  
  if (server_mode == "oracle")
  {
    $server_docroot = $HTTP_ENV_VARS["DOCUMENT_ROOT"] . "/";
    $server_cgiroot = dirname($HTTP_ENV_VARS["DOCUMENT_ROOT"]) . "/cgi-bin/";
    $server_wcespath = "/wces/";
    $server_win32 = false;
    
    function server_getrequest()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["REQUEST_URI"];
    };
    
    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    }
    
    function server_sqlinit()
    {
      return mysql_connect("localhost","root","------");
    };    
  }
  else if (server_mode == "yahweh")
  {
    $server_docroot = "J:/server/shares/oracle/";
    $server_cgiroot = $server_docroot . "cgi-bin" . "/";
    $server_wcespath = "/wces/";
    $server_win32 = true;
    $server_tempdir = "J:/server/shares/php_upload";
       
    function server_getrequest()
    {
      global $HTTP_ENV_VARS, $PHP_SELF;
      $qs = $HTTP_ENV_VARS["QUERY_STRING"];
      return $qs ? $PHP_SELF . "?" . $qs : $PHP_SELF;
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
  else if (server_mode == "yahwehisapi")
  {
    $server_docroot = $HTTP_SERVER_VARS["APPL_PHYSICAL_PATH"];
    $server_cgiroot = $server_docroot . "cgi-bin" . "/";
    $server_wcespath = "/wces/";
    $server_win32 = true;
    $server_tempdir = "J:/server/shares/php_upload";
    
    function server_getrequest()
    {
      global $HTTP_SERVER_VARS,$PHP_SELF;
      $qs = $HTTP_SERVER_VARS["QUERY_STRING"];
      return $qs ? $PHP_SELF . "?" . $qs : $PHP_SELF;
    };
    
    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    }    
    
    function server_sqlinit()
    {
      return mysql_connect("oracle.seas.columbia.edu","russ","insecure");
    };    
  }
  else if (server_mode == "james")
  {
    $server_docroot = "D:/wces/wwwroot/";
    $server_cgiroot = $server_docroot . "cgi-bin" . "/";
    $server_wcespath = "/wces/";
    $server_win32 = true;
    $server_tempdir = "D:/wces/temp";

    function server_getrequest()
    {
      global $HTTP_ENV_VARS,$PHP_SELF;
      $qs = $HTTP_ENV_VARS["QUERY_STRING"];
      return $qs ? $PHP_SELF . "?" . $qs : $PHP_SELF;
    };

    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    };
    
    function server_sqlinit()
    {
      return mysql_connect("oracle.seas.columbia.edu","russ","insecure");
    };    
  }  
  else if (server_mode == "athlon")
  {
    $server_docroot = "I:/server/shares/oracle/";
    $server_cgiroot = $server_docroot . "cgi-bin" . "/";
    $server_wcespath = "/wces/";
    $server_win32 = true;
    $server_tempdir = "I:/server/shares/temp";

    function server_getrequest()
    {
      global $HTTP_ENV_VARS,$PHP_SELF;
      $qs = $HTTP_ENV_VARS["QUERY_STRING"];
      return $qs ? $PHP_SELF . "?" . $qs : $PHP_SELF;
    };

    function server_secure()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    };
    
    function server_sqlinit()
    {
      return mysql_connect("localuser","root","------");
    };    
  }
  else
    die("invalid server type");
%>