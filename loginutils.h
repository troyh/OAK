
const size_t SECRET_LENGTH=32;
const size_t USERKEY_LENGTH=32;

struct LOGININFO
{
	char userid[64];
	char email[64];
	char password[64];
	char secret[SECRET_LENGTH];
	
	LOGININFO();
};

void createSecret(char* buffer,size_t buffer_size);
bool makeUserKey(LOGININFO* login,char* ipaddr,char* buffer,size_t buffer_size);
LOGININFO* lookupLogin(const char* email);
LOGININFO* lookupLoginByUserId(const char* str);
void getDomain(char* buffer,size_t bufsize);
bool createLogin(const char* email, const char* password, const char* userid, const char* secret);
bool updateLogin(LOGININFO* login);
void setLoginCookies(LOGININFO* login);
void clearLoginCookies();
bool userIsValidated();




