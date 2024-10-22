// Copyright Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "ProcessHandleWrapper.h"
#include "VoskServerParameters.h"

#include "VoskComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnectedToServer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionError, FString, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartialResultReceived, FString, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinalResultReceived, FString, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConnectionTerminated, int32, StatusCode, FString, Reason, bool, WasClean);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVoskComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoskComponent();

    UPROPERTY(BlueprintReadOnly, Category = "VoskComponent")
    bool bIsCaptureActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "VoskComponent")
    bool bSendVoiceDataWhenRecording = true;

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    bool BeginCapture();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void FinishCapture(TArray<uint8> &CaptureData, int32 &SamplesRecorded);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent", meta = (AdvancedDisplay = "PacketSize"))
    /**
    * Splits Voice chunk to pieces of PacketSize and sends them to the server.
    * 
    * If you want process to finish faster, increase packet size
    */
    bool SendVoiceDataToLanguageServer(const TArray<uint8>& VoiceChunk, int32 PacketSize = 4096);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void Initialize(FString Addr = TEXT("127.0.0.1"), int32 Port = 8080);

    UFUNCTION(BlueprintPure, Category = "VoskComponent")
    bool IsInitialized();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void Uninitialize();

    UFUNCTION(BlueprintPure, Category = "VoskComponent")
    FString GetPartialResult();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void ResetPartialResult();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void ResetFinalResult();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void RequestFinalResult();

    UFUNCTION(BlueprintPure, Category = "VoskComponent")
    FString GetFinalResult();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    static USoundWave* SamplesToSound(
        const TArray<uint8>& samples,
        int32 SampleRate=16000,
        int32 NumChannels=1);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    static int32 GetSoundSampleRate(USoundWave* SoundWave);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent", meta = (AdvancedDisplay="Detach, Hidden, Priority, OptionalWorkingDirectory"))
    static void CreateProcess(FProcessHandleWrapper& ProcessHandle,
                              FString FullPathOfProgramToRun,
                              TArray<FString> CommandlineArgs,
                              bool Detach=false,
                              bool Hidden=true,
                              int32 Priority=0);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    static float CompareStrings(const FString& left, const FString& right);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    void ResetRecognizer();

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    static void KillProcess(const FProcessHandleWrapper& processHandle);

    UFUNCTION(BlueprintCallable, Category = "VoskComponent")
    static TArray<FString> BuildServerParameters(FVoskServerParameters params, bool& Success);

    UFUNCTION(BlueprintPure, Category = "VoskComponent")
    static FString GetProcessExecutablePath();

    UPROPERTY(BlueprintAssignable, Category = "VoskComponent")
    FOnConnectedToServer OnConnectedToServer;

    UPROPERTY(BlueprintAssignable, Category = "VoskComponent")
    FOnConnectionError OnConnectionError;

    UPROPERTY(BlueprintAssignable, Category = "VoskComponent")
    FOnConnectionTerminated OnConnectionTerminated;

    UPROPERTY(BlueprintAssignable, Category = "VoskComponent")
    FOnPartialResultReceived OnPartialResultReceived;

    UPROPERTY(BlueprintAssignable, Category = "VoskComponent")
    FOnFinalResultReceived OnFinalResultReceived;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void DecodeRresult(const FString &raw);
    TSharedPtr<IWebSocket> Socket;

    TSharedPtr<class IVoiceCapture> _voice_capture;
    TArray<uint8> _recorded_samples;
    FString _res_partial;
    FString _res_final;

    const int32 _sample_rate = 16000;
};