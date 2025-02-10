#include "pch.h"
#include "DBConnection.h"

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	// 환경 핸들로 새로운 DB 연결 핸들 생성
	if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS)
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	// 실제 DB 연결
	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT & resultStringLen,
		SQL_DRIVER_NOPROMPT
	);

	// 연결 핸들과 사용될 상태 핸들 생성
	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
		return false;

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnection::Clear()
{
	if (_connection != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}

	if (_statement != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query)
{
	// 쿼리 실행
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}

bool DBConnection::Fetch()
{
	// 다음 커서로 간 후, 한 행 긁기
	SQLRETURN ret = SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		HandleError(ret);
		return false;
	default:
		return true;
	}
}

int32 DBConnection::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT & count);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::Unbind()
{
	// 칼럼 버퍼 바인딩 삭제
	::SQLFreeStmt(_statement, SQL_UNBIND);
	// 파라미터 바인딩 삭제
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	// sql 결과문 삭제
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(int32 paramIndex, bool* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, sizeof(bool), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, float* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, double* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int8* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, sizeof(int8), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int16* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, sizeof(int16), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int32* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, sizeof(int32), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, int64* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, sizeof(int64), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index)
{
	SQLLEN size = static_cast<SQLLEN>((::wcslen(str) + 1) * 2);
	*index = SQL_NTSL;

	if (size > WVARCHAR_MAX)
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str, index);
	else
		return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str, index);
}

bool DBConnection::BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index)
{
	if (bin == nullptr)
	{
		*index = SQL_NULL_DATA;
		size = 1;
	}
	else
		*index = size;

	if (size > BINARY_MAX)
		return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*)bin, index);
	else
		return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*)bin, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, bool* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_TINYINT, sizeof(bool), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, float* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_FLOAT, sizeof(float), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, double* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_DOUBLE, sizeof(double), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, int8* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_TINYINT, sizeof(int8), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, int16* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_SHORT, sizeof(int16), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, int32* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_LONG, sizeof(int32), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, int64* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_SBIGINT, sizeof(int64), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_TYPE_TIMESTAMP, sizeof(TIMESTAMP_STRUCT), value, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, WCHAR* str, int32 size, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_WCHAR, size, str, index);
}

bool DBConnection::BindCol(int32 clolumnIndex, BYTE* bin, int32 size, SQLLEN* index)
{
	return BindCol(clolumnIndex, SQL_C_BINARY, size, bin, index);
}

/* Private */

bool DBConnection::BindParam(SQLSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLLEN len, SQLPOINTER ptr, SQLLEN* index)
{
	// 쿼리 문 '?' 파라미터 연결될 값 등록
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

bool DBConnection::BindCol(SQLSMALLINT columnIndex, SQLSMALLINT cType, SQLLEN len, SQLPOINTER value, SQLLEN* index)
{
	// 쿼리 문 결과 값 저장될 버퍼 등록
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
	{
		HandleError(ret);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
	if (ret == SQL_SUCCESS)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0 };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0 };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errRet = 0;

	while (true)
	{
		// 에러 파악
		errRet = ::SQLGetDiagRecW(
			SQL_HANDLE_STMT,
			_statement,
			index,
			sqlState,
			OUT & nativeErr,
			errMsg,
			_countof(errMsg),
			OUT & msgLen
		);

		// 더이상 에러 없음
		if (errRet == SQL_NO_DATA)
			break;

		// 에러 파악 실패
		if (errRet != SQL_SUCCESS && errRet != SQL_SUCCESS_WITH_INFO)
			break;

		std::wcout.imbue(std::locale("kor"));
		std::wcout << errMsg << std::endl;

		index++;
	}
}
