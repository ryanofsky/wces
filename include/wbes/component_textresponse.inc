<?

class TextResponse extends Component
{
  var $rows, $cols;
  
  function TextResponse()
  {
    $this->Component();
    $this->rows = 5;
    $this->cols = 60;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
     return new TextResponseEditor($this,$prefix,$form,$formmethod);
  }
  
  function getwidget($prefix,$form,$formmethod)
  {
     return new TextResponseWidget($this,$prefix,$form,$formmethod);
  }
};

class TextResponseEditor extends ComponentEditor
{
  var $rows,$cols; //widgets
  
  function TextResponseEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($question,$prefix,$form,$formmethod);
    $this->rows = new TextBox(0,10,"",$prefix."_rows",$form,$formmethod);
    $this->cols = new TextBox(0,10,"",$prefix."_cols",$form,$formmethod);
    $this->rows->broken = false;
    $this->cols->broken = false;
  }
  
  function loadvalues()
  {
    ComponentEditor::loadvalues();
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
    ComponentEditor::savevalues();
  }
  
  function display()
  {
    ComponentEditor::display();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Component Text:",$this->text->broken); ?></strong></td>
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

class TextResponseWidget extends ComponentWidget
{
  var $response;
  
  function TextResponseWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($question,$prefix,$form,$formmethod);
    $this->response = new TextBox($question->rows,$question->cols," wrap=virtual",$prefix."_response",$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->response->loadvalues();
  }
  
  function display()
  {
    ComponentWidget::display();
    print("<div>");
    $this->response->display();
    print("</div>\n");
  }
};

?>