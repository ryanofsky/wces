<?
  require_once("wces/page.inc");
  require_once("widgets/widgets.inc");
  require_once("widgets/dynamiclist.inc");
  require_once("widgets/basic.inc");
  
  page_top("hello?");
 
  $form = new Form("form", "f", WIDGET_GET);
  $form->loadvalues();
  
  $mc1 = new DynamicList(30,6,false,"mc1","f",WIDGET_GET);
  $mc2 = new DynamicList(30,8,array("the", "winged", "merchant", "flies", "with", "eyes", "of", "fire"),"mc2","f",WIDGET_GET);
  if ($form->isstale)
  {
    $mc1->loadvalues();
    $mc2->loadvalues();
  }
  else
  { 
    $mc1->items = array("check","the","cards","at","the","table");
    $mc1->keys = array(9,8,7,6,5,4);
  }  
?>
<form name="f" method=get>
<? $form->display(); ?>
<? $mc1->display(); ?>
<hr>
<? $mc2->display(); ?>
<input type=submit name=submit value="Submit"><br><br>
</form>
<?
  page_bottom();
?>