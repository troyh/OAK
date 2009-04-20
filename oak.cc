#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern "C" 
{
#include <cgic.h>
}

#include <libxml/xmlwriter.h>
#include <libxml/xmlsave.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>

#include "oak.h"

#define MAX_XSLT_PARAMS 16

namespace OAK
{

Exception::Exception(OAK_RESULT result, const std::string& msg) throw()
	: m_result(result), m_msg(msg)
{
}

Exception::Exception(OAK_RESULT result, const xmlErrorPtr xmlerror) throw()
	: m_result(result), m_msg(xmlerror?xmlerror->message:"Unknown XML error")
{
	if (xmlerror)
	{
		if (xmlerror->str1)
		{
			m_msg+=" ";
			m_msg+=xmlerror->str1;
		}
		if (xmlerror->str2)
		{
			m_msg+=" ";
			m_msg+=xmlerror->str2;
		}
		if (xmlerror->str3)
		{
			m_msg+=" ";
			m_msg+=xmlerror->str3;
		}
	}
}

Exception::Exception(const Exception& x) throw()
	: m_result(x.m_result), m_msg(x.m_msg)
{
}

Exception::Exception& Exception::operator=(const Exception& x) throw()
{
	m_result=x.m_result;
	m_msg=x.m_msg;
	return *this;
}


OAK::OAK(const boost::filesystem::path& conffile)
	: m_cfg(conffile)
{
}


OAK::~OAK()
{
}

OAK_RESULT OAK::auth_user()
{
	char userid[MAX_USERID_LEN];
	if (cgiCookieString((char*)"userid",userid,sizeof(userid))!=cgiFormSuccess)
	{
		return OAK_AUTH_USERID_MISSING;
	}

	LOGININFO* login=lookupLoginByUserId(userid);
	if (!login)
	{
		return OAK_AUTH_USERID_NOT_FOUND;
	}

	char correct_usrkey[USERKEY_LENGTH];
	
	if (!makeUserKey(login,cgiRemoteAddr,correct_usrkey,sizeof(correct_usrkey)))
	{
		return OAK_AUTH_INTERNAL_ERROR;
	}

	char userkey[USERKEY_LENGTH];
	cgiCookieString((char*)"usrkey",userkey,sizeof(userkey));
	// The cookie usrkey must match usrkey
	if (strcmp(userkey,correct_usrkey))
	{
		return OAK_AUTH_KEY_MISMATCH;
	}

	// OK, user is validated as well as we can
	return OAK_OK;
}

OAK_RESULT OAK::get_user_id(char* user_id,size_t user_id_size)
{
	char userid[MAX_USERID_LEN];
	if (cgiCookieString((char*)"userid",userid,sizeof(userid))!=cgiFormSuccess)
	{
		return OAK_AUTH_USERID_MISSING;
	}
	return OAK_OK;
}

OAK_RESULT OAK::store_document(const char* name, const xmlDocPtr xmldoc)
{
	boost::filesystem::path filename=m_cfg.get("DOC_DIR");
	filename/=name;
	
	try
	{
		boost::filesystem::path dirs=filename;
		dirs=dirs.remove_leaf();
		if (!is_directory(dirs))
			boost::filesystem::create_directories(dirs);
	}
	catch (...)
	{
		throw Exception(OAK_STORE_WRITE_FAILED,"Unable to create file path");
	}
	
	xmlSaveCtxtPtr save_context=xmlSaveToFilename(filename.string().c_str(),NULL,XML_SAVE_FORMAT);
	if (!save_context)
		throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());

	if (xmlSaveDoc(save_context,xmldoc)==-1)
	{
		xmlErrorPtr err=xmlGetLastError();
		xmlSaveClose(save_context);
		throw Exception(OAK_STORE_WRITE_FAILED,err);
	}
	
	if (xmlSaveClose(save_context)==-1)
		throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());

	return OAK_OK;
}

xmlBufferPtr OAK::document_in_memory(const xmlDocPtr xmldoc)
{
	xmlBufferPtr buffer=NULL;
	xmlSaveCtxtPtr save_context=NULL;
	
	try
	{
		buffer=xmlBufferCreate();
		if (!buffer)
			throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());
		
		save_context=xmlSaveToBuffer(buffer,"UTF-8",XML_SAVE_FORMAT);
		if (!save_context)
			throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());
			
		if (xmlSaveDoc(save_context,xmldoc)==-1)
			throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());
	
		if (xmlSaveClose(save_context)==-1)
			throw Exception(OAK_STORE_WRITE_FAILED,xmlGetLastError());
	}
	catch (...)
	{
		if (save_context)
			xmlSaveClose(save_context);
		if (buffer)
			xmlBufferFree(buffer);
		throw;
	}

	return buffer;
}

