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
    $this->notFoundMessage = "";
    $this->multiple = (bool)$multiple;
    $this->selected = $multiple ? array() : null;
  }

  function loadState($new)
  {
    if ($new) return;
    
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
    $name = $this->name() . ($this->multiple ? '[]' : '');
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

  function displayUrl()
  {
    if (isset($this->selected))
    {
      if (!$this->multiple)
        $this->printValue('', (int)$this->selected, WIDGET_URL);
      else
      {
        foreach($this->selected as $s)
          $this->printValue('', (int)$s, WIDGET_URL);
      }
    }    
  }
  
  function result()
  {
    if (isset($this->result)) return ($this->result);
    global $wces;
    wces_connect(); 
    $this->result = pg_go($this->sql, $wces, __FILE__, __LINE__);
    return $this->result;
  }
  
  function display($params="")
  {
    
    $name = $this->name() . ($this->multiple ? '[]' : '');
    if ($params) $params = " $params";

    wces_connect();
    $result = $this->result();
    $n = pg_numrows($result);
    
    print("<select name=$name$params>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result, $i, PGSQL_ASSOC));
      if (isset($this->format))
      {
        $f = $this->format;
        $name = $f($name);
      }
      if ((!$this->multiple && $id == $this->selected) || ($this->multiple && in_array($id, $this->selected)))
        print("  <option value=$id selected>$name\n");
      else  
        print("  <option value=$id>$name\n");
    }
    if ($n == 0 && $this->notFoundMessage)
    {
      print("  <option value=\"\">{$this->notFoundMessage}\n");
    }
    print("</select>");
  }
};

?>
