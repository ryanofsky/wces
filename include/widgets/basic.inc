<?

require_once("widgets/widgets.inc");

class Textbox extends Widget
{
  var $text = null;
  var $rows,$cols,$params;

  function Textbox($rows, $cols, $params, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->rows = (int)$rows;
    $this->cols = (int)$cols;
    $this->params = strlen($params) > 0 ? " $params" : $params;
  }

  function displayHidden()
  {
    $this->printValue("", $this->text);
  }

  function display()
  {
    $t = htmlspecialchars($this->text);
    $r = $this->rows;
    $c = $this->cols;
    $s = strlen($this->params > 0) ? " $this->params" : "";

    if ($this->rows == 0)
      print("<input type=text name=$this->name value=\"$t\" size=$c$s>");
    else
      print("<textarea name=$this->name rows=$r cols=$c$s>$t</textarea>");
  }

  function loadState()
  {
    $this->text = $this->readValue();
  }
};

class CheckBox extends Widget
{
  var $checked = false;

  function CheckBox($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function displayHidden()
  {
    if ($this->checked) $this->printValue("", 1);
  }

  function display($params = "")
  {
    if ($params) $params = " $params";
    print("<input type=checkbox name=$this->name id=$this->name value=1"
      . ($this->checked ? " checked" : "") . "$params>");
  }

  function displayLabel($label, $params = "")
  {
    print("<label for=$this->name $params>$label</label>");
  }

  function loadState()
  {
    $this->checked = (bool)$this->readValue();
  }
}

class RadioButton extends Widget
{
  var $exclusive;
  var $checked = null;

  function RadioButton($exclusive, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->exclusive = (bool)$exclusive;
    if (!$exclusive) $this->checked = array();
  }

  function displayHidden()
  {
    if ($this->exclusive && isset($this->checked))
      $this->printValue("", $this->checked);
    else if (!$this->exclusive && is_array($this->checked))
      $this->printArrayValue("", $this->checked);
  }

  function display($index, $params='')
  {
    $n = $this->exclusive ? $this->name : ($this->name . '[]');
    $i = $this->fullName($index);

    if ($this->exclusive)
    {
      $t = 'radio';
      $c = $this->checked == $index ? ' checked' : '';
    }
    else
    {
      $t = 'checkbox';
      $c = in_array($index, $this->checked) ? ' checked' : '';
    }

    if (strlen($params) > 0) $params = " $params";
    print("<input type=$t name=$n value=$index id=$i$params$c>");
  }

  function displayLabel($index, $text, $params = "")
  {
    $i = $this->fullName($index);
    if (strlen($params) > 0) $params = " $params";
    print("<label for=$i$params>$text</label>");
  }

  function loadState()
  {
    $this->checked = $this->readValue();
    if (!$this->exclusive && !is_array($this->checked))
      $this->checked = array();
  }
};

class DropBox extends Widget
{
  var $selected = null;
  var $items;

  function DropBox($items, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->items = $items;
  }

  function displayHidden()
  {
    $this->printValue("", $this->selected);
  }

  function display($params = "")
  {
    if (strlen($params) > 0) $params = " $params";

    print("<select name=$this->name$params>\n");
    foreach($this->items as $k => $v)
    {
      if ($k == $this->selected)
        print("  <option value=$k selected>$v\n");
      else
        print("  <option value=$k>$v\n");
    }
    print("</select>");
  }

  function loadState()
  {
    $this->selected = $this->readValue();
  }
};

define('EVENT_WIDGET_SEPARATOR', '*');

class EventWidget extends Widget
{
  var $event = null;
  var $param = null;
  
