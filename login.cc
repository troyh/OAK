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
	// Take email and password, look them up and, if a valid login, set appropriate cookies
	char email[256];
	char password[256];
	
	cgiFormResultType email_r=cgiFormString("email",email,sizeof(email));
	cgiFormResultType password_r=cgiFormString("password",password,sizeof(password));
	if (email_r!=cgiFormSuccess || password_r!=cgiFormSuccess)
	{
		// Failed login
		cgiHeaderStatus(501,"Missing email and/or password");
	}
	else
	{
		// Look up email & password in user list
		LOGININFO* login=lookupLogin(email);
		if (!login || strcmp(login->password,password))
		{
			// Incorrect password, failed login
			cgiHeaderStatus(501,"Login failed");
		}
		else
		{
			// Successful login, create a new secret and set cookies
			char secret[SECRET_LENGTH];
			createSecret(secret,sizeof(secret));
			memcpy(login->secret,secret,sizeof(login->secret));
			updateLogin(login);
			setLoginCookies(login);
			
			// Generate XML message
			cgiHeaderContentType("text/xml");
			FCGI_printf("<login><userid>%s</userid></login>",login->userid);
		}
	}
	
	return 0;
}
