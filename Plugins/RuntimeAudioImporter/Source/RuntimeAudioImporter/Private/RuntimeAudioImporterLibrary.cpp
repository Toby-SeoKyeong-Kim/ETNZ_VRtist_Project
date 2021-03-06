// Georgy Treshchev 2021.

#include "RuntimeAudioImporterLibrary.h"

#include "PreimportedSoundAsset.h"
#include "Misc/FileHelper.h"
#include "Containers/UnrealString.h"
#include "Async/Async.h"

URuntimeAudioImporterLibrary* URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter()
{
	URuntimeAudioImporterLibrary* Importer = NewObject<URuntimeAudioImporterLibrary>();
	Importer->AddToRoot();
	return Importer;
}

void URuntimeAudioImporterLibrary::ImportAudioFromFile(const FString& FilePath, TEnumAsByte<EAudioFormat> Format)
{
	if (!FPaths::FileExists(FilePath))
	{
		OnResult_Internal(nullptr, AudioDoesNotExist);
		return;
	}

	if (Format == EAudioFormat::Auto) Format = GetAudioFormat(FilePath);

	TArray<uint8> AudioBuffer;
	TArray<uint8> StretchBuffer;
	FString ChunkSizeW = TEXT("94");
	FString ChunkSizeW1 = TEXT("9A");
	FString ChunkSizeW2 = TEXT("D3");
	FString ChunkSizeW3 = TEXT("02");
	FString ChunkSize2 = TEXT("0");
	FString ChunkSize21 = TEXT("3C");
	FString ChunkSize22 = TEXT("2A");
	FString ChunkSize23 = TEXT("2D");
	uint8 Outbytes;
	uint8 *BytesPointer = &Outbytes;
	if (!FFileHelper::LoadFileToArray(AudioBuffer, *FilePath))
	{
		// Callback Dispatcher OnResult
		OnResult_Internal(nullptr, LoadFileToArrayError);
		return;
	}
	/*
	for (size_t i = 0; i < AudioBuffer.Num()-23999; i++)
	{
		if (i < 80) {
			
			StretchBuffer.Add(AudioBuffer[i]);
		}
		else
		{
			for (size_t i2 = 0; i2 < 24000 && i+i2 < AudioBuffer.Num(); i2++)
			{
				StretchBuffer.Add(AudioBuffer[i+i2]);
			}
			for (size_t i2 = 0; i2 < 24000 && i + i2 < AudioBuffer.Num(); i2++)
			{
				StretchBuffer.Add(AudioBuffer[i + i2]);
			}
			i += 23999;
		}
	}
		
	
	HexToBytes(ChunkSizeW, BytesPointer);
	StretchBuffer[4] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[4]);
	HexToBytes(ChunkSizeW1, BytesPointer);
	StretchBuffer[5] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[5]);
	HexToBytes(ChunkSizeW2, BytesPointer);
	StretchBuffer[6] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[6]);
	HexToBytes(ChunkSizeW3, BytesPointer);
	StretchBuffer[7] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[7]);

	HexToBytes(ChunkSize2, BytesPointer);
	StretchBuffer[76] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[68]);
	HexToBytes(ChunkSize21, BytesPointer);
	StretchBuffer[77] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[69]);
	HexToBytes(ChunkSize22, BytesPointer);
	StretchBuffer[78] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[70]);
	HexToBytes(ChunkSize23, BytesPointer);
	StretchBuffer[79] = Outbytes;
	UE_LOG(LogTemp, Warning, TEXT("outBytes, %x"), StretchBuffer[71]);

	UE_LOG(LogTemp, Log, TEXT("AudioBufferNum, %d"), AudioBuffer.Num());
	UE_LOG(LogTemp, Log, TEXT("StretchedNum, %d"), StretchBuffer.Num());
	
	for (size_t i = 1000; i < 1170; i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("fmt, %x"), StretchBuffer[i]);
		UE_LOG(LogTemp, Warning, TEXT("Origin_fmt, %x"), AudioBuffer[i]);
	}
	UE_LOG(LogTemp, Warning, TEXT("SUM  %x +  %x =  %x"), StretchBuffer[1169], StretchBuffer[1169], StretchBuffer[1169]+ StretchBuffer[1169]);
	
	for (size_t i = 0; i < 180; i++)
	{
		const uint8 sum = AudioBuffer[i] + AudioBuffer[i + 1];
		UE_LOG(LogTemp, Warning, TEXT("Origin_PCM, %x + %x = %x"), AudioBuffer[i], AudioBuffer[i + 1], sum);
		UE_LOG(LogTemp, Warning, TEXT("Mult_PCM, %f"), (float)sum);
	}
	*/
	
	ImportAudioFromBuffer(AudioBuffer, Format);
}

