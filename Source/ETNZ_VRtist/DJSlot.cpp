// Copyright Toby Kim, ETNZ. All Rights Reserved.


#include "DJSlot.h"

FDJGenerator::FDJGenerator(int32 InSampleRate, int32 InNumChannels)
	: NumChannels(InNumChannels)
{
	WriteBuffer.SetNum(8192);
	HWindow.SetNum(FFTSize);
	LastInputPhases.SetNum(FFTSize);
	LastOutputPhases.SetNum(FFTSize);
	AnalysisMag.SetNum(FFTSize / 2 + 1);
	AnalysisFreq.SetNum(FFTSize / 2 + 1);
	SynthMag.SetNum(FFTSize / 2 + 1);
	SynthFreq.SetNum(FFTSize / 2 + 1);

	SetWindow(FFTSize);
	CFFTBuffer.resize(FFTSize);
	CFFTBuffer1.resize(FFTSize);
	CFFTBuffer2.resize(FFTSize);
	CFFTBuffer3.resize(FFTSize);
	CFFTBuffer4.resize(FFTSize);
	CFFTBuffer5.resize(FFTSize);
	CFFTBuffer6.resize(FFTSize);
	CFFTBuffer7.resize(FFTSize);
	SynthBuffer.resize(FFTSize);
}
FDJGenerator::~FDJGenerator()
{
}

void FDJGenerator::SetWindow(int32 winSize)
{
	for (size_t i = 0; i < winSize; i++)
	{
		HWindow[i] = .5f * (1.f - cosf(2.f * 3.1415926535897932 * i / (float)(winSize - 1)));
	}
}
void FDJGenerator::fft(CArray& x)
{
	// DFT
	unsigned int N = x.size(), k = N, n;
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
	while (k > 1)
	{
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; l++)
		{
			for (unsigned int a = l; a < N; a += n)
			{
				unsigned int b = a + k;
				Complex t = x[a] - x[b];
				x[a] += x[b];
				x[b] = t * T;
			}
			T *= phiT;
		}
	}
	// Decimate


	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; a++)
	{
		unsigned int b = a;
		// Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a)
		{
			Complex t = x[a];
			x[a] = x[b];
			x[b] = t;
		}
	}


	//// Normalize (This section make it not working correctly)
	//Complex f = 1.0 / sqrt(N);
	//for (unsigned int i = 0; i < N; i++)
	//	x[i] *= f;
}

