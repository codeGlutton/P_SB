import argparse
import jinja2
import ProtoParser
import sys
import io

def main():

    # cmd 명령 분석
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default='../../Common/Protobuf/bin/Protocol.proto', help='proto path') # 자동화 소스 코드
    arg_parser.add_argument('--output', type=str, default='TestPacketHandler', help='output file') # 출력될 클래스명 및 파일명
    arg_parser.add_argument('--recv', type=str, default='C_', help='recv convention') # 수신 연관 prefix명
    arg_parser.add_argument('--send', type=str, default='S_', help='send convention') # 송신 연관 prefix명
    args = arg_parser.parse_args()

    # 자동화 소스 proto 분석 및 변수 저장
    parser = ProtoParser.ProtoParser(1000, args.recv, args.send)
    parser.parse_proto(args.path)

    # 템플릿 엔진으로 기반 c++ 코드에 변수 값들 작성 
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)
    
    template = env.get_template('PacketHandler.h')
    output = template.render(parser=parser, output=args.output)

    f = open(args.output+'.h', 'w+', encoding="UTF-8")
    f.write(output)
    f.close

    sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'UTF-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'UTF-8')
    print(output)
    return

if __name__ == '__main__':
    main()