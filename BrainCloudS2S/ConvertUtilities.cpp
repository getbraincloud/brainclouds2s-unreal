#include "ConvertUtilities.h"

FString ConvertUtilities::BCBytesToString(const uint8* in, int32 count)
{
	FString result2;
	result2.Empty(count);
	while (count)
	{
		result2 += ANSICHAR(*in);
		++in;
		--count;
	}

	return result2;
}

int32 ConvertUtilities::BCStringToBytes(const FString& in_string, uint8* out_bytes, int32 in_maxBufferSize)
{
	int32 numBytes = 0;
	const TCHAR* charPos = *in_string;

	while (*charPos && numBytes < in_maxBufferSize)
	{
		out_bytes[numBytes] = (uint8)(*charPos);
		charPos++;
		++numBytes;
	}
	return numBytes;
}

TSharedPtr<FJsonObject> ConvertUtilities::jsonStringToValue(const FString& jsonString)
{
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	TSharedPtr<FJsonObject> jsonValue;
	FJsonSerializer::Deserialize(reader, jsonValue);

	return jsonValue;
}

FString ConvertUtilities::jsonValueToString(const TSharedRef<FJsonObject>& jsonValue)
{
	FString jsonStr;

	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&jsonStr);
	FJsonSerializer::Serialize(jsonValue, writer);

	return jsonStr;
}
