#pragma once
{%- for parser in parsers %}

/************************
	{{ parser.fileName }}Row
************************/

struct {{ parser.fileName }}Row
{
{%- for field in parser.fields %}
	{{ field.type }} {{ field.name }};
{%- endfor %}
};
{%- endfor %}

/************************
		DataTable
************************/

class DataTable
{
public:
	DataTable()
	{
{%- for parser in parsers %}
{%- for data in parser.datas %}
		{{ parser.fileName }}.insert(std::make_pair({{ data[1] }}, {{ parser.fileName }}Row{
{%- for value in data -%}
{%- if loop.last -%}
			{{ value }}
{%- else -%}
			{{ value }},
{%- endif -%}
{%- endfor -%}
			}));
{%- endfor %}
{%- endfor %}
	}

public:
{%- for parser in parsers %}
	xUnorderedMap <xWString, {{ parser.fileName }}Row> {{ parser.fileName }};
{%- endfor %}
};

extern DataTable*				GDataTable;
