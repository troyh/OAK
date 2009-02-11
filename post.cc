#include <fcgi_stdio.h>
extern "C"
{
#include <cgic.h>
}

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "loginutils.h"

using namespace std;

extern "C" void cgiInit() 
{
	srand(time(0));
}

extern "C" void cgiUninit() 
{
	
}

int cgiMain()
{
	// TODO: verify that the user's id can be trusted! If not, redirect to cgi-bin/login
	if (!userIsValidated())
	{
		return 0;
	}
	
	char* doctype="document";
	if (cgiPathInfo && strlen(cgiPathInfo))
	{
		doctype=cgiPathInfo+1; // +1 skips initial slash
		// Truncate at the first non-alphanumeric, underscore or hyphen char
		for(char* p = doctype; *p; ++p)
		{
			if (!isalnum(*p) && *p!='_' && *p!='-')
			{
				*p='\0';
				break;
			}
		}
		// TODO: verify that the doctype, which is supplied by the user, is controlled, i.e., not a security risk
	}
	
	char xmlFileLoc[256];
	sprintf(xmlFileLoc,"/tmp/post-%s-%lu-%d-%d.xml",doctype,time(0),getpid(),rand());
	FILE* xmlOut=FCGI_fopen(xmlFileLoc,"w");
	if (!xmlOut)
	{
		// TODO: handle this (500 Internal Server error?)
		cgiHeaderStatus(500,"File creation failed");
	}
	else
	{
		FCGI_fprintf(xmlOut,"<post>");
		FCGI_fprintf(xmlOut,"<cgi>");
	
		if (strlen(cgiAuthType))
			FCGI_fprintf(xmlOut,"<AuthType>%s</AuthType>",cgiAuthType);
		if (strlen(cgiRemoteHost))
			FCGI_fprintf(xmlOut,"<RemoteHost>%s</RemoteHost>",cgiRemoteHost);
		if (strlen(cgiRemoteAddr))
			FCGI_fprintf(xmlOut,"<RemoteAddr>%s</RemoteAddr>",cgiRemoteAddr);
		if (strlen(cgiRemoteIdent))
			FCGI_fprintf(xmlOut,"<RemoteIdent>%s</RemoteIdent>",cgiRemoteIdent);
		
		char** cookies;
		cgiCookies(&cookies);
		if (cookies[0])
		{
			FCGI_fprintf(xmlOut,"<cookies>");
			for(size_t i = 0; cookies[i]; ++i)
			{
				char buf[256];
				cgiCookieString(cookies[i],buf,sizeof(buf));
				FCGI_fprintf(xmlOut,"<%s>%s</%s>",cookies[i],buf,cookies[i]);
			}
			FCGI_fprintf(xmlOut,"</cookies>");
			
			cgiStringArrayFree(cookies);
		}
		FCGI_fprintf(xmlOut,"</cgi><doc><%s>",doctype);
		
		char content_type[256];
		cgiFormResultType res=cgiFormFileContentType("file",content_type,sizeof(content_type));
		if (res==cgiFormNoContentType || res==cgiFormNotFound)
		{
			char** fields;
			cgiFormEntries(&fields);
			for(size_t i = 0; fields[i]; ++i)
			{
				char buf[4096];
				cgiFormString(fields[i],buf,sizeof(buf));
				FCGI_fprintf(xmlOut,"<%s>%s</%s>",fields[i],buf,fields[i]);
			}
			cgiStringArrayFree(fields);
		}
		else if (!strcasecmp(content_type,"text/xml"))
		{
			// Write the XML doc as-is
			while (!FCGI_feof(cgiIn))
			{
				char buf[4096];
				size_t n=FCGI_fread(buf,sizeof(char),cgiContentLength<sizeof(buf)?cgiContentLength:sizeof(buf),cgiIn);
				FCGI_fwrite(buf,sizeof(char),n,xmlOut);
			}
		}
		else
		{
			// TODO: what to do?
			// cout << "Unknown content-type: " << content_type << endl;
		}

		FCGI_fprintf(xmlOut,"</%s></doc></post>",doctype);
		FCGI_fclose(xmlOut);

		/*
		 * Run the app's command to process it
		 */
		char command[512];
		char wwwroot[]="/var/www/"; // TODO: make the config'd
		char appprefix[]="/recipes/"; // TODO: make the config'd
		sprintf(command,"%s/%s/bin/post/%s %s",wwwroot,appprefix,doctype,xmlFileLoc);
		FILE* appcgi=popen(command,"r");
		if (appcgi)
		{
			// Read output of command, that's the XML used for the XSL
			
			int ret=pclose(appcgi);
			
			if (ret==0)
			{			
				/*
				 * Ok, generate output using "good" XSL stylesheet
				 */
			}
			else
			{
				/*
				 * Error, generate output using "bad" XSL stylesheet
				 */
			}
		}
		
		cgiHeaderContentType("text/xml");
		FCGI_printf("<post></post>");
		
	}
	
	return 0;
}

// Static Content
// Indexed
// Documents
// XML + XSL
// Application
// Webapp
// High-performance/Fast
// Reliable
// Simple
// 
// SIXD
// DXS
// SIDSX (Static Indexed Documents Served by XML)
// SIDXAWHPSFR
// SWAIDXHPSFR
// RADIX (Reliable App Documents Indexed XML/XSL)  

