#include <fcgi_stdio.h>
extern "C"
{
#include <cgic.h>
}

#include <stdlib.h>
#include <string.h>

using namespace std;

extern "C" void cgiInit() { FCGI_printf("cgiInit()"); }
extern "C" void cgiUninit() { FCGI_printf("cgiUninit()"); }

int cgiMain()
{
	size_t count=0;
	
	FCGI_printf("Content-type: text/html\r\n\r\n");
	FCGI_printf("<html><head><title>Cookies</title></head><body>");

	FCGI_printf("<h1>Cookies</h1>\n");

	char** cookies;
	cgiCookies(&cookies);

	if (!cookies[0])
	{
		FCGI_printf("No cookies set\n");
	}
	else
	{
		for(size_t i = 0; cookies[i]; ++i)
		{
			char buf[256];
			cgiCookieString(cookies[i],buf,sizeof(buf));
			FCGI_printf("<div>%s:%s</div>", cookies[i], buf);
		}
		cgiStringArrayFree(cookies);
	}
	
	FCGI_printf("</body></html>");

	return EXIT_SUCCESS;
}
