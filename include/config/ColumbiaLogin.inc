<?

class ColumbiaLogin extends WcesLogin
{
  function validate($uni, $password, &$error)
  {
    global $wces;
   
    $uni = str_replace('@columbia.edu', '', $uni);
    $uni = str_replace('@barnard.edu', '', $uni);
    $uni = trim($uni);
    $dump = acisget($uni,$password);
    
    if (!$dump)
    {
      sendwarning("Login warning: Could not contact AcIS server to authenticate '$uni'", __FILE__, __LINE__);
      $error = "Error: Failed to contact AcIS authentication server. Please try again.";
      return false;
    }
       
    if (strpos($dump,"no soup for you") !== false)
    {
      $error = "Invalid username or password.";    
      return false; // BAD PASSWORD
    }
    
    if(preg_match('/^REMOTE_USER=(.*)$/m', $dump, $matches))
      $cunix = $matches[1];
    else
    {
      $cunix = $uni;
      sendwarning("Login parser warning: username '$uni' could not be verified.\n\n----------\n\n$dump", __FILE__, __LINE__);
    }  
    
    $email = preg_match('/^USER_EMAIL=(.*)$/m', $dump, $matches) ? $matches[1] : "";
    $affils = preg_match('/^USER_AFFILIATIONS=(.*)$/m', $dump, $matches) ? $matches[1] : "";
    
    $ds=ldap_connect("ldap.columbia.edu");
    $r=ldap_bind($ds);
    $sr=ldap_search($ds,"o=Columbia University,c=US", "uni=$cunix");  
    $info = ldap_get_entries($ds, $sr);
    $ldap = array("givenname" => "", "sn" => "", "mail" => false);
    if ($info["count"] == 1)
    {
      $result = $info[0];
      foreach($ldap as $key => $value)
      {
        if ((int)$result[$key]["count"] > 0)
          $ldap[$key] = $result[$key][0];
      }
    }  
  
    $p = array('cunix' => $cunix, 'email' => $email ? $email : $ldap['mail'], 'last' => $ldap['sn'], 'first' => $ldap['givenname'], 'affils' => $affils);
    aarray_map('quot',$p);
    
    wces_connect();
    $result = pg_go("SELECT login_parse($p[cunix], $p[email], $p[last], $p[first], $p[affils])", $wces, __FILE__, __LINE__);
    $user_id = (int)pg_result($result,0,0);
    $this->update($user_id);
  
    if ($this->get('status') == 0)
      sendwarning("Login parser warning for uni '$uni': User authenticates, but has no access to the site.\n\n----------\n\n$dump", __FILE__, __LINE__);
   
    return true;
  }
  
  function refresh()
  {
    return $this->update($this->get('user_id'), $this->get('fake_id'), $this->get('fake_uni')); 
  }

  function update($user_id, $fake_id = NULL, $fake_uni = NULL)
  {
    global $wces;
    wces_connect();
    
    $result = pg_go("
      SELECT u.uni, btrim(u.firstname || ' ' || u.lastname) AS name,
        u.flags, COALESCE(u.department_id,0) AS dept, 
        CASE WHEN EXISTS 
          (SELECT * FROM enrollments_p AS e WHERE e.user_id = u.user_id)
        THEN 1 ELSE 0 END AS known
      FROM users AS u
      WHERE u.user_id = $user_id
    ", $wces, __FILE__, __LINE__);  
  
    if (pg_numrows($result) == 1)
      $row = pg_fetch_array($result,0,PGSQL_ASSOC);
    else
      $row = array("uni" => "", "name" => "__", "flags" => 0, "dept" => 0, "known" => false);
    
    $status = 0;
    if ($row['flags'] & 8)
      $status |= LOGIN_STUDENT;
  
    if ($row['flags'] & 4)
      $status |= LOGIN_PROFESSOR;
  
    if ($row['flags'] & 2)
      $status |= LOGIN_DEPT_ADMIN;
    
    if ($row['known'])
      $status |= LOGIN_KNOWN_PROFESSOR;
      
    if ($row['flags'] & 1)
      $status |= LOGIN_ADMIN;
      
    $p['user_id'] = $user_id;
    
    $this->setup($user_id, $row['uni'], $status, $row['name'],
      (int)$row['dept'], $fake_id, $fake_uni);
      
  }
  
  function setup($user_id, $uni, $status, $name, $dept_id, $fake_id, $fake_uni)
  {
    session_initialize();
    $n = $GLOBALS['WBES_LOGIN_PREFIX'];
    $_SESSION["{$n}user_id"] = (int)$user_id;
    $_SESSION["{$n}uni"] = $uni;
    $_SESSION["{$n}status"] = (int)$status;
    $_SESSION["{$n}name"] = $name;
    $_SESSION["{$n}depts"] = $dept_id ? array($dept_id) : false;
    $_SESSION["{$n}fake_id"] = $fake_id;
    $_SESSION["{$n}fake_uni"] = $fake_uni;
  }
}

function acisget($uni,$pw)
{
  global $server_win32, $server_tempdir;
  
  $fname = tempnam($server_tempdir, "booty");

  $ch = curl_init("https://www1.columbia.edu/~rey4/info.html");
  $fp = fopen($fname, "w");
  curl_setopt($ch, CURLOPT_FILE, $fp);
  curl_setopt($ch, CURLOPT_USERPWD, "$uni:$pw");
  curl_setopt($ch, CURLOPT_HEADER, 1);
  curl_setopt($ch, CURLOPT_TIMEOUT, 5);
  curl_setopt($ch, CURLOPT_TIMEOUT, 5);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);  
  $r = curl_exec($ch);
  curl_close($ch);
  fclose($fp);

  $fp = fopen($fname,"r");
  $result = fread($fp, filesize($fname));
  fclose($fp);
  unlink($fname);
  return $result;
};

?>