const char* OAK::get_field_value(const char* field_name) const
{
	std::map<std::string,std::string>::const_iterator itr=m_form_fields.find(field_name);
	if (itr==m_form_fields.end())
		return NULL;
	return itr->second.c_str();
}

const char* OAK::skipws(const char* p)
{
	if (p)
	{
		while (*p && isspace(*p))
			++p;
	}
	return p;
}

OAK_RESULT OAK::validate_field(const char* name, OAK_DATATYPE type, OAK_VALIDATE_FIELD_FUNC userfunc)
{
	OAK_RESULT res=OAK_OK;

	char buf[256];
	const char* value=buf;
	const char* orig_value=value;
	
	// TODO: support fields longer than 256 chars
	if (cgiFormString((char*)name,buf,sizeof(buf))!=cgiFormSuccess)
		res=OAK_VALIDATE_FIELD_NONEXISTENT;
	else
	{
		switch (type)
		{
		case OAK_DATATYPE_UINT:
			value=skipws(value);
			if (!value)
				res=OAK_VALIDATE_NOVALUE;
			while (*value)
			{
				if (!isdigit(*value))
				{
					res=OAK_VALIDATE_BADVALUE;
					break;
				}
				else
					++value;
			}
			break;
		case OAK_DATATYPE_MONEY:
			value=skipws(value);
			if (!value)
			{
				res=OAK_VALIDATE_NOVALUE;
				break;
			}
			else if (*value!='$')
			{
				res=OAK_VALIDATE_BADVALUE;
				break;
			}
			else
				++value;
			// Fall through to OAK_DATATYPE_FLOAT
		case OAK_DATATYPE_FLOAT:
			value=skipws(value);
			if (!value)
				res=OAK_VALIDATE_NOVALUE;
			else
			{
				while (*value)
				{
					if (!isdigit(*value) && *value!='.')
					{
						res=OAK_VALIDATE_BADVALUE;
						break;
					}
					++value;
				}
			}
			break;
		case OAK_DATATYPE_TEXT:
			break;
		default:
			res=OAK_VALIDATE_BADTYPE;
			break;
		}
	}

	if (res==OAK_OK)
	{
		// Call user's func (if one specified)
		if (userfunc && userfunc(name,value)!=OAK_OK)
			res=OAK_VALIDATE_FAILED;
		else
		{
			// Store value internally
			m_form_fields[name]=orig_value;
		}
	}
	else if (res!=OAK_VALIDATE_FIELD_NONEXISTENT)
	{
		// Store as an invalid field
		m_form_fields_invalid[name]=orig_value;
	}
	
	return OAK_OK;
}

OAK_RESULT OAK::add_field(const char* name, const char* value)
{
	m_form_fields[name]=value;
	return OAK_OK;
}

unsigned int OAK::invalid_fields_count() const
{
	return m_form_fields_invalid.size();
}

const char* OAK::get_invalid_field_name(size_t n) const
{
	std::map<std::string,std::string>::const_iterator itr=m_form_fields_invalid.begin();
	std::map<std::string,std::string>::const_iterator itr_end=m_form_fields_invalid.end();
	while (n && itr!=itr_end)
	{
		++itr;
		--n;
	}

	if (itr==itr_end) // Didn't find that many items
		return NULL;
		
	return itr->first.c_str();
}

const char* OAK::get_invalid_field_value(size_t n) const
{
	std::map<std::string,std::string>::const_iterator itr=m_form_fields_invalid.begin();
	std::map<std::string,std::string>::const_iterator itr_end=m_form_fields_invalid.end();
	while (itr!=itr_end && n)
	{
		++itr;
		--n;
	}

	if (itr==itr_end) // Didn't find that many items
		return NULL;
		
	return itr->second.c_str();
}

