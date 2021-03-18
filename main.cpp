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

IXAudio2SubmixVoice* wetSubmix = nullptr;
IXAudio2SubmixVoice* drySubmix = nullptr;
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
	xaudio2->CreateSubmixVoice(&wetSubmix, waveFormat.nChannels, waveFormat.nSamplesPerSec, XAUDIO2_VOICE_USEFILTER);
	xaudio2->CreateSubmixVoice(&drySubmix, waveFormat.nChannels, waveFormat.nSamplesPerSec, XAUDIO2_VOICE_USEFILTER);

	XAUDIO2_SEND_DESCRIPTOR desc[2];
	desc[0].Flags = 0;
	desc[0].pOutputVoice = wetSubmix;
	desc[1].Flags = 0;
	desc[1].pOutputVoice = drySubmix;
	XAUDIO2_VOICE_SENDS send = { 2, desc };

	sourceVoice->SetOutputVoices(&send);

	float currentVolume = 0.5f;
	sourceVoice->SetVolume(currentVolume);
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

	wetSubmix->SetEffectChain(&chain);

	reverb->Release();

	XAUDIO2FX_REVERB_I3DL2_PARAMETERS i3dl2Param = XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR;
	XAUDIO2FX_REVERB_PARAMETERS revParam = {};
	ReverbConvertI3DL2ToNative(&i3dl2Param, &revParam);

	result = wetSubmix->SetEffectParameters(0, &revParam, sizeof(revParam));
	assert(SUCCEEDED(result));
	// --------------------------------------------------------------------------------
	
	// ---------------------ボリュームメーター-----------------------------------------
	IUnknown* volumeMeter;
	result = XAudio2CreateVolumeMeter(&volumeMeter);
	assert(SUCCEEDED(result));

	XAUDIO2_EFFECT_DESCRIPTOR vmEffectDesc = {};
	vmEffectDesc.InitialState = true;
	vmEffectDesc.OutputChannels = master.InputChannels;
	vmEffectDesc.pEffect = volumeMeter;

	XAUDIO2_EFFECT_CHAIN vmChain = {};
	vmChain.EffectCount = 1;
	vmChain.pEffectDescriptors = &vmEffectDesc;

	masterVoice->SetEffectChain(&vmChain);

	volumeMeter->Release();

	XAUDIO2FX_VOLUMEMETER_LEVELS vmLevel = {};
	float PeakLevels[2];
	float RMSLevels[2];

	vmLevel.ChannelCount = master.InputChannels;
	vmLevel.pPeakLevels = PeakLevels;
	vmLevel.pRMSLevels = RMSLevels;
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
	result = wetSubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
	result = drySubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
	// --------------------------------------------------------------------------------

	float currentAngle = M_PI / 4.0f;
	float currentWetDry = 0.0f;
	XAUDIO2_VOICE_STATE state;
	{
		float w = 0.6f * currentWetDry * currentWetDry;
		wetSubmix->SetVolume(w);
		drySubmix->SetVolume(1.0f - w);
	}
	while (!CheckHitKey(KEY_INPUT_ESCAPE) && DxLib::ProcessMessage() == 0)
	{
		ClsDrawScreen();

		masterVoice->GetEffectParameters(0, &vmLevel, sizeof(vmLevel));

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
			result = wetSubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
			result = drySubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
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
			result = wetSubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
			result = drySubmix->SetOutputMatrix(masterVoice, waveFormat.nChannels, master.InputChannels, Volumes.data());
		}
		// -------------------------------------------------------------------------

		// -----------音量調整------------------------------------------------------
		if (GetAsyncKeyState(0x57))
		{
			currentVolume += 0.01f;
			if (currentVolume > 1.0f)
			{
				currentVolume = 1.0f;
			}
			sourceVoice->SetVolume(currentVolume);
		}
		else if (GetAsyncKeyState(0x53))
		{
			currentVolume -= 0.01f;
			if (currentVolume < 0.0f)
			{
				currentVolume = 0.0f;
			}
			sourceVoice->SetVolume(currentVolume);
		}
		// -------------------------------------------------------------------------
		// -----------リバーブ調整--------------------------------------------------
		if (GetAsyncKeyState(VK_UP))
		{
			currentWetDry += 0.01f;
			if (currentWetDry > 1.0f)
			{
				currentWetDry = 1.0f;
			}
			float w = 0.6f * currentWetDry * currentWetDry;
			wetSubmix->SetVolume(w);
			drySubmix->SetVolume(1.0f - w);
		}
		else if (GetAsyncKeyState(VK_DOWN))
		{
			currentWetDry -= 0.01f;
			if (currentWetDry < 0.0f)
			{
				currentWetDry = 0.0f;
			}
			float w = 0.6f * currentWetDry * currentWetDry;
			wetSubmix->SetVolume(w);
			drySubmix->SetVolume(1.0f - w);
		}
		// -------------------------------------------------------------------------

		// -----------描画----------------------------------------------------------
		DxLib::DrawBox(285, 430, 355, 450, 0xffffff, true);
		DxLib::DrawBox(290, 410, 350, 470, 0x4444ff, true);
		DxLib::DrawCircle(320.0f + cosf(M_PI - currentAngle * 2.0f) * 160.0f, 440.0f - sinf(currentAngle * 2.0f) * 240.0f,
			40.0f, 0x880088, true);
		DxLib::DrawGraph(288.0f + cosf(M_PI - currentAngle * 2.0f) * 160.0f, 408.0f - sinf(currentAngle * 2.0f) * 240.0f,
			image, true);

		DxLib::DrawString(40.0f, 40.0f, L"Volume", 0x8888ff, 0xffffff);
		DxLib::DrawBox(60.0f, 60.0f, 60.0f + currentVolume * 200.0f, 80.0f, 0x00ff00, true);
		DxLib::DrawBox(60.0f, 60.0f, 260.0f, 80.0f, 0xffffff, false);

		DxLib::DrawString(40.0f, 80.0f, L"Reverb Strength", 0x55ff55, 0xffffff);
		DxLib::DrawBox(60.0f, 100.0f, 60.0f + currentWetDry * 200.0f, 120.0f, 0x00ff00, true);
		DxLib::DrawBox(60.0f, 100.0f, 260.0f, 120.0f, 0xffffff, false);

		DxLib::DrawString(300.0f, 10.0f, (std::wstring(L"Left_Peak :  ") + std::to_wstring(vmLevel.pPeakLevels[0])).c_str(), 0xdd3344);
		DxLib::DrawString(300.0f, 30.0f, (std::wstring(L"Right_Peak : ") + std::to_wstring(vmLevel.pPeakLevels[1])).c_str(), 0xdd3344);
		DxLib::DrawString(300.0f, 50.0f, (std::wstring(L"Left_Avg :   ") + std::to_wstring(vmLevel.pRMSLevels[0])).c_str(), 0x33dd44);
		DxLib::DrawString(300.0f, 70.0f, (std::wstring(L"Right_Avg :  ") + std::to_wstring(vmLevel.pRMSLevels[1])).c_str(), 0x33dd44);

		DxLib::DrawBox(10.0f, 320.0f - vmLevel.pPeakLevels[0] * 100.0f, 25.0f, 320.0f, 0xdd3344, true);
		DxLib::DrawBox(590.0f, 320.0f - vmLevel.pPeakLevels[1] * 100.0f, 605.0f, 320.0f, 0xdd3344, true);
		DxLib::DrawBox(30.0f, 320.0f - vmLevel.pRMSLevels[0] * 100.0f, 45.0f, 320.0f, 0x33dd44, true);
		DxLib::DrawBox(610.0f, 320.0f - vmLevel.pRMSLevels[1] * 100.0f, 625.0f, 320.0f, 0x33dd44, true);

		DxLib::DrawBox(10.0f, 220.0f, 25.0f, 320.0f, 0xffffff, false);
		DxLib::DrawBox(590.0f, 220.0f, 605.0f, 320.0f, 0xffffff, false);
		DxLib::DrawBox(30.0f, 220.0f, 45.0f, 320.0f, 0xffffff, false);
		DxLib::DrawBox(610.0f, 220.0f, 625.0f, 320.0f, 0xffffff, false);

		DxLib::ScreenFlip();
	}

	if(sourceVoice != nullptr)
	sourceVoice->DestroyVoice();

	if (wetSubmix != nullptr)
		wetSubmix->DestroyVoice();
	if (drySubmix != nullptr)
		drySubmix->DestroyVoice();

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
