#pragma once
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "ReadWriteJson.generated.h"

struct FCapsuleLocation;
class FJsonObject;

UCLASS()
class UReadWriteJson : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FString ReadStringFromFile(const FString& FilePath, bool& bOutSuccess, FString& OutInfoMessage);
	static void WriteStringToFile(const FString& FilePath, const FString& StringToWrite, bool& bOutSuccess, FString& OutInfoMessage);
	
	static TSharedPtr<FJsonObject> ReadJson(const FString& JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage);
	static void WriteJson(const FString& JsonFilePath, const TSharedPtr<FJsonObject>& JsonObject, bool& bOutSuccess, FString& OutInfoMessage);

	template <typename T>
	static T ReadStructFromJsonFile(const FString& JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage);

	template <typename T>
	static void WriteStructToJsonFile(const FString& JsonFilePath, const T& Struct, bool& bOutSuccess, FString& OutInfoMessage);
};

template <typename T>
T UReadWriteJson::ReadStructFromJsonFile(const FString& JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject> JsonObject = ReadJson(JsonFilePath, bOutSuccess, OutInfoMessage);
	if(!bOutSuccess)
	{
		return T();
	}

	T RetStruct;

	if(!FJsonObjectToUStruct<T>(JsonObject.ToSharedRef(), &RetStruct))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read Struct Json Failed - Was not able to convert the json object to the desired structure. Is it the right format / struct? - '%ls'"), *JsonFilePath);
		return T();
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read Struct Json Succeeded - '%ls'"), *JsonFilePath);
	return RetStruct;
}

template <typename T>
void UReadWriteJson::WriteStructToJsonFile(const FString& JsonFilePath, const T& Struct, bool& bOutSuccess, FString& OutInfoMessage)
{
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Struct);
	if(!JsonObject)
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write Struct Json Failed - Was not able to convert the struct to a json object. This shouldn't really happen"));
		return;
	}

	WriteJson(JsonFilePath, JsonObject, bOutSuccess, OutInfoMessage);
}
