#define XAUDIO2_HELPER_FUNCTIONS
#include <DxLib.h>
#include <xaudio2fx.h>
#include <memory>
#define _USE_MATH_DEFINES
#include <math.h>
#include "WAVLoader.h"
#include <vector>
#include <cassert>

#pragma comment(lib,"xaudio2.lib")

bool XAudio2Init(void);
void XAudio2End(void);
void PlayWAVFile(const std::string& filename, IXAudio2SourceVoice** sVoice);

IXAudio2* xaudio2 = nullptr;
IXAudio2MasteringVoice* masterVoice = nullptr;

IXAudio2SourceVoice* sourceVoice = nullptr;
WAVEFORMATEX waveFormat = {};

IXAudio2SubmixVoice* submixVoice = nullptr;
XAUDIO2_BUFFER buffer;

WAVLoader wavLoader;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SetOutApplicationLogValidFlag(false);
	ChangeWindowMode(true);
	SetGraphMode(640, 480, 32);
	SetMainWindowText(_T("XAudio2Practice"));
	DxLib_Init();
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (!XAudio2Init())
	{
		DxLib::DxLib_End();
		return -1;
	}
	HRESULT result;
	int image;
	image = DxLib::LoadGraph(L"Resource/Image/speaker.png");

	XAUDIO2_VOICE_DETAILS master = {};
	masterVoice->GetVoiceDetails(&master);

	wavLoader.LoadWAVFile("Resource/Sound/Techno_1.wav");
	PlayWAVFile("Resource/Sound/Techno_1.wav", &sourceVoice);


	// ---------------------サブミックス-----------------------------------------------
	xaudio2->CreateSubmixVoice(&submixVoice, waveFormat.nChannels, waveFormat.nSamplesPerSec, XAUDIO2_VOICE_USEFILTER);

	XAUDIO2_SEND_DESCRIPTOR desc;
	desc.Flags = 0;
	desc.pOutputVoice = submixVoice;
	XAUDIO2_VOICE_SENDS send = { 1, &desc };

	sourceVoice->SetOutputVoices(&send);
	sourceVoice->SetVolume(0.7f);
	// --------------------------------------------------------------------------------

	// ---------------------リバーブ---------------------------------------------------
	
	IUnknown* reverb;
	result = XAudio2CreateReverb(&reverb);
	assert(SUCCEEDED(result));

	XAUDIO2_EFFECT_DESCRIPTOR effectDesc = {};
	effectDesc.InitialState = true;
	effectDesc.OutputChannels = waveFormat.nChannels;
	effectDesc.pEffect = reverb;

	XAUDIO2_EFFECT_CHAIN chain = {};
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &effectDesc;

	submixVoice->SetEffectChain(&chain);

	reverb->Release();
	// --------------------------------------------------------------------------------
	

	// ---------------------速度変更---------------------------------------------------
	//sourceVoice->SetFrequencyRatio(2.0f);
	// --------------------------------------------------------------------------------

	// ----------------------フィルタ--------------------------------------------------
	//XAUDIO2_FILTER_PARAMETERS filter = {};
	//filter.Type = XAUDIO2_FILTER_TYPE::NotchFilter;
	//filter.Frequency = 0.1f;
	//filter.OneOverQ = 1.0f;
	//result = sourceVoice->SetFilterParameters(&filter);
	// --------------------------------------------------------------------------------

	// ---------------------パンの初期化-----------------------------------------------

	XAUDIO2_VOICE_DETAILS source = {};
	sourceVoice->GetVoiceDetails(&source);
	std::vector<float> Volumes;
	Volumes.resize(source.InputChannels * master.InputChannels, 0.0f);
	Volumes[0] = 0.707f;
	Volumes[source.InputChannels * master.InputChannels - 1] = 0.707f;
	result = submixVoice->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
	// --------------------------------------------------------------------------------

	float currentAngle = 0.0f;
	XAUDIO2_VOICE_STATE state;
	while (!CheckHitKey(KEY_INPUT_ESCAPE) && DxLib::ProcessMessage() == 0)
	{
		ClsDrawScreen();

		// ----------------------キューのチェック---------------------------------
		sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		if (state.BuffersQueued == 0) { break; }
		// -----------------------------------------------------------------------



		// ------------------------パン--------------------------------------------
		if (GetAsyncKeyState(VK_LEFT))
		{
			currentAngle -= 0.03f;
			if (currentAngle < 0.0f)
			{
				currentAngle = 0.0f;
			}
			Volumes[0] = cosf(currentAngle);
			//Volumes[1] = cosf(currentAngle);
			//Volumes[2] = sinf(currentAngle);
			Volumes[source.InputChannels * master.InputChannels - 1] = sinf(currentAngle);
			result = submixVoice->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
		
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			currentAngle += 0.03f;
			if (currentAngle > M_PI / 2.0f)
			{
				currentAngle = M_PI / 2.0f;
			}
			Volumes[0] = cosf(currentAngle);
			//Volumes[1] = cosf(currentAngle);
			//Volumes[2] = sinf(currentAngle);
			Volumes[source.InputChannels * master.InputChannels - 1] = sinf(currentAngle);
			result = submixVoice->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
		}
		// -------------------------------------------------------------------------


		// -----------描画----------------------------------------------------------
		DxLib::DrawBox(285, 430, 355, 450, 0xffffff, true);
		DxLib::DrawBox(290, 410, 350, 470, 0x4444ff, true);
		DxLib::DrawCircle(320.0f + cosf(M_PI - currentAngle * 2.0f) * 240.0f, 440.0f - sinf(currentAngle * 2.0f) * 240.0f,
			40.0f, 0x880088, true);
		DxLib::DrawGraph(288.0f + cosf(M_PI - currentAngle * 2.0f) * 240.0f, 408.0f - sinf(currentAngle * 2.0f) * 240.0f,
			image, true);

		DxLib::ScreenFlip();
	}

	if(sourceVoice != nullptr)
	sourceVoice->DestroyVoice();

	if (submixVoice != nullptr)
		submixVoice->DestroyVoice();

	XAudio2End();
	DxLib::DxLib_End();
	return 0;
}

