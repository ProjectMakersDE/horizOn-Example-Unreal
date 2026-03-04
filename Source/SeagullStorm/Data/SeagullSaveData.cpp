#include "Data/SeagullSaveData.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

FString FSeagullSaveData::ToJsonString() const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("coins"), Coins);
	Root->SetNumberField(TEXT("highscore"), Highscore);
	Root->SetNumberField(TEXT("totalRuns"), TotalRuns);

	TSharedRef<FJsonObject> UpgradesObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Upgrades)
	{
		UpgradesObj->SetNumberField(Pair.Key, Pair.Value);
	}
	Root->SetObjectField(TEXT("upgrades"), UpgradesObj);

	TArray<TSharedPtr<FJsonValue>> CodesArray;
	for (const FString& Code : GiftCodesRedeemed)
	{
		CodesArray.Add(MakeShared<FJsonValueString>(Code));
	}
	Root->SetArrayField(TEXT("giftCodesRedeemed"), CodesArray);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Root, Writer);
	return Output;
}

FSeagullSaveData FSeagullSaveData::FromJsonString(const FString& Json)
{
	FSeagullSaveData Data;
	Data.InitDefaults();

	if (Json.IsEmpty())
	{
		return Data;
	}

	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return Data;
	}

	Data.Coins = static_cast<int32>(Root->GetNumberField(TEXT("coins")));
	Data.Highscore = static_cast<int32>(Root->GetNumberField(TEXT("highscore")));
	Data.TotalRuns = static_cast<int32>(Root->GetNumberField(TEXT("totalRuns")));

	const TSharedPtr<FJsonObject>* UpgradesObj = nullptr;
	if (Root->TryGetObjectField(TEXT("upgrades"), UpgradesObj) && UpgradesObj)
	{
		for (const auto& Pair : (*UpgradesObj)->Values)
		{
			Data.Upgrades.Add(Pair.Key, static_cast<int32>(Pair.Value->AsNumber()));
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* CodesArray = nullptr;
	if (Root->TryGetArrayField(TEXT("giftCodesRedeemed"), CodesArray) && CodesArray)
	{
		for (const auto& Val : *CodesArray)
		{
			Data.GiftCodesRedeemed.Add(Val->AsString());
		}
	}

	return Data;
}
