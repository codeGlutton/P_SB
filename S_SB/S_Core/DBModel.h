#pragma once

NAMESPACE_BEGIN(DBModel);

USING_SHARED_PTR(Column);
USING_SHARED_PTR(Index);
USING_SHARED_PTR(Table);
USING_SHARED_PTR(Procedure);

/************************
		DataType
*************************/

// 실제 mssql 오브젝트 타입 번호
enum class DataType
{
	None = 0,
	TinyInt = 48,
	SmallInt = 52,
	Int = 56,
	Real = 59,
	DateTime = 61,
	Float = 62,
	Bit = 104,
	Numeric = 108,
	BigInt = 127,
	VarBinary = 165,
	VarChar = 167,
	Binary = 173,
	NVarChar = 231,
};

/************************
		  Column
*************************/

class Column
{
public:
	// sql CREATE TABLE 문법 내부 칼럼 선언문 리턴
	xWString			CreateText();

public:
	xWString			name;
	int32				columnId = 0;
	DataType			type = DataType::None;
	xWString			typeText;
	int32				maxLength = 0;

	/* NULL */

	bool				nullable = false;

	/* IDENTITIY */

	bool				identity = false;
	int64				seedValue = 0;
	int64				incrementValue = 0;

	/* DEFAULT */

	xWString			defaultValue;
	xWString			defaultConstraintName;
};

/************************
		  Index
*************************/

enum class IndexType
{
	Clustered = 1,
	NonClustered = 2,
};

class Index
{
public:

	/* 인덱스 이름 작명 후 리턴 */

	xWString			GetUniqueName();
	xWString			CreateName(const xWString& tableName);
	
	// 인덱스 종류 sql형식 리턴
	xWString			GetTypeText();
	// 인덱스 옵션 sql형식 리턴
	xWString			GetKeyText();
	// 해당하는 칼럼들 sql형식 리턴
	xWString			CreateColumnsText();
	// 인덱스가 특정 칼럼을 활용하는가
	bool				DependsOn(const xWString& columnName);

public:
	xWString			name;
	int32				indexId = 0;
	IndexType			type = IndexType::NonClustered;
	bool				primaryKey = false;
	bool				uniqueConstraint = false;
	xVector<ColumnRef>	columns;
};

/************************
		  Table
*************************/

class Table
{
public:
	ColumnRef			FindColumn(const xWString& columnName);

public:
	int32				objectId = 0;
	xWString			name;
	xVector<ColumnRef>	columns;
	xVector<IndexRef>	indexes;
};

/************************
		Procedures
*************************/

class Param
{
public:
	xWString			name;
	xWString			type;
};

class Procedure
{
public:
	xWString			GenerateCreateQuery();
	xWString			GenerateAlterQuery();
	xWString			GenerateParamString();

public:
	xWString			name;

	/* DB */

	xWString			fullBody;

	/* XML */

	xWString			body;
	xVector<Param>		parameters;
};

/************************
		 Helpers
*************************/

class Helpers
{
public:
	static xWString		Format(const WCHAR* format, ...);
	static xWString		DataType2WString(DataType type);
	static xWString		RemoveWhiteSpace(const xWString& str);
	static DataType		WString2DataType(const WCHAR* str, OUT int32& maxLen);
};

NAMESPACE_END