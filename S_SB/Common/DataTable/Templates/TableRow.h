#pragma once

enum class TableRowType
{
	NONE = 0,
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Struct' %}
	{{ parser.file_name | upper_snake }} = {{ (parser.prefix_id / 1000) | int }},
{%- endif -%}
{%- endfor %}
};

/************************
		TableRow
************************/

struct TableRow
{
	int32 Id;
	xWString ServerName;
};
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Data' %}

/************************
	{{ parser.file_name }}Row
************************/

struct {{ parser.file_name }}Row : public {{ parser.parent_file_name }}Row
{
{%- for field in parser.fields -%}
{%- if loop.index > parser.parent_size %}
	{{ field.type }} {{ field.name }};
{%- endif -%}
{%- endfor %}
};
{%- endif -%}
{%- endfor %}

/************************
	 DataTableManager
************************/

class DataTableManager
{
public:
	DataTableManager()
	{
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Struct' -%}
{%- for data in parser.datas %}
		{{ parser.file_name }}.emplace(std::make_pair({{ data[0] }}, xnew<
{%- if parser.parsing_range == 'All' -%}
		{{ parser.file_name }}Row>({{ parser.file_name }}
{%- else -%}
		{{ parser.parent_file_name }}Row>({{ parser.parent_file_name }}
{%- endif -%}
		Row{
{%- for value in data -%}
{%- if loop.last -%}
			{{ value }}
{%- else -%}
			{{ value }},
{%- endif -%}
{%- endfor -%}
			})));
{%- endfor %}
		{{ parser.parent_file_name }}s.emplace(std::make_pair(TableRowType::{{ parser.file_name | upper_snake }}, &{{ parser.file_name }}));

{%- endif -%}
{%- endfor %}
	}

	~DataTableManager()
	{
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Struct' %}
		for (auto& tableRow : {{ parser.file_name }})
		{
			xdelete(tableRow.second);
		}
{%- endif -%}
{%- endfor %}
	}

public:
{%- for parser in parsers -%}
{%- if parser.parsing_range == 'Struct' %}
	xUnorderedMap<TableRowType, xUnorderedMap<int32, {{ parser.file_name }}Row*>*> {{ parser.file_name }}s;
{%- endif -%}
{%- endfor %}

private:
{%- for parser in parsers -%}
{%- if parser.parsing_range != 'Struct' %}
	xUnorderedMap<int32, {{ parser.parent_file_name }}Row*> {{ parser.file_name }};
{%- endif -%}
{%- endfor %}
};

extern DataTableManager*				GDataTableManager;
