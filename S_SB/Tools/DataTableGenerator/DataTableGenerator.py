import argparse
import jinja2
import CsvParser
import sys
import io
import os

def main():

    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default='../../S_SB/DataTable/', help='Csv path')
    arg_parser.add_argument('--output', type=str, default='TableRow.h', help='Output file')
    arg_parser.add_argument('--project', type=str, default='SB', help='Project prefix name')
    arg_parser.add_argument('--isUE', type=str, default='False', help='Apply unreal type')
    arg_parser.add_argument('--table', type=str, default='DataTable.h', help='Unreal c++ table output file')
    args = arg_parser.parse_args()

    # 디버깅 로그를 위한 설정
    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'UTF-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'UTF-8')

    isUE = True
    if args.isUE == 'False' :
        isUE = False
    parsers = []
    file_list = os.listdir(args.path)
    file_list_csv = [file for file in file_list if file.endswith(".csv")]
    for file in file_list_csv:
        file_full_path = args.path + file
        struct_name = ''
        if isUE == True :
            struct_name = args.project + file[0:-4]
        else:
            struct_name = file[0:-4]

        # 자동화 소스 csv 분석 및 변수 저장
        parser = CsvParser.CsvParser(struct_name, file[0:-4], isUE)
        parser.parse_csv(file_full_path)
        parsers.append(parser)

    # 템플릿 엔진으로 기반 c++ 코드에 변수 값들 작성 
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
    
    templateName = ''
    if isUE == True :
        templateName = args.project + 'TableRow.h'
    else :
        templateName = 'TableRow.h'
        
    template = env.get_template(templateName)
    output = template.render(parsers=parsers, output=args.output)

    if isUE == True:
        f = open(args.project + args.output, 'w+', encoding="UTF-8")
    else:
        f = open(args.output, 'w+', encoding="UTF-8")
    f.write(output)
    f.close()

    print(output)

    # UE의 추가적인 코드 자동화
    if isUE == True:
        second_template = env.get_template(args.project + 'DataTable.h')
        second_output = second_template.render(parsers=parsers)
        second_f = open(args.project + args.table, 'w+', encoding="UTF-8")
        second_f.write(second_output)
        second_f.close()

        print(second_output)

    return

if __name__ == '__main__':
    main()