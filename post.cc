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
	
	unsigned int status_code=501;
	const char* status_string=NULL;
	
	if (!userIsValidated())
	{
		oak_app_post_failed(applib,"","",POST_INVALIDUSER,&nv_pairs,&nv_pairs_len);

		status_code=401;
		status_string="User could not be validated.";
	}
	else
	{
		char* doctype=(char*)"generic";
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

		char userid[MAX_USERID_LEN];
		cgiCookieString((char*)"userid",userid,sizeof(userid));
		
		char content_type[256];
		cgiFormResultType res=cgiFormFileContentType((char*)"file",content_type,sizeof(content_type));
		if (res==cgiFormNoContentType || res==cgiFormNotFound)
		{
			char** fields;
			cgiFormEntries(&fields);
			
			// Count the number of fields and bytes needed for the data
			size_t total_fields=0;
			size_t total_bytes=0;
			while(fields[total_fields])
			{
				int n;
				cgiFormStringSpaceNeeded(fields[total_fields],&n);
				++total_fields;
				total_bytes+=n;
			}
				
			// Alloc enough space for total items
			char* buf=(char*)calloc(total_bytes,sizeof(char));
			if (!buf)
			{
				status_string="Internal error";
			}
			else
			{
				size_t buflen=0;
				NAMEVAL_PAIR* post_data=(NAMEVAL_PAIR*)calloc(total_fields,sizeof(NAMEVAL_PAIR));
				if (!post_data)
				{
					status_string="Internal error";
				}
				else
				{
					for(size_t i = 0; fields[i]; ++i)
					{
						cgiFormString(fields[i],&buf[buflen],total_bytes-buflen);
				
						post_data[i].name=fields[i];
						post_data[i].val=&buf[buflen];
					
						buflen+=strlen(&buf[buflen])+1;
					}
				
					if (oak_app_post_success(applib,doctype,userid,post_data,total_fields,&nv_pairs,&nv_pairs_len))
					{
						oak_app_post_failed(applib,doctype,"",POST_APPREJECTED,&nv_pairs,&nv_pairs_len);
						status_string="Rejected";
					}
				}
				
				free(buf);
			}
			
			cgiStringArrayFree(fields);
		}
		else if (!strcasecmp(content_type,"text/xml"))
		{
			status_string="XML files not supported";
			// TODO: support uploading of an XML doc
			// // Write the XML doc as-is
			// while (!FCGI_feof(cgiIn))
			// {
			// 	char buf[4096];
			// 	size_t n=FCGI_fread(buf,sizeof(char),cgiContentLength<sizeof(buf)?cgiContentLength:sizeof(buf),cgiIn);
			// 	FCGI_fwrite(buf,sizeof(char),n,xmlOut);
			// }
		}
		else
		{
			status_string="Unsupported content type";
		}
	}
	
	if (status_string)
		cgiHeaderStatus(status_code,(char*)status_string);
	else
	{
		// TODO: support JSON output too
		cgiHeaderContentType((char*)"text/xml");

		FCGI_printf("<post>");

		// Output app's name-value pairs
		for (size_t i=0;i<nv_pairs_len;++i)
		{
			// TODO: UTF-8 encode these
			if (nv_pairs[i].name && nv_pairs[i].val)
				FCGI_printf("<%s>%s</%s>", nv_pairs[i].name, nv_pairs[i].val, nv_pairs[i].name);
		}
	
		FCGI_printf("</post>");
		// FCGI_printf("<!-- Lib: %s -->", applib.dl_filename);
		// FCGI_printf("<!-- Error: %s -->", applib.dl_error);
		// FCGI_printf("<!-- %p -->", applib.post_success);
		// FCGI_printf("<!-- %p -->", applib.login_success);
		// FCGI_printf("<!-- %p -->", applib.logout_success);
		// FCGI_printf("<!-- %p -->", applib.createlogin_success);
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

