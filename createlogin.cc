#include <fcgi_stdio.h>
extern "C"
{
#include <cgic.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gcrypt.h>

#include "oak.h"
#include "loginutils.h"

using namespace std;

struct CGIData
{
	char *cgiServerSoftware;
	char *cgiServerName;
	char *cgiGatewayInterface;
	char *cgiServerProtocol;
	char *cgiServerPort;
	char *cgiRequestMethod;
	char *cgiPathInfo;
	char *cgiPathTranslated;
	char *cgiScriptName;
	char *cgiQueryString;
	char *cgiRemoteHost;
	char *cgiRemoteAddr;
	char *cgiAuthType;
	char *cgiRemoteUser;
	char *cgiRemoteIdent;
	char *cgiContentType;
	char *cgiMultipartBoundary;
	char *cgiCookie;
	int cgiContentLength;
	char *cgiAccept;
	char *cgiUserAgent;
	char *cgiReferrer;
};

OAK_APPLIB_HANDLE applib;

extern "C" void cgiInit() 
{
	oak_load_app_library(&applib);
}

extern "C" void cgiUninit() 
{
	oak_load_app_library(&applib);
}

int cgiMain()
{
	bool bUserCreated=false;
	LOGININFO* login=NULL;
	const char* status_string="";
	
	char email[64];
	char password[64];
	char userid[MAX_USERID_LEN];
	
	NAMEVAL_PAIR* nv_pairs=NULL;
	size_t nv_pairs_len=0;
	
	cgiFormResultType email_r=cgiFormString((char*)"email",email,sizeof(email));
	cgiFormResultType password_r=cgiFormString((char*)"password",password,sizeof(password));
	cgiFormResultType userid_r=cgiFormString((char*)"userid",userid,sizeof(userid));
	
	if (email_r!=cgiFormSuccess || password_r!=cgiFormSuccess || userid_r!=cgiFormSuccess)
	{
		// Can't continue without those
		oak_app_createlogin_failed(applib,"",CREATELOGIN_MISSING_EMAILPASSWORD,&nv_pairs,&nv_pairs_len);
		status_string="Missing email, password and/or userid";
	}
	else
	{
		// Verify that the login doesn't already exist
		login=lookupLogin(email);
		if (login)
		{
			oak_app_createlogin_failed(applib,login->userid(),CREATELOGIN_USERIDINUSE,&nv_pairs,&nv_pairs_len);
			status_string="E-mail address already exists";
		}
		else
		{
			// Can create it, but first, call the app's function
			if (oak_app_createlogin_success(applib,userid,&nv_pairs,&nv_pairs_len))
			{
				oak_app_createlogin_failed(applib,userid,CREATELOGIN_APPREJECTED,&nv_pairs,&nv_pairs_len);
				status_string="Rejected";
			}
			else
			{
				char secret[SECRET_LENGTH];
				createSecret(secret,sizeof(secret));
			
				// Create the login (email, password, userid & secret)
				if (createLogin(email,password,userid,secret)==false)
				{
					oak_app_createlogin_failed(applib,userid,CREATELOGIN_INTERNAL_ERROR,&nv_pairs,&nv_pairs_len);
					status_string="Create failed";
				}
				else
				{
					// Set cookies as if they just logged-in
					login=lookupLogin(email);
					if (!login)
					{
						oak_app_createlogin_failed(applib,login->userid(),CREATELOGIN_INTERNAL_ERROR,&nv_pairs,&nv_pairs_len);
						status_string="Internal login error";
					}
					else
					{
						setLoginCookies(login);
						status_string="Login created";
						bUserCreated=true;
					}
				}
			}
		}
	}
	
	if (!bUserCreated)
	{
		cgiHeaderStatus(501,(char*)status_string);
	}
	else
	{
		// TODO: support JSON too
		cgiHeaderContentType((char*)"text/xml");
		FCGI_printf("<createlogin>");
	
		if (bUserCreated && login)
		{
			FCGI_printf("<userid>%s</userid>", login->userid());
			FCGI_printf("<email>%s</email>", login->email());
		}
		
		// Output app's name-value pairs
		for (size_t i=0;i<nv_pairs_len;++i)
		{
			// TODO: UTF-8 encode these
			if (nv_pairs[i].name && nv_pairs[i].val)
				FCGI_printf("<%s>%s</%s>", nv_pairs[i].name, nv_pairs[i].val, nv_pairs[i].name);
		}
		
		FCGI_printf("</createlogin>\n");
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
