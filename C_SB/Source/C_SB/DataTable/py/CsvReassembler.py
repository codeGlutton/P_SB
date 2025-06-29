from typing import List
import csv

class CsvReassembler():
    def __init__(self, names: List[str], datas: List[List[str]]):
        self.names = names # name Vector
        self.datas = datas # data 2D Vector
        self.reassemble_csv_path = ''

    def reassemble_csv(self, file_name: str):
        self.reassemble_csv_path = 'Tmp_' + file_name
        f = open(self.reassemble_csv_path, 'w+', encoding="UTF-8")
        writer = csv.writer(f)
        self.names.insert(0, '-')
        writer.writerow(self.names)
        for data in self.datas :
            data.insert(0, data[0])
            writer.writerow(data)
        f.close()
