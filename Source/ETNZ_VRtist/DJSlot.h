// Copyright Toby Kim, ETNZ. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <complex>
#include <valarray>
#include "Components/SynthComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundGenerator.h"
#include "DSP/AudioFFT.h"
#include "DJSlot.generated.h"

typedef std::complex<float> Complex;
typedef std::valarray<Complex> CArray;

USTRUCT()
struct FFTTimeDomainData
{
	GENERATED_BODY()
		float* Buffer; // Pointer to a single channel of floats.
	int32 NumSamples; // Number of samples in InBuffer divided by the number of channels. must be a power of 2.
};
USTRUCT()
struct FFTFreqDomainData
{
	GENERATED_BODY()
		// arrays in which real and imaginary values will be populated.
	float* OutReal; // Should point to an already allocated array of floats that is FFTInputParams::NumSamples long.
	float* OutImag; // Should point to an already allocated array of floats that is FFTInputParams::NumSamples long.
};


class FDJGenerator : public ISoundGenerator
{
public:
	FDJGenerator(int32 InSampleRate, int32 InNumChannels);
	virtual ~FDJGenerator();

	//~ Begin FSoundGenerator 
	virtual int32 GetNumChannels() { return NumChannels; };
	virtual int32 OnGenerateAudio(float* OutAudio, int32 NumSamples) override;
	//~ End FSoundGenerator

	virtual void GetAudioDataFromSynthComponent(const TArray<float>& inData);
	void SetShift(float shift);
	void SetSpeed(float speed);
	void ifft(CArray& x);
	void fft(CArray& x);
	void SetWindow(int32 winSize);
	bool BufferIsEmpty = true;
	int32 GlobalPointer = 0;
	bool Pause = false;
	int32 HotCueSlot[8] = {0, 0, 0, 0, 0, 0, 0, 0};

private:
	int32 NumChannels = 2;

	TArray<float> WriteBuffer;
	TArray<float> AudioData;
	TArray<float> HWindow;
	TArray<float> LastInputPhases;
	TArray<float> LastOutputPhases;
	TArray<float> AnalysisMag;
	TArray<float> AnalysisFreq;
	TArray<float> SynthMag;
	TArray<float> SynthFreq;
	void ProcessFFT(CArray& x);
	float wrapPhase(float phaseIn);

	CArray CAudioData;
	CArray CFFTBuffer;
	CArray CFFTBuffer1;
	CArray CFFTBuffer2;
	CArray CFFTBuffer3;
	CArray CFFTBuffer4;
	CArray CFFTBuffer5;
	CArray CFFTBuffer6;
	CArray CFFTBuffer7;
	CArray SynthBuffer;

	float ShiftVal = 0.0;
	float PitchShiftRatio = powf(2.0, ShiftVal / 12.0);
	float Speed = 1.0;
	float ScaleFactor = 0.2f;

	int16 IntimeWritePointer = 0;

	int16 OutReadPointer = 0;
	int16 OutWritePointer = 0;

	int16 test = 0;
	int16 HopSize = 256;
	int16 FFTSize = 1024;
	int32 TotalNumSample;
	


};

UCLASS(ClassGroup = Synth, meta = (BlueprintSpawnableComponent))
class ETNZ_VRTIST_API UDJSlot : public USynthComponent
{
	GENERATED_BODY()

		UDJSlot(const FObjectInitializer& ObjInitializer);
	virtual ~UDJSlot();



public:

	virtual ISoundGeneratorPtr CreateSoundGenerator(const FSoundGeneratorInitParams& InParams);

	int32 GlobalPointer = 0;

	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void GetAudioDataFromBP(const TArray<float>& inData)
	{

		AudioData.Empty();

		for (size_t i = 0; i < inData.Num(); i++)
		{
			AudioData.Add(inData[i]);
		}
		TotalNumSample = AudioData.Num();
		BufferIsEmpty = false;
	};
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void SetShift(float InShift);
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void SetSpeed(float InSpeed);
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		int32 GetGlobalPtr();
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void Pause();
	UFUNCTION(BlueprintPure, Category = "DJMachine")
		bool IsPause();
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void Resume();
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void SetHotCueSlot(int index, int value);
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void HitHotCueSlot(int index);
	UFUNCTION(BlueprintCallable, Category = "DJMachine")
		void SweepGlobalPointer(int value);
private:

	ISoundGeneratorPtr DJSoundGen;


	int32 TotalNumSample;

	TArray<float> AudioData;
	float Speed;
	float PitchShift;

	bool BufferIsEmpty = true;
	int32 HotCueSlot[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
};
