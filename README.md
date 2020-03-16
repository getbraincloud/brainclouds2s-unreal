# brainCloud S2S C++ library for Unreal
Tested in Unreal 2.24

## Installation:

1. Copy `BrainCloudS2S.h` and `BrainCloudS2S.cpp` into your project.
2. In your `*.build.cs` build file, add those 3 modules:
   ```
   PrivateDependencyModuleNames.AddRange(new string[]
   {
       "JsonUtilities",
       "HTTP",
       "Json"
   });
   ```

## Usage

### Initialize S2S library
```
pS2S = MakeShareable(new UBrainCloudS2S("#####", "ServerName", "########-####-####-####-############"));
```

### Perform a request
The requests is in pure form. Use the portal's S2S Explorer to see how it should be formated. Here is an example of READ TIME call:
```
pS2S->request("{\"service\":\"time\",\"operation\":\"READ\",\"data\":{}}", [](const FString &result)
{
    UE_LOG(LogTemp, Log, TEXT("S2S Result %s"), *result);
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
