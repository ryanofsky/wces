<%

require_once("widgets.inc");

define("questions_boxview",  0x1);
define("questions_listview", 0x2);
define("questions_listbreak",0x4);
define("questions_listspace",0x8);

class Question extends Widget
{
  var $text;  // STRING, question text, html allowed
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

class MultipleSelect extends Question
{

}

class FreeResponse extends Question
{

}

class Ranking extends Question
{

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