<?

require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

define("questions_boxview",  0x1);
define("questions_listview", 0x2);
define("questions_listbreak",0x4);
define("questions_listspace",0x8);

class Question
{
  var $text,$usehtml;  // STRING, question text, html allowed
  
  function Question()
  {
    $this->text = "";
    $this->usehtml = false;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new QuestionEditor($this,$prefix,$form,$formmethod);
  }

  function getwidget($prefix,$form,$formmethod)
  {
    return new QuestionWidget($this,$prefix,$form,$formmethod);
  }
};

class QuestionEditor extends FormWidget
{
  var $text,$usehtml,$loadprevious; // Widgets
  var $errors; // Mistakes in the form
  var $question; // source question
  
  function QuestionEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->text = new TextBox(7,40,"wrap=virtual",$this->prefix."_text",$this->form,$this->formmethod);
    $this->usehtml = new CheckBox($this->prefix."_userhtml",$this->form,$this->formmethod);
    $this->question = &$question;
    
    $this->errors = array();
    
    $this->text->broken = false;
  }
  
  function loadvalues()
  {
    $this->loadprevious = $this->loadattribute("loadprevious");
    if ($this->loadprevious)
    {
      $this->text->loadvalues();
      $this->usehtml->loadvalues();
    }
    else
    {
      $this->text->text = $this->question->text;
      $this->usehtml->checked = $this->question->usehtml ? true : false;
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    $this->question->text = $this->text->text;
    $this->question->usehtml = $this->usehtml->checked;
    return true;
  }

  function display()
  {
    $this->printattribute("loadprevious",1);
    if (count($this->errors) > 0)
    {
      print("<font color=red><ul>\n");
      foreach($this->errors as $value)
        print("  <li>$value</li>\n");
      print("</ul></font>\n");
    }
  }
  
  function fieldname($name,$isbroken)
  {
    if ($isbroken)
      print("<font color=red>$name</font>");
    else
      print($name);
  }
};

class QuestionWidget extends FormWidget
{
  var $question;
  
  function QuestionWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->question = &$question;
  }
  
  function display()
  {
    print("<div>");
    if ($this->question->usehtml)
      print($this->question->text);
    else
      print(nl2br(htmlspecialchars($this->question->text)));
    print("</div>\n");
  }
};


class FreeResponse extends Question
{
  var $rows, $cols;
  
  function FreeResponse()
  {
    $this->Question();
    $this->rows = 5;
    $this->cols = 60;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
     return new FreeResponseEditor($this,$prefix,$form,$formmethod);
  }
  
  function getwidget($prefix,$form,$formmethod)
  {
     return new FreeResponseWidget($this,$prefix,$form,$formmethod);
  }
};

class FreeResponseEditor extends QuestionEditor
{
  var $rows,$cols; //widgets
  
  function FreeResponseEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->QuestionEditor($question,$prefix,$form,$formmethod);
    $this->rows = new TextBox(0,10,"",$prefix."_rows",$form,$formmethod);
    $this->cols = new TextBox(0,10,"",$prefix."_cols",$form,$formmethod);
    $this->rows->broken = false;
    $this->cols->broken = false;
  }
  
  function loadvalues()
  {
    QuestionEditor::loadvalues();
    if ($this->loadprevious)
    {
      $this->rows->loadvalues();
      $this->cols->loadvalues();
    }
    else
    {
      $this->rows->text = $this->question->rows;
      $this->cols->text = $this->question->cols;
    }
  }
  
  function checkvalues()
  {
    $rows = (int)$this->rows->text;
    $cols = (int)$this->cols->text;
    
    if ($rows < 1)
    {
      $this->errors[] = "The response box should be at least one row high.";
      $this->rows->broken = true;
    }  
    
    if ($cols < 1)
    {
      $this->errors[] = "The response box should be at least one column wide.";
      $this->cols->broken = true;
    }
    
    return count($this->errors) == 0;
  }
  
  function savevalues()
  {
    $this->question->rows = (int)$this->rows->text;
    $this->question->cols = (int)$this->cols->text;
    QuestionEditor::savevalues();
  }
  
  function display()
  {
    QuestionEditor::display();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Question Text:",$this->text->broken); ?></strong></td>
  <td><? $this->text->display(false); ?><br><? $this->usehtml->display(false); $this->usehtml->displaylabel("Contains HTML Tags"); ?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Height (rows)",$this->rows->broken); ?></strong></td>
  <td><?$this->rows->display(false);?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Width (columns)",$this->cols->broken); ?></strong></td>
  <td><? $this->cols->display(false); ?></td>
</tr>
</table>
<?    
  }
};

class FreeResponseWidget extends QuestionWidget
{
  var $response;
  
  function FreeResponseWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->QuestionWidget($question,$prefix,$form,$formmethod);
    $this->response = new TextBox($question->rows,$question->cols," wrap=virtual",$prefix."_response",$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->response->loadvalues();
  }
  
  function display()
  {
    QuestionWidget::display();
    print("<div>");
    $this->response->display();
    print("</div>\n");
  }
};

