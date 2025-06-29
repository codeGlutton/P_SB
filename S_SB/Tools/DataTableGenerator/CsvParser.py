from asyncio.windows_events import NULL
import csv

class CsvParser():
    def __init__(self, file_name: str, parent_file_name: str, parent_size:int, prefix_id: int, name_prefix: str = ''):
        self.is_ue = name_prefix != ''
        self.struct_name = name_prefix + file_name
        self.file_name = file_name
        self.parent_struct_name = name_prefix + parent_file_name
        self.parent_file_name = parent_file_name
        self.parent_size = parent_size
        self.prefix_id = prefix_id

        self.fields = [] # field Vector
        self.datas = [] # data 2D Vector
        self.parsing_range = None # pasing range

    def parse_csv(self, path: str):
        f = open(path, 'r', encoding="utf-8-sig")
        reader = csv.reader(f)
        rows = list(reader)

        prefix_row = None
        for i, row in enumerate(rows):
            if len(row) >= 2 and row[0] == 'Range' and row[1] in ['Struct', 'Data' , 'All']:
                prefix_row = i
                self.parsing_range = row[1]
                break
        if self.parsing_range is None :
            return

        # 불필요한 필드 구분
        exclude_field_name = ''
        if self.is_ue == True :
            exclude_field_name = 'Server'
        else :
            exclude_field_name = 'Client'
        target_field_indexes = []
        wstring_field_indexes = []

        # 필드 저장
        for i, (type, name) in enumerate(zip(rows[prefix_row + 1], rows[prefix_row + 2])):
            if name.startswith(exclude_field_name) :
                continue
            inverted_type, interface_class = replace_type(type, self.is_ue)
            self.fields.append(Field(inverted_type, name, interface_class))
            target_field_indexes.append(i)
            if inverted_type == 'xWString' or inverted_type == 'FString':
                wstring_field_indexes.append(i)

        # 데이터 저장
        for row in rows[prefix_row + 3 :] :
            row[0] = str(int(self.prefix_id) + int(row[0]))

            if self.is_ue == False:
                for wstring_field_index in wstring_field_indexes:
                    row[wstring_field_index] = change_str_to_wstr(row[wstring_field_index])

            data = []
            for target_field_index in target_field_indexes:
                data.append(row[target_field_index])
            self.datas.append(data)

        f.close()

class Field:
    def __init__(self, type: str, name: str, interface: str):
        self.type = type
        self.name = name
        self.interface = interface

def change_str_to_wstr(string: str) -> str:
    return 'L"' + string + '"'

def replace_type(type: str, is_ue: bool):
    if type == 'Bool' or type == 'Boolean':
        return 'bool', ''
    elif type == 'Char' or type == 'Int8':
        return 'int8', ''
    elif type == 'Short' or type == 'Int16':
        return 'int16', ''
    elif type == 'Int' or type == 'Int32':
        return 'int32', ''
    elif type == 'Longlong' or type == 'Int64':
        return 'int64', ''
    elif type == 'Uchar' or type == 'Uint8':
        return 'uint8', ''
    elif type == 'Ulonglong' or type == 'Uint16':
        return 'uint16', ''
    elif type == 'Uint' or type == 'Uint32':
        return 'uint32', ''
    elif type == 'Ulonglong' or type == 'Uint64':
        return 'uint64', ''
    elif type == 'Float':
        return 'float', ''
    elif type == 'Double':
        return 'double', ''
    elif type == 'String' or type == 'Wstring' or type == 'Fstring':
        if is_ue == True :
            return 'FString', ''
        else :
            return 'xWString', ''
    elif type.startswith('Tsoftclassptr<') or type.startswith('TSoftClassPtr<'):
        if is_ue == True :
            if type[14] == 'I' :
                type = type[:-1]
                return 'TSoftClassPtr<UObject>', type[15:]
            else :
                return 'TSoftClassPtr<' + type[14:], ''
        else :
            return 'xWString', ''
    return type, ''