<?

require_once("widgets/widgets.inc");

class SqlBox extends Widget
{
  var $sql;
  var $multiple;
  var $selected;
  
  function SqlBox($sql, $multiple, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->sql = (string)$sql;
    $this->multiple = (bool)$multiple;
    $this->selected = $multiple ? array() : null;
  }

  function loadState()
  {
    $selected = $this->readValue();
    if (isset($selected))
    {
      if (!$this->multiple)
        $this->selected = (int)$selected;
      else if (is_array($selected))
        $this->selected = $selected;
    }
  }
  
  function displayHidden()
  {
    $name = $this->name . ($this->multiple ? '[]' : '');
    if (isset($this->selected))
    {
      if (!$this->multiple)
          print("<input type=hidden name=$name value=" . ((int)$this->selected) . ">");    
      else
      {
        foreach($this->selected as $s)
          print("<input type=hidden name=$name value=" . ((int)$s) . ">");
      }
    }
  }
  
  function display($params="")
  {
    global $wces;
    $name = $this->name . ($this->multiple ? '[]' : '');
    if ($params) $params = " $params";

    wces_connect();
    $result = pg_go($this->sql, $wces, __FILE__, __LINE__);
    $n = pg_numrows($result);
    
    print("<select name=$name$params>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result, $i, PGSQL_ASSOC));
      if ((!$this->multiple && $id == $this->selected) || ($this->multiple && in_array($id, $this->selected)))
        print("  <option value=$id selected>$name\n");
      else  
        print("  <option value=$id>$name\n");
    }
    print("</select>");
  }
};

?>