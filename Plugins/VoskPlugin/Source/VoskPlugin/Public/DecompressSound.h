// Copyright Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DecompressSound.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoundDecompressed, const TArray<uint8>&, RawSamples);


/**
 * 
 */
UCLASS()
class VOSKPLUGIN_API UDecompressSound final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "VoskPlugin")
		FOnSoundDecompressed Finished;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "VoskPlugin")
	static UDecompressSound* DecompressSound(USoundWave* Sound, bool StopDeviceSounds = true);

	virtual void Activate() override;

	USoundWave* Sound = nullptr;
	bool StopDeviceSounds = true;
};