// inverse fft (in-place)
void FDJGenerator::ifft(CArray& x)
{
	// conjugate the complex numbers
	x = x.apply(std::conj);

	// forward fft
	fft(x);

	// conjugate the complex numbers again
	x = x.apply(std::conj);

	// scale the numbers
	x /= x.size();
}
float FDJGenerator::wrapPhase(float phaseIn)
{
	if (phaseIn >= 0)
		return fmodf(phaseIn + PI, 2.0 * PI) - PI;
	else
		return fmodf(phaseIn - PI, -2.0 * PI) + PI;
}
void FDJGenerator::SetShift(float shift)
{
	SynthCommand([this, shift]()
		{
			ShiftVal = shift;
		});
}
void FDJGenerator::SetSpeed(float speed)
{
	SynthCommand([this, speed]()
		{
			Speed = speed;
		});
}
void FDJGenerator::ProcessFFT(CArray& x)
{
	for (size_t i = 0; i < FFTSize / 2; i++)
	{
		float amplitude = std::abs(x[i]);
		float phase = std::arg(x[i]);

		float phaseDiff = phase - LastInputPhases[i];

		float binCentreFrequency = 2.f * PI * (float)i / (float)FFTSize;
		phaseDiff = wrapPhase(phaseDiff - binCentreFrequency * (float)HopSize);

		float binDeviation = phaseDiff * (float)FFTSize / (float)HopSize / (2.f * PI);
		AnalysisFreq[i] = (float)i + binDeviation;
		AnalysisMag[i] = amplitude;

		LastInputPhases[i] = phase;
	}

	for (size_t i = 0; i < FFTSize / 2; i++)
	{
		SynthMag[i] = SynthFreq[i] = 0;
	}

	for (size_t i = 0; i < FFTSize / 2; i++)
	{
		int newBin = floorf(i * PitchShiftRatio + .5);

		if (newBin <= FFTSize / 2) {
			SynthMag[newBin] += AnalysisMag[i];
			SynthFreq[newBin] = AnalysisFreq[i] * PitchShiftRatio;
		}
	}

	for (size_t i = 0; i < FFTSize / 2; i++)
	{
		float amplitude = SynthMag[i];

		float binDeviation = SynthFreq[i] - i;

		float phaseDiff = binDeviation * 2.f * PI * (float)HopSize / (float)FFTSize;

		float binCentreFrequency = 2.f * PI * (float)i / (float)FFTSize;
		phaseDiff += binCentreFrequency * (float)HopSize;

		float outPhase = wrapPhase(LastOutputPhases[i] + phaseDiff);

		x[i].real(amplitude * cosf(outPhase));
		x[i].imag(amplitude * sinf(outPhase));

		if (i > 0 && i < FFTSize / 2) {
			x[FFTSize - i].real(x[i].real());
			x[FFTSize - i].imag(-1.f * x[i].imag());
		}
		LastOutputPhases[i] = outPhase;
	}
}
int32 FDJGenerator::OnGenerateAudio(float* OutAudio, int32 NumSamples)
{
	if (Pause) {
		for (size_t i = 0; i < NumSamples; i++)
		{
			OutAudio[i] = 0.f;
		}
		return NumSamples;
	}
	NumSamples = 2024;
	PitchShiftRatio = powf(2.0, ShiftVal / 12.0);
	check(NumChannels != 0);
	if (GlobalPointer >= TotalNumSample) {
		return 0;
	}
	if (GlobalPointer + NumSamples + HopSize * 7 >= TotalNumSample) {
		NumSamples = TotalNumSample - GlobalPointer - (HopSize * 7);
		return 0;
	}

	for (size_t i = 0; i < NumSamples; i += 2)
	{
		CFFTBuffer[i / 2] = CAudioData[GlobalPointer + (i)] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer1[i / 2] = CAudioData[GlobalPointer + (i)+HopSize * 2] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer2[i / 2] = CAudioData[GlobalPointer + (i)+HopSize * 4] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer3[i / 2] = CAudioData[GlobalPointer + (i)+HopSize * 6] * HWindow[i / 2] * ScaleFactor;

		CFFTBuffer4[i / 2] = CAudioData[GlobalPointer + (i + 1)] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer5[i / 2] = CAudioData[GlobalPointer + (i + 1)+HopSize * 2] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer6[i / 2] = CAudioData[GlobalPointer + (i + 1)+HopSize * 4] * HWindow[i / 2] * ScaleFactor;
		CFFTBuffer7[i / 2] = CAudioData[GlobalPointer + (i + 1)+HopSize * 6] * HWindow[i / 2] * ScaleFactor;
	}
	fft(CFFTBuffer);
	//ProcessFFT(CFFTBuffer);
	ifft(CFFTBuffer);
	fft(CFFTBuffer1);
	//ProcessFFT(CFFTBuffer1);
	ifft(CFFTBuffer1);

	fft(CFFTBuffer2);
	//ProcessFFT(CFFTBuffer2);
	ifft(CFFTBuffer2);
	fft(CFFTBuffer3);
	//ProcessFFT(CFFTBuffer3);
	ifft(CFFTBuffer3);

	fft(CFFTBuffer4);
	//ProcessFFT(CFFTBuffer4);
	ifft(CFFTBuffer4);
	fft(CFFTBuffer5);
	//ProcessFFT(CFFTBuffer5);
	ifft(CFFTBuffer5);

	fft(CFFTBuffer6);
	//ProcessFFT(CFFTBuffer6);
	ifft(CFFTBuffer6);
	fft(CFFTBuffer7);
	//ProcessFFT(CFFTBuffer7);
	ifft(CFFTBuffer7);

	for (size_t i = 0; i < NumSamples; i += 2)
	{

		WriteBuffer[(i + 4096 + IntimeWritePointer) % 8192] += CFFTBuffer[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(i + 4096 + IntimeWritePointer + HopSize * 2) % 8192] += CFFTBuffer1[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(i + 4096 + IntimeWritePointer + HopSize * 4) % 8192] += CFFTBuffer2[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(i + 4096 + IntimeWritePointer + HopSize * 6) % 8192] += CFFTBuffer3[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		

		WriteBuffer[(1 + i + 4096 + IntimeWritePointer) % 8192] += CFFTBuffer4[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(1 + i + 4096 + IntimeWritePointer + HopSize * 2) % 8192] += CFFTBuffer5[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(1 + i + 4096 + IntimeWritePointer + HopSize * 4) % 8192] += CFFTBuffer6[i / 2].real() * HWindow[i / 2] * ScaleFactor;
		WriteBuffer[(1 + i + 4096 + IntimeWritePointer + HopSize * 6) % 8192] += CFFTBuffer7[i / 2].real() * HWindow[i / 2] * ScaleFactor;

	}

	for (size_t i = 0; i < NumSamples; i++)
	{
		OutAudio[i] = WriteBuffer[(8192 + i + IntimeWritePointer) % 8192] * 16;
		WriteBuffer[(8192 + i + IntimeWritePointer) % 8192] = 0.f;
	}
	IntimeWritePointer += NumSamples;
	if (IntimeWritePointer >= 8192) {
		IntimeWritePointer -= 8192;
	}
	CFFTBuffer = CFFTBuffer.shift(FFTSize);
	CFFTBuffer1 = CFFTBuffer1.shift(FFTSize);
	CFFTBuffer2 = CFFTBuffer2.shift(FFTSize);
	CFFTBuffer3 = CFFTBuffer3.shift(FFTSize);
	CFFTBuffer4 = CFFTBuffer4.shift(FFTSize);
	CFFTBuffer5 = CFFTBuffer5.shift(FFTSize);
	CFFTBuffer6 = CFFTBuffer6.shift(FFTSize);
	CFFTBuffer7 = CFFTBuffer7.shift(FFTSize);
	GlobalPointer += (int32)(NumSamples * Speed);
	return NumSamples;

}

void FDJGenerator::GetAudioDataFromSynthComponent(const TArray<float>& inData)
{
	AudioData.Empty();
	CAudioData.resize(inData.Num());
	for (size_t i = 0; i < inData.Num(); i++)
	{
		AudioData.Add(inData[i]);
		CAudioData[i] = inData[i];
	}
	TotalNumSample = AudioData.Num();
}

// Sets default values
UDJSlot::UDJSlot(const FObjectInitializer& ObjInitializer)
	: Super(ObjInitializer)
{
	NumChannels = 2;
}

UDJSlot::~UDJSlot()
{
}


ISoundGeneratorPtr UDJSlot::CreateSoundGenerator(const FSoundGeneratorInitParams& InParams)
{
	DJSoundGen = ISoundGeneratorPtr(new FDJGenerator(InParams.SampleRate, InParams.NumChannels));
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->GetAudioDataFromSynthComponent(AudioData);
		ToneGen->BufferIsEmpty = false;
		ToneGen->GlobalPointer = GlobalPointer;
	}


	return DJSoundGen;
}


