<?
  require_once("wces/general.inc");
  require_once("test/test.inc");

  param($editclass); param($previewclass);
 
  test_top("Student Home","professor");
  param($classid); param($editclass); param($previewclass);
  $name = $classid == 1 ? "COMS3210 Discrete Math - Professor Jonathan Gross" : "COMS3156 Software Engineering - Professor Gail Kaiser";


function doquest($number)
{
  switch($number)
  {
    case 0:
      ?><h4>General Questions<?
    break;
    case 1:
?>
<p><b>Instructor: Organization and Preparation</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1a" value="a" ><label for="preview_Q1_MC1a">excellent</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1b" value="b" ><label for="preview_Q1_MC1b">very good</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1c" value="c" ><label for="preview_Q1_MC1c">satisfactory</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1d" value="d" ><label for="preview_Q1_MC1d">poor</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1e" value="e" ><label for="preview_Q1_MC1e">disastrous</label>
</blockquote>
<?
    break;
    case 2:
?>
<p><b>Instructor: Classroom Delivery</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2a" value="a" ><label for="preview_Q1_MC2a">excellent</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2b" value="b" ><label for="preview_Q1_MC2b">very good</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2c" value="c" ><label for="preview_Q1_MC2c">satisfactory</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2d" value="d" ><label for="preview_Q1_MC2d">poor</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2e" value="e" ><label for="preview_Q1_MC2e">disastrous</label>
</blockquote>
<?
    break;
    case 3:
?>
<p><b>Instructor: Approachability</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3a" value="a" ><label for="preview_Q1_MC3a">excellent</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3b" value="b" ><label for="preview_Q1_MC3b">very good</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3c" value="c" ><label for="preview_Q1_MC3c">satisfactory</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3d" value="d" ><label for="preview_Q1_MC3d">poor</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3e" value="e" ><label for="preview_Q1_MC3e">disastrous</label>
</blockquote>
<?
    break;
    case 4:
      ?><h4>ABET Questions<?
    break;
    case 5:
?>
<p><strong>To what degree did this course enhance your ability to ...</strong><br>
<font size="-1">(0 = not at all, 5 = a great deal)</font></p>
<table>
  <tr>
    <td>&nbsp;</td>
    <td align=center>5</td>
    <td align=center>4</td>
    <td align=center>3</td>
    <td align=center>2</td>
    <td align=center>1</td>
    <td align=center>0</td>
  </tr>
  <tr>
    <td>Design experiments</td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="5"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="4"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="3"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="2"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="1"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_0" value="0"></td>
  </tr>
  <tr>
    <td>Analyze and interpret data</td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="5"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="4"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="3"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="2"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="1"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_1" value="0"></td>
  </tr>
  <tr>
    <td>Conduct experiments</td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="5"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="4"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="3"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="2"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="1"></td>
    <td align=center><input type=radio name="prefix_main_qwidget_0_2" value="0"></td>
  </tr>
</table>
<?
    break;
    case 6:
      ?><h4>TA Ratings<?
    break;
    case 7:
?>
  <p>[ <i>embedded question set</i> ]</p>
<?
    break;
    case 8:
      ?><h4>Comments<?
    break;
    case 9:
?>
  <textarea rows=8 cols=60></textarea>
<?
  }
}

