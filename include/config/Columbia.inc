<?

// Web server software PHP is running under, only used where web servers
// assign environment variables and handle redirects differently
// Possible values: apache, iis-cgi, iis-isapi
$WBES_SERVER = "apache";

// Whether or this installation is being used for testing and development
// or if it's being accessed by real users. If set to true, it intercepts
// any outgoing mail and redirects it to the developer address to prevent
// test mail from going to real users. Also enables detailed error
// messages on SQL errors
$WBES_DEBUG = true;

// Whether to send warning emails to the developer address whenever there
// is a runtime error to make it easier to debug problems
$WBES_DEBUG_WARNINGS = true;

// Name of wbes database
$WBES_DATABASE_NAME = "wces";

// Email address to send feedback form responses to. Also used as default
// reply-to address in sent emails
$WBES_FEEDBACK = "wces@columbia.edu";

// Developer email address. This is the address test and warning emails are
// sent to (see SERVER_DEBUG). Feedback form responses are also BCCed to this
// address
$WBES_DEVELOPER = "rey4@columbia.edu";

// URL to page which does nothing but call LoginInstance()->handle(). The
// page will be used to display login prompts, display error messages,
// and possibly authenticate and issue redirects.
$WBES_LOGIN_HANDLER = "/wces/login/login.php";

// URL to logout page
$WBES_LOGIN_LOGOUT = "/wces/login/logout.php";

// Optional. URL to a page on an SSL server which does nothing but call
// LoginInstance()->bounce(). Depending on the results of authentication it
// will redirect either to the destination url or to the handler page. If this
// value is set and the site is being accessed through a non-SSL connection,
// the login form will submit to this location, so the username and password
// will be transmitted via SSL, but all other traffic will stay unencrypted.
// If the site is already being accessed through an SSL connection, this value
// is ignored and the login form submits to $WBES_LOGIN_HANDLER.
$WBES_LOGIN_BOUNCER = "https://oracle.seas.columbia.edu/wces/login/bouncer.php";

// Optional. URL to a page on an SSL server which does nothing but call
// LoginInstance()->handle(). This value is only used when the login prompt is
// viewed over an non-SSL connection. It is shown as a link that the user
// can click to view the login prompt over an SSL connection, giving them 
// a chance to to see the lock icon and examine the certificate before
// entering their password. This value is also used as a default value
// for WBES_LOGIN_BOUNCER if for some reason that variable is unset.
$WBES_LOGIN_SECURE_HANDLER = "https://oracle.seas.columbia.edu/wces/login/login.php";

// When site is being viewed over a non-SSL connection and WBES_LOGIN_BOUNCER
// is set, determines whether to display a link to a login form that submits
// username and password over a non-SSL connection, which might be useful to
// users with browsers that can't do SSL.
$WBES_LOGIN_SSL_ONLY = true;

// String to prepend to login session variable names
$WBES_LOGIN_PREFIX = "login/";

// Include path and class name for login-processing class
$WBES_LOGIN_INCLUDE = "config/ColumbiaLogin.inc";
$WBES_LOGIN = "ColumbiaLogin";

// Include path and class name for page-rendering class
$WCES_CONFIG_PAGE_INCLUDE = "config/ColumbiaPage.inc";
$WCES_CONFIG_PAGE = "ColumbiaPage";

// URLs to WCES locations
$WCES_CONFIG_PATH = "/wces/";
$WCES_CONFIG_MEDIA_PATH = "{$WCES_CONFIG_PATH}media/";
$WCES_CONFIG_HELP_PATH = "{$WCES_CONFIG_PATH}help.php";

// URL and physical path to upload directory
$WCES_CONFIG_UPLOAD_PATH = "/wces/upload/";
$WCES_CONFIG_UPLOAD_PHYSICAL_PATH = "/home/httpd/apps/wces/www/wces/upload/";

// Include path for a file that will initialize $WCES_CONFIG_FACTORIES
// to an array of factory objects for all survey-component types supported
// by this installation. $WCES_CONFIG_FACTORIES is defined there instead
// of here because it can be expensive to load and instantiate all those
// factories
$WCES_CONFIG_FACTORIES_INCLUDE = "config/survey.inc";

// Whether or not to enable Columbia-specific hacks in the code. This enables
// features that rely on hardcoded database ids from the Columbia database
// and code to integrate survey results from an old MySQL database
$WCES_COLUMBIA = true;

// Whether to disable links for filling out surveys. This is a convenient
// toggle to temporarily disable survey taking during a question period
// without taking down the whole site or interrupting users who are in
// the middle of filling out a survey.
$WCES_DISABLE_LINKS = false;

// URL where Oracle lives
$ORACLE_PATH = "/oracle/";

?>
