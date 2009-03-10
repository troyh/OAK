extern "C" 
{
	#include <cgic.h>
}
#include <fstream>
#include <gcrypt.h>

#include "loginutils.h"

using namespace std;


LOGININFO::LOGININFO()
{
	memset(userid,0,sizeof(userid));
	memset(email,0,sizeof(email));
	memset(password,0,sizeof(password));
	memset(secret,0,sizeof(secret));
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
	
	ofstream userdb("/tmp/userdb",ios::app);
	userdb << email << '\t'
		   << password << '\t'
		   << userid << '\t'
		   << secret << endl;
		
	return true;
}

bool updateLogin(LOGININFO* login)
{
	if (!login || 
	    !login->email    || !strlen(login->email) ||
		!login->password || !strlen(login->password) ||
		!login->userid   || !strlen(login->userid) ||
		!login->secret   || !strlen(login->secret))
		return false;
	
	ofstream userdb("/tmp/userdb",ios::app);
	userdb << login->email << '\t'
		   << login->password << '\t'
		   << login->userid << '\t'
		   << login->secret << endl;
		
	return true;
}

LOGININFO* lookupLogin(const char* email)
{
	string ea,password,userid,secret;
	ifstream userdb("/tmp/userdb");
	while (userdb.good())
	{
		userdb >> ea >> password >> userid >> secret;
		if (!strcmp(ea.c_str(),email))
		{
			LOGININFO* login=new LOGININFO;

			strncpy(login->email,ea.c_str(),sizeof(login->email));
			strncpy(login->password,password.c_str(),sizeof(login->password));
			strncpy(login->userid,userid.c_str(),sizeof(login->userid));
			strncpy(login->secret,secret.c_str(),sizeof(login->secret));
			
			login->email   [sizeof(login->email   )-1]='\0';
			login->userid  [sizeof(login->userid  )-1]='\0';
			login->password[sizeof(login->password)-1]='\0';
			login->secret  [sizeof(login->secret  )-1]='\0';
			
			return login;
		}
	}
	
	return NULL;
}

LOGININFO* lookupLoginByUserId(const char* str)
{
	string ea,password,userid,secret;
	ifstream userdb("/tmp/userdb");
	while (userdb.good())
	{
		userdb >> ea >> password >> userid >> secret;
		if (!strcmp(userid.c_str(),str))
		{
			LOGININFO* login=new LOGININFO;

			strncpy(login->email,ea.c_str(),sizeof(login->email));
			strncpy(login->password,password.c_str(),sizeof(login->password));
			strncpy(login->userid,userid.c_str(),sizeof(login->userid));
			strncpy(login->secret,secret.c_str(),sizeof(login->secret));
			
			login->email   [sizeof(login->email   )-1]='\0';
			login->userid  [sizeof(login->userid  )-1]='\0';
			login->password[sizeof(login->password)-1]='\0';
			login->secret  [sizeof(login->secret  )-1]='\0';
			
			return login;
		}
	}
	
	return NULL;
}

bool makeUserKey(LOGININFO* login,char* ipaddr,char* buffer,size_t buffer_size)
{
	if (buffer_size<USERKEY_LENGTH)
		return false;

	// Create the hash, which is an MD5 on of a concatenation of secretkey+userid+cgiRemoteAddr
	char to_be_hashed[256]="";
	strncat(to_be_hashed,login->secret,sizeof(to_be_hashed));
	strncat(to_be_hashed,login->userid,sizeof(to_be_hashed)-strlen(to_be_hashed));
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
		
	cgiHeaderCookieSetString((char*)"userid",login->userid,86400*14,(char*)"/",domain);
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
		// TODO: redirect to login
		return false;
	}

	// OK, user is validated as well as we can
	return true;
}
