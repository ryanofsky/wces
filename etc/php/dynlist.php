<?
 require_once("wces/page.inc");
 require_once("widgets/widgets.inc");
 require_once("widgets/dynamiclist.inc");
 require_once("widgets/basic.inc");
 
 page_top("hello?");

 $form = new Form("form", "f", WIDGET_GET);
 $form->loadvalues();
 
 $mc1 = new DynamicList(30,7,false,"mc1",f,WIDGET_GET);
 if ($form->isstale) $mc1->loadvalues(); else $mc1->items = array("check","the","cards","at","the","table");
 
?>
<form name="f" method=get>
<? $form->display(); ?>
<? $mc1->display(); ?>
<input type=submit name=submit value="Submit"><br><br>
</form>
<?
  page_bottom();
?>