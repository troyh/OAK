#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "oak.h"

OAK_APPLIB_HANDLE::OAK_APPLIB_HANDLE()
	: dl_handle(0),login_success(0),login_failed(0),logout_success(0),logout_failed(0)
{
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

		// If the app has a library available, load it
		struct stat sb;
		if (!stat(libfilename,&sb))
		{
			h->dl_handle=dlopen(libfilename,RTLD_LAZY);
			if (!h->dl_handle)
			{
				// TODO: handle this
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
