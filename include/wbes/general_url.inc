<?

class URL
{
  /**
  * True if this is an https connection, false otherwise.
  * @var boolean
  * @access public
  */
  var $secure;

  /**
  * Name of the server
  * @var string
  * @access public
  */
  var $server;

  /**
  * Port number.
  * @var int
  * @access public
  */
  var $path;

  /**
  * Path to a location on the server. Begins with /.
  * @var string
  * @access public
  */
  var $path;

  /**
  * Path appended to an actual location. Scripts can have paths appended to
  * their location that they use like query strings. For example in
  * /cgi-bin/myprog.exe/arg1/arg2, the xpath would be /arg1/arg2.
  * @var string
  * @access public
  */
  var $xpath;
  
  /**
  * Query string. (Part after ?)
  * @var string
  * @access public
  */
  var $query;
  
  function URL($secure = false, $server = "", $port = 80, $path = "", $xpath = "", $query = "")
  {
    $this->secure = $secure;
    $this->server = $server;
    $this->port = (int)$port;
    $this->path = $path;
    $this->xpath = $xpath;
    $this->query = $query;
  }
  
  function toString($includeserver = false, $includexpath = false, $includequery = false)
  {
    $url = "";
    if ($includeserver)
    {
      if($this->secure)
      {
        $url = "https://";
        $dport = 443;
      }
      else
      {
        $url = "http://";
        $dport = 80;
      }
      $url .= $this->server;
      if($dport != $this->port) $url .= ":" . $this->port;
    }  
    $url .= $this->path;
    if ($includexpath) $url .= $this->xpath;
    if ($includequery && $this->query !== "") $url .= "?$this->query";
    return $url;
  }
}

?>