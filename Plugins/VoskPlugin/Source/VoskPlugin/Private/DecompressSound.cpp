// Copyright Ilgar Lunin. All Rights Reserved.

#include "DecompressSound.h"
#include "Audio.h"
#include "Sound/SoundWave.h"
#include "VoskSoundUtils.h"


void StereoToMono(TArray<uint8> StereoWavBytes, TArray<uint8>& MonoWavBytes)
{
	for (int i = 0; i < 44; i++) {
		if (i == 22) {
			short NumChannels = (*reinterpret_cast<short*>(&StereoWavBytes[i])) / 2;
			MonoWavBytes.Append(reinterpret_cast<uint8*>(&NumChannels), sizeof(NumChannels));
			i++;
		} else if (i == 28) {
			int ByteRate = (*reinterpret_cast<int*>(&StereoWavBytes[i])) / 2;
			MonoWavBytes.Append(reinterpret_cast<uint8*>(&ByteRate), sizeof(ByteRate));
			i += 3;
		} else if (i == 32) {
			short BlockAlign = (*reinterpret_cast<short*>(&StereoWavBytes[i])) / 2;
			MonoWavBytes.Append(reinterpret_cast<uint8*>(&BlockAlign), sizeof(BlockAlign));
			i++;
		} else if (i == 40) {
			int SubChunkSize = (*reinterpret_cast<int*>(&StereoWavBytes[i])) / 2;
			MonoWavBytes.Append(reinterpret_cast<uint8*>(&SubChunkSize), sizeof(SubChunkSize));
			i += 3;
		} else
			MonoWavBytes.Add(StereoWavBytes[i]);
	}

	//Copies only the left channel and ignores the right channel
	for (int i = 44; i < StereoWavBytes.Num(); i += 4) {
		MonoWavBytes.Add(StereoWavBytes[i]);
		MonoWavBytes.Add(StereoWavBytes[i+1]);
	}
}


UDecompressSound* UDecompressSound::DecompressSound(USoundWave* Sound, bool StopDeviceSounds)
{
	UDecompressSound* BPNode = NewObject<UDecompressSound>();
	BPNode->Sound = Sound;
	BPNode->StopDeviceSounds = StopDeviceSounds;
	return BPNode;
}

void UDecompressSound::Activate() {
	if (Sound == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Null sound was provided. Make sure AudioCapture component Start/Stop was called"));
		return;
	}
	// decompress
	VoskComponentUtils::EnsureSoundWaveDecompressed(Sound, StopDeviceSounds);

	FWaveModInfo WaveInfo;
	TArray<uint8> result;
	
	const uint8* RawWaveData = (const uint8*)Sound->RawPCMData;
	int32 RawDataSize = Sound->RawPCMDataSize;

	if (!RawWaveData || RawDataSize == 0) {
		UE_LOG(LogTemp, Warning, TEXT("LPCM data failed to load for sound %s"), *Sound->GetFullName());

	} else if (!WaveInfo.ReadWaveHeader(RawWaveData, RawDataSize, 0)) {
		// If we failed to parse the wave header, it's either because of an
		// invalid bitdepth or channel configuration.
		UE_LOG(LogTemp, Warning,
			TEXT("Only mono or stereo 16 bit waves allowed: %s (%d bytes)"),
			*Sound->GetFullName(), RawDataSize);

	} else {
		result.AddUninitialized(WaveInfo.SampleDataSize);
		FMemory::Memcpy(result.GetData(), WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);
		// Convert stereo to mono and unlock sound raw data
		if (*WaveInfo.pChannels != 1)
		{
			TArray<uint8> MonoResult;
			StereoToMono(result, MonoResult);
			Finished.Broadcast(MonoResult);
			return;
		}
	}

	if (!result.Num())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Can't cook %s because there is no source LPCM data"), *Sound->GetFullName());
	}

	// output result
	Finished.Broadcast(result);
}
