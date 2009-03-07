#include <fcgi_stdio.h>
extern "C"
{
#include <cgic.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "loginutils.h"

using namespace std;

extern "C" void cgiInit() 
{
	
}

extern "C" void cgiUninit() 
{
	
}

int cgiMain()
{
	clearLoginCookies();
			
	// Generate XML message
	cgiHeaderContentType((char*)"text/xml");
	FCGI_printf("<login></login>");
	
	return 0;
}
