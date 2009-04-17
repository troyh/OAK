#include <stdlib.h>

#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>

#include "Config.h"
#include "loginutils.h"

struct NAMEVAL_PAIR
{
	char* name;
	char* val;
};

typedef enum
{
	CREATELOGIN_MISSING_EMAILPASSWORD = 1,
	CREATELOGIN_USERIDINUSE = 2,
	CREATELOGIN_APPREJECTED = 3,
	CREATELOGIN_INTERNAL_ERROR = 4,
} CREATELOGIN_FAILED_REASON;

typedef enum
{
	LOGIN_MISSING_EMAILPASSWORD = 1,
	LOGIN_INVALID=2,
	LOGIN_APPREJECTED = 3,
} LOGIN_FAILED_REASON;

typedef enum
{
	POST_INVALIDUSER = 1,
	POST_APPREJECTED = 3,
} POST_FAILED_REASON;

typedef int (*CREATELOGIN_SUCCESS_FUNC)(const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*CREATELOGIN_FAILED_FUNC)(const char* userid, CREATELOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*LOGIN_SUCCESS_FUNC)(const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*LOGIN_FAILED_FUNC)(const char* userid, LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*LOGOUT_SUCCESS_FUNC)(const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*LOGOUT_FAILED_FUNC)(const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*POST_SUCCESS_FUNC)(const char* doctype,const char* userid, NAMEVAL_PAIR* post_data, size_t post_data_len, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
typedef int (*POST_FAILED_FUNC)(const char* doctype,const char* userid,POST_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

struct OAK_APPLIB_HANDLE
{
	void* dl_handle;
	char dl_filename[256];
	char dl_error[256];
	CREATELOGIN_SUCCESS_FUNC 	createlogin_success;
	CREATELOGIN_FAILED_FUNC 	createlogin_failed;
	LOGIN_SUCCESS_FUNC 	login_success;
	LOGIN_FAILED_FUNC 	login_failed;
	LOGOUT_SUCCESS_FUNC logout_success;
	LOGOUT_FAILED_FUNC 	logout_failed;
	POST_SUCCESS_FUNC 	post_success;
	POST_FAILED_FUNC 	post_failed;
	
	OAK_APPLIB_HANDLE();
};

#define OAK_MAX_DOCNAME_LENGTH 1024

void oak_load_app_library(OAK_APPLIB_HANDLE* h);
void oak_unload_app_library(OAK_APPLIB_HANDLE h);

int oak_app_createlogin_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
int oak_app_createlogin_failed(OAK_APPLIB_HANDLE h,const char* userid,CREATELOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

int oak_app_login_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
int oak_app_login_failed(OAK_APPLIB_HANDLE h,const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

int oak_app_logout_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
int oak_app_logout_failed(OAK_APPLIB_HANDLE h,const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

int oak_app_logout_success(OAK_APPLIB_HANDLE h,const char* userid, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
int oak_app_logout_failed(OAK_APPLIB_HANDLE h,const char* userid,LOGIN_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

int oak_app_post_success(OAK_APPLIB_HANDLE h,const char* doctype,const char* userid, NAMEVAL_PAIR* post_data, size_t post_data_len, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);
int oak_app_post_failed(OAK_APPLIB_HANDLE h,const char* doctype,const char* userid,POST_FAILED_REASON reason, NAMEVAL_PAIR** nv_pairs, size_t* nv_pairs_count);

typedef enum 
{
	OAK_OK=0,
	
	OAK_AUTH_USERID_NOT_FOUND,
	OAK_AUTH_INTERNAL_ERROR,
	OAK_AUTH_KEY_MISMATCH,
	OAK_AUTH_USERID_MISSING,
	
	OAK_VALIDATE_FIELD_NONEXISTENT,
	OAK_VALIDATE_NOVALUE,
	OAK_VALIDATE_BADVALUE,
	OAK_VALIDATE_BADTYPE,
	OAK_VALIDATE_FAILED,
	OAK_VALIDATE_BADFUNC,
	
	OAK_XSLT_BADXSL,
	OAK_XSLT_FAILED,

	OAK_STORE_WRITE_FAILED
	// NOTE: When adding a result code, add a string to s_result_strings in oak.cc!!
} OAK_RESULT;

typedef enum 
{
	OAK_DATATYPE_UINT=1,
	OAK_DATATYPE_FLOAT,
	OAK_DATATYPE_TEXT,
	OAK_DATATYPE_MONEY,
	OAK_DATATYPE_CUSTOM
} OAK_DATATYPE;

typedef bool (*OAK_VALIDATE_FIELD_FUNC)(const char* name, const char* value);

namespace OAK
{

class Exception : public std::exception
{
	OAK_RESULT m_result;
	std::string m_msg;
public:	
	Exception(OAK_RESULT result, const std::string& msg) throw();
	Exception(OAK_RESULT result, const xmlErrorPtr xmlerror) throw();
	Exception (const Exception&) throw();
	Exception& operator= (const Exception&) throw();
	~Exception() throw() {}
	const char* what() const throw() { return m_msg.c_str(); }
};

class OAK
{
	static const char* s_result_strings[];
	
	Config m_cfg;
	std::map<std::string, std::string> m_form_fields;
	std::map<std::string, std::string> m_form_fields_invalid;
	
	const char* skipws(const char* p);
public:
	OAK(const boost::filesystem::path& conffile);
	~OAK();
	
	OAK_RESULT auth_user();
	OAK_RESULT get_user_id(char* user_id,size_t user_id_size);
	OAK_RESULT store_document(const char* name, const xmlDocPtr xmldoc);
	xmlBufferPtr document_in_memory(const xmlDocPtr xmldoc);
	
	OAK_RESULT validate_field(const char* name, OAK_DATATYPE type, OAK_VALIDATE_FIELD_FUNC userfunc=NULL);
	OAK_RESULT add_field(const char* name, const char* value);

	const char* get_field_value(const char* field_name) const;

	unsigned int invalid_fields_count() const;
	const char* get_invalid_field_name(size_t n) const;
	const char* get_invalid_field_value(size_t n) const;

	OAK_RESULT xslt(const char* xslname, xmlDocPtr* result_doc, xmlDocPtr doc=NULL) const;
	
	const char* get_result_string(OAK_RESULT result) { return s_result_strings[result]; }
};

}

class NAMEVAL_PAIR_OBJ
{
	const NAMEVAL_PAIR* m_nvp;
	size_t m_len;
public:
	NAMEVAL_PAIR_OBJ(NAMEVAL_PAIR* nvp, size_t count)
		: m_nvp(nvp), m_len(count)
	{
	}
	
	const char* get(const char* name) const;
};
