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


int cgiMain()
{
	clearLoginCookies();
			
	// Generate XML message
	cgiHeaderContentType("text/xml");
	fprintf(cgiOut,"<login></login>");
	
	return 0;
}
