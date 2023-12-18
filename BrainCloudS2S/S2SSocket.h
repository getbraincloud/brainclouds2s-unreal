// Copyright 2023 bitHeads, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Online/WebSockets/Public/IWebSocket.h"
#include "Runtime/Online/WebSockets/Public/IWebSocketsManager.h"
#include "Runtime/Online/WebSockets/Public/WebSocketsModule.h"
#include "S2SSocket.generated.h"

DEFINE_LOG_CATEGORY_STATIC(S2SWebSocket, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FS2SWebSocketClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FS2SWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FS2SWebSocketReceiveData, const TArray<uint8>&, data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FS2SWebSocketReceiveMessage, const FString&, data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FS2SWebSocketConnectError, const FString&, error);

class IS2SWebSocketBaseCallbacks
{
public:
	virtual void OnConnectError(const FString& error) = 0;
	virtual void OnClosed() = 0;
	virtual void OnConnectComplete() = 0;
	virtual void OnReceiveData(const TArray<uint8>& data) = 0;
};

UCLASS(Blueprintable, BlueprintType)
class US2SSocket : public UObject
{
	GENERATED_BODY()
public:
	US2SSocket();
	~US2SSocket();

	void SetupSocket(const FString& url);

	void Connect();

	void Close();

	void SendText(const FString& data);

	void SendData(const TArray<uint8>& data);

	bool IsConnected();

	FS2SWebSocketReceiveData OnReceiveData;

	FS2SWebSocketReceiveMessage OnReceiveMessage;

	FS2SWebSocketConnected OnConnectComplete;

	FS2SWebSocketClosed OnClosed;

	FS2SWebSocketConnectError OnConnectError;

	IS2SWebSocketBaseCallbacks* mCallbacks;

private:
	FString BytesToString(const void* Data, SIZE_T Size);
	TSharedPtr<IWebSocket> WebSocket;
	FString ServerUrl;
	TArray<FString> mSendQueue;
	TArray<TArray<uint8>> mSendQueueData;
};
