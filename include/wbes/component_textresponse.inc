<?

require_once("wbes/component.inc");

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
    $sids = implode($ids, ", ");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, ctext, is_html, rows, cols FROM textresponse_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $id = (int) $wr->row['revision_id'];
      $ord = $ordinals[$rids[$id]];
      $components[$ord] = new TextResponse(); $c = &$components[$ord];
      $c->revision_id   = (int)$wr->row['revision_id'];
      $c->is_html       = $wr->row['is_html'] == 't';
      $c->text          = $wr->row['ctext'];
      $c->rows          = (int) $wr->row['rows'];
      $c->cols          = (int) $wr->row['cols'];
    }
  }  
};

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
  
  function save($save_id)
  {
    global $wbes;
    $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';
    $ctext        = addslashes($this->text);
    $is_html      = $this->is_html ? "'t'" : "'f'";
    $rows         = (int) $this->rows;
    $cols         = (int) $this->cols;
    $result = pg_query("SELECT textresponse_component_save($save_id, $revision_id::integer, '$ctext'::text, $is_html::boolean, $rows, $cols)", $wbes, __FILE__, __LINE__);
    return pg_result($result, 0, 0);
  }  
};

class TextResponseEditor extends ComponentEditor
{
  var $rows,$cols; //widgets
  
  function TextResponseEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
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
      $this->rows->text = $this->component->rows;
      $this->cols->text = $this->component->cols;
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
  
  function TextResponseWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
    $this->response = new TextBox($component->rows,$component->cols," wrap=virtual",$prefix."_response",$form,$formmethod);
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