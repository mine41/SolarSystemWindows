// Copyright Ilgar Lunin. All Rights Reserved.

#include "VoskSoundUtils.h"
#include "Sound/SoundWave.h"
#include "AudioDecompress.h"
#include "AudioDevice.h"
#include "Misc/EngineVersionComparison.h"
#include "Runtime/Launch/Resources/Version.h"

#if PLATFORM_WINDOWS
    #include "Runtime/Core/Public/Windows/WindowsPlatformMisc.h"
#endif

#if PLATFORM_MAC
    #include "Runtime/Core/Public/Apple/ApplePlatformMisc.h"
#endif

#if PLATFORM_LINUX
    #include "Runtime/Core/Public/Linux/LinuxPlatformMisc.h"
#endif

#include "Misc/FileHelper.h"


USoundWave* VoskComponentUtils::CreateSoundFromWaveDataWithHeader(const TArray<uint8> &rawData)
{
    USoundWave* sWave = nullptr;

	// reading wave information
	FWaveModInfo waveInfo;

    #if ENGINE_MINOR_VERSION > 19 && ENGINE_MAJOR_VERSION == 4
        uint8* waveData = const_cast<uint8*>(rawData.GetData());
    #else
        const uint8* waveData = rawData.GetData();
    #endif

	if (waveInfo.ReadWaveInfo(waveData, rawData.Num()))
	{
	    // Construct USoundWave and feed received bytes
        sWave = NewObject<USoundWave>();

	    // apply wave info
	    int32 DurationDiv = *waveInfo.pChannels * *waveInfo.pBitsPerSample * *waveInfo.pSamplesPerSec;
	    if (DurationDiv)
		    sWave->Duration = *waveInfo.pWaveDataSize * 8.0f / DurationDiv;
	    else
		    sWave->Duration = 0.0f;
        sWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
        sWave->NumChannels = *waveInfo.pChannels;
        sWave->RawPCMDataSize = waveInfo.SampleDataSize;

        #if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 19
            sWave->SampleRate = *waveInfo.pSamplesPerSec;
        #else
            sWave->SetSampleRate(*waveInfo.pSamplesPerSec);
        #endif

        sWave->bLooping = false;
        sWave->RawPCMData = (uint8*)FMemory::Malloc(sWave->RawPCMDataSize);
        const uint8 headerOffset = 44;
        FMemory::Memcpy(sWave->RawPCMData, rawData.GetData() + headerOffset, rawData.Num() - headerOffset);
	}

	return sWave;
}

class USoundWave* VoskComponentUtils::CreateSoundFromWaveDataWithoutHeader(const TArray<uint8> &rawData, uint16 InSampleRate/*=48000*/, uint8 InNumChannels/*=2*/)
{
    USoundWave* sWave = nullptr;

    // reading wave information
    FWaveModInfo waveInfo;

    #if UE_VERSION_NEWER_THAN(4, 19, 0)
        uint8* waveData = const_cast<uint8*>(rawData.GetData());
    #else
        const uint8* waveData = rawData.GetData();
    #endif

    if (!waveInfo.ReadWaveInfo(waveData, rawData.Num()))
    {
        TArray<uint8> waveDataWithHeader = GenerateWaveHeader(rawData, InSampleRate, InNumChannels);

        FString errorMsg;
        if(waveInfo.ReadWaveInfo(waveDataWithHeader.GetData(), waveDataWithHeader.Num(), &errorMsg))
        {
            // Construct USoundWave and feed received bytes
            sWave = NewObject<USoundWave>();

            // apply wave info
            int32 DurationDiv = *waveInfo.pChannels * *waveInfo.pBitsPerSample * *waveInfo.pSamplesPerSec;
            if (DurationDiv)
                sWave->Duration = *waveInfo.pWaveDataSize * 8.0f / DurationDiv;
            else
                sWave->Duration = 0.0f;
            sWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
            sWave->NumChannels = *waveInfo.pChannels;
            sWave->RawPCMDataSize = waveInfo.SampleDataSize;

            #if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 19
                sWave->SampleRate = *waveInfo.pSamplesPerSec;
            #else
                sWave->SetSampleRate(*waveInfo.pSamplesPerSec);
            #endif

            sWave->bLooping = false;
            sWave->RawPCMData = (uint8*)FMemory::Malloc(sWave->RawPCMDataSize);
            const uint8 headerOffset = 44;
            FMemory::Memcpy(sWave->RawPCMData, waveDataWithHeader.GetData() + headerOffset, waveDataWithHeader.Num() - headerOffset);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Error generating wave info\n%s"), *errorMsg);
        }
    }

    return sWave;
}

