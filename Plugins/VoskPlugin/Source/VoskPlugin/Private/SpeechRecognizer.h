// Copyright Ilgar Lunin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "vosk_api.h"
#include "VoskComponent.h"

#include "SpeechRecognizer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USpeechRecognizer : public UActorComponent
{
	friend class USpeechRecognizerInitialize;

	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USpeechRecognizer();

	UPROPERTY(BlueprintReadOnly, Category = "SpeechRecognizer")
		bool bIsCaptureActive = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SpeechRecognizer")
		bool bSendVoiceDataWhenRecording = true;

	UPROPERTY(BlueprintAssignable, Category = "SpeechRecognizer")
		FOnPartialResultReceived OnPartialResultReceived;

	UPROPERTY(BlueprintAssignable, Category = "SpeechRecognizer")
		FOnFinalResultReceived OnFinalResultReceived;

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
		void Uninitialize();

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
		void ResetRecognizer();

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
		bool BeginCapture();

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
		void RequestFinalResult();

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer")
		void FinishCapture(TArray<uint8>& CaptureData, int32& SamplesRecorded);

	UFUNCTION(BlueprintCallable, Category = "SpeechRecognizer", meta = (AdvancedDisplay = "PacketSize"))
		/**
		* Splits Voice chunk to pieces of PacketSize and sends them to the server.
		*
		* If you want process to finish faster, increase packet size
		*/
		bool FeedVoiceData(const TArray<uint8>& VoiceChunk, int32 PacketSize = 4096);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ParseRawResultAndDecode(const uint8* data, int32 size);

	void DecodeRresult(const FString& raw);
	void Initialize(const FString& PathToLanguageModel);

	VoskModel* model_ = nullptr;
	VoskRecognizer* recognizer_ = nullptr;

	TSharedPtr<class IVoiceCapture> _voice_capture;
	const int32 _sample_rate = 16000;
	TArray<uint8> _recorded_samples;

	bool initialization_in_progress = false;
	bool want_final_result_ = false;
};
