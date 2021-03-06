// Georgy Treshchev 2021.

#pragma once

#include "ImportedSoundWave.h"
#include "RuntimeAudioImporterLibrary.generated.h"

/** Possible audio importing results */
UENUM(BlueprintType, Category = "Runtime Audio Importer")
enum ETranscodingStatus
{
	/** Successful import */
	SuccessfulImport UMETA(DisplayName = "Success"),

	/** Failed to read Audio Data Array */
	FailedToReadAudioDataArray UMETA(DisplayName = "Failed to read Audio Data Array"),

	/** SoundWave declaration error */
	SoundWaveDeclarationError UMETA(DisplayName = "SoundWave declaration error"),

	/** Invalid audio format (Can't determine the format of the audio file) */
	InvalidAudioFormat UMETA(DisplayName = "Invalid audio format"),

	/** The audio file does not exist */
	AudioDoesNotExist UMETA(DisplayName = "Audio does not exist"),

	/** Load file to array error */
	LoadFileToArrayError UMETA(DisplayName = "Load file to array error")
};

/** Possible audio formats (extensions) */
UENUM(BlueprintType, Category = "Runtime Audio Importer")
enum EAudioFormat
{
	/** Determine format automatically */
	Auto UMETA(DisplayName = "Determine format automatically"),

	/** MP3 format */
	Mp3 UMETA(DisplayName = "mp3"),

	/** WAV format */
	Wav UMETA(DisplayName = "wav"),

	/** FLAC format */
	Flac UMETA(DisplayName = "flac"),

	/** Invalid format */
	Invalid UMETA(DisplayName = "invalid (not defined format, CPP use only)", Hidden)
};

/** Basic SoundWave data. CPP use only. */
struct FSoundWaveBasicStruct
{
	/** Number of channels */
	int32 ChannelsNum;

	/** Sample rate (samples per second, sampling frequency) */
	uint32 SampleRate;

	/** Sound wave duration, sec */
	float Duration;
};

/** Main, mostly in-memory information (like PCM, Wav, etc) */
struct FTranscodingFillStruct
{
	/** SoundWave basic info (e.g. duration, number of channels, etc) */
	FSoundWaveBasicStruct SoundWaveBasicInfo;

	/** PCM Data buffer */
	FPCMStruct PCMInfo;
};

// Forward declaration of the UPreImportedSoundAsset class
class UPreImportedSoundAsset;

/**
 * Delegate broadcast to get the audio importer progress
 *
 * @param Percentage Percentage of importing completed (0-100%)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioImporterProgress, const int32, Percentage);

/**
 * Delegate broadcast to get the audio importer result
 * 
 * @param RuntimeAudioImporterObject Runtime Audio Importer object reference
 * @param ReadySoundWave Ready SoundWave object reference
 * @param Status TranscodingStatus Enum in case an error occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAudioImporterResult, class URuntimeAudioImporterLibrary*,
                                               RuntimeAudioImporterObjectRef, UImportedSoundWave*, SoundWaveRef,
                                               const TEnumAsByte < ETranscodingStatus >&, Status);

/**
 * Runtime Audio Importer object
 * Designed primarily for importing audio files in real time
 */
UCLASS(BlueprintType, Category = "Runtime Audio Importer")
class RUNTIMEAUDIOIMPORTER_API URuntimeAudioImporterLibrary : public UObject
{
	GENERATED_BODY()
public:
	/** Bind to know when the transcoding is on progress */
	UPROPERTY(BlueprintAssignable, Category = "Runtime Audio Importer")
	FOnAudioImporterProgress OnProgress;

	/** Bind to know when the transcoding is complete (even if it fails) */
	UPROPERTY(BlueprintAssignable, Category = "Runtime Audio Importer")
	FOnAudioImporterResult OnResult;

	/** Transcoding fill info. CPP use only */
	FTranscodingFillStruct TranscodingFillInfo;

