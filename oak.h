#include <stdlib.h>

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
