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
	
	// Take email and password, look them up and, if a valid login, set appropriate cookies
	char email[256];
	char password[256];
	
	cgiFormResultType email_r=cgiFormString((char*)"email",email,sizeof(email));
	cgiFormResultType password_r=cgiFormString((char*)"password",password,sizeof(password));
	if (email_r!=cgiFormSuccess || password_r!=cgiFormSuccess)
	{
		// Failed login
		oak_app_login_failed(applib,"",LOGIN_MISSING_EMAILPASSWORD,&nv_pairs,&nv_pairs_len);
		cgiHeaderStatus(400,(char*)"Missing email and/or password");
	}
	else
	{
		// Look up email & password in user list
		LOGININFO* login=lookupLogin(email);
		if (!login || strcmp(login->password(),password))
		{
			// Incorrect password, failed login
			oak_app_login_failed(applib,(login?login->userid():""),LOGIN_INVALID,&nv_pairs,&nv_pairs_len);
			cgiHeaderStatus(401,(char*)"Login failed");
		}
		else
		{
			// Successful login, create a new secret and set cookies
			char secret[SECRET_LENGTH];
			createSecret(secret,sizeof(secret));
			login->secret(secret);
			updateLogin(login);
			setLoginCookies(login);

			if (oak_app_login_success(applib,(login->userid()?login->userid():""),&nv_pairs,&nv_pairs_len))
			{
				oak_app_login_failed(applib,(login->userid()?login->userid():""),LOGIN_APPREJECTED,&nv_pairs,&nv_pairs_len);
				cgiHeaderStatus(501,(char*)"Login rejected");
			}
			else
			{
				// Generate XML message
				cgiHeaderContentType((char*)"text/xml");
				FCGI_printf("<login><userid>%s</userid>",login->userid());
			
				// Output app's name-value pairs
				for (size_t i=0;i<nv_pairs_len;++i)
				{
					// TODO: UTF-8 encode these
					if (nv_pairs[i].name && nv_pairs[i].val)
						FCGI_printf("<%s>%s</%s>", nv_pairs[i].name, nv_pairs[i].val, nv_pairs[i].name);
				}
			
				FCGI_printf("</login>");
			}
		}
	}

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
