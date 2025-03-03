#pragma once
#include "HttpProtocol.pb.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "C_SB.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHttpHandler, Log, All);

/*************************
  ServerHttpPacketHandler
*************************/

class ServerHttpPacketHandler
{
public:
	template<typename PktType>
	static bool									SendPkt(const FString& InUrl, PktType& InBodyPkt, TFunction<FHttpRequestCompleteDelegate::TFuncType> ResFunc);

	static bool									SendPkt(Protocol::REQ_CHECK_EXISTS_ACCOUNT Pkt) { return SendPkt(TEXT("/ReqCheckExistsAccount"), Pkt, &ServerHttpPacketHandler::RecvReqCheckExistsAccount); }
	static bool									SendPkt(Protocol::REQ_CREATE_ACCOUNT Pkt) { return SendPkt(TEXT("/ReqCreateAccount"), Pkt, &ServerHttpPacketHandler::RecvReqCreateAccount); }
	static bool									SendPkt(Protocol::REQ_LOGIN_ACCOUNT Pkt) { return SendPkt(TEXT("/ReqLoginAccount"), Pkt, &ServerHttpPacketHandler::RecvReqLoginAccount); }
	static bool									SendPkt(Protocol::REQ_LOGIN_GOOGLE_ACCOUNT Pkt) { return SendPkt(TEXT("/ReqLoginGoogleAccount"), Pkt, &ServerHttpPacketHandler::RecvReqLoginGoogleAccount); }
	static bool									SendPkt(Protocol::REQ_RECHECK_SERVER Pkt) { return SendPkt(TEXT("/ReqRecheckServer"), Pkt, &ServerHttpPacketHandler::RecvReqRecheckServer); }

	static class USBWebNetworkManager* const	GetWebNetworkManager();

private:
	template<typename PktType>
	static bool									ParseBodyToPkt(FHttpResponsePtr& Response, OUT PktType& OutPkt);
	static bool									DebugHttpFailPkt(FHttpRequestPtr& Request, FHttpResponsePtr& Response);

	static void									RecvReqCheckExistsAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
	static void									RecvReqCreateAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
	static void									RecvReqLoginAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
	static void									RecvReqLoginGoogleAccount(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
	static void									RecvReqRecheckServer(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);

private:
	static const FString						s_BaseUrl;
};

template<typename PktType>
inline bool ServerHttpPacketHandler::SendPkt(const FString& InUrl, PktType& InBodyPkt, TFunction<FHttpRequestCompleteDelegate::TFuncType> ResFunc)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(s_BaseUrl + InUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-protobuf"));
	Request->OnProcessRequestComplete().BindLambda(ResFunc);

	const uint16 dataSize = static_cast<uint16>(InBodyPkt.ByteSizeLong());
	TArray<BYTE> SerializedResult;
	SerializedResult.Reserve(dataSize);
	check(InBodyPkt.SerializeToArray(SerializedResult.GetData(), dataSize));
	TArray<uint8> InBinaryBody(reinterpret_cast<uint8*>(SerializedResult.GetData()), dataSize);
	Request->SetContent(InBinaryBody);

	if (Request->ProcessRequest() == true)
	{
		UE_LOG(LogHttpHandler, Log, TEXT("Send http pkt to %s"), *Request->GetURL());

		return true;
	}

	UE_LOG(LogHttpHandler, Log, TEXT("Sending http pkt is failed"));
	return false;
}

template<typename PktType>
inline bool ServerHttpPacketHandler::ParseBodyToPkt(FHttpResponsePtr& Response, OUT PktType& OutPkt)
{
	if (OutPkt.ParseFromArray(Response->GetContent().GetData(), Response->GetContent().Num()) == false)
	{
		UE_LOG(LogHttpHandler, Warning, TEXT("Http parsing to packet is failed"));
		return false;
	}

	UE_LOG(LogHttpHandler, Log, TEXT("Http parsing to packet succeeded"));
	return true;
}