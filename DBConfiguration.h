#pragma once

#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Data/Session.h"
#include <map>
#include <istream>



#ifdef DBCONFIG_EXPORTS
#define DBCONFIG_API __declspec(dllexport)
#else
#define DBCONFIG_API
#endif

using Poco::Util::AbstractConfiguration;

class DBCONFIG_API DBConfiguration :
	public AbstractConfiguration
{
public:
    DBConfiguration(void);
	virtual ~DBConfiguration(void);
    void setDBPath(const std::string& path);
    DBConfiguration(const std::string& path,bool entrans=false);
    bool commit();


	/// Loads the configuration from the given stream
protected:

    bool getRaw(const std::string & key, std::string & value) const;
    void setRaw(const std::string& key, const std::string& value);
    void enumerate(const std::string& key, Keys& range) const;
    void removeRaw(const std::string& key);
private:

	bool existTable(const std::string& table) const;
    bool existDBKey(Poco::Data::Session& session,const std::string& table,const std::string& key);
	std::string _path;
	std::map<std::string,std::string>  _mapKeyValue;
    bool _entrans;

    void setDbRaw(Poco::Data::Session &session, const std::string &key, const std::string &value);
    Poco::Mutex _mutex;
};

