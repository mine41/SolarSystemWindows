// Copyright Ilgar Lunin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SpeechRecognizer.h"
#include "SpeechRecognizerInitialize.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeechRecognizerInitialized, bool, Success);


USTRUCT(BlueprintType)
struct FSpeechRecognizerParameters
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpeechRecognizer")
        USpeechRecognizer* Recognizer;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpeechRecognizer")
        FString PathToModel;
};

/**
 * 
 */
UCLASS()
class USpeechRecognizerInitialize : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
    FSpeechRecognizerParameters params;

    UPROPERTY(BlueprintAssignable, Category = "SpeechRecognizer")
        FOnSpeechRecognizerInitialized Finished;
private:
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "SpeechRecognizer")
    static USpeechRecognizerInitialize* SpeechRecognizerInitialize(FSpeechRecognizerParameters params);

    virtual void Activate() override;

};
