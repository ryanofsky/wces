<?

// used for scripts that output dynamically generated php images.
// these functions generate headers that cause the images to be cached
// and that allow HTTP 1.1 connections to be reused

function png_start($file, $seconds)
{
  session_cache_limiter('public');
  header('Cache-Control: public');
  header('Pragma: public');
  header('ETag: ' . md5($_SERVER['QUERY_STRING']));
  header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
  header('Last-Modified: ' . gmdate('D, d M Y H:i:s', filemtime($file)) . ' GMT');
  header('Content-type: image/png');
  if ($_SERVER['REQUEST_METHOD'] == 'HEAD') exit();
  return tempnam(null, "png");
}

function png_end($filename)
{
  header('Content-Length: ' . filesize($filename));
  $fp = fopen($filename, 'rb');
  fpassthru($fp);
  unlink($filename);
}

?>
