class Rating extends Component
{
  var $items;   // array of stringed items to rate
  var $low,$high;  // integer bounds, inclusive
  
  function Rating()
  {
    $this->Component();
    $this->items = array();
    $this->low = 5;
    $this->high = 1;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new RatingEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new RatingWidget($this,$prefix,$form,$formmethod);
  }
};

class RatingEditor extends ComponentEditor
{
  var $lowvalue,$highvalue,$items; //widgets
 
  function RatingEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($question,$prefix,$form,$formmethod);
    $this->lowvalue = new TextBox(0,5,"",$this->prefix."_lowvalue",$this->form,$this->formmethod);
    $this->highvalue = new TextBox(0,5,"",$this->prefix."_highvalue",$this->form,$this->formmethod);
    $this->items = new DynamicList(30,7,false,$this->prefix."_listitems",$this->form,$this->formmethod);
    
    $this->lowvalue->broken = false;
    $this->highvalue->broken = false;
    $this->items->broken = false;
  }
  
  function loadvalues() // public abstract
  {
    ComponentEditor::loadvalues();
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
    ComponentEditor::savevalues();
  }
  
  function display()
  {
    ComponentEditor::display();
?>    
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Component Text:",$this->text->broken); ?></strong></td>
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

class RatingWidget extends ComponentWidget
{
  var $choices; // persistent value
  
  function RatingWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($question,$prefix,$form,$formmethod);
    $this->choices = array();
  }
  
  function loadvalues() // public abstract
  {
    foreach($this->question->items as $key => $value)
      $this->choices[$key] = $this->loadattribute($key);
  }
  
  function display()
  {
    ComponentWidget::display();
    
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