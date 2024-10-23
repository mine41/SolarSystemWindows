// Copyright Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ProcessHandleWrapper.generated.h"


USTRUCT(BlueprintType)
struct VOSKPLUGIN_API FProcessHandleWrapper
{
    GENERATED_USTRUCT_BODY()

    FProcHandle handle;
    uint32 processId;

};