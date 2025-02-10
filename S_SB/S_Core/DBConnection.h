#pragma once
#include <sql.h>
#include <sqlext.h>

enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000
};

class DBConnection
{
public:
	// 초기 핸들 초기화 및 db 연결 함수
	bool		Connect(SQLHENV henv, const WCHAR* connectionString);
	void		Clear();

	// sql 쿼리 실행
	bool		Execute(const WCHAR* query);
	// sql 결과 값 한 행씩 버퍼에 기록
	bool		Fetch();
	int32		GetRowCount();
	// sql 상태 핸들 초기화 (결과 및 파라미터, 칼럼 버퍼 초기화)
	void		Unbind();

public:
	bool		BindParam(int32 paramIndex, bool* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, float* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, double* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, int8* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, int16* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, int32* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, int64* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool		BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index);
	bool		BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index);

	bool		BindCol(int32 clolumnIndex, bool* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, float* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, double* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, int8* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, int16* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, int32* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, int64* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, WCHAR* str, int32 size, SQLLEN* index);
	bool		BindCol(int32 clolumnIndex, BYTE* bin, int32 size, SQLLEN* index);

private:
	// sql 실행 시 사용될 파라미터 연결
	bool		BindParam(SQLSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLLEN len, SQLPOINTER ptr, SQLLEN* index);
	// sql 결과 시 받아오는 칼럼 버퍼 연결
	bool		BindCol(SQLSMALLINT columnIndex, SQLSMALLINT cType, SQLLEN len, SQLPOINTER value, SQLLEN* index);
	// 에러 로그 표기
	void		HandleError(SQLRETURN ret);

private:
	SQLHDBC		_connection = SQL_NULL_HANDLE;
	SQLHSTMT	_statement = SQL_NULL_HANDLE;
};

