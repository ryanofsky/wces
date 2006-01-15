<?

require_once("config/ColumbiaLogin.inc");

define("NUM_STUDENTS", 100);
define("NUM_PROFS", 20);

define("DEMO_ADMIN", 1);
define("DEMO_PROF", 2);
define("DEMO_STUDENT", 3);

class DemoLoginWidget extends LoginWidget
{
  function DemoLoginWidget($name, &$parent)
  {
    $this->LoginWidget($name, $parent);
    $this->demo =& new EventWidget("demo", $this);
  }

  function printGreeting()
  {
?>
  <div style="background-color: #b2cfe2; padding: 10px; margin: 20px; border: thick solid #1943be;">
  <h3>Welcome to the WCES Demo!</h3>
  <p>You can log yourself in automatically by pressing the buttons below:</p>
  <div align="center">
  <? $this->demo->displayButton("Log in as an administrator", DEMO_ADMIN); ?>
  <? $this->demo->displayButton("Log in as a random professor", DEMO_PROF); ?>
  <? $this->demo->displayButton("Log in as a random student", DEMO_STUDENT); ?>
  </div>
  <p>Or, you can use the form below to log in with any of the following usernames and a blank password:</p>
  <ul>
    <li><strong>admin</strong></li>
    <li><strong>professor1</strong> to <strong>professor<?=NUM_PROFS?></strong></li>
    <li><strong>student1</strong> to <strong>student<?=NUM_STUDENTS?></li>
  </ul>
  </div>
<?
    LoginWidget::printGreeting();
  }

  function validate()
  {
    $demo_name = null;
    if ($this->demo->event == DEMO_ADMIN)
      $demo_name = "admin";
    else if ($this->demo->event == DEMO_PROF)
      $demo_name = "prof" . rand(1, NUM_PROFS);
    else if ($this->demo->event == DEMO_STUDENT)
      $demo_name = "student" . rand(1, NUM_STUDENTS);

    if ($demo_name)
    {
      $login =& LoginInstance();
      return $login->validate($demo_name, "", $this->error);
    }

    return LoginWidget::validate();
  }
};

class DemoLogin extends ColumbiaLogin
{
  function validate($uni, $password, &$error)
  {
    global $wces;
    wces_connect();
    $uni = quot($uni);
    $result = pg_go("SELECT user_id FROM users WHERE uni = $uni", $wces, __FILE__, __LINE__);
    if (pg_numrows($result))
    {
      $user_id = (int)pg_result($result,0,0);
      pg_go("UPDATE users SET lastlogin = CURRENT_TIMESTAMP WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
      $this->update($user_id);
      return true;
    }
    else
    {
      $error = "Invalid username.";
      return false;
    }
  }

  function & getWidget($name, &$parent)
  {
    return new DemoLoginWidget($name, $parent);
  }
};

?>
