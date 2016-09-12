#include <iostream>
#include "DBConfiguration.h"
#include "Poco/Stopwatch.h"

using namespace std;
int main(int argc, char *argv[])
{

    //DBKeyValueConfig ky;

    Poco::Stopwatch sw;

    DBConfiguration config("mydb.db",true);

    std::vector<std::string> akeys;

    config.keys(akeys);

    config.keys("ports",akeys);
    for(size_t i = 0; i < akeys.size(); i++)
    {
        std::string key = akeys[i];
        std::string value = config.getString("ports."+akeys[i]);
        printf("key=%s,value=%s\n",key.c_str(),value.c_str());
    }
    printf("%s ports.baud \n",config.has("ports.baud")?"has":"has not");
    printf("%s ports.name \n",config.has("ports.name")?"has":"has not");
    printf("%s ports.name \n",config.hasOption("ports.name")?"has":"has not");

    config.remove("ports.name");
    printf("%s ports.name \n",config.hasOption("ports.name")?"has":"has not");

    printf("name=%s\n",config.getString("ports.name","/dev/ttymxc0").c_str());
    printf("index=%d\n",config.getInt("ports.index",2));
    printf("num=%d\n",config.getInt("ports.num",3));
    printf("timeout=%0.2f\n",config.getDouble("ports.timeout",4.3));

//    printf("index=%d\n",config.getInt("ports.index"));
//    printf("num=%d\n",config.getInt("ports.num"));
//    printf("timeout=%0.2f\n",config.getDouble("ports.timeout"));

    //DBConfiguration config("mydb.db",true);
    sw.start();
    config.setInt("ports.index",1);
    config.setInt("ports.num",2);
    config.setBool("ports.enable",true);
    config.setDouble("ports.timeout",123.44);
    config.setString("ports.name","wwww");

    int count = 2000;
    for(int i = 0; i < count; i++)
    {
        std::string key = Poco::format("ports.index%d",i);
        config.setInt(key,1);
    }
    config.commit();
    sw.stop();

    cout << "write " << count << " params elaseped " << sw.elapsed()/1000 << " ms"<<  endl;

    sw.restart();

    for(int i = 0; i < count; i++)
    {
        std::string key = Poco::format("ports.index%d",i);
        config.getInt(key);
       // printf( " read %d=%d \n", i,config.getInt(key));
    }
    sw.stop();

    printf("index=%d\n",config.getInt("ports.index"));
    printf("num=%d\n",config.getInt("ports.num"));
    printf("timeout=%0.2f\n",config.getDouble("ports.timeout"));
    printf("name=%s\n",config.getString("ports.name","22222").c_str());
    cout << "read " << count << " params elaseped " << sw.elapsed()/1000 << " ms"<<  endl;
    return 0;
}