if ($editclass)
{
  $num = 10;
?>   

<script>
<!--
 
  function EditMenu_td_mouseover()
  {
    this.className = "editmenuhover";
  }

  function EditMenu_td_mouseout()
  {
    this.className = "editmenu";
  }
  
  function EditMenu_init(membername, tdname)
  {
    var t = this[membername] = document.all(tdname);
    t.onmouseover = EditMenu_td_mouseover;
    t.onmouseout = EditMenu_td_mouseout;
    t.onclick = new Function("document.forms['f'].submit()");
  }
  
  function getoffset(element,offset)
  {
    if (!offset)
    {
      offset = new Object();
      offset.x = 0;
      offset.y = 0;
    }
    if (element.offsetParent)
      offset = getoffset(element.offsetParent, offset);
    offset.x += element.offsetLeft;
    offset.y += element.offsetTop;
    return offset;
  }
  
  function EditMenu_node_mouseover()
  {
    this.hover = true;
    var offset = getoffset(this);
    this.menu.style.pixelLeft = offset.x + 9;
    this.menu.style.pixelTop = offset.y + 9;
    this.menu.style.visibility = "visible";
    this.menu.node = this;
  }
  
  function EditMenu_node_mouseout()
  {
    this.menu.timeout();
    this.hover = false;
  }
  
  function EditMenu_table_mouseover()
  {
    this.hover = true;
  }

  function EditMenu_table_mouseout()
  {
    this.timeout();
    this.hover = false;
  }

  function EditMenu_table_timeout()
  {
    if (!window.themenu)
    {
      window.themenu = this;
      window.setTimeout("window.themenu.disappear()",100);
    }  
  }
  
  function EditMenu_table_disappear()
  {
    if (!this.hover && !this.node.hover)
      this.style.visibility = "hidden";
    window.themenu = false;  
  }

  function EditMenu(menu,nodes,edit,deleteb,moveup,movedown)
  {
    this.init = EditMenu_init;
    this.init("edit",edit);
    this.init("deleteb",deleteb);
    this.init("moveup",moveup);
    this.init("movedown",movedown);
    
    this.menu = document.all(menu);
    this.menu.onmouseout = EditMenu_table_mouseout;
    this.menu.onmouseover = EditMenu_table_mouseover;
    this.menu.timeout = EditMenu_table_timeout;
    this.menu.disappear = EditMenu_table_disappear;
    
    
    this.nodes = nodes;
    this.hover = false;
    
    for(i in this.nodes)
    {
      var node = this.nodes[i];
      node.menu = this.menu;
      node.onmouseover = EditMenu_node_mouseover;
      node.onmouseout = EditMenu_node_mouseout;
    }
  }
// -->
</script>



<p>The following is a preview of the question set. Use the blue nodes to the right of each question to make changes.</p>
<form name=f>
<input type=hidden name=editclass value="<?=$editclass?>">
<table border=0 cellpadding=0 cellspacing=0>
<?  for($i=0;;++$i) { ?>
<tr><td colspan=4><img src="<?=$testroot?>/media/nada.gif" width=1 height=10 alt="[ spacer ]"></td></tr>
<? if ($i == $num) break; ?>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td rowspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=2><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr>
  <td width=40 valign=center><img src="<?=$testroot?>/media/editor/node.gif" width=40 height=18 border=0 align=absmiddle alt=node id=node<?=$i?>></td>
  <td width=1><img src="<?=$testroot?>/media/nada.gif" width=3 height=1 alt="[ spacer ]"></td>
  <td><? doquest($i); ?></td>
</tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td colspan=2><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<? } ?>
</table>
</form>
<p>&nbsp</p><p>&nbsp</p>

<table id=cmdmenu border=1 bordercolor=black cellspacing=0 cellpadding=3 style="position:absolute; visibility: visible">
<tr><td id=editcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle> Edit Question</td></tr>
<tr><td id=deletecmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle> Delete Question</td></tr>
<tr><td id=moveupcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle> Move Up</td></tr>
<tr><td id=movedowncmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle> Move Down</td></tr>
</table>
<script>
<!--
  if (document.all)
  {
    var nodes = new Array();
    for(var i=0; i < <?=$num?>; ++i)
      nodes[i] = document.all("node" + i);
    menu = new EditMenu("cmdmenu", nodes, "editcmd", "deletecmd", "moveupcmd", "movedowncmd");
  }
// -->
</script>

<p>&nbsp</p><p>&nbsp</p><p>&nbsp</p><p>&nbsp</p><p>&nbsp</p><p>&nbsp</p>  
  
  
  <table border=2 bordercolor=black bordercolorlight=black bordercolordark=black cellspacing=0>
  <tr>
    <td bgcolor="#DDDDDD">&nbsp;</td>
    <td align=center valign=middle><input type=submit name="prefix_action_2_0" value="Insert a component here" class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"></td>
  </tr>
  <tr>
    <td bgcolor="#DDDDDD">
      <table>
        <tr>
          <td><input type=submit name="prefix_action_3_0" value=" Modify " class="tinybutton" style="width:100%; BACKGROUND: #0030E7; COLOR: white;"></td>
          <td><input type=submit name="prefix_main_action_2_0" value=" Move Up " class="tinybutton" style="width:100%; BACKGROUND: #EFDF18; COLOR: black;"></td>
        </tr>
        <tr>
          <td><input type=submit name="prefix_main_action_1_0" value=" Delete " class="tinybutton" style="width:100%; BACKGROUND: red; COLOR: white;"></td>
          <td><input type=submit name="prefix_main_action_3_0" value=" Move Down " class="tinybutton" style="width:100%; BACKGROUND: #EFDF18; COLOR: black;"></td>
        </tr>
      </table>
    </td>
</tr></table>      
<?
}
else if ($previewclass)
{
  print("PREVIEW CLASS");
}
else
{
?>

<p>Choose a class from the list below to edit or preview its survey questions.</p>
<ul>
<li>Fall 2001 - ELEN4411 FUNDAMENTALS OF PHOTONICS (section 001)  [ <a href="edit.php?editclass=1">Edit</a> | <a href="edit.php?previewclass=1">Preview</a> ]</li>
<li>Fall 2001 - COMS4203 GRAPH THEORY (section 001) [ <a href="edit.php?editclass=2">Edit</a> | <a href="edit.php?&previewclass=2">Preview</a> ]</li>
</ul>
<? 
};
test_bottom();
?>
