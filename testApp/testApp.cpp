
#include <iostream>

#include "libCore.h"
#include "util.h"
#include "db/db.h"
using namespace std;
using namespace core::util;
using namespace core::db;
int main()
{
    std::cout << utf8(ws("Hello world!")) << endl; 
	
	string conn_str = "host=127.0.0.1 port=5432 dbname=postgres user=postgres password=foobar";
	DbConnection db = getConnection(conn_str);
	DbTransaction t = db->createTransaction();
	DbResult R = t->exec("SELECT * FROM pg_catalog.pg_user");
	while (R && !R->eof())
	{
		std::cout << R->field("usename")->asString().c_str() << endl;
		R->next();
	}
	Sleep(1000);


}

