<%

require_once("widgets.inc");

define("questions_boxview",  0x1);
define("questions_listview", 0x2);
define("questions_listbreak",0x4);
define("questions_listspace",0x8);

class Question
{
  var $text;  // STRING, question text, html allowed
  
  function getwidget($prefix,$form,$formmethod)
  {
    assert(0);
  }
  
  function geteditor()
  {
    assert(0);
  }
}

class RatingsBlock extends Question
{
  var $items // Array of strings
  var $lowest    // integer
  var $highest   // integer
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new RatingsBlockWidget($this->items,$this->low,$this->high,$this->text,$prefix,$form,$formmethod);
  }
}

class RatingsBlockWidget extends Widget
{
  var $items;
  var $low, high;
  var $text;
  var $choices;
  
  function RatingsBlockWidget($items, $low, $high, $text, $prefix,$form,$formmethod)
  {
    Widget($prefix,$form,$formmethod);
    $this->items = $items;
    $this->low = $low;
    $this->high = $high;
    $this->text = $text;
  }
  
  function loadvalues() // public abstract
  {
    $choices = array();    
    foreach($this->items as $key => $value)
      $this->choices[$key] = $this->loadattribute($key);
  }
  
  function display($hidden) // public abstract
  {
    print($this->text);
    
    $direction = $this->high > $this->low ? 1 : -1;
    $l = abs($this->high - $this->low);
    
    print("<table>\n  <tr>\n    <td>&nbsp;</td>\n");
    for($i = 0; $i < $l; ++$i)
      print("    <td>" . ($this->low + $i * $direction) . "</td>\n");
    print("  </tr>"
    
    foreach($this->items as $key => $item)
    {
      print("  <tr>\n    <td>$item</td>\n");
      for($i = 0; $i < $l; ++$i)
        print("    <td><input type=radio name=\"" . $this->prefix . "_" . $key . "\ value=\"" . $i . "\"" . ($i == $this->choices[$key] ? " checked" : "") . "</td>\n");
      print("  </tr>\n");
    }  
  }
}

class RatingsBlockEditor extends Widget
{
  function RatingsBlockEditor($prefix,$form,$formmethod) // public constructor
  {
    $this->Widget($prefix,$form,$formmethod);
  }
  
  function loadvalues() // public abstract
  {
    $this->text->loadvalues();
    $this->usehtml->loadvalues();
    $this->lowvalue->loadvalues();
    $this->highvalue->loadvalues();
    $this->listitem->loadvalues();
  }
  
  function display($hidden) // public abstract
  {
%>    
<table>
<tr>
  <td>Question Text</td>
  <td><%$this->text->display(false);%><br><%$this->usehtml->display(false); $this->usehtml->displaylabel("Interpret HTML Tags");%></td>
</tr>
<tr>
  <td>Low Value</td>
  <td><%$this->lowvalue->display(false);%></td>
</tr>
<tr>
  <td>High Value</td>
  <td><% $this->highvalue->display(false); %></td>
</tr>
<tr>
  <td>Items to rate</td>
  <td><% $this->listitems->display(false); %></td>
</tr>
</table>

<%    
  }
}

class MultipleChoice extends Question
{
  var $choices;
  var $selected; // array index of selected item OR the 'false' value (to be checked like: $this->selected === false)
  
  function MultipleChoice($text,$choices,$selected,$prefix,$form,$formmethod)
  {
    $this->Widget($prefix,$form,$formmethod);
    $this->text = $text;
    $this->choices = $choices;
    $this->selected = $selected;
  }
   
  function display()
  {
    print("<div class=\"questiontext\">" . $this->text . "</div>\n");
    print("<div class=\"questionbody\">\n");
    $name = $this->prefix . "_choice";
    foreach($this->choices as $key => $value)
    {
      $id = $this->prefix . "_" . $key;
      $checked = !($this->selected === false) && ($this->selected == $key) ? " checked" : "";
      print("<input type=radio name=\"$name\" id=\"$id\" value=\"$key\"$checked>");
      print("<label for=\"$id\">$value</label><br>\n");
    };
    print('</div>');
  }
  
  function loadvalues()
  {
    $this->selected = $this->loadattribute("choice");
  }
}

class QuestionSet
{
  var $questions;
  
  function QuestionSet()
  {
    $this->questions = Array();
  }
  
  function addquestionbefore($qdata,$questionno)
  {
    if ($questionno === false)
    {
      array_unshift($this->questions,$qdata);  
    }
    else
    {
      array_splice($this->questions,$questionno,0,$qdata);
    };
  }
  
  function addquestionafter($qdata,$questionno)
  {
    if ($questionno === false)
    {
      array_push($this->questions,$qdata);
    }
    else
    {
      array_splice($this->questions,$questionno+1,0,$qdata);
    };
  }
  
  function deletequestion($questionno)
  {
    array_splice($this->questions,$questionno,1);
  }
}

%>