	/**
	 * Instantiates a RuntimeAudioImporter object
	 *
	 * @return The RuntimeAudioImporter object. Bind to it's OnProgress and OnResult delegates to know when it is in the process of importing and imported
	 */
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Create, Audio, Runtime, MP3, FLAC, WAV"), Category =
		"Runtime Audio Importer")
	static URuntimeAudioImporterLibrary* CreateRuntimeAudioImporter();

	/**
	 * Import audio from file
	 *
	 * @param FilePath Path to the audio file to import
	 * @param Format Audio file format (extension)
	 */
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Importer, Transcoder, Converter, Runtime, MP3, FLAC, WAV"),
		Category = "Runtime Audio Importer")
	void ImportAudioFromFile(const FString& FilePath,
	                         TEnumAsByte<EAudioFormat> Format);

	/**
	 * Import audio file from the preimported sound asset
	 *
	 * @param PreImportedSoundAssetRef PreImportedSoundAsset object reference. Should contain "BaseAudioDataArray" buffer
	 */
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Importer, Transcoder, Converter, Runtime, MP3"), Category =
		"Runtime Audio Importer")
	void ImportAudioFromPreImportedSound(UPreImportedSoundAsset* PreImportedSoundAssetRef);

	/**
	* Import audio data to SoundWave static object
	*
	* @param AudioDataArray Array of Audio byte data
	* @param Format Audio file format (extension)
	*/
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Importer, Transcoder, Converter, Runtime, MP3, FLAC, WAV"),
		Category = "Runtime Audio Importer")
	void ImportAudioFromBuffer(TArray<uint8>& AudioDataArray,
	                           const TEnumAsByte<EAudioFormat>& Format);
private:
	/**
	 * Internal main audio importing method
	 *
	 * @param AudioDataArray Array of Audio byte data
	 * @param Format Audio file format (extension)
	 */
	void ImportAudioFromBuffer_Internal(const TArray<uint8>& AudioDataArray, const TEnumAsByte<EAudioFormat>& Format);


	/**
	 * Define SoundWave object reference
	 *
	 * @param SoundWaveRef SoundWave object reference to define
	 * @return Whether the defining was successful or not
	 */
	bool DefineSoundWave(UImportedSoundWave* SoundWaveRef);

	/**
	 * Fill SoundWave basic information (e.g. duration, number of channels, etc)
	 *
	 * @param SoundWaveRef SoundWave object reference
	 */
	void FillSoundWaveBasicInfo(UImportedSoundWave* SoundWaveRef) const;

	/**
	 * Fill SoundWave PCM data buffer
	 *
	 * @param SoundWaveRef SoundWave object reference
	 */
	void FillPCMData(UImportedSoundWave* SoundWaveRef) const;


	/**
	 * Check if the WAV audio data with the RIFF container has a correct byte size.
	 * Made by https://github.com/kass-kass
	 *
	 * @param WavData Pointer to memory location of the Wav byte data
	 */
	bool CheckAndFixWavDurationErrors(TArray<uint8>& WavData);

	/**
	 * Transcode Audio from Audio Data to PCM Data
	 *
	 * @param AudioData Pointer to memory location of the Audio byte data
	 * @param AudioDataSize Memory size allocated for the Audio byte data
	 * @param Format Format of the audio file (e.g. mp3. flac, etc)
	 * @return Whether the transcoding was successful or not
	 */
	bool TranscodeAudioDataArrayToPCMData(const uint8* AudioData, uint32 AudioDataSize,
	                                      TEnumAsByte<EAudioFormat> Format);

	/**
	 * Get audio format by extension
	 *
	 * @param FilePath File path where to find the format (by extension)
	 * @return Returns the found audio format (e.g. mp3. flac, etc) by AudioFormat Enum
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Audio Importer")
	static TEnumAsByte<EAudioFormat> GetAudioFormat(const FString& FilePath);

	/**
	 * Audio transcoding progress callback
	 * 
	 * @param Percentage Percentage of importing completion (0-100%)
	 */
	void OnProgress_Internal(int32 Percentage);

	/**
	 * Audio importing finished callback
	 * 
	 * @param SoundWaveRef A ready SoundWave object
	 * @param Status Importing status
	 */
	void OnResult_Internal(UImportedSoundWave* SoundWaveRef, const TEnumAsByte<ETranscodingStatus>& Status);
};
