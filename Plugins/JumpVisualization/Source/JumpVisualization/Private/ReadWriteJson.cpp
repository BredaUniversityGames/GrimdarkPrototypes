#include "ReadWriteJson.h"

FString UReadWriteJson::ReadStringFromFile(const FString& FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read String From File Failed - File doesn't exist - '%ls'"), *FilePath);
		return "";
	}

	FString RetString = "";

	if(!FFileHelper::LoadFileToString(RetString, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read String From File Failed - Was not able to read file. Is this a text file? - '%ls'"), *FilePath);
		return "";
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read String From File Succeeded - '%ls'"), *FilePath);
	return RetString;
}

void UReadWriteJson::WriteStringToFile(const FString& FilePath, const FString& StringToWrite, bool& bOutSuccess, FString& OutInfoMessage)
{
	if(!FFileHelper::SaveStringToFile(StringToWrite, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write String To File Failed - Was not able to write to file. Is your file read only? Is the path valid? - '%ls'"), *FilePath);
		return;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write String To File Succeeded - '%ls'"), *FilePath);
}

TSharedPtr<FJsonObject> UReadWriteJson::ReadJson(const FString& JsonFilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString JsonString = ReadStringFromFile(JsonFilePath, bOutSuccess, OutInfoMessage);
	if(!bOutSuccess)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> RetJsonObject;

	if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), RetJsonObject))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Read Json From File Failed - Was not able to deserialize the json string. Is it the right format? - '%ls'"), *JsonString);
		return nullptr;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Read Json Succeeded - '%ls'"), *JsonFilePath);
	return RetJsonObject;
}
	

void UReadWriteJson::WriteJson(const FString& JsonFilePath, const TSharedPtr<FJsonObject>& JsonObject, bool& bOutSuccess,
                               FString& OutInfoMessage)
{
	FString JsonString;

	if(!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString, 0)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Write Json Failed - Was not able to serialize json to string. Is the JsonObject valid?"));
		return;
	}

	WriteStringToFile(JsonFilePath, JsonString, bOutSuccess, OutInfoMessage);
	if(!bOutSuccess)
	{
		return;
	}

	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Write Json Succeeded - '%ls'"), *JsonFilePath);
}