void URuntimeAudioImporterLibrary::ImportAudioFromPreImportedSound(UPreImportedSoundAsset* PreImportedSoundAssetRef)
{
	ImportAudioFromBuffer(PreImportedSoundAssetRef->AudioDataArray, Mp3);
}

void URuntimeAudioImporterLibrary::ImportAudioFromBuffer(TArray<uint8>& AudioDataArray,
                                                         const TEnumAsByte<EAudioFormat>& Format)
{
	if (Format == Wav)
	{
		if (!CheckAndFixWavDurationErrors(AudioDataArray)) return;
	}
	TranscodingFillInfo = FTranscodingFillStruct();
	AsyncTask(ENamedThreads::AnyThread, [=]()
	{
		ImportAudioFromBuffer_Internal(AudioDataArray, Format);
	});
}

void URuntimeAudioImporterLibrary::ImportAudioFromBuffer_Internal(const TArray<uint8>& AudioDataArray,
                                                                  const TEnumAsByte<EAudioFormat>& Format)
{
	// Callback Dispatcher OnProgress
	OnProgress_Internal(5);
	if (Format == Auto || Format == Invalid)
	{
		// Callback Dispatcher OnResult
		OnResult_Internal(nullptr, InvalidAudioFormat);
		return;
	}
	

	// Transcoding the imported Audio Data to PCM Data
	if (!TranscodeAudioDataArrayToPCMData(AudioDataArray.GetData(), AudioDataArray.Num() - 2, Format)) return;

	// Callback Dispatcher OnProgress
	OnProgress_Internal(65);
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		UImportedSoundWave* SoundWaveRef = NewObject<UImportedSoundWave>(UImportedSoundWave::StaticClass());
		if (!SoundWaveRef)
		{
			OnResult_Internal(nullptr, SoundWaveDeclarationError);
			return nullptr;
		}
		AsyncTask(ENamedThreads::AnyThread, [=]()
		{
			if (DefineSoundWave(SoundWaveRef))
			{
				// Callback Dispatcher OnProgress
				OnProgress_Internal(100);

				// Callback Dispatcher OnResult, with the created SoundWave object
				OnResult_Internal(SoundWaveRef, SuccessfulImport);
			}
		});
		return nullptr;
	});
}

bool URuntimeAudioImporterLibrary::DefineSoundWave(UImportedSoundWave* SoundWaveRef)
{
	// Callback Dispatcher OnProgress
	OnProgress_Internal(70);

	// Fill SoundWave basic information (e.g. duration, number of channels, etc)
	FillSoundWaveBasicInfo(SoundWaveRef);

	// Callback Dispatcher OnProgress
	OnProgress_Internal(75);

	// Fill PCM data buffer
	FillPCMData(SoundWaveRef);

	// Callback Dispatcher OnProgress
	OnProgress_Internal(95);

	return true;
}

void URuntimeAudioImporterLibrary::FillSoundWaveBasicInfo(UImportedSoundWave* SoundWaveRef) const
{
	SoundWaveRef->RawPCMDataSize = TranscodingFillInfo.PCMInfo.PCMDataSize;
	SoundWaveRef->Duration = TranscodingFillInfo.SoundWaveBasicInfo.Duration;
	SoundWaveRef->SetSampleRate(TranscodingFillInfo.SoundWaveBasicInfo.SampleRate);
	SoundWaveRef->SamplingRate = TranscodingFillInfo.SoundWaveBasicInfo.SampleRate;
	SoundWaveRef->NumChannels = TranscodingFillInfo.SoundWaveBasicInfo.ChannelsNum;
	SoundWaveRef->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	if (SoundWaveRef->NumChannels >= 4)
	{
		SoundWaveRef->bIsAmbisonics = 1;
	}
	SoundWaveRef->bProcedural = true;
	SoundWaveRef->bLooping = false;
}

