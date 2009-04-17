#include <map>
#include <string>

#include <boost/filesystem.hpp>

class Config
{
	std::map<std::string,std::string> m_settings;
	
	void readConfig(boost::filesystem::path path);
public:
	Config(boost::filesystem::path path);
	~Config() {}
	
	const char* get(const char* name) const;
};
