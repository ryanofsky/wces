<?

require_once("wbes/component.inc");

define("FLAG_ALIGN", 0x1);

class TextResponseFactory extends ComponentFactory
{
  function code() { return 3; }
  function name() { return "Text Response Question"; }
  
  function create()
  {
    return new TextResponse();
  }
  
  function load(&$components, $ids, $ordinals)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_go("SELECT revision_id, ctext, flags, rows, cols FROM textresponse_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new TextResponse();
      $c->revision_id   = (int)$wr->row['revision_id'];
      $c->text          = $wr->row['ctext'];
      $c->rows          = (int) $wr->row['rows'];
      $c->cols          = (int) $wr->row['cols'];

      $flagGroup        = $wr->row['flags'];
      $c->locked        = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html       = (bool)($flagGroup & FLAG_ISHTML);
      $c->label_align   = (bool)($flagGroup & FLAG_ALIGN);
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }  
};

class TextResponse extends Component
{
  var $rows = 5;
  var $cols = 60;
  var $label_align = false;
  
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
  
  function save($topic_id, $save_id)
  {
    global $wbes;

    $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';
    $ctext        = addslashes($this->text);
    $rows         = (int) $this->rows;
    $cols         = (int) $this->cols;

    $flagGroup = 0;
    if ($this->is_html)     $flagGroup |= FLAG_ISHTML;
    if ($this->locked)      $flagGroup |= FLAG_LOCKED;
    if ($this->label_align) $flagGroup |= FLAG_ALIGN;

    $result = pg_go("SELECT textresponse_component_save($topic_id, $revision_id, $save_id, '$ctext', $flagGroup, $rows, $cols)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }  
};

class TextResponseEditor extends ComponentEditor
{
  //my addition. (the $label_align part) [tyler]
  var $rows,$cols,$label_align; //widgets
  
  function TextResponseEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    $this->rows = new TextBox(0,10,"",$prefix."_rows",$form,$formmethod);
    $this->cols = new TextBox(0,10,"",$prefix."_cols",$form,$formmethod);
    $this->rows->broken = false;
    $this->cols->broken = false;
    //mine [tyler]
    $choices = array("above","left");
    $this->label_align = new DropBox($choices,$prefix."_label_align",$form,$formmethod);
  }
  
  function loadvalues()
  {
    ComponentEditor::loadvalues();
    if ($this->loadprevious)
    {
      $this->rows->loadvalues();
      $this->cols->loadvalues();
      //mine [tyler]
      $this->label_align->loadvalues();
    }
    else
    {
      $this->rows->text = $this->component->rows;
      $this->cols->text = $this->component->cols;
      //mine [tyler]
      $this->label_align->selected = $this->component->label_align;
    }
  }
  
  function checkvalues()
  {
    $rows = (int)$this->rows->text;
    $cols = (int)$this->cols->text;
    
    //if ($rows < 1)
    //{
    //  $this->errors[] = "The response box should be at least one row high.";
    //  $this->rows->broken = true;
    //}  
    
    if ($cols < 1)
    {
      $this->errors[] = "The response box should be at least one column wide.";
      $this->cols->broken = true;
    }
    
    return count($this->errors) == 0;
  }
  
  function savevalues()
  {
    $this->component->rows = (int)$this->rows->text;
    $this->component->cols = (int)$this->cols->text;
    //mine [tyler]
    $this->component->label_align = $this->label_align->selected;
    ComponentEditor::savevalues();
  }
  
  function display()
  {
    global $server_media;
    
    ComponentEditor::display();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Component Text:",$this->text->broken); ?></strong></td>
  <td><? $this->text->display(false); ?><br><? $this->is_html->display(false); $this->is_html->displaylabel("Contains HTML Tags"); ?></td>
</tr>
<?// mine [tyler]
?>

<tr>
<td valign=top><br><strong><? $this->fieldname("Label Placement:",false); ?></strong></td>
<td><? $this->label_align->display(false); ?></td>
</tr>

<?// /mine
?>

<tr>
  <td valign=top><strong><? $this->fieldname("Height (rows)",$this->rows->broken); ?></strong></td>
  <td>
    <?$this->rows->display(false);?>
    <a href="javascript:showhelp('textsize')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
  </td>
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
    //mine [tyler]
    var $align = null;  
  function TextResponseWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
    $this->response = new TextBox($component->rows,$component->cols," wrap=virtual",$prefix."_response",$form,$formmethod);
    //mine [tyler]
    $this->align = $component->label_align;
  }
  
  function loadvalues()
  {
    $this->response->loadvalues();
  }

  function save($parent)
  {
    global $wbes;
    $t = addslashes(trim($this->response->text));
    
    if ($t)
      return (boolean) pg_go("
        INSERT INTO textresponse_responses(revision_id, parent, rtext)
        VALUES ({$this->component->revision_id}, $parent, '$t')
      ", $wbes, __FILE__, __LINE__);
    else
      return true;
  }
  
  function display($hidden = false, $showerrors = false)
  {
    if ($hidden)
    {
      $this->response->display($hidden);
    }
    else
    {
     //mine [tyler]
      if($this->align == 1)
      {
?>
<table>
<tr>
<td valign=top><?= ComponentWidget::display(); ?></td>
<td valign=top><?= $this->response->display(); ?>
</tr>
</table>
<?
      }
      else
      {
        ComponentWidget::display();
        print("<div>");
        $this->response->display();
        print("</div>\n");
      }
    }
  }
};

?>