<?

require_once("wbes/server.inc");
require_once("wces/database.inc");
require_once("team/page.inc");

  $server_isproduction = false;

    $fp = fopen("teams.csv","r");
    $team = team_connect();
  
    $i = 0;
    $row = 0;

    $teamcache = array();
  
    while ($data = fgetcsv ($fp, 8192, ","))
    {
      if (count($data) != 3)
      {
        print("skip row");
        continue;
      }  
      
      $teamname = "$data[0] $data[1]";
      $uni = $data[2];
      
      if (isset($teamcache[$teamname]))
        $teamno = $teamcache[$teamname];
      else
      {
        $r = db_exec("INSERT INTO teams(name) VALUES('$teamname')", $team, __FILE__, __LINE__);
        $teamno = mysql_insert_id($team);
        $teamcache[$teamname] = $teamno;
      }
      
      db_exec("INSERT INTO users(cunix) VALUES ('$uni')", $team, __FILE__, __LINE__);
      $userid = mysql_insert_id($team);
      
      db_exec("INSERT INTO memberships(team_id, user_id) VALUES ($teamno, $userid)", $team, __FILE__, __LINE__);
      
      print("inserted $teamname | $teamno | $uni | $userid<br>\n");
    }
?>