#pragma once
#include <xapobase.h>


class _declspec(uuid("{81023159-D858-40CB-A586-09CEAC89FFF7}")) MyXAPO :
	public CXAPOBase
{
public:
	MyXAPO();
	~MyXAPO() {};

	STDMETHOD(LockForProcess)(UINT32 InputLockedParameterCount, 
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters,
		UINT32 OutputLockedParameterCount,
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)override;

	STDMETHOD_(void, Process)(UINT32 InputProcessParameterCount,
		const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters,
		UINT32 OutputProcessParameterCount,
		XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters,
		BOOL IsEnabled)override;
private:
	static XAPO_REGISTRATION_PROPERTIES registProp_;
	WAVEFORMATEX input_;
	WAVEFORMATEX output_;
};

