
#include "db\CDbConnection.h"
#include "util.h"
#include <fstream>

using namespace std;
namespace core
{
namespace db
{

string IDbConnection::make_conn_str(const json::Value & connection_info)
{
	json::Value tmp;
	json::Reader reader;
	if (connection_info.isMember("host")  && connection_info.isMember("dbname") && connection_info.isMember("user"))
	{
		json::Value info = connection_info;
		if (!info.isMember("port"))
			info["port"] = 5432;
		if (!info.isMember("password"))
			info["password"] = "";
		return	"host="+info["host"].asString() +
				" port=" + to_string(info["port"].asInt()) +
				" dbname=" + info["dbname"].asString() +
				" user=" + info["user"].asString() +
				" password=" + info["password"].asString();
	}
	return "";
}

DLLEXPORT DbConnection getConnection(const json::Value & connection_info)
{
	return DbConnection(new CDbConnection(connection_info));
}

DLLEXPORT DbConnection getConnection(const std::string & conn_str)
{
	return DbConnection(new CDbConnection(conn_str));
}

}
}