#pragma once
#include "libCore.h"
#include "db\db.h"
#include "pqxx\pqxx"
#include "json.h"
namespace core
{
namespace db
{

class CDbTransaction;
class CDbNonTransaction;
class CDbConnection;

class CDbField : public IDbField
{
private:
	pqxx::field F;
public:
	CDbField(const pqxx::field & f);
	std::string asString();
	int asInt();
	unsigned int asUInt();
	__int64 asInt64();
	unsigned __int64 asUInt64();
	float asFloat();
	double asDouble();
};

class CDbResult : public IDbResult
{
private:
	pqxx::result R;
	pqxx::result::iterator cur;
public:
	CDbResult(CDbTransaction * t, const std::string & sql);
	CDbResult(CDbNonTransaction * t, const std::string & sql);

	int size();
	bool eof();
	void next();
	DbField field(int idx);
	DbField field(const std::string & field_name);
};

class CDbTransaction : public IDbTransaction
{
friend class CDbResult;
private:
	pqxx::work W;
public:
	CDbTransaction(CDbConnection * C);
	DbResult exec(const std::string & sql);
	void commit();
};

class CDbNonTransaction : public IDbNonTransaction
{
friend class CDbResult;
private:
	pqxx::nontransaction W;
public:
	CDbNonTransaction(CDbConnection * C);
	DbResult exec(const std::string & sql);
};

class CDbConnection : public IDbConnection
{
friend class CDbTransaction;
friend class CDbNonTransaction;
private:
	pqxx::lazyconnection C;
public:
	CDbConnection(const json::Value & connection_info);
	CDbConnection(const std::string & conn_str);
	DbTransaction createTransaction();
	DbNonTransaction createNonTransaction();
	bool isOpen();
	bool Open();
};



}
}