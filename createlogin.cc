extern "C"
{
#include <cgic.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include <gcrypt.h>

#include "loginutils.h"

using namespace std;

extern "C" void cgiInit() {}
extern "C" void cgiUninit() {}

int cgiMain()
{
	char email[256];
	char password[256];
	char userid[256];
	
	cgiFormResultType email_r=cgiFormString((char*)"email",email,sizeof(email));
	cgiFormResultType password_r=cgiFormString((char*)"password",password,sizeof(password));
	cgiFormResultType userid_r=cgiFormString((char*)"userid",userid,sizeof(userid));
	
	if (email_r!=cgiFormSuccess || password_r!=cgiFormSuccess)
	{
		// Can't continue without those
		cgiHeaderStatus(501,(char*)"Missing email and/or password");
	}
	else
	{
		// Verify that the login doesn't already exist
		LOGININFO* login=lookupLogin(email);
		if (login)
		{
			// TODO: offer another userid?
			cgiHeaderStatus(501,(char*)"Login already exists");
		}
		else
		{
			char secret[SECRET_LENGTH];
			createSecret(secret,sizeof(secret));
			
			// Create the login (email, password, userid & secret)
			createLogin(email,password,userid,secret);
			
			// Set cookies as if they just logged-in
			login=lookupLogin(email);
			if (!login)
			{
				cgiHeaderStatus(501,(char*)"Internal login error");
			}
			else
			{
				setLoginCookies(login);
				
				cgiHeaderStatus(200,(char*)"Login created");
				cgiHeaderContentType((char*)"text/xml");

				fprintf(cgiOut, "<login><userid>%s</userid></login>\n", login->userid);
			}
		}

	}
	
	return 0;
}
