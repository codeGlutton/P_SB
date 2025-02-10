import csv

class CsvParser():
    def __init__(self, fileName, origin, isUE):
        self.fileName = fileName
        self.origin = origin
        self.isUE = isUE
        self.prefixId = 0 # prefix id
        self.fields = [] # field Vector
        self.datas = [] # data 2D Vector

    def parse_csv(self, path):
        f = open(path, 'r', encoding="utf-8-sig")
        reader = csv.reader(f)
        rows = list(reader)

        prefix_row = None
        for i, row in enumerate(rows):
            if row and row[0] == 'PrefixId':
                prefix_row = i
                self.prefixId = row[1]
                break
        if prefix_row is None :
            return

        # 불필요한 필드 구분
        exclude_field_name = ''
        if self.isUE == True :
            exclude_field_name = 'Server'
        else :
            exclude_field_name = 'Client'
        target_field_indexes = []
        wstring_field_indexes = []

        # 필드 저장
        for i, (type, name) in enumerate(zip(rows[prefix_row + 1], rows[prefix_row + 2])):
            if name.startswith(exclude_field_name) :
                continue
            inverted_type, interface_class = ReplaceType(type, self.isUE)
            self.fields.append(Field(inverted_type, name, interface_class))
            target_field_indexes.append(i)
            if inverted_type == 'xWString' or inverted_type == 'FString':
                wstring_field_indexes.append(i)

        # 데이터 저장
        for row in rows[prefix_row + 3 :] :
            row[0] = str(int(self.prefixId) + int(row[0]))

            for wstring_field_index in wstring_field_indexes:
                row[wstring_field_index] = ReplaceWString(row[wstring_field_index], self.isUE)

            data = []
            for target_field_index in target_field_indexes:
                data.append(row[target_field_index])
            self.datas.append(data)

        f.close()

class Field:
    def __init__(self, type, name, interface):
        self.type = type
        self.name = name
        self.interface = interface

def ReplaceWString(str, isUE):
    if isUE :
        return str
    else:
        return 'L"' + str + '"'

def ReplaceType(type, isUE):
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
        if isUE == True :
            return 'FString', ''
        else :
            return 'xWString', ''
    elif type.startswith('Tsoftclassptr<'):
        if isUE == True :
            if type[14] == 'I' :
                type = type[:-1]
                return 'TSoftClassPtr<UObject>', type[15:]
            else :
                return 'TSoftClassPtr<' + type[14:], ''
        else :
            return 'xWString', ''
    return type, ''
