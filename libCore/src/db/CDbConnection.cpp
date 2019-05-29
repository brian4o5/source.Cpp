
#include "db\CDbConnection.h"
#include <fstream>
#include "util.h"

// Wrapper classes for LIBPQXX DLL

using namespace std;
using namespace pqxx;
namespace core
{
	using namespace util;
namespace db
{

CDbField::CDbField(const pqxx::field & f)
try :F(f)
{
}
catch (const exception & e)
{
	AddLog("DbField exception: " + string(e.what()));
}
std::string CDbField::asString()
{
	return F.as<string>();
}
int CDbField::asInt()
{
	return F.as<int>();
}
unsigned int CDbField::asUInt()
{
	return F.as<unsigned int>();
}
__int64 CDbField::asInt64()
{
	return F.as<__int64>();
}
unsigned __int64 CDbField::asUInt64()
{
	return F.as<unsigned __int64>();
}
float CDbField::asFloat()
{
	return F.as<float>();
}
double CDbField::asDouble()
{
	return F.as<double>();
}

CDbResult::CDbResult(CDbTransaction * t, const string & sql)
try :R(t->W.exec(sql))
{
	cur = R.begin();
}
catch (const exception & e)
{
	AddLog("DbResult exception: " + string(e.what()));
}

CDbResult::CDbResult(CDbNonTransaction * t, const string & sql)
try :R(t->W.exec(sql))
{
	cur = R.begin();
}
catch (const exception & e)
{
	AddLog("DbResult exception: " + string(e.what()));
}

int CDbResult::size()
{
	return R.size();
}
bool CDbResult::eof()
{
	return ( cur == R.end());
}
void CDbResult::next()
{
	cur++;
}

DbField CDbResult::field(int idx)
{
	return DbField( new CDbField((*cur)[idx]) );
}

DbField CDbResult::field(const std::string & field_name)
{
	return DbField( new CDbField((*cur)[field_name]));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

CDbConnection::CDbConnection(const json::Value & connection_info)
	: C(make_conn_str(connection_info))
{ }

CDbConnection::CDbConnection(const std::string & conn_str)
	: C(conn_str)
{ }


DbTransaction CDbConnection::createTransaction()
{
	try
	{
		return DbTransaction(new CDbTransaction(this));
	}
	catch (const exception & e)
	{
		AddLog("CDbConnection::createTransaction() : " + string(e.what()));
	}
	return DbTransaction(NULL);
}

DbNonTransaction CDbConnection::createNonTransaction()
{
	try
	{
		return DbNonTransaction(new CDbNonTransaction(this));
	}
	catch (const exception & e)
	{
		AddLog("CDbConnection::createNonTransaction() : " + string(e.what()));
	}
	return DbNonTransaction(NULL);
}

bool CDbConnection::isOpen()
{
	return C.is_open();
}

bool CDbConnection::Open()
{
	try
	{
		C.activate();
	}
	catch (const exception & e)
	{
		AddLog("DbConnection::Open() - " + string(e.what()));
	}

	return C.is_open();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

CDbTransaction::CDbTransaction(CDbConnection * C) : W(C->C) { }
void CDbTransaction::commit()
{
	W.commit();
}
DbResult CDbTransaction::exec(const string & sql)
{
	return DbResult(new CDbResult(this, sql));
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

CDbNonTransaction::CDbNonTransaction(CDbConnection * C) : W(C->C) { }
DbResult CDbNonTransaction::exec(const string & sql)
{
	return DbResult(new CDbResult(this, sql));
}




}
}