void URuntimeAudioImporterLibrary::FillPCMData(UImportedSoundWave* SoundWaveRef) const
{
	/*SoundWaveRef->RawPCMData = static_cast<uint8*>(FMemory::Malloc(SoundWaveRef->RawPCMDataSize));
	FMemory::Memmove(SoundWaveRef->RawPCMData, TranscodingFillInfo.PCMInfo.RawPCMData,
	                 SoundWaveRef->RawPCMDataSize);*/
	// We do not need to fill a standard PCM buffer since we have a custom sound wave with custom buffer. But if you want to fill the standard PCM buffer, just uncomment the code above.

	SoundWaveRef->PCMBufferInfo = TranscodingFillInfo.PCMInfo;
}


/**
* Including dr_wav, dr_mp3 and dr_flac libraries
*/
#include "ThirdParty/dr_wav.h"
#include "ThirdParty/dr_mp3.h"
#include "ThirdParty/dr_flac.h"


/**
* Replacing standard CPP memory methods (malloc, realloc, free) with engine ones
*/
void* Unreal_Malloc(size_t sz, void* pUserData)
{
	return FMemory::Malloc(sz);
}

void* Unreal_Realloc(void* p, size_t sz, void* pUserData)
{
	return FMemory::Realloc(p, sz);
}

void Unreal_Free(void* p, void* pUserData)
{
	FMemory::Free(p);
}


bool URuntimeAudioImporterLibrary::CheckAndFixWavDurationErrors(TArray<uint8>& WavData)
{
	drwav_allocation_callbacks allocationCallbacksDecoding;
	allocationCallbacksDecoding.pUserData = nullptr;

	// Replacing standard methods for working with memory with engine ones
	{
		allocationCallbacksDecoding.onMalloc = Unreal_Malloc;
		allocationCallbacksDecoding.onRealloc = Unreal_Realloc;
		allocationCallbacksDecoding.onFree = Unreal_Free;
	}

	drwav wav;
	// Initializing transcoding of audio data in memory
	if (!drwav_init_memory(&wav, WavData.GetData(), WavData.Num() - 2, &allocationCallbacksDecoding))
	{
		// Callback Dispatcher OnResult
		OnResult_Internal(nullptr, FailedToReadAudioDataArray);
		UE_LOG(LogTemp, Log, TEXT("drwav init memory"));
		return false;
	}

	// Check if the container is RIFF (not Wave64 or any other containers)
	if (wav.container != drwav_container_riff)
	{
		drwav_uninit(&wav);
		return true;
	}

	/*
	* Get 4-byte field at byte 4, which is the overall file size as uint32, according to RIFF specification.
	* If the field is set to nothing (hex FFFFFFFF), replace the incorrectly set field with the actual size.
	* The field should be (size of file - 8 bytes), as the chunk identifier for the whole file (4 bytes spelling out RIFF at the start of the file), and the chunk length (4 bytes that we're replacing) are excluded.
	*/
	if (BytesToHex(WavData.GetData() + 4, 4) == "FFFFFFFF")
	{
		int32 ActualFileSize = WavData.Num() - 8;
		FMemory::Memcpy(WavData.GetData() + 4, &ActualFileSize, 4);
	}

	/*
	* Search for the place in the file after the chunk id "data", which is where the data length is stored.
	* First 36 bytes are skipped, as they're always "RIFF", 4 bytes filesize, "WAVE", "fmt ", and 20 bytes of format data.
	*/
	int32 DataSizeLocation = INDEX_NONE;
	for (int32 i = 36; i < WavData.Num() - 4; ++i)
	{
		if (BytesToHex(WavData.GetData() + i, 4) == "64617461" /* hex for string "data" */)
		{
			DataSizeLocation = i + 4;
			break;
		}
	}
	if (DataSizeLocation == INDEX_NONE) // should never happen but just in case
	{
		drwav_uninit(&wav);

		// Callback Dispatcher OnResult
		OnResult_Internal(nullptr, FailedToReadAudioDataArray);
		UE_LOG(LogTemp, Log, TEXT("DataSizeLocation"));
		return false;
	}

	// Same process as replacing full file size, except DataSize counts bytes from end of DataSize int to end of file.
	if (BytesToHex(WavData.GetData() + DataSizeLocation, 4) == "FFFFFFFF")
	{
		int32 ActualDataSize = WavData.Num() - DataSizeLocation - 4 /*-4 to not include the DataSize int itself*/;
		FMemory::Memcpy(WavData.GetData() + DataSizeLocation, &ActualDataSize, 4);
	}

	drwav_uninit(&wav);

	return true;
}


