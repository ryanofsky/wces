<?

require_once("wbes/component.inc");

define("FLAG_ALIGN", 0x1);

class TextResponseFactory extends ComponentFactory
{
  function code() { return 3; }
  function name() { return "Text Response Question"; }

  function & create()
  {
    return new TextResponse;
  }

  function loadComponents(&$components)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, ctext, flags, rows, cols FROM textresponse_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);
    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c =& new TextResponse;
      $c->text       = $wr->row['ctext'];
      $c->rows       = (int) $wr->row['rows'];
      $c->cols       = (int) $wr->row['cols'];
      $flagGroup     = $wr->row['flags'];
      $c->locked     = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html    = (bool)($flagGroup & FLAG_ISHTML);
      $c->labelAlign =  (bool)($flagGroup & FLAG_ALIGN);
      $components[$wr->row['component_id']] =& $c;
    }
  }
};

class TextResponse extends Component
{
  var $rows = 5;
  var $cols = 60;
  var $labelAlign = false;

  function TextResponse()
  {
    $this->Component();
    $this->rows = 5;
    $this->cols = 60;
  }

  function & geteditor($name, &$parent)
  {
     return new TextResponseEditor($this, $name, $parent);
  }

  function & getwidget($name, &$parent)
  {
     return new TextResponseWidget($this, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    $ctext = addslashes($this->text);
    $ctext        = addslashes($this->text);
    $rows         = (int) $this->rows;
    $cols         = (int) $this->cols;
    $flags = 0;
    if ($this->is_html) $flags |= FLAG_ISHTML;
    if ($this->locked)  $flags |= FLAG_LOCKED;
    if ($this->labelAlign)  $flagGroup |= FLAG_ALIGN;
    return "textresponse_component_save($component_id, '$ctext', $flags, $rows, $cols)";
  }
};

class TextResponseEditor extends ComponentEditor
{
  // widgets
  var $rows, $cols, $labelAlign;

  function TextResponseEditor(&$component, $name, &$parent)
  {
    $this->ComponentEditor($component, $name, $parent);
    $this->rows = & new TextBox(0, 10, "", "rows", $this);
    $this->cols = & new TextBox(0, 10, "", "cols", $this);
    $this->rows->broken = false;
    $this->cols->broken = false;
    $this->labelAlign = & new DropBox(array("above","left"), "labelAlign", $this);
  }

  function loadInitialState()
  {
    $this->rows->text = $this->component->rows;
    $this->cols->text = $this->component->cols;
    $this->labelAlign->selected = $this->component->labelAlign;
  }

  function validate()
  {
    $rows = (int)$this->rows->text;
    $cols = (int)$this->cols->text;

    if ($cols < 1)
    {
      $this->errors[] = "The response box should be at least one column wide.";
      $this->cols->broken = true;
    }

    return count($this->errors) == 0;
  }

  function save()
  {
    $this->component->rows = (int)$this->rows->text;
    $this->component->cols = (int)$this->cols->text;
    if ($this->component->rows < 0) $this->component->rows = 0;
    $this->component->labelAlign = $this->labelAlign->selected;
    ComponentEditor::save();
  }

  function printVisible()
  {
    global $server_media;

    $this->printErrors();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Component Text:",$this->text->broken); ?></strong></td>
  <td><? $this->text->display(); ?><br><? $this->is_html->display(); $this->is_html->displayLabel("Contains HTML Tags"); ?></td>
</tr>
<tr>
<td valign=top><br><strong><? $this->fieldname("Label Placement:",false); ?></strong></td>
<td><? $this->labelAlign->display(); ?></td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Height (rows)",$this->rows->broken); ?></strong></td>
  <td>
    <?$this->rows->display();?>
    <a href="javascript:showhelp('textsize')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
  </td>
</tr>
<tr>
  <td valign=top><strong><? $this->fieldname("Width (columns)",$this->cols->broken); ?></strong></td>
  <td><? $this->cols->display(); ?></td>
</tr>
</table>
<?
  }
};

class TextResponseWidget extends ComponentWidget
{
  // widgets
  var $response;

  // private
  var $align = null;

  function TextResponseWidget(&$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
    $this->response =& new TextBox($component->rows, $component->cols, " wrap=virtual", "response", $this);
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

  function printVisible()
  {
    if($this->component->labelAlign)
    {
?>
<table>
<tr>
<td valign=top><? ComponentWidget::printVisible(); ?></td>
<td valign=top><? $this->response->display(); ?>
</tr>
</table>
<?
    }
    else
    {
      ComponentWidget::printVisible();
      print("<div>");
      $this->response->display();
      print("</div>\n");
    }
  }
};

?>