  function EventWidget($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function parseParam($param)
  {
    if (!isset($param)) return null;
    if (strlen($param) == 0) return '';
    $r = explode(EVENT_WIDGET_SEPARATOR, $param);
    return count($r) == 1 ? $r[0] : $r;
  }

  function setEvent($str)
  {
    $p = strpos($str, EVENT_WIDGET_SEPARATOR);
    if ($p === false)
    {
      $this->event = $str;
      $this->param = null;
    }
    else
    {
      $this->event = substr($str, 0, $p);
      $this->param = $this->parseParam(substr($str, $p + 1));
    }
  }

  function makeEvent($event, $param)
  {
    if (isset($param))
      return $event . EVENT_WIDGET_SEPARATOR . $this->makeParam($param);
    else
      return $event;
  }

  function makeParam($param)
  {
    if (is_array($param))
      return implode(EVENT_WIDGET_SEPARATOR, $param);
    else
      return $param;
  }

  function loadState() // public abstract
  {
    $v = $this->readValue();
    if(isset($v) && strlen($v) > 0)
    {
      $this->setEvent($v);
      return;
    }

    $search = array(&$_POST, &$_GET);

    $prefix = $this->name . WIDGET_SEPARATOR;
    $length = strlen($prefix);

    foreach(array_keys($search) as $i)
    foreach(array_keys($search[$i]) as $j)
    if ((strlen($j) >= $length) && (substr($j, 0, $length) == $prefix))
    {
      $this->setEvent(substr($j,$length));
      return;
    };
  }

  function displayPersistent()
  {
    $this->printValue('', $this->makeEvent($this->event, $this->param));
  }

  function display($event = null, $param = null)
  {
    $this->printValue('', $this->makeEvent($event, $param));
  }

  function displayButton($label, $event, $param = null, $attribs = "")
  {
    $n = $this->fullName($this->makeEvent($event, $param));
    $l = '"' . htmlspecialchars($label) . '"';
    if (strlen($attribs) > 0) $attribs = " $attribs";
    print("<input type=submit name=$n value=$l$attribs>");
  }

  function displayNoJavascriptRefresh($label, $attribs = '')
  {
    print("<noscript>");
    $this->displayButton($label, null, null, $attribs);
    print("</noscript>");
  }

  function displayField()
  {
    $this->printValue('', '');
  }

  function gethref($event,$param)
  {
    $p = $this->makeParam($param);
    return "javascript:EventWidget_sgo('$event','$param','{$this->form->formName}','$this->name');";
  }

  function DumpScript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;
    $as = EVENT_WIDGET_SEPARATOR;
    $ws = WIDGET_SEPARATOR;

    $str = <<<EOD
<!-- Begin EventWidget Script -->
<script>
<!--

  WIDGET_SEPARATOR = '$ws';
  EVENT_WIDGET_SEPARATOR = '$as';

  function EventWidget_go(event,object,eventObject)
  {
    //alert(eventObject.name + " set to '" + event + "'");
    eventObject.value = event;
    if (object != null) eventObject.value += EVENT_WIDGET_SEPARATOR + object;

    if (eventObject.length)
      alert("Warning: An event object has been displayed multiple times on the same page");

    if (eventObject.form.onsubmit) eventObject.form.onsubmit();
    eventObject.form.submit();
  }

  function EventWidget_sgo(event,object,form,eventelement)
  {
    //alert(document.forms[form][eventelement]);
    EventWidget_go(event,object,document.forms[form][eventelement]);
  }
// -->
</script>
<!-- End EventWidget Script -->
EOD;
    headprint($str);
  }
};

/*

The idea behind this class is to factor out repetitive state-management
code that tended to exist in complex widget classes.

These classes would typically have some kind of menu screen where a selection
from the menu would result in construction of a new modal widget, which would
last for a one or more page loads before it was destroyed and the main menu
screen shown again.

With this class, the logic behind the system no longer needs to be reimplemented.
Complex menu widgets need only inherit from StateWidget and write a
handleEvent() method. The handleEvent() method should look like this:

  function & handleEvent($event, $param, $isNew)
  {
    // decide what to do based on the value of $event and $param
    // if this merits creation of a new modal widget then
    // return a create that widget and return a reference to it,
    // otherwise return null.
  }

When the handleEvent method returns a non-null value, it indicates that
a new modal Widget has been created and should be recreated on the next
page load. The modal widget creation always occurs in the handleEvent method.
In the case of an initial creation, the $isNew parameter will be true, and
during a recreation it will be false. Recreation at subsequent page loads will
occur until the modal widget sets its "done" parameter to true or until a future
call to handleEvent returns a different modal widget that will replace the
current one.

*/

class StatefullWidget extends ParentWidget
{
  var $event;
  var $currentEvent = null;
  var $currentParam = null;
  var $newEvent = null;
  var $newParam = null;

  function StatefullWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->event =& new EventWidget("event", $this);
  }

  function loadState()
  {
    ParentWidget::loadState();
    $this->eventLoop($this->readValue("currentEvent"), $this->event->parseParam($this->readValue("currentParam")), false);
    $this->eventLoop($this->event->event, $this->event->param, true);
  }

  function eventLoop($event, $param, $new)
  {
    while(isset($event))
    {
      $this->newEvent = $this->newParam = null;
      $widget =& $this->handleEvent($event, $param, $new);
      $this->setCurrent($widget, $event, $param);
      $event = $this->newEvent;
      $param = $this->newParam;
      $new = true;
    }
  }

  function setCurrent(&$widget, $event, $param)
  {
    if (isset($widget))
    {
      if (isset($this->modalChild))
      {
        $this->modalChild->modal = false;
        unset($this->modalChild);
      }

      if ($widget->done)
      {
        $widget->modal = false;
        $this->currentEvent = null;
        $this->currentParam = null;
        $this->modalChild = null;
      }
      else
      {
        $widget->modal = true;
        $this->currentEvent = $event;
        $this->currentParam = $param;
        $this->modalChild =& $widget;
      }
    }
  }

  function addEvent($event, $param)
  {
    $this->newEvent = $event;
    $this->newParam = $param;
  }

  function printState()
  {
    $this->printValue('currentEvent', $this->currentEvent);
    $this->printValue('currentParam', $this->event->makeParam($this->currentParam));
  }
};

