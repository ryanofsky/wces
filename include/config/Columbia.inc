<?

$WCES_CONFIG_PATH = '/wces/';
$WCES_CONFIG_MEDIA_PATH = "{$WCES_CONFIG_PATH}media/";
$WCES_CONFIG_HELP_PATH = "{$WCES_CONFIG_PATH}help.php";

$WCES_CONFIG_UPLOAD_PATH = "/wces/upload/";
$WCES_CONFIG_UPLOAD_PHYSICAL_PATH = "/home/httpd/apps/wces/www/wces/upload/";

$WCES_CONFIG_PAGE_INCLUDE = 'config/ColumbiaPage.inc';
$WCES_CONFIG_PAGE = 'ColumbiaPage';

$WCES_CONFIG_FACTORIES_INCLUDE = 'config/survey.inc';

$WBES_LOGIN = 'ColumbiaLogin';
$WBES_LOGIN_INCLUDE = 'config/ColumbiaLogin.inc';
$WBES_LOGIN_PREFIX = 'login/';

// path to page which does nothing but call LoginInstance()->handle()
// page will be used to display a login prompts, display error messages,
// and possibly authenticate and issue redirects
$WBES_LOGIN_HANDLER = '/wces/login/login.php';

// path to logout page
$WBES_LOGIN_LOGOUT = '/wces/login/logout.php';

// optional. can be set to a full path of a page on a secure server which
// does nothing but call LoginInstance()->bounce(). The page is used to process
// login's from insecure urls. Depending on the results of authentication
// it will redirect either to the destination url or to the handler page.
// This value is only used when the login prompt is viewed over an insecure
// connection.
$WBES_LOGIN_BOUNCER = 'https://oracle.seas.columbia.edu/wces/login/bouncer.php';

// optional. can be set to a full path of a page on a secure server which
// does nothing but call LoginInstance()->handle(). This value is only used when
// the login prompt is viewed over an insecure connection. It is used to
// make a link that the user can click to connect to the secure server before
// enering their password. Using the link doesn't provide any extra security,
// it just lets people a chance to see the lock icon. There isn't really any
// situation where it makes sense to set this and not set the bouncer, but in
// this case, this handler will be used by default to perform authentication.
$WBES_LOGIN_SECURE_HANDLER = 'https://oracle.seas.columbia.edu/wces/login/login.php';

$WBES_CONFIG_MEDIA_PATH = "{$WCES_CONFIG_PATH}media/";

$WCES_DISABLE_LINKS = false;

?>