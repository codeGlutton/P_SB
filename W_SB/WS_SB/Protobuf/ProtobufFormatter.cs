using Azure.Core;
using Google.Protobuf;
using Microsoft.AspNetCore.Mvc.Formatters;
using Microsoft.Net.Http.Headers;
using System.Reflection;

namespace WS_SB.Protobuf
{
    /// <summary>
    /// HttpBody에 담긴 Protobuf Data decoder
    /// </summary>
    public class ProtobufInputFormatter : InputFormatter
    {
        public ProtobufInputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/x-protobuf"));
        }

        public override async Task<InputFormatterResult> ReadRequestBodyAsync(InputFormatterContext context)
        {
            try
            {
                /* Body를 지정한 Protobuf Class로 역직렬화 */

                MemoryStream stream = new MemoryStream();
                HttpRequest request = context.HttpContext.Request;
                await request.Body.CopyToAsync(stream);
                stream.Position = 0;

                IMessage? protoMsg = (IMessage?)Activator.CreateInstance(context.ModelType);
                protoMsg.MergeFrom(stream);

                return await InputFormatterResult.SuccessAsync(protoMsg);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception: " + ex);
                return await InputFormatterResult.FailureAsync();
            }
        }
    }

    public class ProtobufOutputFormatter : OutputFormatter
    {
        public ProtobufOutputFormatter()
        {
            SupportedMediaTypes.Add(MediaTypeHeaderValue.Parse("application/x-protobuf"));
        }

        public override bool CanWriteResult(OutputFormatterCanWriteContext context)
        {
            /* 응답이 Protobuf인가 확인을 위해 MediaType을 확인 */

            if (base.CanWriteResult(context) == false)
            {
                return false;
            }

            /* 응답하고자 하는 객체가 IMessage를 상속받은 Protobuf class인지 확인 */

            return context.ObjectType.GetTypeInfo()
                .ImplementedInterfaces
                .Where(i => i.GetTypeInfo().IsGenericType)
                .Any(i => i.GetGenericTypeDefinition() == typeof(IMessage<>));
        }

        public override Task WriteResponseBodyAsync(OutputFormatterWriteContext context)
        {
            /* 지정한 Protobuf Class를 직렬화하여 body에 채우기 */

            HttpResponse response = context.HttpContext.Response;
            IMessage? protoMsg = context.Object as IMessage;
            byte[] body = protoMsg.ToByteArray();

            return response.Body.WriteAsync(body, 0, body.Length);
        }
    }
}