void UDJSlot::SetShift(float InShift)
{
	PitchShift = InShift;
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->SetShift(InShift);
	}
}
void UDJSlot::SetSpeed(float InSpeed)
{
	Speed = InSpeed;
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->SetSpeed(InSpeed);
	}
}
int32 UDJSlot::GetGlobalPtr()
{
	int32 GlobalPtr = 0;
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		GlobalPtr = ToneGen->GlobalPointer;
		return GlobalPtr;
	}
	else {
		return GlobalPointer;
	}
	
}
void UDJSlot::Pause()
{
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->Pause = true;
	}
}
bool UDJSlot::IsPause()
{
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		return ToneGen->Pause;
	}
	return false;
}
void UDJSlot::Resume()
{
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->Pause = false;
	}
}

void UDJSlot::SetHotCueSlot(int index, int value)
{
		
		HotCueSlot[index] = value;
}

void UDJSlot::HitHotCueSlot(int index)
{
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->GlobalPointer = HotCueSlot[index];
	}
	GlobalPointer = HotCueSlot[index];
}

void UDJSlot::SweepGlobalPointer(int value)
{
	if (DJSoundGen.IsValid())
	{
		FDJGenerator* ToneGen = static_cast<FDJGenerator*>(DJSoundGen.Get());
		ToneGen->GlobalPointer += value * 256;
		if (ToneGen->GlobalPointer < 0) {
			ToneGen->GlobalPointer = 0;
		}
		if (ToneGen->GlobalPointer > TotalNumSample - 1) {
			ToneGen->GlobalPointer = TotalNumSample -1;
		}
	}
	
		GlobalPointer += value * 256;
		if (GlobalPointer < 0) {
			GlobalPointer = 0;
		}
		if (GlobalPointer > TotalNumSample - 1) {
			GlobalPointer = TotalNumSample - 1;
		}
	
}