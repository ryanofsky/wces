<?

require_once("wbes/general.inc");
require_once("wbes/server.inc");
$page = $server_url->path;
param($frame); param($help);

?>
<html>
<head>
<title>Help</title>
<LINK REL="stylesheet" type="text/css" href="media/style.css">
</head>
<?
if($frame == "top")
{
?>
<body bgcolor="#68ACFF"><p align=center><strong>Course Evaluation Online Help</strong></p></body>
<?  
}
else if($frame == "bottom")
{
?>
<body bgcolor="#68ACFF">
<script language="JavaScript">
<!--

var happy = document.images && window.Image;

// fix for problematic Netscape 3 string object
if (happy && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

function swap(img)
{
  if(happy)
  {
    var i = document.images[img];
    var temp = i.src;
    i.src = i.flipimage.src;
    i.flipimage.src = temp;
  };
}

function AttachImage(img, filename)
{
  if(happy)
  {
    var i = document.images[img];
    var p = i.src.lastIndexOf("/");
    var q = i.src.lastIndexOf("\\");
    i.flipimage = new Image();  
    i.flipimage.src = i.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
  };
}
// -->
</script>
<p align=right><a target=main href="<?=$page?>?frame=main&help=contents"
onmouseover="swap('b1')" onmouseout="swap('b1')"><img name=b1
src="media/help/contents1.gif" alt="Contents" width=20 height=20
border=0></a><a target=main href="<?=$page?>?frame=main&help=feedback"
onmouseover="swap('b2')" onmouseout="swap('b2')"><img name=b2
src="media/help/reply1.gif" alt="Reply" width=20 height=20
border=0></a><a href="javascript:void(top.close())"
onmouseover="swap('b3')" onmouseout="swap('b3')"><img name=b3
src="media/help/close1.gif" alt="Close" width=20 height=20
border=0></a></p>
<script language="javascript">
<!--
  AttachImage("b1","contents.gif");
  AttachImage("b2","reply.gif");
  AttachImage("b3","close.gif");
// -->
</script>
</body>
<?  
}
else if ($frame == "main")
{
?><body bgcolor="#FFFFEF" class=help><?
  if ($help == "login")
  {
?>
<p>You need to have a University Network ID (UNI) and password use the course evaluation system.</p>
<p align=center><img src="media/help/login.gif"></p>
<p>Students and professors
can use the <a href="http://www.columbia.edu/acis/accounts/create/current.html" target="_blank">AcIS Account Maintenance</a>
page to create and activate their UNI's.</p>
<?
  }
  else if ($help == "contents")
  {
?>
<p><i><small>At the moment, somewhat sparse ...</small></i></p>
<p><strong>Table Of Contents</strong></p>
<ul>
  <li><a href="help.php?frame=main&help=login">Login Page</a></li>
  <li>Survey Builder</li>
  <ul>
  <li><a href="help.php?frame=main&help=inserttypes">Component Types</a></li>  
  <li><a href="help.php?frame=main&help=choice_mult">Choice Component: Question Quantity</a></li>  
  <li><a href="help.php?frame=main&help=choice_type">Choice Component: Choice Type</a></li>  
  <li><a href="help.php?frame=main&help=choice_more">Choice Component: More Choices</a></li>  
  <li><a href="help.php?frame=main&help=choice_other">Choice Component: Other Choices</a></li>  
  <li><a href="help.php?frame=main&help=choice_where">Choice Component: Orientation</a></li>  
  <li><a href="help.php?frame=main&help=choice_comp">Choice Component: Widgets</a></li>  
  <li><a href="help.php?frame=main&help=textsize">Response Component: Field Size</a></li>  
  </ul> 
  <li><a href="help.php?frame=main&help=feedback">Feedback</a></li>  
</ul>
<?
  }
  else if ($help == "choice_mult")
  {
?>  
    <p>Choose <b>Single Question</b> to add just one multiple choice question to your survey.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice01.gif" width=398 height=56 alt=example>
    
    <p>Choose <b>Multiple Questions</b> to add many questions to your survey at once. All of the
    questions will have the same formatting and the same set of choices.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice02.gif" width=327 height=209 alt=example>

    <h5>Example:</h5>
    <img src="media/help/choice03.gif" width=392 height=152 alt=example>
<?    
  }
  else if ($help == "choice_type")
  {
?>
    <p>Choose <b>Text Choices</b> to enter a list of answers for the person taking the survey to choose from. In this
    example, the choices are <i>Orange</i>, <i>Green</i>, <i>Yellow</i>, and <i>Brown</i>.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice11.gif" width=379 height=152 alt=example>
    
    <p><b>Quantitative Text Choices</b> look the same to the user as normal <b>Text Choices</b>. The difference
    with quantitative answers is that you can assign numeric values to the first and last choices.
    The numeric values can be used during the reporting process to calculate additional statistics like
    averages and standard deviations. Associating numeric values with your answers really only makes sense
    when your choices can be treated as scalar values.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice03.gif" width=392 height=152 alt=example>

    <p><b>Numeric Choices</b> can be used interchangeably with <b>Quantitative Text Choices</b>.
    The difference is that with numeric choices, the user will choose a from a range of
    integers rather than a set of words or phrases.
    
    <h5>Example:</h5>
    <img src="media/help/choice05.gif" width=275 height=171 alt=example>
<?
  }
  else if ($help == "choice_more")
  {
?>
    <p>Choose the "<b>Allow users to select more than one choice</b>" option when you want the
    user to be able to select more than one of your choices.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice06.gif" width=378 height=133 alt=example>
<?
  }  
  else if ($help == "choice_other")
  {
?>
    <p>Include an "Other" option when you want the person taking the survey to be able to enter
    their own choice.</p>
    
    <h5>Example:</h5>
    <img src="media/help/choice07.gif" width=343 height=159 alt=example>
<?
  }
  else if ($help == "choice_where")
  {
?>
    <h5>Put choices underneath question text</h5>
    <img src="media/help/choice02.gif" width=327 height=209 alt=example>
    
    <h5>Put choices to the right of question text</h5>
    <img src="media/help/choice03.gif" width=392 height=152 alt=example>
<?
  }
  else if ($help == "choice_comp")
  {
?>
    <h5>Checkboxes oriented horizontally</h5>
    <img src="media/help/choice04.gif" width=240 height=56 alt=example>
    
    <h5>Checkboxes oriented vertically</h5>
    <img src="media/help/choice10.gif" width=398 height=113 alt=example>
    
    <h5>List Boxes</h5>
    <img src="media/help/choice09.gif" width=399 height=107 alt=example>

    <h5>Drop Down Boxes</h5>
    <img src="media/help/choice08.gif" width=399 height=58 alt=example>
   
<?
  }
  else if ($help == "textsize")
  {
?>
    <p>Use the <b>rows</b> and <b>columns</b> field to specify how large you want
    the text response field (where the user types their answer to your question) to be.
    Using an appropriately sized field is a good way of suggesting to the user how much
    feedback you expect to get. The user will not be limited by what you specifiy here, however,
    because the text field can scroll.</p>
    
    <p>You should avoid specifying a field much larger than 60 columns wide and 15 rows tall. Otherwise
    the field may not fit in the user's browser window.</p>
    
    <p>If your text field has 1 or more rows, it will have a vertical scrollbar. You
    can specify a 0 for the number of rows to get a short response field that 
    does not have any scrollbars.</p>
    
    <h5>Example: 5x40 Text Field</h5>
    
    <img src="media/help/text_5x40.gif" width=365 height=122 alt=example>
    
    <h5>Example: 0x20 Short Response Field</h5>
    
    <img src="media/help/text_0x20.gif" width=166 height=59 alt=example>
<?
  }
  else if ($help == "inserttypes")
  {
?>

    <h5>Multiple Choice Question Set Examples</h5>
    <p><img src="media/help/choice01.gif" width=398 height=56 alt=example></p>
    <p><img src="media/help/choice03.gif" width=392 height=152 alt=example></p>
    <p><img src="media/help/choice06.gif" width=378 height=133 alt=example></p>

    <h5>Text Response Examples</h5>

    <p><img src="media/help/text_5x40.gif" width=365 height=122 alt=example></p>
    <p><img src="media/help/text_0x20.gif" width=166 height=59 alt=example></p>
    
    <h5>Heading Component Example</h5>
    
    <p><img src="media/help/heading.gif" width=394 height=46 alt=example></p>
    
    <h5>Text Component Example</h5>
    
    <p><img src="media/help/text.gif" width=369 height=88 alt=example></p>
  
<?
  }
  else if ($help == "feedback")
  {
    if (isset($email) || isset($comments))
    {
      sendfeedback($comments, $email, "Documentation Feedback");
      print("Your mail has been sent.");
    }
    else
    {
    ?>
    <form method=post>
    <p>Thank You for visiting the SEAS Oracle and WCES. Send us your feedback so we can make improvements to the site.</p>
    <p><strong>Email Address (Optional):</strong><br><input type=text name=email size=30></p>
    <p><strong>Comments:</strong><br><textarea name=comments rows=7 cols=30></textarea></p>
    <p><input type=submit name=submit value="Submit"></p>
    </form>
    <?
    }
  }
  else
  {
    print("Topic '$help' not found.");
  }
}
else
{
?>
<frameset rows="23,*,20" border=0>
  <frame name=top  marginheight=0  src="<?=$page?>?frame=top">
  <frame name=main src="<?=$page?>?frame=main&help=<?=$help?>">
  <frame name=bot  marginheight=0 src="<?=$page?>?frame=bottom">
</frameset>
<?
}
?>
</html>