/*!
\file VCSystem.h
\brief Header class for main VC System class.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: VCSystem.cpp

Notes:

*/

/*
   
*/

#include "Defines.h"

#include "VCSystem.h"

#include "LogOutput_TextFile.h"

//SAPI
#include <sphelper.h>

#include "grammar/grammar.h"

namespace TRC
{
	namespace VCS
	{
		//=====================================================
		//Function: CVCSystem::Init()
		//Last Revised: 29.11.2006
		//	Initialize Voice Controll System.
		//=====================================================
		void CVCSystem::Init()
		{
			bShouldQuit = false;
			TTSVoice = NULL;
			logger.Init();

			textOutput = new CLogOutput_TextFile("vcs.log");

			if(textOutput)
			{
				textOutput->Init();
			}

			logger.AddLogOutput(textOutput, LMTF_Default);

			try
			{
				//-------- vvv SAPI INITIALIZATION
				HRESULT hRes;

				logger.Log(LMT_Info, "CVCSystem::Init() - Initializing SAPI");
				if(FAILED(CoInitialize(NULL)))
				{
					throw std::runtime_error("Failed initializing COM");
				}
				logger.Log(LMT_Success, "CVCSystem::Init() - COM initialized!");

				hRes = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&TTSVoice);
				if(FAILED(hRes))
				{
					TTSVoice = NULL;
					throw std::runtime_error("Failed to initialize TTS Voice");
				}
				logger.Log(LMT_Success, "CVCSystem::Init() - TTS Voice initialized!");

				hRes = recoEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
				if(FAILED(hRes))
				{
					throw std::runtime_error("Failed to initialize Recognition Engine");
				}
				logger.Log(LMT_Success, "CVCSystem::Init() - Recognition Engine initialized!");

				hRes = recoEngine->CreateRecoContext(&recoContext);
				if(FAILED(hRes))
				{
					throw std::runtime_error("Failed to create Core Recognition Context");
				}
				logger.Log(LMT_Success, "CVCSystem::Init() - Core Recognition Context initialized!");

				hRes = recoContext->CreateGrammar(CORE_GRAMMAR_ID, &recoGrammar);
				if(FAILED(hRes))
				{
					throw std::runtime_error("Failed to create Core Grammar!");
				}
				recoContext->SetNotifyWin32Event();
				//recoContext->SetInterest( SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS), SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS) );
				recoContext->SetInterest( SPFEI(SPEI_RECOGNITION) , SPFEI(SPEI_RECOGNITION) );
				logger.Log(LMT_Success, "CVCSystem::Init() - Core Grammar created!");

				hRes = recoGrammar->LoadCmdFromFile(L"grammar/core.xml", SPLO_STATIC);
				if(FAILED(hRes))
				{
					throw std::runtime_error("Failed to load Core Grammar from file!");
				}
				recoGrammar->SetRuleState( NULL, NULL, SPRS_ACTIVE );
				logger.Log(LMT_Success, "CVCSystem::Init() - Core Grammar loaded!");

				logger.Log(LMT_Success, "CVCSystem::Init() - SAPI initialized!");
				//-------- ^^^ SAPI INITIALIZED
				soundList.resize(S_MaxSounds);
				soundList[S_Exit] = "exit.wav";
				soundList[S_Error] = "error.wav";
				soundList[S_Accepted] = "accepted.wav";
				soundList[S_Executing] = "executing.wav";
				soundList[S_NotYetImplemented] = "nyi.wav";
				soundList[S_RestateCommand] = "restate.wav";
				soundList[S_Activate] = "activate.wav";
				soundList[S_Deny] = "deny.wav";

				winAmpController.Init();
			}
			catch(std::exception& ex)
			{
				logger.Log(LMT_Fatal, boost::format("CVCSystem::Init() - Failed to initialize VC System - %s") % ex.what());
				bShouldQuit = true;
			}
			return;
		}

		//=====================================================
		//Function: CVCSystem::DeInit()
		//Last Revised: 29.11.2006
		//	Deinitialize Voice Controll System.
		//=====================================================
		void CVCSystem::DeInit()
		{
			try
			{
				winAmpController.DeInit();
				if(recoGrammar)
				{
					recoGrammar = NULL;
					logger.Log(LMT_Success, "CVCSystem::DeInit() - Core Grammar deinitialized!");
				}
				if(recoContext)
				{
					recoContext->SetNotifySink(NULL);
					recoContext = NULL;
					logger.Log(LMT_Success, "CVCSystem::DeInit() - Core Recognition Context deinitialized!");
				}
				if(recoEngine)
				{
					recoEngine = NULL;
					logger.Log(LMT_Success, "CVCSystem::DeInit() - Recognition Engine deinitialized!");
				}
				if(TTSVoice)
				{
					TTSVoice->Release();
					TTSVoice = NULL;
					logger.Log(LMT_Success, "CVCSystem::DeInit() - TTS Voice deinitialized!");
				}
				CoUninitialize();
				logger.Log(LMT_Success, "CVCSystem::DeInit() - COM deinitialized!");
			}
			catch(std::exception& ex)
			{
				logger.Log(LMT_Fatal, boost::format("CVCSystem::DeInit() Failed to deinitialize VC System - %s") % ex.what());
			}

			logger.RemoveLogOutput(textOutput);
			if(textOutput)
			{
				textOutput->DeInit();
				delete textOutput;
			}
			logger.DeInit();
			return;
		}