const TArray<uint8> VoskComponentUtils::GenerateWaveHeader(const TArray<uint8> &rawData, uint16 InSampleRate, uint8 InNumChannels)
{
    int  chunkSize = 36 + rawData.Num();
    int subchunk1Size = 16;
    short audioFormat = 1;
    short numChannels = InNumChannels;
    int sampleRate = InSampleRate;
    int byteRate = sampleRate * numChannels * rawData.GetTypeSize();
    short blockAlign = numChannels * rawData.GetTypeSize();
    short bitsPerSample = 16;
    int buffSize = rawData.Num();

    uint8 header[44];
    uint8* end = header;
    FMemory::Memcpy(end, "RIFF", 4);            end += 4;
    FMemory::Memcpy(end, &chunkSize, 4);        end += 4;
    FMemory::Memcpy(end, "WAVE", 4);            end += 4;
    FMemory::Memcpy(end, "fmt ", 4);            end += 4;
    FMemory::Memcpy(end, &subchunk1Size, 4);    end += 4;
    FMemory::Memcpy(end, &audioFormat, 2);      end += 2;
    FMemory::Memcpy(end, &numChannels, 2);      end += 2;
    FMemory::Memcpy(end, &sampleRate, 4);       end += 4;
    FMemory::Memcpy(end, &byteRate, 4);	        end += 4;
    FMemory::Memcpy(end, &blockAlign, 2);       end += 2;
    FMemory::Memcpy(end, &bitsPerSample, 2);    end += 2;
    FMemory::Memcpy(end, "data", 4);            end += 4;
    FMemory::Memcpy(end, &buffSize, 4);         end += 4;

    TArray<uint8> wav = TArray<uint8>(header, 44);
    wav.Append(rawData);
    return wav;
}


bool VoskComponentUtils::WriteRawAudioToWavFile(const TArray<uint8> &CaptureData, FString Path, uint16 InSampleRate, uint8 InNumChannels, bool generateHeader)
{
    FText err;
    if (FFileHelper::IsFilenameValidForSaving(Path, err))
    {
        if (generateHeader)
        {
            const TArray<uint8> wav = GenerateWaveHeader(CaptureData, InSampleRate, InNumChannels);
            return FFileHelper::SaveArrayToFile(wav, *Path);
        }
        else
        {
            return FFileHelper::SaveArrayToFile(CaptureData, *Path);
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Error saving capture data: %s"), *err.ToString());
    return false;
}

FString VoskComponentUtils::GetEnvironmentVariable(FString key)
{
    FString result;
    #if PLATFORM_WINDOWS
        result = FWindowsPlatformMisc::GetEnvironmentVariable(*key);
    #endif

    #if PLATFORM_MAC
        result = FApplePlatformMisc::GetEnvironmentVariable(*key);
    #endif

    #if PLATFORM_LINUX
        result = FLinuxPlatformMisc::GetEnvironmentVariable(*key);
    #endif

    return result;
}

void VoskComponentUtils::EnsureSoundWaveDecompressed(USoundWave* Sound, bool StopDeviceSounds)
{
    if (Sound == nullptr)
        return;

    if (Sound->RawPCMData) {
        UE_LOG(LogTemp, Display, TEXT("PCM data already exists, no need to decompress"));
        return;
    }

    auto AudioDevice = GEngine->GetMainAudioDevice();
    if (!AudioDevice) {
        return;
    }

    AudioDevice->StopAllSounds(StopDeviceSounds);
    auto OriginalDecompressionType = Sound->DecompressionType;

    Sound->DecompressionType = DTYPE_Native;
    
#if UE_VERSION_OLDER_THAN(5, 1, 1)
    FName runtimeFormat = AudioDevice->GetRuntimeFormat(Sound);
#else
    FName runtimeFormat = Sound->GetRuntimeFormat();
#endif

    if (Sound->InitAudioResource(runtimeFormat))
    {
#if UE_VERSION_OLDER_THAN(4, 22, 0)
        USoundWave::FAsyncAudioDecompress Decompress(SoundWave);
#else
        USoundWave::FAsyncAudioDecompress Decompress(Sound, MONO_PCM_BUFFER_SAMPLES);
#endif
        UE_LOG(LogTemp, Display, TEXT("start audio decompression"));
        Decompress.StartSynchronousTask();
    }
    
    Sound->DecompressionType = OriginalDecompressionType;

}

int32 VoskComponentUtils::GetSoundSampleRate(USoundWave* Sound)
{
    FWaveModInfo WaveInfo;
    TArray<uint8> result;

    const uint8* RawWaveData = (const uint8*)Sound->RawPCMData;
    int32 RawDataSize = Sound->RawPCMDataSize;

    if (!RawWaveData || RawDataSize == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("LPCM data failed to load for sound %s"), *Sound->GetFullName());
    }
    else if (!WaveInfo.ReadWaveHeader(RawWaveData, RawDataSize, 0))
    {
        // If we failed to parse the wave header, it's either because of an
        // invalid bitdepth or channel configuration.
        UE_LOG(LogTemp, Warning,
            TEXT("Only mono or stereo 16 bit waves allowed: %s (%d bytes)"),
            *Sound->GetFullName(), RawDataSize);
    }
    return *WaveInfo.pSamplesPerSec;
}
