function elem(str)
{
  if (document.all)
  {
    return document.all(str);
  }
  else if (document.getElementById)
  {
    return document.getElementById(str);
  }
}

function getoffset(element)
{
  var offset = new Object();
  if((element.x || element.x == 0) && (element.y || element.y == 0))
  {
    offset.x = element.x;
    offset.y = element.y;
    return offset;
  }
  else
  {
    offset.x = 0;
    offset.y = 0;
    for(;;element = element.offsetParent)
    {
      offset.x += element.offsetLeft;
      offset.y += element.offsetTop;
      if (!element.offsetParent) return offset;
    }
  }
}

function SurveyEditor(nodes,prefix,form)
{
  var actionobject = document.forms[form][prefix + "_main_action"];
  var marker = elem(prefix + "_insertmarker");
 
  for(var i=0;;++i)
  {
    var insert = elem(prefix + "_insert" + i);
    insert.onclick = SurveyEditor_insert_onclick;
    insert.onmouseover = SurveyEditor_insert_onmouseover;
    insert.onmouseout = SurveyEditor_insert_onmouseout;
    insert.key = i;
    insert.actionobject = actionobject;
    insert.marker = marker;
    
    if (i >= nodes) break;

    var node = elem(prefix + "_node" + i);

    if (node == null) continue;
    
    node.onmouseover = SurveyEditor_node_onmouseover;
    node.onmouseout = SurveyEditor_node_onmouseout;
    node.key = i;
    node.actionobject = actionobject;
  }  
}

function SurveyEditor_node_onmouseover()
{
  this.hover = true;
  window.SurveyEditor_menu.show(this,this.actionobject);
}

function SurveyEditor_node_onmouseout()
{
  this.hover = false;
  window.SurveyEditor_menu.timeout();
}

function SurveyEditor_insert_onmouseover()
{
  this.className = "insertmenuhover";
  var offset = getoffset(this);

  if (this.clientWidth) // Internet Explorer
    w = offset.x + this.clientWidth + 4;
  else // DOM browser workaround
    w = getoffset(this.marker).x;

  var s = elem("SurveyEditor_insert").style;
  s.left = w + "px";
  s.top = (offset.y + -3) + "px";
  s.visibility = "visible";
}

function SurveyEditor_insert_onmouseout()
{
  this.className = "insertmenu";
  var s = elem("SurveyEditor_insert").style;
  s.visibility = "hidden";
}  

function SurveyEditor_insert_onclick()
{
  ActionButton_go(SurveyEditor_main_insert,this.key,this.actionobject);
}

function SurveyEditor_Menu() // extender
{
  var a = elem("SurveyEditor_cmdmenu");
  a.TDinit = SurveyEditor_Menu_TDinit;
  a.TDinit(SurveyEditor_main_modify, "SurveyEditor_editcmd");
  a.TDinit(SurveyEditor_main_delete, "SurveyEditor_deletecmd");
  a.TDinit(SurveyEditor_main_moveup, "SurveyEditor_moveupcmd");
  a.TDinit(SurveyEditor_main_movedown, "SurveyEditor_movedowncmd");

  a.onmouseout = SurveyEditor_Menu_onmouseout;
  a.onmouseover = SurveyEditor_Menu_onmouseover;
  a.timeout = SurveyEditor_Menu_timeout;
  a.show = SurveyEditor_Menu_show;
  a.hide = SurveyEditor_Menu_hide;
  a.hover = a.activenode = a.actionobject = false; //state variables
  return a;    
}

function SurveyEditor_Menu_TDinit(action, tdname)
{
  var t = elem(tdname);
  t.table = this;
  t.onmouseover = SurveyEditor_Menu_TD_onmouseover;
  t.onmouseout = SurveyEditor_Menu_TD_onmouseout;
  t.onclick = SurveyEditor_Menu_TD_onclick;
  t.action = action;
}

function SurveyEditor_Menu_TD_onclick()
{
  ActionButton_go(this.action, this.table.activenode.key, this.table.actionobject);
}

function SurveyEditor_Menu_TD_onmouseover()
{
  this.className = "editmenuhover";
}

function SurveyEditor_Menu_TD_onmouseout()
{
  this.className = "editmenu";
}

function SurveyEditor_Menu_onmouseover()
{
  this.hover = true;
}

function SurveyEditor_Menu_onmouseout()
{
  this.hover = false;
  this.timeout();
}

function SurveyEditor_Menu_timeout()
{
  window.setTimeout("window.SurveyEditor_menu.hide()",100);
}

function SurveyEditor_Menu_show(activenode,actionobject)
{
  this.activenode = activenode;
  this.actionobject = actionobject;

  var offset = getoffset(activenode);
  this.style.left = (offset.x + 9) + "px";
  this.style.top  = (offset.y + 9) + "px";
  this.style.visibility = "visible";
}  

function SurveyEditor_Menu_hide()
{
  if (!this.hover && !this.activenode.hover)
    this.style.visibility = "hidden";
}  

function SurveyEditor_preview(form,actionelement,state)
{
  var w = window.open("about:blank", "previewwin", "width=500,height=410,status=0,toolbar=0,menubar=0,scrollbars=1,screenX=5,screeny=5,left=0,top=0,resizable=1",true);
  form.target = "previewwin";
  ActionButton_go(state,0,form[actionelement]);
  form.target = "_self";
  form[actionelement].value = "";
  w.focus();
}
  