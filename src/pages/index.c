#include "resources.h"

char* template_page="<html>"
		"<head><title>Dead Hand</title><link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" rel=\"stylesheet\">"
		"<style type=\"text/css\">%s</style><script type=\"text/javascript\">%s</script>"
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
	sprintf(page_index, template_page, static_style, static_javascript, content);
	return 0;
}



