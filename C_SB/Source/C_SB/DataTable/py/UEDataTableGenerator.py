import argparse
import CsvParser
import CsvReassembler
import unreal
import io
import os
from pathlib import Path

def main():

    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description = 'TableGenerator')
    arg_parser.add_argument('--path', type=str, default='../../../../../S_SB/S_SB/DataTable/', help='Csv path')
    arg_parser.add_argument('--table', type=str, default='../../../../Content/SB/Data/DataTable/', help='UE data table path')
    arg_parser.add_argument('--project', type=str, default='C_SB', help='Project full path')
    arg_parser.add_argument('--prefix', type=str, default='SB', help='Project prefix name')
    args = arg_parser.parse_args()

    args_full_path = Path(__file__).parent.__str__() + '/' + args.path
    file_list = os.listdir(args_full_path)
    file_list_csv = [file for file in file_list if file.endswith(".csv")]
    for file in file_list_csv:
        file_full_path = args_full_path + file
        struct_name = args.prefix + file[0:-4]

        # 자동화 소스 csv 분석 및 변수 저장
        parser = CsvParser.CsvParser(struct_name, file[0:-4], True)
        parser.parse_csv(file_full_path)

        # UE Data Table 리소스 형식으로 임시 csv 파일 생성
        reassembler = CsvReassembler.CsvReassembler([field.name for field in parser.fields], parser.datas)
        reassembler.reassemble_csv(file)

        # UE Data Table 생성
        asset_factory = unreal.DataTableFactory()
        asset_factory.struct = unreal.load_object(None, args.project + '.' + struct_name + 'Row')
        if asset_factory.struct is None:
            unreal.log_error("Asset factory struct is none.")

        csv_factory = unreal.CSVImportFactory()
        csv_factory.automated_import_settings.import_row_struct = asset_factory.struct

        task = unreal.AssetImportTask()
        task.filename = reassembler.reassemble_csv_path
        task.destination_name = 'DT_' + struct_name
        task.destination_path = Path(__file__).parent.__str__() + '/' + args.table
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
    main()
