//----------------------------------------------------------------------------
// Object Printer Function (for debug)

window._objwindows = 0;

function objprint(a,name,depth)
{
  var win = window.open("", "_blank", "width=300,height=400,channelmode=0,dependent=0,directories=0,fullscreen=0,location=0,menubar=0,resizable=1,scrollbars=1,status=1,toolbar=0"); // a window object
  window.taskdoc = win.document;
  ++window._objwindows;
  window._depth = 0;
  window._maxdepth = depth;
  taskdoc.open("text/html", "replace");
  taskdoc.write('<title>Object Window #' + window._objwindows + ' for ' + name + '</title>');
  taskdoc.write("<table border='1' bgcolor='#EEEEEE' border='1' cellpadding='2' cellspacing='0' bordercolorlight='#000000'>");
  _objprint(a,name);  
  taskdoc.write("</table>");
  taskdoc.close();
};

function _objprint(obj,name)
{
  if(typeof obj == "object") // all objects in here
  { 
    if (obj && obj.constructor) // detect specific objects for special handling here
    {
      if (obj.constructor == Array) _parray(obj,name);
      else if (obj.constructor == String) _pstring(obj,name);
      else _pgeneric(obj,name);
    }  
    else _pgeneric(obj,name);
  }
  else _pbase(obj,name);
}

function _parray(a,name) // array object
{
  if (a.length==0) _pempty("array",name)
  for(var i=0; i < a.length; i++)  
    _objprint(a[i],name+"["+i+"]");
};

function _pstring(obj,prefix) // string object
{
  taskdoc.write("<tr><td>",prefix,"</td><td>\"",obj,"\" (String object)</td></tr>");
}

function _pgeneric(obj, name) // generic object
{
  if (!_listprops(obj,name)) _pempty("object",name);
};

function _pempty(type,prefix) // object with no properties
{
  taskdoc.write("<tr><td>",prefix,"</td><td>[empty "+type+"]</td></tr>");
};

function _pbase(obj,prefix) // base type
{
  taskdoc.write("<tr><td>",prefix,"</td><td>",obj," (",typeof obj,")</td></tr>");
  _listprops(obj,prefix);
};

function _listprops(obj,name)
{
  if (++window._depth > window._maxdepth) return false;
  var hasprops = false;
  for (var i in obj)
  { 
    hasprops = true;
    _objprint(obj[i],name+"."+i);
  }
  return hasprops;  
  --window._depth;
};