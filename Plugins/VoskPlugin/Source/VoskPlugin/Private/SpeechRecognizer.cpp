// Copyright Ilgar Lunin. All Rights Reserved.

#include "SpeechRecognizer.h"
#include "Voice.h"
#include "Serialization/JsonSerializer.h"
#include <string>

// Sets default values for this component's properties
USpeechRecognizer::USpeechRecognizer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpeechRecognizer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void USpeechRecognizer::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (_voice_capture.IsValid())
		_voice_capture->Stop();
	bIsCaptureActive = false;

	Uninitialize();
}

void USpeechRecognizer::ParseRawResultAndDecode(const uint8* data, int32 size)
{
	// process audio and fire events
	const char* voice_data = reinterpret_cast<const char*>(data);

	FString recognition_result;
	if (want_final_result_) {
		recognition_result = UTF8_TO_TCHAR(vosk_recognizer_final_result(recognizer_));
		want_final_result_ = false;
	}
	else if (vosk_recognizer_accept_waveform(recognizer_, voice_data, size)) {
		recognition_result = UTF8_TO_TCHAR(vosk_recognizer_result(recognizer_));
	}
	else {
		recognition_result = UTF8_TO_TCHAR(vosk_recognizer_partial_result(recognizer_));
	}

	DecodeRresult(recognition_result);
}

void USpeechRecognizer::DecodeRresult(const FString& raw)
{
	TSharedPtr<FJsonObject> result;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(raw);

	if (FJsonSerializer::Deserialize(reader, result))
	{
		FString _res;
		if (result->TryGetStringField(TEXT("partial"), _res))
		{
			OnPartialResultReceived.Broadcast(_res);
		}
		else if (result->TryGetStringField(TEXT("text"), _res))
		{
			OnFinalResultReceived.Broadcast(_res);
		}
	}
}

void USpeechRecognizer::Initialize(const FString& PathToLanguageModel) {

	if (model_ != nullptr || recognizer_ != nullptr) {
		Uninitialize();
	}

	// TODO: this is blocking call, mote to async
	std::string model_path = std::string(TCHAR_TO_UTF8(*PathToLanguageModel));
	model_ = vosk_model_new(model_path.c_str());
	recognizer_ = vosk_recognizer_new(model_, 16000);
	vosk_recognizer_set_max_alternatives(recognizer_, 0);
	vosk_recognizer_set_words(recognizer_, false);
}

void USpeechRecognizer::Uninitialize()
{
	if (recognizer_ != nullptr) {
		vosk_recognizer_free(recognizer_);
		recognizer_ = nullptr;
	}

	if (model_ != nullptr) {
		vosk_model_free(model_);
		model_ = nullptr;
	}
	
}

void USpeechRecognizer::ResetRecognizer()
{
	if (recognizer_) {
		vosk_recognizer_reset(recognizer_);
	}
}

bool USpeechRecognizer::BeginCapture()
{
	if (bIsCaptureActive) return false;

	if (!FVoiceModule::Get().DoesPlatformSupportVoiceCapture())
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), TEXT("VoiceCapture is not supported on this platform!"));
	}

	if (!_voice_capture.IsValid())
	{
		_voice_capture = FVoiceModule::Get().CreateVoiceCapture("");
		if (_voice_capture.IsValid())
		{
			FString DeviceName;
			if (!_voice_capture->Init(DeviceName, _sample_rate, 1))
				return false;
			UE_LOG(LogTemp, Log, TEXT("IVoiceCapture initialized"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to obtain IVoiceCapture, no voice available!"));
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Capture started"));
	_recorded_samples.Empty();
	_voice_capture->Start();
	bIsCaptureActive = true;

	return true;
}

void USpeechRecognizer::RequestFinalResult()
{
	want_final_result_ = true;
}

void USpeechRecognizer::FinishCapture(TArray<uint8>& CaptureData, int32& SamplesRecorded)
{
	bIsCaptureActive = false;

	SamplesRecorded = _recorded_samples.Num();
	CaptureData = _recorded_samples;

	if (_voice_capture.IsValid())
		_voice_capture->Stop();
}

bool USpeechRecognizer::FeedVoiceData(const TArray<uint8>& VoiceChunk, int32 PacketSize)
{
	if (recognizer_ == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Component is not initialized!"));
		return false;
	}

	PacketSize = FMath::Clamp<int32>(PacketSize, 0, TNumericLimits<int32>::Max());

	if (VoiceChunk.Num() < PacketSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Voice chunk is too small"));
		return false;
	}

	const int32 NumPackets = VoiceChunk.Num() / PacketSize;
	size_t BytesSent = 0;
	for (int i = 0; i < NumPackets; i++)
	{
		const uint8* data = VoiceChunk.GetData() + (i * PacketSize);
		ParseRawResultAndDecode(data, PacketSize);
		/*Socket->Send(VoiceChunk.GetData() + (i * PacketSize), PacketSize, true);*/
		BytesSent += PacketSize;
	}

	if (BytesSent < (size_t)VoiceChunk.Num())
	{
		// send remainder
		const size_t remainder = VoiceChunk.Num() - BytesSent;
		const uint8* data = VoiceChunk.GetData() + BytesSent;
		ParseRawResultAndDecode(data, remainder);
		//Socket->Send(VoiceChunk.GetData() + BytesSent, remainder, true);
		BytesSent += remainder;
	}

	bool all_sent = VoiceChunk.Num() == BytesSent;

	RequestFinalResult();
	ParseRawResultAndDecode(0, 0);

	return all_sent;
}


// Called every frame
void USpeechRecognizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_voice_capture.IsValid() && bIsCaptureActive)
	{
		uint32 VoiceCaptureBytesAvailable = 0;
		EVoiceCaptureState::Type CaptureState = _voice_capture->GetCaptureState(VoiceCaptureBytesAvailable);

		if (CaptureState == EVoiceCaptureState::Ok && VoiceCaptureBytesAvailable > 0) {
			uint32 VoiceCaptureReadBytes = 0;

			TArray<uint8> _recorded_chunk;
			uint32 record_start = _recorded_chunk.AddUninitialized(VoiceCaptureBytesAvailable);

			EVoiceCaptureState::Type microphone_state = _voice_capture->GetVoiceData(_recorded_chunk.GetData(), VoiceCaptureBytesAvailable, VoiceCaptureReadBytes);
			if (microphone_state == EVoiceCaptureState::Ok) {
				if (VoiceCaptureReadBytes > 0) {
					_recorded_samples += _recorded_chunk;
					if (recognizer_ && bSendVoiceDataWhenRecording) {
						ParseRawResultAndDecode(_recorded_chunk.GetData(), VoiceCaptureReadBytes);
					}
				}
			}
		}
		
		if (want_final_result_) {
			const FString RecognitionResult = UTF8_TO_TCHAR(vosk_recognizer_final_result(recognizer_));
			DecodeRresult(RecognitionResult);
			want_final_result_ = false;
		}
	}
}

