import argparse
from asyncio.windows_events import NULL
import jinja2
import CsvParser
import sys
import re
import io
import os

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

def convert_to_snake(target:str) -> str:
    snake = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', target)
    snake = re.sub(r'([a-z\d])([A-Z])', r'\1_\2', target)
    return snake.lower()

def convert_to_uppper_snake(target:str) -> str:
    snake = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', target)
    snake = re.sub(r'([a-z\d])([A-Z])', r'\1_\2', target)
    return snake.upper()

def main():

    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default='../../S_SB/DataTable/', help='Csv path')
    arg_parser.add_argument('--row_output', type=str, default='TableRow.h', help='Output file')
    arg_parser.add_argument('--prefix', type=str, default='', help='Project prefix name')
    args = arg_parser.parse_args()

    # 디버깅 로그를 위한 설정
    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'UTF-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'UTF-8')

    # 대상 위치의 csv 파일들 파싱
    parsing_csv_files(args.path, proj_prefix=args.prefix)

    # 템플릿 엔진으로 기반 c++ 코드에 변수 값들 작성 
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
    env.filters['upper_snake'] = convert_to_uppper_snake
    
    template = env.get_template(args.prefix + 'TableRow.h')
    output = template.render(parsers=parsers)

    f = open(args.prefix + args.row_output, 'w+', encoding="UTF-8")
    f.write(output)
    f.close()

    print(output)

    return

if __name__ == '__main__':
    main()