bool XAudio2Init(void)
{
	HRESULT result;
	result = XAudio2Create(&xaudio2);
	if (FAILED(result)) { DxLib::DxLib_End(); return false; }

	result = xaudio2->CreateMasteringVoice(&masterVoice);
	if (FAILED(result)) { xaudio2->Release(); xaudio2 = nullptr; DxLib::DxLib_End(); return false; }

	XAUDIO2_DEBUG_CONFIGURATION debug = { 0 };
	debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	debug.BreakMask = XAUDIO2_LOG_ERRORS;
	xaudio2->SetDebugConfiguration(&debug, 0);

	return true;
}

void XAudio2End(void)
{
	if (masterVoice != nullptr)
	{
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}
	if (xaudio2 != nullptr)
	{
		xaudio2->Release();
		xaudio2 = nullptr;
	}
}

void PlayWAVFile(const std::string& filename, IXAudio2SourceVoice** sVoice)
{
	HRESULT result;

	const auto& data = wavLoader.GetWAVFile(filename);

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = data.fmt_.channel_;
	waveFormat.nSamplesPerSec = data.fmt_.samplesPerSec_;
	waveFormat.nAvgBytesPerSec = data.fmt_.bytePerSec_;
	waveFormat.nBlockAlign = data.fmt_.blockAlign_;
	waveFormat.wBitsPerSample = data.fmt_.bitPerSample_;

	buffer.AudioBytes = data.dataSize_;
	buffer.pAudioData = data.data_;
	buffer.PlayBegin = 0;
	buffer.PlayLength = 0;
	buffer.LoopBegin = 0;
	buffer.LoopCount = 0;
	buffer.LoopLength = 0;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	result = xaudio2->CreateSourceVoice(sVoice, &waveFormat, XAUDIO2_VOICE_USEFILTER, 4.0f);
	if (FAILED(result)) { return; }

	result = (*sVoice)->SubmitSourceBuffer(&buffer);
	if (FAILED(result)) { return; }

	(*sVoice)->Start();
}
