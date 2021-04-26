#pragma once
#include <xaudio2.h>
#include <xapo.h>
#include <xapobase.h>

#define XAPO_PARAMETER_ELEMENTS_COUNT 3

struct TestParameter
{
	float param_ = 0.0f;
};

class _declspec(uuid("{81023159-D858-40CB-A586-09CEAC89FFF7}")) ReceiveXAPO :
	public CXAPOParametersBase
{
public:
	ReceiveXAPO();
	~ReceiveXAPO() {};

	STDMETHOD(LockForProcess)(UINT32 InputLockedParameterCount, 
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters,
		UINT32 OutputLockedParameterCount,
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)override;

	STDMETHOD_(void, Process)(UINT32 InputProcessParameterCount,
		const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters,
		UINT32 OutputProcessParameterCount,
		XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters,
		BOOL IsEnabled)override;

	STDMETHOD_(void, SetParameters)(const void* pParameters, UINT32 ParameterByteSize)override;
private:
	static XAPO_REGISTRATION_PROPERTIES registProp_;
	WAVEFORMATEX input_;
	WAVEFORMATEX output_;

	TestParameter param_[XAPO_PARAMETER_ELEMENTS_COUNT];
};

