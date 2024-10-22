// Copyright Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace VoskComponentUtils {

	/**
	* Converts byte array to USoundWave ready to be performed by AudioComponent
	*/
	class USoundWave* CreateSoundFromWaveDataWithHeader(const TArray<uint8> &rawData);

	class USoundWave* CreateSoundFromWaveDataWithoutHeader(const TArray<uint8> &rawData, uint16 InSampleRate=16000, uint8 InNumChannels=1);

	/**
	* Adds wave header
	*/
	const TArray<uint8> GenerateWaveHeader(const TArray<uint8> &rawData, uint16 InSampleRate=16000, uint8 InNumChannels=1);


	/**
	* Wraps raw audio samples with wave header and saves to a file
	*/
	static bool WriteRawAudioToWavFile(const TArray<uint8> &CaptureData, FString Path, uint16 InSampleRate=16000, uint8 InNumChannels=1, bool generateHeader=true);

	static FString GetEnvironmentVariable(FString key);

	void EnsureSoundWaveDecompressed(class USoundWave* Sound, bool StopDeviceSounds=true);

	int32 GetSoundSampleRate(USoundWave* Sound);

}