<?
  define("server_mode","oraclemod"); // "oracle" "yahweh" "oraclemod" "yahwehisapi"
  
  if (server_mode == "oraclemod")
  {
    $server_wcespath = "/wces/";
    $server_win32 = false;
    $server_base = "http://oracle.seas.columbia.edu";
    $server_sbase = "https://oracle.seas.columbia.edu";
    $server_tempdir = "";
    $server_isproduction = true;
    $oracleroot = "/oracle/";
        
    function server_getpage()
    {
      global $HTTP_SERVER_VARS;
      return $HTTP_SERVER_VARS["REQUEST_URI"];
    }
    
    function server_getrequest($absolute = false)
    {
      global $HTTP_SERVER_VARS;
      if ($absolute)
        return MakeAbsoluteURL
        (
          $HTTP_SERVER_VARS["HTTPS"] == "on",
          $HTTP_SERVER_VARS["SERVER_NAME"],
          $HTTP_SERVER_VARS["SERVER_PORT"],
          $HTTP_SERVER_VARS["REQUEST_URI"]
        );
      else
        return $HTTP_SERVER_VARS["REQUEST_URI"];
    };
    
    function server_secure()
    {
      global $HTTP_SERVER_VARS;
      return $HTTP_SERVER_VARS["HTTPS"] == 'on';
    }
    
    function server_sqlinit()
    {
      return mysql_connect("localhost","root","------");
    };    
  }
  else if (server_mode == "oracle")
  {
    $server_wcespath = "/wces/";
    $server_win32 = false;
    $server_base = "http://oracle.seas.columbia.edu";
    $server_sbase = "https://oracle.seas.columbia.edu";
    $server_tempdir = "";
    $server_isproduction = true;
    $oracleroot = "/oracle/";
        
    function server_getpage()
    {
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["PATH_INFO"];
    }
    
    function server_getrequest($absolute = false)
    {
      global $HTTP_ENV_VARS;
      if ($absolute)
        return MakeAbsoluteURL
        (
          $HTTP_ENV_VARS["HTTPS"] == "on",
          $HTTP_ENV_VARS["SERVER_NAME"],
          $HTTP_ENV_VARS["SERVER_PORT"],
          $HTTP_ENV_VARS["REQUEST_URI"]
        );
      else
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
    $server_wcespath = "/wces/";
    $server_win32 = true;  
    $server_base = "http://oracle.yahweh.dyndns.org";
    $server_sbase = "https://oracle.yahweh.dyndns.org:999";
    $server_tempdir = "M:/server/shares/temp";
    $server_isproduction = false;
    $oracleroot = "/oracle/";
                           
    function server_getpage()
    {
      global $PHP_SELF;
      return $PHP_SELF;
    }

    function server_getrequest($absolute = false)
    {                      
      global $HTTP_ENV_VARS, $PHP_SELF;
      $req = isset($HTTP_ENV_VARS["QUERY_STRING"]) ? "$PHP_SELF?" . $HTTP_ENV_VARS["QUERY_STRING"] : $PHP_SELF;
      if ($absolute)
        return MakeAbsoluteURL
        (
          $HTTP_ENV_VARS["HTTPS"] == "on",
          $HTTP_ENV_VARS["SERVER_NAME"],
          $HTTP_ENV_VARS["SERVER_PORT"],
          $req
        );
      else
        return $req; 
    };                     
                           
    function server_secure()
    {                      
      global $HTTP_ENV_VARS;
      return $HTTP_ENV_VARS["HTTPS"] == 'on';
    }                      
                           
    function server_sqlinit()
    {
      //return mysql_connect("localhost","root","------");
      return mysql_connect("oracle.seas.columbia.edu","russ","insecure");
    };    

  }
  else if (server_mode == "yahwehisapi")
  {
    $server_wcespath = "/wces/";
    $server_win32 = true;
    $server_base = "http://oracle.yahweh.dyndns.org";
    $server_sbase = "https://oracle.yahweh.dyndns.org:999";
    $server_tempdir = "M:/server/shares/temp";
    $server_isproduction = false;
    $oracleroot = "/oracle/";

    function server_getpage()
    {
      global $PHP_SELF;
      return $PHP_SELF;
    }

    function server_getrequest($absolute = false)
    {
      global $PHP_SELF, $HTTP_SERVER_VARS;
      $req = isset($HTTP_SERVER_VARS["QUERY_STRING"]) ? "$PHP_SELF?" . $HTTP_SERVER_VARS["QUERY_STRING"] : $PHP_SELF;
      if ($absolute)
        return MakeAbsoluteURL
        (
          $HTTP_SERVER_VARS["HTTPS"] == "on" ,
          $HTTP_SERVER_VARS["SERVER_NAME"],
          $HTTP_SERVER_VARS["SERVER_PORT"],
          $req
        );
      else
        return $req;     
    };

    function server_secure()
    {
      global $HTTP_SERVER_VARS;
      return $HTTP_SERVER_VARS["HTTPS"] == 'on';
    }

    function server_sqlinit()
    {
      return mysql_connect("localhost","root","------");
      //return mysql_connect("oracle.seas.columbia.edu","russ","insecure");
    };
  }
  else
    die("Fatal Error in <b>" . __FILE__ . ", line <b>" . __LINE__ . "</b>: Invalid Server Name");
?>