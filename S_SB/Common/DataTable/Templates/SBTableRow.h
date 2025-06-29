#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SBTableRow.generated.h"

UENUM(BlueprintType)
enum class ESBTableRowType : uint8
{
	NONE = 0 UMETA(Hidden),
{%- for parser in parsers %}
{%- if parser.parsing_range != 'Struct' %}
	{{ parser.file_name | upper_snake }} = {{ (parser.prefix_id / 1000) | int }} UMETA(DisplayName = "{{ parser.file_name }}"),
{%- endif -%}
{%- endfor %}
};

/**********************
	  FSBTableRow
**********************/

USTRUCT(Blueprintable)
struct FSBTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FSBTableRow() :Id(), ClientName() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClientName;
};
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Data' %}

/**********************
	F{{ parser.struct_name }}Row
**********************/

USTRUCT(Blueprintable)
struct F{{ parser.struct_name }}Row : public F{{ parser.parent_struct_name }}Row
{
	GENERATED_USTRUCT_BODY()

public:
	F{{ parser.struct_name }}Row() : F{{ parser.parent_struct_name }}Row()
{%- for field in parser.fields -%}
{%- if loop.index > parser.parent_size -%}
	, {{ field.name }}()
{%- endif -%}
{%- endfor %} {}
{%- for field in parser.fields -%}
{%- if loop.index > parser.parent_size %}

	UPROPERTY(EditAnywhere, BlueprintReadWrite
{%- if field.interface != '' -%}
	, meta = (MustImplement = "{{ field.interface }}")
{%- endif -%}
	)
	{{ field.type }} {{ field.name }};
{%- endif -%}
{%- endfor %}
};
{%- endif -%}
{%- endfor %}
