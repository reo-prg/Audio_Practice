#define XAUDIO2_HELPER_FUNCTIONS
#include <DxLib.h>
#include <xaudio2fx.h>
#include <xapofx.h>
#include <xapo.h>
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

void ControlEqualizer(void);

IXAudio2* xaudio2 = nullptr;
IXAudio2MasteringVoice* masterVoice = nullptr;

IXAudio2SourceVoice* sourceVoice = nullptr;
WAVEFORMATEX waveFormat = {};

IXAudio2SubmixVoice* wetSubmix = nullptr;
IXAudio2SubmixVoice* drySubmix = nullptr;
XAUDIO2_BUFFER buffer;

WAVLoader wavLoader;

FXEQ_PARAMETERS eqParam = {};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SetOutApplicationLogValidFlag(false);
	ChangeWindowMode(true);
	SetGraphMode(1080, 720, 32);
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
	
	//IUnknown* reverb;
	//result = XAudio2CreateReverb(&reverb);
	//assert(SUCCEEDED(result));

	//XAUDIO2_EFFECT_DESCRIPTOR effectDesc = {};
	//effectDesc.InitialState = true;
	//effectDesc.OutputChannels = waveFormat.nChannels;
	//effectDesc.pEffect = reverb;

	//XAUDIO2_EFFECT_CHAIN chain = {};
	//chain.EffectCount = 1;
	//chain.pEffectDescriptors = &effectDesc;

	//wetSubmix->SetEffectChain(&chain);

	//reverb->Release();

	//XAUDIO2FX_REVERB_I3DL2_PARAMETERS i3dl2Param = XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR;
	//XAUDIO2FX_REVERB_PARAMETERS revParam = {};
	//ReverbConvertI3DL2ToNative(&i3dl2Param, &revParam);

	//result = wetSubmix->SetEffectParameters(0, &revParam, sizeof(revParam));
	//assert(SUCCEEDED(result));
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

	// ---------------------エコー-----------------------------------------------------
	//IUnknown* echo;
	//result = CreateFX(__uuidof(FXEcho), &echo);
	//assert(SUCCEEDED(result));

	//XAUDIO2_EFFECT_DESCRIPTOR effectDesc = {};
	//effectDesc.InitialState = true;
	//effectDesc.OutputChannels = waveFormat.nChannels;
	//effectDesc.pEffect = echo;

	//XAUDIO2_EFFECT_CHAIN chain = {};
	//chain.EffectCount = 1;
	//chain.pEffectDescriptors = &effectDesc;

	//result = wetSubmix->SetEffectChain(&chain);
	//assert(SUCCEEDED(result));
	//echo->Release();

	//FXECHO_PARAMETERS echoParam = {};
	//echoParam.Delay = 350.0f;
	//echoParam.Feedback = FXECHO_DEFAULT_FEEDBACK;
	//echoParam.WetDryMix = FXECHO_MAX_WETDRYMIX;

	//result = wetSubmix->SetEffectParameters(0, &echoParam, sizeof(echoParam));
	//assert(SUCCEEDED(result));
	// --------------------------------------------------------------------------------

	// ---------------------イコライザー-----------------------------------------------
	IUnknown* eq;
	result = CreateFX(CLSID_FXEQ, &eq);
	assert(SUCCEEDED(result));

	XAUDIO2_EFFECT_DESCRIPTOR effectDesc = {};
	effectDesc.InitialState = true;
	effectDesc.OutputChannels = waveFormat.nChannels;
	effectDesc.pEffect = eq;

	XAUDIO2_EFFECT_CHAIN chain = {};
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &effectDesc;

	result = drySubmix->SetEffectChain(&chain);
	assert(SUCCEEDED(result));
	eq->Release();

	eqParam.Bandwidth0 = FXEQ_DEFAULT_BANDWIDTH;
	eqParam.Bandwidth1 = FXEQ_DEFAULT_BANDWIDTH;
	eqParam.Bandwidth2 = FXEQ_DEFAULT_BANDWIDTH;
	eqParam.Bandwidth3 = FXEQ_DEFAULT_BANDWIDTH;
	eqParam.FrequencyCenter0 = FXEQ_DEFAULT_FREQUENCY_CENTER_0;
	eqParam.FrequencyCenter1 = FXEQ_DEFAULT_FREQUENCY_CENTER_1;
	eqParam.FrequencyCenter2 = FXEQ_DEFAULT_FREQUENCY_CENTER_2;
	eqParam.FrequencyCenter3 = 7000.0f;
	eqParam.Gain0 = FXEQ_DEFAULT_GAIN;
	eqParam.Gain1 = FXEQ_DEFAULT_GAIN;
	eqParam.Gain2 = FXEQ_DEFAULT_GAIN;
	eqParam.Gain3 = FXEQ_DEFAULT_GAIN;

	result = drySubmix->SetEffectParameters(0, &eqParam, sizeof(eqParam));
	assert(SUCCEEDED(result));
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
		if (GetAsyncKeyState(0x51))
		{
			currentVolume += 0.01f;
			if (currentVolume > 1.0f)
			{
				currentVolume = 1.0f;
			}
			sourceVoice->SetVolume(currentVolume);
		}
		else if (GetAsyncKeyState(0x41))
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

		ControlEqualizer();

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

		DxLib::DrawLine(640, 0, 640, 480, 0x6666ff);
		DxLib::DrawLine(0, 480, 640, 480, 0x6666ff);

		// イコライザー
		DxLib::DrawString(40, 490, std::to_wstring(static_cast<int>(eqParam.FrequencyCenter0)).c_str(), 0xffffff);
		DxLib::DrawString(140, 490, std::to_wstring(static_cast<int>(eqParam.FrequencyCenter1)).c_str(), 0xffffff);
		DxLib::DrawString(240, 490, std::to_wstring(static_cast<int>(eqParam.FrequencyCenter2)).c_str(), 0xffffff);
		DxLib::DrawString(340, 490, std::to_wstring(static_cast<int>(eqParam.FrequencyCenter3)).c_str(), 0xffffff);

		DxLib::DrawBox(55, 550, 65, 700, 0x555555, true);
		DxLib::DrawBox(155, 550, 165, 700, 0x555555, true);
		DxLib::DrawBox(255, 550, 265, 700, 0x555555, true);
		DxLib::DrawBox(355, 550, 365, 700, 0x555555, true);

		DxLib::DrawBox(45, 680 - (eqParam.Gain0 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 75, 720 - (eqParam.Gain0 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 0xccaa88, true);
		DxLib::DrawBox(145, 680 - (eqParam.Gain1 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 175, 720 - (eqParam.Gain1 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 0xccaa88, true);
		DxLib::DrawBox(245, 680 - (eqParam.Gain2 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 275, 720 - (eqParam.Gain2 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 0xccaa88, true);
		DxLib::DrawBox(345, 680 - (eqParam.Gain3 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 375, 720 - (eqParam.Gain3 - FXEQ_MIN_GAIN) / (2.0f - FXEQ_MIN_GAIN) * 150.0f, 0xccaa88, true);


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

void ControlEqualizer(void)
{
	bool f = false;
	if (GetAsyncKeyState(0x45))
	{
		eqParam.Gain0 += 0.02f;
		if (eqParam.Gain0 > 2.0f)
		{
			eqParam.Gain0 = 2.0f;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x44))
	{
		eqParam.Gain0 -= 0.02f;
		if (eqParam.Gain0 < FXEQ_MIN_GAIN)
		{
			eqParam.Gain0 = FXEQ_MIN_GAIN;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x52))
	{
		eqParam.Gain1 += 0.02f;
		if (eqParam.Gain1 > 2.0f)
		{
			eqParam.Gain1 = 2.0f;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x46))
	{
		eqParam.Gain1 -= 0.02f;
		if (eqParam.Gain1 < FXEQ_MIN_GAIN)
		{
			eqParam.Gain1 = FXEQ_MIN_GAIN;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x54))
	{
		eqParam.Gain2 += 0.02f;
		if (eqParam.Gain2 > 2.0f)
		{
			eqParam.Gain2 = 2.0f;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x47))
	{
		eqParam.Gain2 -= 0.02f;
		if (eqParam.Gain2 < FXEQ_MIN_GAIN)
		{
			eqParam.Gain2 = FXEQ_MIN_GAIN;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x59))
	{
		eqParam.Gain3 += 0.02f;
		if (eqParam.Gain3 > 2.0f)
		{
			eqParam.Gain3 = 2.0f;
		}
		f = true;
	}
	if (GetAsyncKeyState(0x48))
	{
		eqParam.Gain3 -= 0.02f;
		if (eqParam.Gain3 < FXEQ_MIN_GAIN)
		{
			eqParam.Gain3 = FXEQ_MIN_GAIN;
		}
		f = true;
	}

	if (f)
	{
		HRESULT result;
		result = drySubmix->SetEffectParameters(0, &eqParam, sizeof(eqParam));
		assert(SUCCEEDED(result));
	}
}
