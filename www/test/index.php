<?
  require_once("wces/general.inc");
  require_once("test/test.inc");

  param($status);

  if ($status == "student")
  {
    test_top("Student Home",$status);
?>
<h3>Welcome Russell Yanofsky,</h3>
<p>Choose a class to evaluate from the list below.</p>
<ul>
  <li><a href="<?=$testroot?>/student/survey.php?classid=1">COMS3210 Discrete Math - Professor Jonathan Gross</a></li>
  <li><a href="<?=$testroot?>/student/survey.php?classid=2">COMS3156 Software Engineering - Professor Gail Kaiser</a></li>
</ul>
<p>Sick of automated emails? Adjust your <a href="<?=$testroot?>/student/optout.php">email settings</a>.</p>
<?
  }
  else if ($status == "professor")
  {
    test_top("Professor Home",$status);
?>
<h3>Welcome Russell Yanofsky,</h3>
<ul>
  <li><a href="<?=$testroot?>/professor/edit.php">Edit Upcoming Surveys</a></li>
  <li><a href="<?=$testroot?>/professor/seeresults.php">See survey results</a></li>
  <li><a href="<?=$testroot?>/professor/info.php">Edit profile</a></li>
</ul>
<?
  }
  else if ($status == "administrator")
  {
    test_top("Administrator Home",$status);
?>
<h3>Welcome Russell Yanofsky,</h3>
<ul>
  <li><a href="<?=$testroot?>/administrator/susage.php">Student Usage</a></li>
  <li><a href="<?=$testroot?>/administrator/pusage.php">Professor Usage</a></li>
  <li><a href="<?=$testroot?>/administrator/results.php">Survey Results</a></li>
  <li><a href="<?=$testroot?>/administrator/email.php">Email Services</a></li>
  <li><a href="<?=$testroot?>/administrator/dedit.php">Database Editor</a></li>
  <li><a href="<?=$testroot?>/administrator/fake.php">Fake Logon</a></li>
</ul>
<?
  }
  else
  {
    test_top("Login");
?>
    <p><strong>Enter your CUNIX username and password to log in:</strong> <? test_showhelp("login"); ?></p>
    <form method=get action="index.php">
    <input type=hidden name=status value=student>
      <table>
      <tr><td>Username:</td><td><input name="uni" type="text" value="" size="20"></td></tr>
      <tr><td>Password:</td><td><input name="password" type="password" value="" size="20"></td></tr>
      <tr><td>&nbsp;</td><td><input type="submit" value="Log In" name=submit></td></tr>
      </table>
    </form>
<?
  }
  
  
 test_bottom() ?>    

