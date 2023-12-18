# brainCloud S2S C++ library for Unreal
Tested in Unreal 4.24

## Installation:

1. Copy the `BrainCloudS2S` folder into your projects source folder.
2. In your `*.build.cs` build file, add those 3 modules:
   ```
   PrivateDependencyModuleNames.AddRange(new string[]
   {
       "JsonUtilities",
       "HTTP",
       "Json",
       "WebSockets"
   });
   ```

## Usage

### Initialize S2S library
```
US2SRTTComms *pS2S;
pS2S = NewObject<US2SRTTComms>();
pS2S->AddToRoot();
pS2S->InitializeS2S(appId, serverName, serverSecret, serverUrl, autoAuth, logEnabled)

```

### Perform a request
The requests is in pure form. Use the portal's S2S Explorer to see how it should be formated. Here is an example:
```
pS2S->request(JsonString, [callback](const FString &result)
{
    callback.ExecuteIfBound(result);
    UE_LOG(LogTemp, Log, TEXT("S2S Result %s"), *result);
});
```

### Enable RTT
To use RTT features you must enable RTT which will connect to brainClouds WebSocket RTT server. 
```
pS2S->enableRTT([this, successCallback](const FString& result)
		{
			successCallback.ExecuteIfBound(result);
		}, [this, failureCallback](const FString& result)
		{
			failureCallback.ExecuteIfBound(result);
		});
```

###Register an RTT callback
This is how you can register a callback that will be triggered whenever you receive an RTT message from the brainCloud WebSocket server
```
pS2S->registerRTTCallback([this, rttCallback](const FString& result)
		{
			rttCallback.ExecuteIfBound(result);
		});
```

### Verbose
Can enable more log of packet going or coming:
```
pS2S->setLogEnabled(true);
```


### Life cycle and Shutdown
The library will keep the connection alive with a heartbeat at a constant interval (Default 30minutes).
To shutdown, simply destroy the instance:
```
pS2S.Reset();
```
