char* template_page="<html>"
		"<head><title>Dead Hand</title><link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" rel=\"stylesheet\">"
		"<style type=\"text/css\"> \
		body { padding-top: 50px; } \
		.sub-header { padding-bottom: 10px;border-bottom: 1px solid #eee;} \
		bar-fixed-top {  border: 0;} \
		.sidebar {  display: none;} \
		@media (min-width: 768px) {.sidebar{position: fixed;top: 51px;bottom: 0;left: 0;z-index: 1000;display: block;padding: 20px;overflow-x: hidden; overflow-y: auto;background-color: #f5f5f5;border-right: 1px solid #eee;}} \
		.nav-sidebar {margin-right: -21px;margin-bottom: 20px;margin-left: -20px;} \
		.nav-sidebar > li > a {padding-right: 20px;padding-left: 20px;} \
		.nav-sidebar > .active > a,.nav-sidebar > .active > a:hover,.nav-sidebar > .active > a:focus {color: #fff;background-color: #428bca;} \
		.main {padding: 20px;} \
		@media (min-width: 768px) {.main {padding-right: 40px;padding-left: 40px;}} \
		.main .page-header {margin-top: 0;} \
		.placeholders {margin-bottom: 30px;text-align: center;} \
		.placeholders h4 {margin-bottom: 0;} \
		.placeholder {margin-bottom: 20px;} \
		.placeholder img {display: inline-block;border-radius: 50%;} \
		</style>"
		"</head>"
		"<body>"
		"<nav class=\"navbar navbar-inverse navbar-fixed-top\">"
		"<div class=\"container-fluid\">"
		"<div class=\"navbar-header\"><a class=\"navbar-brand\" href=\"#\">Dead Hand</a></div>"
		"<div id=\"navbar\" class=\"navbar-collapse collapse\">"
		"<ul class=\"nav navbar-nav navbar-right\"><!-- <li><a href=\"#\">Dashboard</a></li><li><a href=\"#\">Settings</a></li><li><a href=\"#\">Profile</a></li><li><a href=\"#\">Help</a></li> -->"
		"</ul></div></div></nav>"
		"<div class=\"container-fluid\">"
		" <div class=\"row\"><div class=\"col-sm-3 col-md-2 sidebar\">"
		"<ul class=\"nav nav-sidebar\">"
		"<li class=\"active\"><a href=\"#\">Overview <span class=\"sr-only\">(current)</span></a></li>"
		"<li><a href=\"/password\">Password</a></li>"
		"</ul></div>"
		"<div class=\"col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main\">"
		"%s"
		"</div></div></div>"
		"<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script><script type=\"text/javascript\" src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrfddap.min.js\"></script>"
		"</body>"
		"</html>";


int page_index(char* page_index, char* content){
	sprintf(page_index, template_page, content);
	return 0;
}