		//=====================================================
		//Function: CVCSystem::Run()
		//Last Revised: 29.11.2006
		//	Run VC System.
		//=====================================================
		void CVCSystem::Run(uint32 argc, char8** argv)
		{
			if(!bShouldQuit)
			{
				logger.Log(LMT_Info, "ojej!");
				TTSVoice->Speak(L"Good day, Commander!", SPF_ASYNC, NULL);
			}

			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			//recoGrammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE );
			//recoGrammar->SetRuleIdState(MODE_SelectModule, SPRS_INACTIVE );

			while(!bShouldQuit)
			{
				USES_CONVERSION;	//something COM-specific
				logger.Log(LMT_Debug, "Before recognition!");
				while(SUCCEEDED(CVCSystem::BlockForResult(recoContext, &result)))
				{
					CSpDynamicString dstrText;
					if (SUCCEEDED(result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &dstrText, NULL)))
					{
						//TTSVoice->Speak(dstrText, SPF_ASYNC, NULL);
					}
					logger.Log(LMT_Debug, boost::format("Reco! - %s") % W2A(dstrText));
					//if (_wcsicmp(dstrText, L"exit") == 0)
					//{
						//logger.Log(LMT_Debug, "egzit");
						//bShouldQuit = true;
						//break;
					//}

					if (SUCCEEDED(result->GetPhrase(&pElements)))
					{        
						switch ( pElements->Rule.ulId )
						{
							case MODE_Select:
							{
								switch( pElements->pProperties->vValue.ulVal )
								{
									case CMD_ActivateVC:
									{
										PlayNotifySound(S_Activate);

										//disable mode select rule
										recoGrammar->SetRuleIdState(MODE_SelectModule, SPRS_ACTIVE );
										recoGrammar->SetRuleIdState(MODE_Select, SPRS_INACTIVE );
										SelectModule();

										//switch back to mode select input
										recoGrammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE );
										recoGrammar->SetRuleIdState(MODE_SelectModule, SPRS_INACTIVE );
										//PlayNotifySound(S_RestateCommand);

										break;
									}
								}
							}
							break;
						}
						::CoTaskMemFree(pElements);
					}
					if(bShouldQuit)
					{
						break;
					}

				}
				logger.Log(LMT_Debug, "After successful recognition!");
			}
			return;
		}

//----

		HRESULT CVCSystem::BlockForResult(CComPtr<ISpRecoContext> pRecoCtxt, ISpRecoResult ** ppResult, DWORD dwHowLong)
		{
			HRESULT hr = S_OK;
			CSpEvent event;

			while (SUCCEEDED(hr) && SUCCEEDED(hr = event.GetFrom(pRecoCtxt)) && hr == S_FALSE)
			//if(SUCCEEDED(hr) && SUCCEEDED(hr = event.GetFrom(pRecoCtxt)) && hr == S_FALSE)
			{
				logger.Log(LMT_Debug, boost::format("Awaiting for event...[%x]") % dwHowLong);
				hr = pRecoCtxt->WaitForNotifyEvent(dwHowLong);
				if((hr == S_FALSE) && dwHowLong != INFINITE)
				{
					return E_FAIL;
				}
			}

			logger.Log(LMT_Debug, "Event received");

			(*ppResult) = event.RecoResult();
			if (*ppResult)
			{
				(*ppResult)->AddRef();
			}
			logger.Log(LMT_Debug, "Event processed");

			return hr;
		}

		//---
		void CVCSystem::SelectModule()
		{
			USES_CONVERSION;	//something COM-specific
			logger.Log(LMT_Debug, "module select!");

			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			//if(SUCCEEDED(CVCSystem::BlockForResult(recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
			if(SUCCEEDED(CVCSystem::BlockForResult(recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
			//if(0)
			{
				CSpDynamicString dstrText;
				//logger.Log(LMT_Debug, boost::format("Reco module! - %s") % W2A(dstrText));

				if (SUCCEEDED(result->GetPhrase(&pElements)))
				{        
					switch ( pElements->Rule.ulId )
					{
						case MODE_SelectModule:
						{
							switch( pElements->pProperties->vValue.ulVal )
							{
								case CMD_MusicControl:
								{
									//TTSVoice->Speak(L"exit exit exit", SPF_ASYNC, NULL);
									PlayNotifySound(S_Accepted);
									//pass control to WinAMP Controller
									recoGrammar->SetGrammarState(SPGS_DISABLED);
									winAmpController.TakeControll();
									recoGrammar->SetGrammarState(SPGS_ENABLED);
									break;
								}
								case CMD_ShutdownVC:
								{
									bShouldQuit = true;
									//TTSVoice->Speak(L"exit exit exit", SPF_ASYNC, NULL);
									PlayNotifySound(S_Exit, false);
									break;
								}
							}
						}
						break;
					}
					::CoTaskMemFree(pElements);
				}
			}
			else
			{
				PlayNotifySound(S_RestateCommand);
				//PlayNotifySound(S_Exit);
			}
		}
	} //end of namespace VCS
} //end of namespace TRC