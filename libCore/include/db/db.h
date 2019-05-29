#pragma once
#include "libCore.h"
#include <string>
#include "json.h"

namespace core
{
namespace db
{

class IDbConnection;
class IDbTransaction;
class IDbNonTransaction;
class IDbResult;
class IDbField;

using DbConnection = std::shared_ptr<IDbConnection>;
using DbTransaction = std::shared_ptr<IDbTransaction>;
using DbNonTransaction = std::shared_ptr<IDbNonTransaction>;
using DbResult = std::shared_ptr<IDbResult>;
using DbField = std::shared_ptr<IDbField>;

class IDbField
{
public:
	virtual ~IDbField() {}
	virtual std::string asString()=0;
	virtual int asInt()=0;
	virtual unsigned int asUInt()=0;
	virtual __int64 asInt64()=0;
	virtual unsigned __int64 asUInt64()=0;
	virtual float asFloat()=0;
	virtual double asDouble()=0;
};

class IDbResult 
{
public:
	virtual ~IDbResult() {}
	virtual int size()=0;
	virtual bool eof()=0;
	virtual void next()=0;
	virtual DbField field(int idx)=0;
	virtual DbField field(const std::string & field_name)=0;
};

class IDbNonTransaction
{
public:
	virtual ~IDbNonTransaction() {}
	virtual DbResult exec(const std::string & sql)=0;
};

class IDbTransaction : public IDbNonTransaction
{
public:
	virtual ~IDbTransaction() {}
	virtual void commit()=0;
};

class IDbConnection 
{
protected:
	std::string make_conn_str(const json::Value & connection);
public:
	virtual ~IDbConnection() {}
	virtual DbTransaction createTransaction()=0;
	virtual DbNonTransaction createNonTransaction()=0;
	virtual bool isOpen()=0;
	virtual bool Open()=0;
};


DLLEXPORT DbConnection getConnection(const json::Value & connection_info);
DLLEXPORT DbConnection getConnection(const std::string & conn_str);
}
}
