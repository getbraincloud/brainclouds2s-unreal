// Copyright 2023 bitHeads, Inc. All Rights Reserved.

#include "S2SSocket.h"

US2SSocket::US2SSocket()
{
	FModuleManager::Get().LoadModuleChecked(TEXT("WebSockets"));
}

US2SSocket::~US2SSocket()
{
	Close();
}

void US2SSocket::SetupSocket(const FString& url)
{
	if (url.IsEmpty()) {
		UE_LOG(S2SWebSocket, Warning, TEXT("[S2SWebSocket] URL is empty"));
		OnConnectError.Broadcast(TEXT("URL is empty"));
		return;
	}

	ServerUrl = url;
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(url);

	if (WebSocket.IsValid()) {
		WebSocket->OnMessage().AddLambda([this](const FString& data)
			{
				if (OnReceiveMessage.IsBound()) OnReceiveMessage.Broadcast(data);
			});

		WebSocket->OnRawMessage().AddLambda([this](const void* Data, SIZE_T Size, SIZE_T)
			{

				TArray<uint8> DataArray;
		DataArray.Append((const uint8*)Data, Size);

		if (mCallbacks) mCallbacks->OnReceiveData(DataArray);
		if (OnReceiveData.IsBound()) OnReceiveData.Broadcast(DataArray);
			});

		WebSocket->OnConnected().AddLambda([this]()
			{
				UE_LOG(S2SWebSocket, Log, TEXT("[S2SWebSocket] Connected"));
		if (mCallbacks) mCallbacks->OnConnectComplete();
		if (OnConnectComplete.IsBound()) OnConnectComplete.Broadcast();
			});

		WebSocket->OnClosed().AddLambda([this](uint32 StatusCode, const FString& Reason, bool bWasClean)
			{
				UE_LOG(S2SWebSocket, Log, TEXT("[S2SWebSocket] Closed - StatusCode: %d Reason: %s WasClean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
		if (mCallbacks) mCallbacks->OnClosed();
		if (OnClosed.IsBound()) OnClosed.Broadcast();
			});

		WebSocket->OnConnectionError().AddLambda([this](const FString& reason)
			{
				UE_LOG(S2SWebSocket, Warning, TEXT("[S2SWebSocket] Connection error: %s"), *reason);
		if (mCallbacks) mCallbacks->OnConnectError(reason);
		if (OnConnectError.IsBound()) OnConnectError.Broadcast(reason);
			});

	}
	else {
		UE_LOG(S2SWebSocket, Warning, TEXT("[S2SWebSocket] Couldn't setup"));
		OnConnectError.Broadcast(TEXT("WebSocket couldn't setup"));
	}
}

void US2SSocket::Connect()
{
	if (WebSocket.IsValid() && !WebSocket->IsConnected())
	{
		WebSocket->Connect();
		UE_LOG(LogTemp, Log, TEXT("[S2SWebSocket] Connecting..."));
	}
}

void US2SSocket::Close()
{
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
	}
}

void US2SSocket::SendText(const FString& data)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Send(data);
	}
}

void US2SSocket::SendData(const TArray<uint8>& data)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		SIZE_T DataSize = data.Num();
		const void* DataPtr = data.GetData();
		WebSocket->Send(DataPtr, DataSize, true);
	}
}

bool US2SSocket::IsConnected()
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

FString US2SSocket::BytesToString(const void* Data, SIZE_T Size)
{
	const uint8* ByteData = static_cast<const uint8*>(Data);
	FString message;

	for (SIZE_T i = 0; i < Size; i++)
	{
		message += FString::Printf(TEXT("%02X"), ByteData[i]);
	}

	return message;
}
