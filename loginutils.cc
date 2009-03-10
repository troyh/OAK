extern "C" 
{
	#include <cgic.h>
}
#include <fstream>
#include <gcrypt.h>
#include <libmemcached/memcached.h>

#include "loginutils.h"

using namespace std;


const size_t MAX_MEMCACHED_KEY_LEN=256;

LOGININFO::LOGININFO()
{
	memset(m_userid,0,sizeof(m_userid));
	memset(m_email,0,sizeof(m_email));
	memset(m_password,0,sizeof(m_password));
	memset(m_secret,0,sizeof(m_secret));
}

LOGININFO::LOGININFO(const char* p)
{
	memcpy(m_userid,p,size());
}

void LOGININFO::userid(const char* p)
{
	strncpy(m_userid,p,sizeof(m_userid)-1);
	m_userid[sizeof(m_userid)-1]='\0';
}

void LOGININFO::email(const char* p)
{
	strncpy(m_email,p,sizeof(m_email)-1);
	m_email[sizeof(m_email)-1]='\0';
}

void LOGININFO::password(const char* p)
{
	strncpy(m_password,p,sizeof(m_password)-1);
	m_password[sizeof(m_password)-1]='\0';
}

void LOGININFO::secret(const char* p)
{
	strncpy(m_secret,p,sizeof(m_secret)-1);
	m_secret[sizeof(m_secret)-1]='\0';
}



void getDomain(char* buffer,size_t bufsize)
{
	const char* lastdot=strrchr(cgiServerName,'.');
	if (!lastdot) // No dot at all, just an internal hostname, i.e., http://dev
	{
		buffer[0]='\0';
	}
	else // At least 1 dot, could be like either http://www.troyandgay.com, http://blah.blah.blah.troyandgay.com or http://troyandgay.com
	{
		// Walk backwards in cgiServerName to find the 2nd dot
		const char* p;
		for(p = lastdot-1; cgiServerName <= p; --p)
		{
			if (*p=='.')
			{
				break;
			}
		}
		++p; // We've gone too far, either past the start of cgiServerName or to the 2nd-to-last dot, so move back
	
		strncpy(buffer,".",bufsize); // make sure the first char is a dot
		strncat(buffer,p,bufsize);   // copy rest of domain
		buffer[bufsize-1]='\0';		// null-terminate it
	}
	
}

//
// Generate a secret string
//
void createSecret(char* buffer,size_t buffer_size)
{
	char alphabet[]="abcdefghijklmnopqrstuvwxyz0123456789";
	char nonce[SECRET_LENGTH];

	gcry_check_version(0);
	gcry_create_nonce(nonce,sizeof(nonce));
	for(size_t i = 0; i < sizeof(nonce); ++i)
	{
		buffer[i]=alphabet[((unsigned char)nonce[i]) % (sizeof(alphabet)-1)];
	}
	buffer[buffer_size-1]='\0';
}

bool createLogin(const char* email, const char* password, const char* userid, const char* secret)
{
	if (!email || !strlen(email) ||
		!password || !strlen(password) ||
		!userid || !strlen(userid) ||
		!secret || !strlen(secret))
		return false;

	bool bReturn=false;
	
	LOGININFO info;
	info.email(email);
	info.password(password);
	info.userid(userid);
	info.secret(secret);
	
	char key[MAX_MEMCACHED_KEY_LEN];
	strncpy(key,"oak/userdb/email/",sizeof(key)-1);
	key[sizeof(key)-1]='\0';
	strncat(key,email,sizeof(key)-strlen(key)-1);
	key[sizeof(key)-1]='\0';

	char key2[MAX_MEMCACHED_KEY_LEN];
	strncpy(key2,"oak/userdb/userid/",sizeof(key2)-1);
	key[sizeof(key2)-1]='\0';
	strncat(key2,userid,sizeof(key2)-strlen(key2)-1);
	key[sizeof(key2)-1]='\0';

	memcached_st* st=memcached_create(NULL);
	
	memcached_return r=memcached_server_add(st,"localhost",21201);
	if (r!=MEMCACHED_SUCCESS)
	{
	}
	else
	{
		r=memcached_add(st,key,strlen(key),info.ptr(),info.size(),0,0);
		if (r!=MEMCACHED_SUCCESS)
		{
		}
		else
		{
			r=memcached_add(st,key2,strlen(key2),info.ptr(),info.size(),0,0);
			if (r!=MEMCACHED_SUCCESS)
			{
				// TODO: Erase the one we just set
			}
			else
			{
				bReturn=true;
			}
		}
	}
	
	memcached_free(st);
	
	return bReturn;
}

bool updateLogin(LOGININFO* login)
{
	if (!login || 
	    !login->email()    || !strlen(login->email()) ||
		!login->password() || !strlen(login->password()) ||
		!login->userid()   || !strlen(login->userid()) ||
		!login->secret()   || !strlen(login->secret()))
		return false;
		
	bool bReturn=false;

	char key[MAX_MEMCACHED_KEY_LEN];
	strncpy(key,"oak/userdb/email/",sizeof(key)-1);
	key[sizeof(key)-1]='\0';
	strncat(key,login->email(),sizeof(key)-strlen(key)-1);
	key[sizeof(key)-1]='\0';

	char key2[MAX_MEMCACHED_KEY_LEN];
	strncpy(key2,"oak/userdb/userid/",sizeof(key2)-1);
	key[sizeof(key2)-1]='\0';
	strncat(key2,login->userid(),sizeof(key2)-strlen(key2)-1);
	key[sizeof(key2)-1]='\0';

	memcached_st* st=memcached_create(NULL);
	
	memcached_return r=memcached_server_add(st,"localhost",21201);
	if (r!=MEMCACHED_SUCCESS)
	{
		
	}
	else
	{
		r=memcached_set(st,key,strlen(key),login->ptr(),login->size(),0,0);
		if (r!=MEMCACHED_SUCCESS)
		{
		}
		else
		{
			r=memcached_set(st,key2,strlen(key2),login->ptr(),login->size(),0,0);
			if (r!=MEMCACHED_SUCCESS)
			{
				// TODO: Erase the one we just set
			}
			else
			{
				bReturn=true;
			}
		}
	}
	
	memcached_free(st);
	
	
	return bReturn;
}