OAK_RESULT OAK::xslt(const char* xslname, xmlDocPtr* result_doc, xmlDocPtr doc)
{
	const char* params[(MAX_XSLT_PARAMS+1)*2+1]; // +1 for timestamp, +1 for terminating NULL

	size_t param_count=0;
	size_t params_buf_size=0;

	// First, count the params
	std::map<std::string,std::string>::const_iterator itr=m_form_fields.begin();
	std::map<std::string,std::string>::const_iterator itr_end=m_form_fields.end();
	while (itr!=itr_end)
	{
		params_buf_size+=itr->second.length()+3; // +3 for 2 quotes and a null-terminator
		++param_count;
		++itr;
	}
	
	if (param_count>MAX_XSLT_PARAMS)
		throw Exception(OAK_XSLT_FAILED,"Too many parameters");

	char* params_buf=(char*)xmlMemMalloc(params_buf_size);
	if (!params_buf)
		throw Exception(OAK_XSLT_FAILED,"Out of memory");
		
	xsltStylesheetPtr xsl=NULL;
	xsltTransformContextPtr ctxt=NULL;
	bool bMustFreeDoc=false;
		
	try 
	{
		memset(params_buf,0,params_buf_size);
	
		// Put form fields into params
		size_t pi=0;
		char* params_buf_ptr=params_buf;
		itr=m_form_fields.begin();
		itr_end=m_form_fields.end();
		while (itr!=itr_end)
		{
			strcat(params_buf_ptr,"'");
			strcat(params_buf_ptr,itr->second.c_str());
			strcat(params_buf_ptr,"'");
					
			params[pi++]=itr->first.c_str();
			params[pi++]=params_buf_ptr;
					
			params_buf_ptr+=strlen(params_buf_ptr)+1;
	
			itr++;
		}
		
		// Add timestamp
		time_t now=time(0);
		struct tm* now_tm=gmtime(&now);
		params[pi++]="timestamp";
		char now_str[24]="";
		sprintf(now_str,"'%04d-%02d-%02dT%02d:%02d:%02dZ'",now_tm->tm_year+1900,now_tm->tm_mon+1,now_tm->tm_mday,now_tm->tm_hour,now_tm->tm_min,now_tm->tm_sec);
		params[pi++]=now_str;
		
		// Add terminator
		params[pi]=NULL;
	
		xsltInit();
		xmlSubstituteEntitiesDefault(1);
		xmlLoadExtDtdDefaultValue=1;
	
		char xslfilename[256];
		sprintf(xslfilename,"%s/xsl/%s",m_cfg.get("APP_DIR"),xslname);
	
		xsl=xsltParseStylesheetFile((const xmlChar*)xslfilename);
		if (!xsl)
			throw Exception(OAK_XSLT_BADXSL,xmlGetLastError());
		
		if (!doc) // We weren't provided a doc...
		{
			// Create an empty XML doc
			doc=xmlParseDoc((xmlChar*)"<document/>");
			if (!doc)
				throw Exception(OAK_XSLT_FAILED,xmlGetLastError());
			bMustFreeDoc=true;
		}
		
		if (!doc)
			throw Exception(OAK_XSLT_FAILED,"No doc provided");
	
		ctxt=xsltNewTransformContext(xsl,doc);
		if (!ctxt)
			throw Exception(OAK_XSLT_FAILED,xmlGetLastError());
	
		*result_doc=xsltApplyStylesheetUser(xsl,doc,params,NULL,NULL,ctxt);
		if (!*result_doc)
			throw Exception(OAK_XSLT_FAILED,xmlGetLastError());
	}
	catch (...)
	{
		// Free alloc'd space
		xmlMemFree(params_buf);
		if (ctxt)
			xsltFreeTransformContext(ctxt);
		if (xsl)
			xsltFreeStylesheet(xsl);
		if (bMustFreeDoc && doc)
			xmlFreeDoc(doc);
			
		xsltCleanupGlobals();
		xmlCleanupParser();
		
		throw;
	}
	
	// Free alloc'd space
	xmlMemFree(params_buf);
	if (ctxt)
		xsltFreeTransformContext(ctxt);
	if (xsl)
		xsltFreeStylesheet(xsl);
	if (bMustFreeDoc && doc)
		xmlFreeDoc(doc);
		
	xsltCleanupGlobals();
	xmlCleanupParser();

	return OAK_OK;
}

} // End OAK namespace

OAK_APPLIB_HANDLE::OAK_APPLIB_HANDLE()
	: dl_handle(0),login_success(0),login_failed(0),logout_success(0),logout_failed(0)
{
	memset(dl_filename,0,sizeof(dl_filename));
}