bool URuntimeAudioImporterLibrary::TranscodeAudioDataArrayToPCMData(const uint8* AudioData, uint32 AudioDataSize,
                                                                    TEnumAsByte<EAudioFormat> Format)
{
	OnProgress_Internal(10);
	switch (Format)
	{
	case Mp3:
		{
			drmp3_allocation_callbacks allocationCallbacksDecoding;
			allocationCallbacksDecoding.pUserData = nullptr;

			// Replacing standard methods for working with memory with engine ones
			{
				allocationCallbacksDecoding.onMalloc = Unreal_Malloc;
				allocationCallbacksDecoding.onRealloc = Unreal_Realloc;
				allocationCallbacksDecoding.onFree = Unreal_Free;
			}

			drmp3 mp3;
			// Initializing transcoding of audio data in memory
			if (!drmp3_init_memory(&mp3, AudioData, AudioDataSize, &allocationCallbacksDecoding))
			{
				// Callback Dispatcher OnResult
				OnResult_Internal(nullptr, FailedToReadAudioDataArray);
				return false;
			}

			// Callback Dispatcher OnProgress
			OnProgress_Internal(25);

			// Getting PCM data
			TranscodingFillInfo.PCMInfo.PCMData = static_cast<uint8*>(FMemory::Malloc(
				static_cast<size_t>(drmp3_get_pcm_frame_count(&mp3)) * mp3.channels * sizeof(float)));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(35);

			// Getting the number of frames
			TranscodingFillInfo.PCMInfo.PCMNumOfFrames = drmp3_read_pcm_frames_f32(
				&mp3, drmp3_get_pcm_frame_count(&mp3), reinterpret_cast<float*>(TranscodingFillInfo.PCMInfo.PCMData));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(45);

			// Getting PCM data size
			TranscodingFillInfo.PCMInfo.PCMDataSize = static_cast<uint32>(TranscodingFillInfo.PCMInfo.PCMNumOfFrames *
				mp3.channels * sizeof(float));

			// Getting basic audio information
			{
				TranscodingFillInfo.SoundWaveBasicInfo.Duration = static_cast<float>(drmp3_get_pcm_frame_count(&mp3)) /
					mp3.
					sampleRate;
				TranscodingFillInfo.SoundWaveBasicInfo.ChannelsNum = mp3.channels;
				TranscodingFillInfo.SoundWaveBasicInfo.SampleRate = mp3.sampleRate;
			}

			// Uninitializing transcoding of audio data in memory
			drmp3_uninit(&mp3);

			// Callback Dispatcher OnProgress
			OnProgress_Internal(55);

			return true;
		}
	case Wav:
		{
			drwav_allocation_callbacks allocationCallbacksDecoding;
			allocationCallbacksDecoding.pUserData = nullptr;

			// Replacing standard methods for working with memory with engine ones
			{
				allocationCallbacksDecoding.onMalloc = Unreal_Malloc;
				allocationCallbacksDecoding.onRealloc = Unreal_Realloc;
				allocationCallbacksDecoding.onFree = Unreal_Free;
			}

			drwav wav;
			// Initializing transcoding of audio data in memory
			if (!drwav_init_memory(&wav, AudioData, AudioDataSize, &allocationCallbacksDecoding))

			{
				// Callback Dispatcher OnResult
				OnResult_Internal(nullptr, FailedToReadAudioDataArray);
				return false;
			}

			// Callback Dispatcher OnProgress
			OnProgress_Internal(25);

			// Getting PCM data
			TranscodingFillInfo.PCMInfo.PCMData = static_cast<uint8*>(FMemory::Malloc(
				static_cast<size_t>(wav.totalPCMFrameCount) * wav.channels * sizeof(float)));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(35);

			// Getting the number of frames
			TranscodingFillInfo.PCMInfo.PCMNumOfFrames = drwav_read_pcm_frames_f32(
				&wav, wav.totalPCMFrameCount, reinterpret_cast<float*>(TranscodingFillInfo.PCMInfo.PCMData));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(45);

			// Getting PCM data size
			TranscodingFillInfo.PCMInfo.PCMDataSize = static_cast<uint32>(TranscodingFillInfo.PCMInfo.PCMNumOfFrames *
				wav.channels * sizeof(float));

			// Getting basic audio information
			{
				TranscodingFillInfo.SoundWaveBasicInfo.Duration = static_cast<float>(wav.totalPCMFrameCount) / wav.
					sampleRate;
				TranscodingFillInfo.SoundWaveBasicInfo.ChannelsNum = wav.channels;
				TranscodingFillInfo.SoundWaveBasicInfo.SampleRate = wav.sampleRate;
			}

			// Uninitializing transcoding of audio data in memory
			drwav_uninit(&wav);

			// Callback Dispatcher OnProgress
			OnProgress_Internal(55);

			return true;
		}
	case Flac:
		{
			drflac_allocation_callbacks allocationCallbacksDecoding;
			allocationCallbacksDecoding.pUserData = nullptr;

			// Replacing standard methods for working with memory with engine ones
			{
				allocationCallbacksDecoding.onMalloc = Unreal_Malloc;
				allocationCallbacksDecoding.onRealloc = Unreal_Realloc;
				allocationCallbacksDecoding.onFree = Unreal_Free;
			}

			// Initializing transcoding of audio data in memory
			drflac* pFlac = drflac_open_memory(AudioData, AudioDataSize, &allocationCallbacksDecoding);
			if (pFlac == nullptr)
			{
				// Callback Dispatcher OnResult
				OnResult_Internal(nullptr, FailedToReadAudioDataArray);
				return false;
			}

			// Callback Dispatcher OnProgress
			OnProgress_Internal(25);

			// Getting PCM data
			TranscodingFillInfo.PCMInfo.PCMData = static_cast<uint8*>(FMemory::Malloc(
				static_cast<size_t>(pFlac->totalPCMFrameCount) * pFlac->channels * sizeof(float)));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(35);

			// Getting the number of frames
			TranscodingFillInfo.PCMInfo.PCMNumOfFrames = drflac_read_pcm_frames_f32(
				pFlac, pFlac->totalPCMFrameCount, reinterpret_cast<float*>(TranscodingFillInfo.PCMInfo.PCMData));

			// Callback Dispatcher OnProgress
			OnProgress_Internal(45);

			// Getting PCM data size
			TranscodingFillInfo.PCMInfo.PCMDataSize = static_cast<uint32>(TranscodingFillInfo.PCMInfo.PCMNumOfFrames *
				pFlac->channels * sizeof(float));

			// Getting basic audio information
			{
				TranscodingFillInfo.SoundWaveBasicInfo.Duration = static_cast<float>(pFlac->totalPCMFrameCount) / pFlac
					->
					sampleRate;
				TranscodingFillInfo.SoundWaveBasicInfo.ChannelsNum = pFlac->channels;
				TranscodingFillInfo.SoundWaveBasicInfo.SampleRate = pFlac->sampleRate;
			}

			// Uninitializing transcoding of audio data in memory
			drflac_close(pFlac);

			// Callback Dispatcher OnProgress
			OnProgress_Internal(55);

			return true;
		}
	default:
		{
			// Callback Dispatcher OnResult
			OnResult_Internal(nullptr, InvalidAudioFormat);
			return false;
		}
	}
}

TEnumAsByte<EAudioFormat> URuntimeAudioImporterLibrary::GetAudioFormat(const FString& FilePath)
{
	if (FPaths::GetExtension(FilePath, false).Equals(TEXT("mp3"), ESearchCase::IgnoreCase))
	{
		return Mp3;
	}
	if (FPaths::GetExtension(FilePath, false).Equals(TEXT("wav"), ESearchCase::IgnoreCase))
	{
		return Wav;
	}
	if (FPaths::GetExtension(FilePath, false).Equals(TEXT("flac"), ESearchCase::IgnoreCase))
	{
		return Flac;
	}
	return Invalid;
}

void URuntimeAudioImporterLibrary::OnProgress_Internal(int32 Percentage)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		if (OnProgress.IsBound())
		{
			OnProgress.Broadcast(Percentage);
		}
	});
}

void URuntimeAudioImporterLibrary::OnResult_Internal(UImportedSoundWave* SoundWaveRef,
                                                     const TEnumAsByte<ETranscodingStatus>& Status)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		if (OnResult.IsBound())
		{
			OnResult.Broadcast(this, SoundWaveRef, Status);
		}
		RemoveFromRoot();
	});
}
