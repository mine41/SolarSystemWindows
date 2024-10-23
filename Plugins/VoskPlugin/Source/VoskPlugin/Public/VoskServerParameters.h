// Copyright Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "VoskServerParameters.generated.h"


USTRUCT(BlueprintType)
struct VOSKPLUGIN_API FVoskServerParameters
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin")
        FString Address = "localhost";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin", meta = (ClampMin = "1024", UIMin = "1024"))
        int32 Port = 8080;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin", meta = (ClampMin = "1", UIMin = "1"))
        int32 Threads = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin")
        int32 SampleRate = 16000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin", meta = (ClampMin = "0", UIMin = "0"))
        int32 MaxAlternatives = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin")
        bool ShowWords = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoskPlugin")
    FString PathToModel = "";
};