void oak_load_app_library(OAK_APPLIB_HANDLE* h)
{
	// TODO: open multiple app libs, if they're found
	// TODO: first try to open lib letting the OS search the lib path
	
	char libfilename[256];
	if (!getcwd(libfilename,sizeof(libfilename)))
	{
		// TODO: handle this error
	}
	else
	{
		libfilename[sizeof(libfilename)-1]='\0';

		strncat(libfilename,"/libapp.so",sizeof(libfilename)-1);
		libfilename[sizeof(libfilename)-1]='\0';
		
		strncpy(h->dl_filename,libfilename,sizeof(h->dl_filename)-1);
		h->dl_filename[sizeof(h->dl_filename)-1]='\0';

		// If the app has a library available, load it
		struct stat sb;
		if (!stat(libfilename,&sb))
		{
			h->dl_handle=dlopen(libfilename,RTLD_LAZY);
			if (!h->dl_handle)
			{
				// TODO: handle this
				strncpy(h->dl_error,dlerror(),sizeof(h->dl_error));
				h->dl_error[sizeof(h->dl_error)-1]='\0';
			}
			else
			{
				h->createlogin_success=(CREATELOGIN_SUCCESS_FUNC)dlsym(h->dl_handle,"createlogin_success");
				h->createlogin_failed=(CREATELOGIN_FAILED_FUNC)dlsym(h->dl_handle,"createlogin_failed");
				
				h->login_success=(LOGIN_SUCCESS_FUNC)dlsym(h->dl_handle,"login_success");
				h->login_failed=(LOGIN_FAILED_FUNC)dlsym(h->dl_handle,"login_failed");

				h->logout_success=(LOGOUT_SUCCESS_FUNC)dlsym(h->dl_handle,"logout_success");
				h->logout_failed=(LOGOUT_FAILED_FUNC)dlsym(h->dl_handle,"logout_failed");

				h->post_success=(POST_SUCCESS_FUNC)dlsym(h->dl_handle,"post_success");
				h->post_failed=(POST_FAILED_FUNC)dlsym(h->dl_handle,"post_failed");
			}
		}
	}
}

void oak_unload_app_library(OAK_APPLIB_HANDLE h)
{
	// TODO: unload multiple app libs
	if (h.dl_handle)
		dlclose(h.dl_handle);
}

int oak_app_createlogin_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.login_success)
		h.createlogin_success(userid,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_createlogin_failed(OAK_APPLIB_HANDLE h,const char* userid,CREATELOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.login_failed)
		h.createlogin_failed(userid,reason,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_login_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.login_success)
		h.login_success(userid,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_login_failed(OAK_APPLIB_HANDLE h,const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.login_failed)
		h.login_failed(userid,reason,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_logout_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.logout_success)
		h.logout_success(userid,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_logout_failed(OAK_APPLIB_HANDLE h,const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.logout_failed)
		h.logout_failed(userid,reason,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_post_success(OAK_APPLIB_HANDLE h,const char* doctype,const char* userid, NAMEVAL_PAIR* post_data, size_t post_data_len, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.post_success)
		h.post_success(doctype,userid,post_data,post_data_len,nv_pairs,nv_pairs_count);
	return 0;
}

int oak_app_post_failed(OAK_APPLIB_HANDLE h,const char* doctype,const char* userid,POST_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count)
{
	// TODO: call multiple functions in each app lib
	if (h.post_failed)
		h.post_failed(doctype,userid,reason,nv_pairs,nv_pairs_count);
	return 0;
}

const char* NAMEVAL_PAIR_OBJ::get(const char* name) const
{
	for(size_t i = 0; i < m_len; ++i)
	{
		if (!strcmp(m_nvp[i].name,name))
			return m_nvp[i].val;
	}
	return NULL;
}

namespace OAK
{
// TODO: give English strings here:
const char* OAK::s_result_strings[]=
{
	"OAK_OK",
	
	"OAK_AUTH_USERID_NOT_FOUND",
	"OAK_AUTH_INTERNAL_ERROR",
	"OAK_AUTH_KEY_MISMATCH",
	"OAK_AUTH_USERID_MISSING",
	
	"OAK_VALIDATE_FIELD_NONEXISTENT",
	"OAK_VALIDATE_NOVALUE",
	"OAK_VALIDATE_BADVALUE",
	"OAK_VALIDATE_BADTYPE",
	"OAK_VALIDATE_FAILED",
	"OAK_VALIDATE_BADFUNC",
	
	"OAK_XSLT_BADXSL",
	"OAK_XSLT_FAILED",

	"OAK_STORE_WRITE_FAILED"
};

}
