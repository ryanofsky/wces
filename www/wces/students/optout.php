<?
  require_once("wbes/general.inc");
  require_once("wces/page.inc");
  page_top("Email Settings");
  param('panic'); param('newemail');
?>

<h3>Limitations</h3>
<form>
<p>You can check off the options below to limit the amount of email that you recieve:</p>
<p><input type=checkbox name=noreminders id=noreminders> <label for=noreminders>Do not send more than <input type=text name=reminders value=2 size=3> reminders per survey period.</label><br>
<input type=checkbox name=nothanks id=nothanks> <label for=nothanks>Do not send any thank you emails.</label></p>
<input type=submit name=limits value="Save Limitations">
</form>

<? if (!$panic) { ?>
<h3>Panic Button</h3>
<form><input type=submit name=panic value="Do not send me any more email until the next survey period."></form>
<? } else { ?>
<h3><i>Un-</i>Panic Button</h3>
<form><input type=submit name=unpanic value="Allow me to recieve more email this survey period."></form>
<? } ?>
<h3>Email Address</h3>
<p>If you would prefer to receive WCES email at another address, enter it below. We will send a test message to the new address to verify that you can recieve mail there.</p>
<form>
<p>New Email address: <input type=text name=email value="rey4@columbia.edu" size=20>
<input type=submit name=newemail value="Save Address"></p>
</form>
<? if ($newemail) print("<p><font color=red>Address Change Pending</font></p>"); ?>
<h3>Permanent Opt-Out</h3>
If you never want to recieve a WCES email again, send a message to <a href="mailto:<?=$server_feedback?>"><?=$server_feedback?></a>.


<? page_bottom(); ?>  