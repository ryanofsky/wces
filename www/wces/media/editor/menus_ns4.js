function SurveyEditor_Spot(x1,y1,x2,y2,obj)
{
  this.x1 = x1;
  this.y1 = y1;
  this.x2 = x2;
  this.y2 = y2;
  this.obj = obj;
}

function SurveyEditor_onload(e)
{
  if (window.SurveyEditor_onload_old) window.SurveyEditor_onload_old(e);
  
  for (p in window.SurveyEditor_loadpackage)
  {
    var o = window.SurveyEditor_loadpackage[p];    
    var r = document.images[o.prefix + WIDGET_SEPARATOR + "insertMarker"].x;
    var eventParam = document.forms[o.form][o.prefix + WIDGET_SEPARATOR + "event"];
       
    for(var i=0;;++i)
    {
      var insert = document.images[o.prefix + WIDGET_SEPARATOR + "ns4marker" + i];
      insert.ns4_onclick = SurveyEditor_insert_onclick;
      insert.ns4_onmouseover = SurveyEditor_insert_onmouseover;
      insert.ns4_onmouseout = SurveyEditor_insert_onmouseout;
      insert.key = i;
      insert.eventParam = eventParam;
      insert.right_edge = r;
      window.SurveyEditor_spots.push(new SurveyEditor_Spot(insert.x, insert.y, r, insert.y + 10, insert)); 
      
      if (i >= o.nodes) break;
      
      var node = document.images[o.prefix + WIDGET_SEPARATOR + "node" + i];
      
      if (!node) continue;
      
      node.ns4_onmouseover = SurveyEditor_node_onmouseover;
      node.ns4_onmouseout = SurveyEditor_node_onmouseout;
      node.key = i;
      node.eventParam = eventParam;
      window.SurveyEditor_spots.push(new SurveyEditor_Spot(node.x, node.y, node.x + node.width, node.y + node.height, node));
    }
  }
  
  window.SurveyEditor_loadpackage = false;
}

function SurveyEditor_mousemove(e)
{
  if (window.SurveyEditor_mousemove_old) window.SurveyEditor_mousemove_old(e);
  
  //window.status = "hello from " + e.pageX + "," + e.pageY + (window.hovermenu ? " ON" : " OFF") ;
 
  var x = e.pageX;
  var y = e.pageY;
  var oldhot = window.SurveyEditor_hotspot;

  var spot = false;
  window.SurveyEditor_hotspot = 0;

  if (!window.SurveyEditor_menu.hover)
  for(i in window.SurveyEditor_spots)
  {
    var s = window.SurveyEditor_spots[i];
    if (s.x1 <= x && x <= s.x2 && s.y1 <= y && y <= s.y2)
    {
      window.SurveyEditor_hotspot = i - -1; // netscape 4.08 thinks i is a string
      spot = s.obj;
      break;
    }
  }
  
  if(oldhot != window.SurveyEditor_hotspot)
  {
    var oldspot = oldhot ? window.SurveyEditor_spots[oldhot - 1].obj : false;
    
    if (oldspot && oldspot.ns4_onmouseout)
      oldspot.ns4_onmouseout(e);
    
    if (spot && spot.ns4_onmouseover)
      spot.ns4_onmouseover(e);
  }
}

function SurveyEditor_mouseclick(e)
{
  if(window.SurveyEditor_mouseclick_old) window.SurveyEditor_mouseclick_old(e);
  
  var spot = window.SurveyEditor_hotspot ? window.SurveyEditor_spots[window.SurveyEditor_hotspot - 1].obj : false;
  if (spot && spot.ns4_onclick) spot.ns4_onclick(e);
}

function ns_choose(event)
{
  var m = window.SurveyEditor_menu;
  EventWidget_go(event, m.activenode.key, m.eventParam);
}

function SurveyEditor(nodes, prefix, form)
{
  var o = new Object();
  o.nodes = nodes;
  o.prefix = prefix;
  o.form = form;
  window.SurveyEditor_loadpackage.push(o);
}

function SurveyEditor_insert_onmouseover()
{
  var l = document.SurveyEditor_mryellow;
  l.pageX = this.x;
  l.pageY = this.y;
  l.clip.right = this.right_edge - this.x;
  l.visibility = 'show';
  
  var c = document.SurveyEditor_insert 
  c.pageX = this.right_edge;
  c.pageY = this.y;
  c.visibility = 'show';
}

function SurveyEditor_insert_onmouseout()
{
  var l = document.SurveyEditor_mryellow;
  l.visibility = 'hide';
  var c = document.SurveyEditor_insert;
  c.visibility = 'hide';
}  

function SurveyEditor_Menu() // extender
{
  var a = document.SurveyEditor_cmdmenu;

  window.captureEvents(Event.MOUSEMOVE);
  window.SurveyEditor_mousemove_old = window.onmousemove;
  window.onmousemove = SurveyEditor_mousemove;
  
  window.captureEvents(Event.MOUSEUP);
  window.SurveyEditor_mouseclick_old = window.onmouseup;
  window.onmouseup = SurveyEditor_mouseclick;
 
  a.onmouseout = SurveyEditor_Menu_onmouseout;
  a.onmouseover = SurveyEditor_Menu_onmouseover;
  a.timeout = SurveyEditor_Menu_timeout;
  a.show = SurveyEditor_Menu_show;
  a.hide = SurveyEditor_Menu_hide;
  a.hover = a.activenode = a.eventParam = false; //state variables
  return a;    
}

function SurveyEditor_Menu_show(activenode,eventParam)
{
  this.activenode = activenode;
  this.eventParam = eventParam;

  this.pageX = activenode.x + 9;
  this.pageY = activenode.y + 9;
  this.visibility = 'show';
}  

function SurveyEditor_Menu_hide()
{
  if (!this.hover && !this.activenode.hover)
    this.visibility = "hide";
}  

function SurveyEditor_preview(form,eventelement,state)
{
  // workaround due to a race condition in netscape 4, the commands in the
  // DOM preview code are not serialized and execute out of turn
  
  window.SurveyEditor_form = form;
  
  var w = window.open("about:blank", "previewwin", "width=500,height=410,status=0,toolbar=0,menubar=0,scrollbars=1,screenX=5,screeny=5,left=0,top=0,resizable=1",true);
  w.opener = window;

  var d = w.document;
  d.open();
  d.write("<script>\n");
  d.write("window.opener.SurveyEditor_form.target = 'previewwin';\n");
  d.write("window.opener.EventWidget_sgo(" + state + ", 0, '" + form.name + "', '" + eventelement + "');\n");
  d.write("window.opener.SurveyEditor_form.target = '_self';\n");
  d.write("window.opener.SurveyEditor_form['" + eventelement + "'].value = '';\n");
  d.write("</script>\n");
  d.close();
  w.focus();
}
  