class RatingsBlock extends Question
{
  var $items;   // array of stringed items to rate
  var $low,$high;  // integer bounds, inclusive
  
  function RatingsBlock()
  {
    $this->Question();
    $this->items = array();
    $this->low = 5;
    $this->high = 1;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new RatingsBlockEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new RatingsBlockWidget($this,$prefix,$form,$formmethod);
  }
};

class RatingsBlockEditor extends QuestionEditor
{
  var $lowvalue,$highvalue,$items; //widgets
 
  function RatingsBlockEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->QuestionEditor($question,$prefix,$form,$formmethod);
    $this->lowvalue = new TextBox(0,5,"",$this->prefix."_lowvalue",$this->form,$this->formmethod);
    $this->highvalue = new TextBox(0,5,"",$this->prefix."_highvalue",$this->form,$this->formmethod);
    $this->items = new DynamicList(30,7,false,$this->prefix."_listitems",$this->form,$this->formmethod);
    
    $this->lowvalue->broken = false;
    $this->highvalue->broken = false;
    $this->items->broken = false;
  }
  
  function loadvalues() // public abstract
  {
    QuestionEditor::loadvalues();
    if ($this->loadprevious)
    {
      $this->lowvalue->loadvalues();
      $this->highvalue->loadvalues();
      $this->items->loadvalues();
    }  
    else
    {
      $this->lowvalue->text = $this->question->low;
      $this->highvalue->text = $this->question->high;
      $this->items->items = $this->question->items;
    }
  }
  
  function checkvalues()
  {
    $low = (int)$this->lowvalue->text;
    $high = (int)$this->highvalue->text;
    
    if (abs($low-$high) < 1)
    {
      $this->errors[] = "Low and high values are identical.";
      $this->lowvalue->broken = true;
      $this->highvalue->broken = true;
    }  
      
    if (count($this->items->items) < 1)
    {
      $this->errors[] = "The 'Items to Rate' list is empty.";
      $this->items->broken = true;
    }  
    
    return count($this->errors) == 0;
  }
  
  function savevalues()
  {
    $this->question->low = (int)$this->lowvalue->text;
    $this->question->high = (int)$this->highvalue->text;
    $this->question->items = $this->items->items;
    QuestionEditor::savevalues();
  }
  
  function display()
  {
    QuestionEditor::display();
?>    
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Question Text:",$this->text->broken); ?></strong></td>
  <td><?$this->text->display(false);?><br><?$this->usehtml->display(false); $this->usehtml->displaylabel("Contains HTML Tags");?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Low Value:",$this->lowvalue->broken); ?></strong></td>
  <td><?$this->lowvalue->display(false);?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("High Value:",$this->highvalue->broken); ?></strong></td>
  <td><? $this->highvalue->display(false); ?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Items to rate:",$this->items->broken); ?></strong></td>
  <td><? $this->items->display(false); ?></td>
</tr>
</table>
<?    
  }
}

class RatingsBlockWidget extends QuestionWidget
{
  var $choices; // persistent value
  
  function RatingsBlockWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->QuestionWidget($question,$prefix,$form,$formmethod);
    $this->choices = array();
  }
  
  function loadvalues() // public abstract
  {
    foreach($this->question->items as $key => $value)
      $this->choices[$key] = $this->loadattribute($key);
  }
  
  function display()
  {
    QuestionWidget::display();
    
    $direction = $this->question->high > $this->question->low ? 1 : -1;
    $l = abs($this->question->high - $this->question->low);
    
    print("<table>\n  <tr>\n    <td>&nbsp;</td>\n");
    for($i = 0; $i <= $l; ++$i)
      print("    <td align=center>" . ($this->question->low + $i * $direction) . "</td>\n");
    print("  </tr>\n");
    
    foreach($this->question->items as $key => $item)
    {
      print("  <tr>\n    <td>$item</td>\n");
      for($i = 0; $i <= $l; ++$i)
      {
        $j = ($this->question->low + $i * $direction);
        print("    <td align=center><input type=radio name=\"" . $this->prefix . "_" . $key . "\" value=\"$j\"" . ($j == $this->choices[$key] ? " checked" : "") . "></td>\n");
      }  
      print("  </tr>\n");
    }
    print("</table>\n"); 
  }
}

class QuestionSet
{
  var $questions;
  
  function QuestionSet()
  {
    $this->questions = array();
  }
  
  function insert($question,$position = 0)
  {
    $position = ((int)$position < 0) ? count($this->questions) + (int)$position + 1 : (int)$position;
    array_splice($this->questions,(int)$position,0,array($question));
    return $position;    
  }
  
  function delete($questionno)
  {
    array_splice($this->questions,$questionno,1);
  }
  
  function movedown($questionno)
  {
    if ($questionno >= count($this->questions) - 1) return;
    $q = $this->questions[$questionno];
    $this->delete($questionno);
    $this->insert($q,$questionno + 1);
  }
  
  function moveup($questionno)
  {
    if ($questionno < 1) return;
    $q = $this->questions[$questionno];
    $this->delete($questionno);
    $this->insert($q,$questionno - 1);
  }
}

?>