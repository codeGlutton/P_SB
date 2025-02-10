import argparse
import jinja2
import XmlDBParser
import sys
import io

def main():
    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description= 'StoredProcedure Generator')
    arg_parser.add_argument('--path', type=str, default='../../S_SB/GameDB.xml', help='Xml Path')
    arg_parser.add_argument('--output', type=str, default='GenProcedures.h', help='Output File')
    args = arg_parser.parse_args()

    if args.path == None or args.output == None:
        print('[Error] --path --output required')
        return

    # 자동화 소스 xml 분석 및 변수 저장
    parser = XmlDBParser.XmlDBParser()
    parser.parse_xml(args.path)

    # 템플릿 엔진으로 기반 c++ 코드에 변수 값들 작성 
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)

    template = env.get_template('GenProcedures.h')
    output = template.render(procs=parser.procedures)

    f = open(args.output, 'w+', encoding="UTF-8")
    f.write(output)
    f.close()

    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'UTF-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'UTF-8')
    print(output)
    return

if __name__ == '__main__':
    main()
