#pragma once

#include "CoreMinimal.h"
#include "SBObject.h"
#include "Engine/DataTable.h"
#include "SBTableRow.generated.h"

/**********************
	  FSBTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FSBTableRow() :Id(), ClientName(), ClientBasePath() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClientName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MustImplement = "SBObject"))
	TSoftClassPtr<UObject> ClientBasePath;
};
{%- for parser in parsers %}

/**********************
	F{{ parser.fileName }}Row
**********************/

USTRUCT(Blueprintable)
struct F{{ parser.fileName }}Row : public FSBTableRow
{
	GENERATED_USTRUCT_BODY()

public:
	F{{ parser.fileName }}Row() : FSBTableRow()
{%- for field in parser.fields -%}
{%- if loop.index0 > 2 -%}
	, {{ field.name }}()
{%- endif -%}
{%- endfor %} {}
{%- for field in parser.fields -%}
{%- if loop.index0 > 2 %}

	UPROPERTY(EditAnywhere, BlueprintReadWrite
{%- if field.interface != '' -%}
	, meta = (MustImplement = "{{ field.interface }}")
{%- endif -%}
	)
	{{ field.type }} {{ field.name }};
{%- endif -%}
{%- endfor %}
};
{%- endfor %}
