#include <fstream>
#include <string.h>

#include "Config.h"

Config::Config(boost::filesystem::path path)
{
	readConfig(path);
}

const char* Config::get(const char* name) const
{
	std::map<std::string,std::string>::const_iterator itr=m_settings.find(name);
	if (itr==m_settings.end())
		return NULL;
	return itr->second.c_str();
}

void Config::readConfig(boost::filesystem::path path)
{
	std::ifstream f(path.string().c_str());
	while (f.good())
	{
		char buf[256];
		f.getline(buf,sizeof(buf));
		if (f.good())
		{
			char* p=buf;
			while (isspace(*p) && *p)
				++p;
			if (*p!='#') // a comment line
			{
				p=strchr(p,'=');
				if (p)
				{
					*p++='\0';
					m_settings[buf]=p;
				}
			}
		}
	}
}
