#include "DBConfiguration.h"
#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/SQLiteException.h"


#if 0
/#include "CWLogger.h"
#else
#define cw_error printf
#endif



using Poco::Data::Session;



using namespace Poco::Data::Keywords;

static bool init = false;
static void init_sqlite3()
{
    if(!init)
    {
        Poco::Data::SQLite::Connector::registerConnector();
        init = true;
    }
}
DBConfiguration::DBConfiguration(void):
    _entrans(false)
{
   init_sqlite3();
}


DBConfiguration::~DBConfiguration(void)
{
    if(_entrans)
    {
        commit();
    }
}
DBConfiguration::DBConfiguration(const std::string& path,bool entrans):
    _entrans(entrans)
{
    init_sqlite3();
	setDBPath(path);
    if(entrans)
    {

    }
}
void DBConfiguration::setDBPath(const std::string& path)
{
	_path = path;
}
void DBConfiguration::removeRaw(const std::string& key)
{
	Poco::StringTokenizer token(key,".");
	if(token.count() != 2) return;

	std::string table_name  = token[0];
	std::string table_key   = token[1];

	Session session (Poco::Data::SQLite::Connector::KEY, _path);
	try
	{
		session << "DELETE  FROM " << table_name << " WHERE key=?", use(table_key), now;
	}
	catch(Poco::Data::SQLite::SQLiteException& e)
	{
		cw_error("sql err %s\r\n",e.displayText().c_str());
	}

}

bool DBConfiguration::getRaw(const std::string & key, std::string & value) const
{
	Poco::StringTokenizer token(key,".");
	if(token.count() != 2) return false;
    std::string table_name = token[0];
    std::string table_key   = token[1];
    int count = 0;

	try
	{
        Session session (Poco::Data::SQLite::Connector::KEY, _path);



       session << "SELECT COUNT(*) FROM " << table_name << " WHERE key=?", into(count),useRef(table_key), now;

      if(count <= 0) return false;


		session << "SELECT value FROM " + table_name + " WHERE key=?", into(value),use(table_key), now;
        return true;
	}
	catch(Poco::Data::SQLite::SQLiteException& e)
	{
		cw_error("sql err %s\r\n",e.displayText().c_str());
		return false;
	}
    catch(Poco::Exception& e)
    {
        cw_error("getRaw err %s",e.displayText().c_str());
        return false;
    }
    catch(...)
    {
        cw_error("getRaw unkown err");
        return false;
    }
	
    return false;
}

bool DBConfiguration::existDBKey(Poco::Data::Session& session,const std::string& table,const std::string& key)
{
	int count = 0;

	try
	{
		session << "SELECT COUNT(*) FROM " << table << " WHERE key=?", into(count),useRef(key), now;
	}
	catch(Poco::Data::SQLite::SQLiteException& e)
	{
		cw_error("sql err %s\r\n",e.displayText().c_str());
		return false;
	}
	

	return (count>=1)?true:false;
}

void DBConfiguration::setRaw(const std::string& key, const std::string& value)
{
    if(_entrans)
    {
        Poco::Mutex::ScopedLock lock(_mutex);
        _mapKeyValue[key] = value;
    }
    else
    {
        Session session (Poco::Data::SQLite::Connector::KEY, _path);
        setDbRaw(session,key, value);
    }
}


void DBConfiguration::enumerate(const std::string& key, Keys& range) const
{
    if(key.empty())
    {
        cw_error("key empty");
        return;
    }
	Session session (Poco::Data::SQLite::Connector::KEY, _path);
	std::string table_name = key;
	try
	{
		session << "SELECT key FROM " << table_name, into(range), now;
	}
	catch(Poco::Data::SQLite::SQLiteException& e)
	{
        cw_error("sql err %s\r\n",e.displayText().c_str());
	}
	
}



bool DBConfiguration::existTable(const std::string& table) const
{
	int count = 0;
	try
	{
		Session session (Poco::Data::SQLite::Connector::KEY, _path);

		std::string sql;
		Poco::format(sql,"select count(*) from sqlite_master where type='table' and tbl_name='%s' ; ",table);
		session << sql,into(count), now;

	}
	catch(Poco::Data::SQLite::SQLiteException& e)
	{
		cw_error("sql err %s\r\n",e.displayText().c_str());
		return false;
	}
	return (count>0)?true:false;
	
}

void DBConfiguration::setDbRaw(Poco::Data::Session& session,const std::string& key, const std::string& value)
{
    Poco::StringTokenizer token(key,".");
    if(token.count() != 2) return;
    std::string table_name  = token[0];
    std::string table_key   = token[1];


    try
    {


        std::string sql;

        Poco::format(sql,"CREATE TABLE IF NOT EXISTS %s (key VARCHAR,value VARCHAR)",table_name);

        session << sql, now;


        if(existDBKey(session,table_name,table_key))
        {
            session << "UPDATE "<<table_name<<" SET value=? WHERE key = ?", useRef(value), useRef(table_key), now;
        }
        else
        {
            session << "INSERT INTO " + table_name +" VALUES(?,?)",use(table_key),useRef(value), now;
        }
    }
    catch(Poco::Data::SQLite::SQLiteException& e)
    {
        cw_error("sql err %s\r\n",e.displayText().c_str());
    }
    return;
}

bool DBConfiguration::commit()
{
    Poco::Mutex::ScopedLock lock(_mutex);
    try
    {
        if(_mapKeyValue.size() == 0)
            return false;

        Session session (Poco::Data::SQLite::Connector::KEY, _path);

        session.begin();

        for (std::map<std::string,std::string>::const_iterator it = _mapKeyValue.begin(); it != _mapKeyValue.end(); ++it)
        {
            std::string key = it->first;
            std::string value = it->second;
            setDbRaw(session,key, value);
        }

        session.commit();

        _mapKeyValue.clear();
    }
    catch(Poco::Data::SQLite::SQLiteException& e)
    {
        cw_error("sql err %s\r\n",e.displayText().c_str());
        return false;
    }
    return true;
}



void dbconfig_test()
{
	DBConfiguration config("cwConfig.db");

	config.setString("person.name","zhou");
	config.setInt("person.age",123);
	config.setBool("person.man",true);
	config.setDouble("person.tall",165.7);
	config.remove("ntp_time.devicetime");
	//config.setString("user.password","E10ADC3949BA59ABBE56E057F20F883E");
	Poco::Util::AbstractConfiguration::Keys keys;

	config.keys("person",keys);

	for(int i = 0 ;i < keys.size();i++)
	{
        cw_error("key[%d]=%s\r\n",i+1,keys[i].c_str());
	}
}
