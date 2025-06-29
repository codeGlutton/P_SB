import argparse
import CsvParser
import CsvReassembler
import unreal
import io
import os
from pathlib import Path

parsers = []
def parsing_csv_files(csv_dir: str, parent_file_name: str = 'Table', parent_size:int = 2, proj_prefix: str = ''):
    with os.scandir(csv_dir) as entries:
        prefix_id = 1000
        struct_parser = None
        for entry in entries:
            if entry.is_file() and entry.name.endswith('.csv'):
                # 자동화 소스 csv 분석 및 변수 저장
                parser = CsvParser.CsvParser(entry.name[0:-4], parent_file_name, parent_size, prefix_id, proj_prefix)
                parser.parse_csv(entry.path)
                parsers.append(parser)
                if parser.parsing_range == 'Struct':
                    struct_parser = parser
                else :
                    prefix_id += 1000

            elif entry.is_dir() and struct_parser and entry.name == struct_parser.file_name + 'Children':
                # 하위 상속 객체에 대해 깊이 우선 탐색
                parsing_csv_files(entry.path, struct_parser.file_name, len(struct_parser.fields), proj_prefix)
    return

def main():

    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description = 'TableGenerator')
    arg_parser.add_argument('--path', type=str, default='../../../../../S_SB/S_SB/DataTable/', help='Csv path')
    arg_parser.add_argument('--table_output', type=str, default='../../../../Content/SB/Data/DataTable/', help='UE Datatable Output file')
    arg_parser.add_argument('--project', type=str, default='C_SB', help='Project full path')
    arg_parser.add_argument('--prefix', type=str, default='SB', help='Project prefix name')
    args = arg_parser.parse_args()

    # 대상 위치의 csv 파일들 파싱
    dir_full_path = Path(__file__).parent.__str__() + '/' + args.path
    parsing_csv_files(dir_full_path, proj_prefix=args.prefix)

    data_parsers = [parser for parser in parsers if parser.parsing_range != 'Struct']
    for data_parser in data_parsers:
        row_struct_name = None
        if data_parser.parsing_range == 'All':
            row_struct_name = data_parser.struct_name + 'Row'
        else:
            row_struct_name = data_parser.parent_struct_name + 'Row'

        # UE Data Table 리소스 형식으로 임시 csv 파일 생성
        reassembler = CsvReassembler.CsvReassembler([field.name for field in data_parser.fields], data_parser.datas)
        reassembler.reassemble_csv(data_parser.file_name + '.csv')

        # UE Data Table 생성
        asset_factory = unreal.DataTableFactory()
        asset_factory.struct = unreal.load_object(None, args.project + '.' + row_struct_name)
        if asset_factory.struct is None:
            unreal.log_error("Asset factory struct is none.")

        csv_factory = unreal.CSVImportFactory()
        csv_factory.automated_import_settings.import_row_struct = asset_factory.struct

        task = unreal.AssetImportTask()
        task.filename = reassembler.reassemble_csv_path
        task.destination_name = 'DT_' + data_parser.struct_name
        task.destination_path = Path(__file__).parent.__str__() + '/' + args.table_output
        task.replace_existing = True
        task.automated = True
        task.save = True
        task.factory = csv_factory

        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

        # 임시 csv 파일 삭제
        try:
            os.remove(reassembler.reassemble_csv_path)
        except FileNotFoundError:
            return
        except Exception as e:
            unreal.log_error(e)

    return

if __name__ == '__main__':
    if unreal.is_editor():
        main()
