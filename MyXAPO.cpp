#include "MyXAPO.h"

XAPO_REGISTRATION_PROPERTIES MyXAPO::registProp_ =
{
	_uuidof(MyXAPO),
	L"TestXAPO",
	L"me :)",
	1,0,
	XAPOBASE_DEFAULT_FLAG,
	1, 1, 1, 1
};

MyXAPO::MyXAPO():CXAPOParametersBase(&registProp_, (BYTE*)(param_), sizeof(TestParameter), false)
{
}

STDMETHODIMP_(HRESULT __stdcall) 
MyXAPO::LockForProcess(UINT32 InputLockedParameterCount, 
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters, 
	UINT32 OutputLockedParameterCount, 
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)
{
	input_ = *pInputLockedParameters->pFormat;
	output_ = *pOutputLockedParameters->pFormat;

	return CXAPOParametersBase::LockForProcess(InputLockedParameterCount,
		pInputLockedParameters, OutputLockedParameterCount, 
		pOutputLockedParameters);
}

STDMETHODIMP_(void __stdcall) 
MyXAPO::Process(UINT32 InputProcessParameterCount, 
	const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters, 
	UINT32 OutputProcessParameterCount, 
	XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters, 
	BOOL IsEnabled)
{
	TestParameter* param = (TestParameter*)BeginProcess();

	const XAPO_PROCESS_BUFFER_PARAMETERS& inputPrm = pInputProcessParameters[0];
	XAPO_PROCESS_BUFFER_PARAMETERS& outputPrm = pOutputProcessParameters[0];

	memcpy(outputPrm.pBuffer, inputPrm.pBuffer, output_.nBlockAlign * inputPrm.ValidFrameCount);

	outputPrm.ValidFrameCount = inputPrm.ValidFrameCount;
	outputPrm.BufferFlags = inputPrm.BufferFlags;

	EndProcess();
}

STDMETHODIMP_(void __stdcall) 
MyXAPO::SetParameters(const void* pParameters, UINT32 ParameterByteSize)
{
	if (ParameterByteSize == sizeof(TestParameter))
	{
		CXAPOParametersBase::SetParameters(pParameters, ParameterByteSize);
	}
}
