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
  var eventObject = document.forms[form][prefix + WIDGET_SEPARATOR + "event"];
  //alert(
  //  "eventObject = '" + eventObject + "'\n"
  //  + "form = '" + form + "'\n"
  //  + "prefix = '" + prefix + "'\n"
  //  + "name = '" + prefix + WIDGET_SEPARATOR + "event" + "'\n"
  //);
  var marker = elem(prefix + WIDGET_SEPARATOR + "insertMarker");
 
  for(var i=0;;++i)
  {
    var insert = elem(prefix + WIDGET_SEPARATOR + "insert" + i);
    insert.onclick = SurveyEditor_insert_onclick;
    insert.onmouseover = SurveyEditor_insert_onmouseover;
    insert.onmouseout = SurveyEditor_insert_onmouseout;
    insert.key = i;
    insert.eventObject = eventObject;
    insert.marker = marker;
    
    if (i >= nodes) break;

    var node = elem(prefix + WIDGET_SEPARATOR + "node" + i);

    if (node == null) continue;
    
    node.onmouseover = SurveyEditor_node_onmouseover;
    node.onmouseout = SurveyEditor_node_onmouseout;
    node.key = i;
    node.eventObject = eventObject;
  }  
}

function SurveyEditor_node_onmouseover()
{
  this.hover = true;
  window.SurveyEditor_menu.show(this,this.eventObject);
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
  EventWidget_go(SurveyEditor_inserte, this.key, this.eventObject);
}

function SurveyEditor_Menu() // extender
{
  var a = elem("SurveyEditor_cmdmenu");
  a.TDinit = SurveyEditor_Menu_TDinit;
  a.TDinit(SurveyEditor_modify, "SurveyEditor_editcmd");
  a.TDinit(SurveyEditor_delete, "SurveyEditor_deletecmd");
  a.TDinit(SurveyEditor_moveup, "SurveyEditor_moveupcmd");
  a.TDinit(SurveyEditor_movedown, "SurveyEditor_movedowncmd");

  a.onmouseout = SurveyEditor_Menu_onmouseout;
  a.onmouseover = SurveyEditor_Menu_onmouseover;
  a.timeout = SurveyEditor_Menu_timeout;
  a.show = SurveyEditor_Menu_show;
  a.hide = SurveyEditor_Menu_hide;
  a.hover = a.activenode = a.eventObject = false; //state variables
  return a;    
}

function SurveyEditor_Menu_TDinit(event, tdname)
{
  var t = elem(tdname);
  t.table = this;
  t.onmouseover = SurveyEditor_Menu_TD_onmouseover;
  t.onmouseout = SurveyEditor_Menu_TD_onmouseout;
  t.onclick = SurveyEditor_Menu_TD_onclick;
  t.event = event;
}

function SurveyEditor_Menu_TD_onclick()
{
  EventWidget_go(this.event, this.table.activenode.key, this.table.eventObject);
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

function SurveyEditor_Menu_show(activenode,eventObject)
{
  this.activenode = activenode;
  this.eventObject = eventObject;

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

function SurveyEditor_preview(form,eventElement,state)
{
  var w = window.open("about:blank", "previewwin", "width=500,height=410,status=0,toolbar=0,menubar=0,scrollbars=1,screenX=5,screeny=5,left=0,top=0,resizable=1",true);
  form.target = "previewwin";
  EventWidget_go(state,null,form[eventElement]);
  form.target = "_self";
  form[eventElement].value = "";
  w.focus();
}
  