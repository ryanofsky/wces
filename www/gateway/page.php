<?
require_once("config/ColumbiaPage.inc");

class GatewayPage extends ColumbiaPage
{
  var $loggingIn = false;
  
  function printLinks()
  {
    global $gateway_path, $server_url, $ASID, $QSID, $wces_this, $wces_ns4;

    $login =& LoginInstance();
    $uni = $login->get('uni');
    $user_id = $login->get('user_id');
    $status = $login->get('status');

    if ($user_id)
    {
      $this->printLink("Log off $uni", "logout.php$QSID", true, "$gateway_path/");
      if ($fake = $login->get('fake'))
        $this->printLink("Log on as $fake", "administrators/info.php?nofake=1$ASID", true, "$gateway_path/");
    }
    else
      $this->printLink("Log In", "index.php$QSID", !$this->loggingIn, "$gateway_path/");

    if ($user_id)
      $this->printLink("Home", "index.php$QSID", "$gateway_path/index.php" == $wces_this || "$gateway_path/" == $wces_this ? false : true, "$gateway_path/");
  
    if ($status & LOGIN_ADMIN)
    {
      $this->printSpacer();
      $this->printLink("Edit Surveys", "questionedit.php",  $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID, "$gateway_path/");
      $this->printLink("Dates", "dates.php",  "", "$gateway_path/");
      $this->printLink("Results", "results.php", "", "$gateway_path/");
    }         
  }  
}

?>