LOGININFO* lookupLogin(const char* email)
{
	LOGININFO* login=NULL;

	memcached_st* st=memcached_create(NULL);
	if (st)
	{
		memcached_return r=memcached_server_add(st,"localhost",21201);
		if (r!=MEMCACHED_SUCCESS)
		{
		
		}
		else
		{
			size_t val_len;
			uint32_t flags;

			char key[MAX_MEMCACHED_KEY_LEN];
			strncpy(key,"oak/userdb/email/",sizeof(key)-1);
			key[sizeof(key)-1]='\0';
			strncat(key,email,sizeof(key)-strlen(key)-1);
			key[sizeof(key)-1]='\0';
	
			char* p=memcached_get(st,key,strlen(key),&val_len,&flags,&r);
			if (!p)
			{
			}
			else
			{
				login=new LOGININFO(p);
				free(p);
		
				if (val_len!=login->size())
				{
				}
			}
		}
	
		memcached_free(st);
	}
	
	return login;
}

LOGININFO* lookupLoginByUserId(const char* str)
{
	LOGININFO* login=NULL;

	memcached_st* st=memcached_create(NULL);
	if (st)
	{
		memcached_return r=memcached_server_add(st,"localhost",21201);
		if (r!=MEMCACHED_SUCCESS)
		{
			
		}
		else
		{
			size_t val_len;
			uint32_t flags;
	
			char key[MAX_MEMCACHED_KEY_LEN];
			strncpy(key,"oak/userdb/userid/",sizeof(key)-1);
			key[sizeof(key)-1]='\0';
			strncat(key,str,sizeof(key)-strlen(key)-1);
			key[sizeof(key)-1]='\0';
	
			char* p=memcached_get(st,key,strlen(key),&val_len,&flags,&r);
			if (!p)
			{
			}
			else
			{
				login=new LOGININFO(p);
				free(p);
		
				if (val_len!=login->size())
				{
				}
			}
		}
	
		memcached_free(st);
	}
	
	return login;
}

bool makeUserKey(LOGININFO* login,char* ipaddr,char* buffer,size_t buffer_size)
{
	if (buffer_size<USERKEY_LENGTH)
		return false;

	// Create the hash, which is an MD5 on of a concatenation of secretkey+userid+cgiRemoteAddr
	char to_be_hashed[256]="";
	strncat(to_be_hashed,login->secret(),sizeof(to_be_hashed));
	strncat(to_be_hashed,login->userid(),sizeof(to_be_hashed)-strlen(to_be_hashed));
	strncat(to_be_hashed,ipaddr,sizeof(to_be_hashed)-strlen(to_be_hashed));
	to_be_hashed[sizeof(to_be_hashed)-1]='\0';

	memset(buffer,0,buffer_size);
	
	gcry_check_version(0);
	gcry_md_hash_buffer(GCRY_MD_MD5,buffer,to_be_hashed,strlen(to_be_hashed));
	
	char alphabet[]="abcdefghijklmnopqrstuvwxyz0123456789";
	for(size_t i = 0,j=strlen(buffer); i < j; ++i)
	{
		buffer[i]=alphabet[((unsigned char)buffer[i]) % (sizeof(alphabet)-1)];
	}
	buffer[buffer_size-1]='\0';
	return true;
}

void setLoginCookies(LOGININFO* login)
{
	// Get domain
	char domain[256];
	getDomain(domain,sizeof(domain));

	char hash[USERKEY_LENGTH];
	makeUserKey(login,cgiRemoteAddr,hash,sizeof(hash));
		
	cgiHeaderCookieSetString((char*)"userid",(char*)login->userid(),86400*14,(char*)"/",domain);
	cgiHeaderCookieSetString((char*)"usrkey",hash  ,86400*14,(char*)"/",domain);
}

void clearLoginCookies()
{
	char domain[256];
	getDomain(domain,sizeof(domain));
	
	cgiHeaderCookieSetString((char*)"userid",(char*)"",-86400,(char*)"/",domain);
	cgiHeaderCookieSetString((char*)"usrkey",(char*)"",-86400,(char*)"/",domain);
}

bool userIsValidated()
{
	char userid[MAX_USERID_LEN];
	cgiCookieString((char*)"userid",userid,sizeof(userid));

	LOGININFO* login=lookupLoginByUserId(userid);
	if (!login)
	{
		return false;
	}

	char correct_usrkey[USERKEY_LENGTH];
	
	if (!makeUserKey(login,cgiRemoteAddr,correct_usrkey,sizeof(correct_usrkey)))
	{
		return false;
	}

	char userkey[USERKEY_LENGTH];
	cgiCookieString((char*)"usrkey",userkey,sizeof(userkey));
	// The cookie usrkey must match usrkey
	if (strcmp(userkey,correct_usrkey))
	{
		return false;
	}

	// OK, user is validated as well as we can
	return true;
}
