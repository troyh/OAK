
const size_t SECRET_LENGTH=32;
const size_t USERKEY_LENGTH=32;
const size_t MAX_USERID_LEN=32;

class LOGININFO
{
	char m_userid[MAX_USERID_LEN];
	char m_email[64];
	char m_password[64];
	char m_secret[SECRET_LENGTH];
public:
	LOGININFO();
	LOGININFO(const char* p);

	const char* ptr() const { return m_userid; }
	size_t size() const { return sizeof(m_userid)+sizeof(m_email)+sizeof(m_password)+sizeof(m_secret); }
	
	void userid(const char* p);
	void email(const char* p);
	void password(const char* p);
	void secret(const char* p);

	const char* userid() const		{ return m_userid; }
	const char* email() const		{ return m_email; }
	const char* password() const	{ return m_password; }
	const char* secret() const		{ return m_secret; }
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




