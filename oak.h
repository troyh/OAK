#define NO_FCGI_DEFINES
#include <fcgi_stdio.h>
extern "C" 
{
#include <cgic.h>
}

#include <stdlib.h>

#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>

#include <Thrudoc.h>

#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>

#include "Config.h"
#include "loginutils.h"

using namespace thrudoc;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

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
	
	OAK_INVALID_DATATYPE,
	
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
	OAK_VALIDATE_NOFIELDS,
	
	OAK_XSLT_BADXSL,
	OAK_XSLT_FAILED,

	OAK_STORE_WRITE_FAILED,
	OAK_MEM_ALLOC_FAILED
	// NOTE: When adding a result code, add a string to s_result_strings in oak.cc!!
} OAK_RESULT;

typedef enum 
{
	OAK_DATATYPE_UNKNOWN=0,
	OAK_DATATYPE_UINT16=1,
	OAK_DATATYPE_UINT32,
	OAK_DATATYPE_UINT64,

	OAK_DATATYPE_INT16,
	OAK_DATATYPE_INT32,
	OAK_DATATYPE_INT64,

	OAK_DATATYPE_FLOAT,
	OAK_DATATYPE_TEXT,
	OAK_DATATYPE_MONEY,
	OAK_DATATYPE_BOOL
} OAK_DATATYPE;

union OAK_CGIFIELD_VALUE_TYPE
{
	uint8_t uint8;
	uint16_t uint16;
	uint32_t uint32;
	uint64_t uint64;
	int8_t int8;
	int16_t int16;
	int32_t int32;
	int64_t int64;
	const char* p;
	double dbl;
	double money;
	bool boolean;
	
	OAK_CGIFIELD_VALUE_TYPE() : uint64(0) {}
	OAK_CGIFIELD_VALUE_TYPE(const OAK_CGIFIELD_VALUE_TYPE& vt) : uint64(vt.uint64) {}
	OAK_CGIFIELD_VALUE_TYPE& operator=(const OAK_CGIFIELD_VALUE_TYPE& vt) { uint64=vt.uint64; }
	
	operator const char*()  {return p;}
	operator uint64_t()		{return uint64;}
	operator int64_t()		{return int64;}
	operator uint32_t()		{return uint32;}
	operator int32_t()		{return int32;}
	operator uint16_t()		{return uint16;}
	operator int16_t()		{return int16;}
	operator uint8_t()		{return uint8;}
	operator int8_t()		{return int8;}
	operator double()		{return dbl;}
	operator bool()			{return boolean;}
};

typedef OAK_RESULT (*OAK_VALIDATE_FIELD_FUNC)(const char* name, const char* value, OAK_CGIFIELD_VALUE_TYPE converted_value);

struct OAK_CGI_FIELD
{
	const char* field_name;
	unsigned int flags;
	OAK_DATATYPE type;
	int min;
	int max;
	OAK_VALIDATE_FIELD_FUNC validate_func;
	bool isset;
	bool validated;
	const char* original_value;
	OAK_CGIFIELD_VALUE_TYPE converted_value;
	
	OAK_CGI_FIELD(const char* _field_name=0,unsigned int _flags=0,OAK_DATATYPE _type=OAK_DATATYPE_UNKNOWN,int _min=0,int _max=0,OAK_VALIDATE_FIELD_FUNC _validate_func=0)
		: field_name(_field_name),flags(_flags),type(_type),min(_min),max(_max),validate_func(_validate_func),isset(false),validated(false),original_value(0) {}
	
};

// General CGI flags (must be bits, not sequential)
const int OAK_CGI_REQUIRE_USERID=1;

// CGI Field flags (must be bits, not sequential)
const int OAK_CGIFIELD_REQUIRED=1;


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
	
	char* m_variables_buffer;
	size_t m_buffer_size;
	
	const char* skipws(const char* p);
	void load_cgi_fields();
	OAK_CGI_FIELD* find_field(const char* field_name) const;
	
public:

	static const char* conf_file;
	static unsigned int cgi_flags;
	static OAK_CGI_FIELD cgi_fields[];

	OAK(const boost::filesystem::path& conffile);
	~OAK();
	
	OAK_RESULT auth_user();
	OAK_RESULT get_user_id(char* user_id,size_t user_id_size);
	OAK_RESULT store_document(const char* name, const xmlDocPtr xmldoc);
	xmlBufferPtr document_in_memory(const xmlDocPtr xmldoc);
	
	OAK_RESULT validate_field(size_t n);
	OAK_RESULT add_field(const char* name, const char* value);

	OAK_CGIFIELD_VALUE_TYPE get_field_value(const char* field_name) const;
	bool field_exists(const char* field_name) const;

	unsigned int invalid_fields_count() const;
	const char* get_invalid_field_name(size_t n) const;
	const char* get_invalid_field_value(size_t n) const;

	OAK_RESULT xslt(const char* xslname, xmlDocPtr* result_doc, xmlDocPtr doc=NULL);
	
	const char* get_result_string(OAK_RESULT result) { return s_result_strings[result]; }
	
	template<typename A>
	void assign_to_thriftobj(A& attr, const char* field_name)
	{
		if (this->field_exists(field_name))
			attr=(A)(this->get_field_value(field_name));
	}
	
	template<typename THRIFT_OBJ>
	void store_object(const char* bucket, const char* id, THRIFT_OBJ& obj)
	{
		boost::shared_ptr<TSocket> socket(new TSocket("localhost",9091));
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		ThrudocClient client(protocol);
		transport->open();
		
		boost::shared_ptr<TMemoryBuffer> mbuf(new TMemoryBuffer());
		TBinaryProtocol* mbuf_protocol=new TBinaryProtocol(mbuf);
		//Convert object to string
		obj.write(mbuf_protocol);
		std::string obj_str = mbuf->getBufferAsString();

		client.put(bucket, id, obj_str);
	}

	template<typename THRIFT_OBJ>
	void get_object(const char* bucket, const char* id, THRIFT_OBJ& obj)
	{
		boost::shared_ptr<TSocket> socket(new TSocket("localhost",9091));
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		ThrudocClient client(protocol);
		transport->open();

		std::string obj_str;
		std::string bucket_str(bucket);
		std::string id_str(id);
		client.get(obj_str, bucket_str, id_str);

		boost::shared_ptr<TMemoryBuffer> mbuf(new TMemoryBuffer());
		TBinaryProtocol* mbuf_protocol=new TBinaryProtocol(mbuf);
		//Convert string to object
		mbuf->write((uint8_t*)obj_str.c_str(),obj_str.length());
		obj.read(mbuf_protocol);
	}
	
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

int oakInit();
int oakUninit();
bool oakException(OAK::Exception&);
int oakMain(OAK::OAK&);
