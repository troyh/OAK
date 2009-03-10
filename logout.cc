#include <fcgi_stdio.h>
extern "C"
{
#include <cgic.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "oak.h"
#include "loginutils.h"

using namespace std;

OAK_APPLIB_HANDLE applib;

extern "C" void cgiInit() 
{
	oak_load_app_library(&applib);
}

extern "C" void cgiUninit() 
{
	oak_unload_app_library(applib);
}

int cgiMain()
{
	NAMEVAL_PAIR* nv_pairs=NULL;
	size_t nv_pairs_len=0;

	clearLoginCookies();
	
	oak_app_logout_success(applib,"",&nv_pairs,&nv_pairs_len);
			
	// Generate XML message
	cgiHeaderContentType((char*)"text/xml");
	FCGI_printf("<login>");
	
	// Output app's name-value pairs
	for (size_t i=0;i<nv_pairs_len;++i)
	{
		// TODO: UTF-8 encode these
		if (nv_pairs[i].name && nv_pairs[i].val)
			FCGI_printf("<%s>%s</%s>", nv_pairs[i].name, nv_pairs[i].val, nv_pairs[i].name);
	}

	FCGI_printf("</login>");

	if (nv_pairs)
	{
		for(size_t i = 0; i < nv_pairs_len; ++i)
		{
			free(nv_pairs[i].name);
			free(nv_pairs[i].val);
		}
		free(nv_pairs);
	}
	
	return 0;
}