class TabbedWidget extends ParentWidget
{
  // eventwidget
  var $event;

  // array all widget-returning events seen so far
  //   true if widget was modal, false otherwise
  var $seen = array();

  // references to child widgets corresponding to events
  var $handled = array();

  var $currentEvent;

  function TabbedWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->event =& new EventWidget('event', $this);
  }

  function addEvent($event, $param)
  {
    $this->newEvent = $event;
    $this->newParam = $param;
  }

  function setCurrent(&$widget, $event, $setCurrent)
  {
    if (isset($widget))
    {
      $this->handled[$event] =& $widget;

      if (!$setCurrent)
      {
        $this->seen[$event] = false;
        return;
      }

      if (isset($this->currentEvent))
      {
        $this->seen[$this->currentEvent] = false;
        $this->currentEvent = null;
      }

      if (isset($this->modalChild))
      {
        $this->modalChild->modal = false;
        unset($this->modalChild);
      }

      if ($widget->done)
      {
        $this->seen[$event] = false;
        $widget->done = false;
      }
      else
      {
        $widget->modal = true;
        $this->currentEvent = $event;
        $this->seen[$event] = true;
        $this->modalChild =& $widget;
      }
    }
  }

  function eventLoop($event, $param, $new, $setCurrent)
  {
    while(isset($event))
    {
      $this->newEvent = $this->newParam = null;
      if (isset($this->handled[$event]))
        $widget =& $this->handled[$event];
      else
        $widget =& $this->handleEvent($event, $param, $new);
      $this->setCurrent($widget, $event, $setCurrent);
      $event = $this->newEvent;
      $param = $this->newParam;
      $new = true;
    }
  }

  function loadState()
  {
    ParentWidget::loadState();
    $this->seen = unserialize($this->readValue('seen'));

    foreach($this->seen as $k => $v)
      $this->eventLoop($k, null, false, $v);

    $this->eventLoop($this->event->event, $this->event->param, true, true);
  }

  function printState()
  {
    $this->printValue('seen', serialize($this->seen));
  }

  function printVisible()
  {
    foreach(array_keys($this->handled) as $k)
      $this->handled[$k]->displayHidden();
  }
}

class ImageButton extends Widget
{
  var $raisedsrc;
  var $pushedsrc;
  var $iparams;
  var $aparams;

  function ImageButton($href,$raisedsrc,$pushedsrc,$iparams,$aparams,$name,&$parent)
  {
    ImageButton::DumpScript();
    $this->Widget($name, $parent);
    $this->href = $href; //XXX: this parameter never seems to be used
    $this->raisedsrc = $raisedsrc;
    $this->pushedsrc = $pushedsrc;
    $this->iparams = $iparams;
    $this->aparams = $aparams;
  }

  function display($hidden = false)
  {
    if ($hidden) return;
    ?><a onmousedown="button_push('<?=$this->name?>')" onmouseup="button_unpush('<?=$this->name?>')" onmouseout="button_unpush('<?=$this->name?>')" <?=$this->aparams?>"><img name="<?=$this->name?>" src="<?=$this->raisedsrc?>" border=0 <?=$this->iparams?>"></a><script>AttachPushed('<?=$this->name?>','<?=$this->pushedsrc?>')</script><?
  }

  function dumpScript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;

    $str = <<<EOS
<!-- Begin ImageButton Script -->
<script>
<!--

  var happyrollover = document.images && window.Image ? true : false;
  var happytree = happyrollover && RegExp && (document.all || document.getElementById) ? true : false;
  var detect = navigator.userAgent.toLowerCase();
  var refreshbug =  (detect.indexOf('compatible') < 0 && detect.charAt(8) == '5') ? true : false; // check for mozilla

  // fix for Netscape 3 string object
  if (happyrollover && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

  function getImage(oldimage,filename)
  {
    var p = oldimage.src.lastIndexOf("/");
    var q = oldimage.src.lastIndexOf("\\\\");
    var i = new Image();
    i.src = oldimage.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
    return i;
  };

  function AttachPushed(img, filename)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      i.flipimage = getImage(i,filename);
      i.pushed = false;
    };
  };

  function button_push(img)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      if (!i.pushed)
      {
        i.pushed = true;
        var temp = i.src;
        i.src = i.flipimage.src;
        i.flipimage.src = temp;
      };
    };
  };

  function button_unpush(img)
  {
    if(happyrollover)
      window.setTimeout("button_realunpush('" + img + "')", 0);  // defeat ie's lazy refresh
  };

  function button_realunpush(img)
  {
    var i = document.images[img];
    if (i.pushed)
    {
      i.pushed = false;
      var temp = i.src;
      i.src = i.flipimage.src;
      i.flipimage.src = temp;
    };
  };
// -->
</script>
<!-- End ImageButton Script -->
EOS;
    headPrint($str);
  }
}

?>
