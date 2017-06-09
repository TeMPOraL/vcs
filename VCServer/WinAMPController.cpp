/*!
\file VCSystem.h
\brief Header class for main VC System class.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 02.12.2006
Last Revised:	02.12.2006

Implementation File: VCSystem.cpp

Notes:

*/

/*
   
*/

#include "Defines.h"

#include "VCSystem.h"
#include "WinAMPController.h"

#include "LogOutput_TextFile.h"

//SAPI
#include <sapi.h>
#include <sphelper.h>

#include "grammar/winamp.h"
#include "wa_ipc.h"	//<-- from WinAMP SDK

namespace TRC
{
	namespace VCS
	{
		const std::string winampClassName = "Winamp v1.x";	//i guess...
		//=====================================================
		//Function: CWinAMPController::Init()
		//Last Revised: 02.12.2006
		//	Initialize WinAMP Voice Controll Module
		//=====================================================
		void CWinAMPController::Init()
		{
			HRESULT hRes;
			CVCSystem::GetSingleton().logger.Log(LMT_Info, "WinAMP Controller INIT!");
			
			hRes = CVCSystem::GetSingleton().recoContext->CreateGrammar(CORE_GRAMMAR_ID, &grammar);
			if(FAILED(hRes))
			{
				throw std::runtime_error("Failed to create WinAMP Grammar!");
			}
			hRes = grammar->LoadCmdFromFile(L"grammar/winamp.xml", SPLO_STATIC);
			if(FAILED(hRes))
			{
				throw std::runtime_error("Failed to load WinAMP Grammar from file!");
			}
			grammar->SetRuleState( NULL, NULL, SPRS_ACTIVE );
			grammar->SetGrammarState(SPGS_DISABLED);
			CVCSystem::GetSingleton().logger.Log(LMT_Success, "WinAMP Controller init done!!");

			bPreserve = false;
		}
		
		//=====================================================
		//Function: CWinAMPController::Init()
		//Last Revised: 02.12.2006
		//	Deinitialize WinAMP Voice Controll Module
		//=====================================================
		void CWinAMPController::DeInit()
		{
			CVCSystem::GetSingleton().logger.Log(LMT_Info, "WinAMP Controller DE-INIT!");
			if(grammar)
			{
				grammar = NULL;
				CVCSystem::GetSingleton().logger.Log(LMT_Success, "CWinAMPController::DeInit() - WinAMP Grammar deinitialized!");
			}
		}

		//=====================================================
		//Function: CWinAMPController::TakeControll()
		//Last Revised: 02.12.2006
		//	Listen for WinAMP commands.
		//=====================================================
		void CWinAMPController::TakeControll()
		{
			CVCSystem::GetSingleton().logger.Log(LMT_Info, "WinAMP Controller TakeControll!");
			//check for WinAMP presense
			hWinAMP = FindWindow(winampClassName.c_str(), NULL);
			if(hWinAMP == NULL)
			{
				CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Error);
				ShellExecute(NULL, "open", "startup.mp3", NULL, NULL, SW_SHOWNORMAL);
				return;
			}

			//----
			grammar->SetGrammarState(SPGS_ENABLED);
			grammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE);
			
			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			do
			{
				if(SUCCEEDED(CVCSystem::GetSingleton().BlockForResult(CVCSystem::GetSingleton().recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
				{
					CSpDynamicString dstrText;
					if (SUCCEEDED(result->GetPhrase(&pElements)))
					{        

						switch ( pElements->Rule.ulId )
						{
							case MODE_Select:
							{
								switch( pElements->pProperties->vValue.ulVal )
								{
									case CMD_Preserve:
									{
										//TTSVoice->Speak(L"exit exit exit", SPF_ASYNC, NULL);
										if(bPreserve == false)
										{
											bPreserve=true;
											CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
										}
										else
										{
											CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Deny);
										}
										continue;
									}
									case CMD_Release:
									{
										if(bPreserve)
										{
											bPreserve = false;
											CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
										}
										else
										{
											CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Deny);
										}
										continue;
									}
									case CMD_TrackInfo:
									{
										CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_NotYetImplemented);
										continue;
									}
									case MODE_Volume:
									{
										CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Accepted);
										VolumeMenu();
										continue;
									}
									case MODE_Playback:
									{
										CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Accepted);
										PlaybackMenu();
										continue;
									}
									case MODE_Playlist:
									{
										CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Accepted);
										PlaylistMenu();
										continue;
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
					if(!bPreserve)
					{
						CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_RestateCommand);
					}
					//PlayNotifySound(S_Exit);
				}
				CVCSystem::GetSingleton().logger.Log(LMT_Debug, "perserve!");
			}
			while(bPreserve);
			grammar->SetGrammarState(SPGS_DISABLED);
		}

		//=====================================================
		//Function: CWinAMPController::VolumeMenu()
		//Last Revised: 02.12.2006
		//	Listen for WinAMP commands.
		//=====================================================
		void CWinAMPController::VolumeMenu()
		{
			grammar->SetRuleIdState(MODE_Select, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Volume, SPRS_ACTIVE);

			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			if(SUCCEEDED(CVCSystem::GetSingleton().BlockForResult(CVCSystem::GetSingleton().recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
			{
				CSpDynamicString dstrText;
				if (SUCCEEDED(result->GetPhrase(&pElements)))
				{        

					switch ( pElements->Rule.ulId )
					{
						case MODE_Volume:
						{
							switch( pElements->pProperties->vValue.ulVal )
							{
								case CMD_Mute:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_WA_IPC, 0, IPC_SETVOLUME);
									break;
								}
								case CMD_Full:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_WA_IPC, 255, IPC_SETVOLUME);
									break;
								}
								case CMD_Half:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_WA_IPC, 128, IPC_SETVOLUME);
									break;
								}
								case CMD_OneQuater:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_WA_IPC, 255/3, IPC_SETVOLUME);
									break;
								}
								case CMD_ThreeQuater:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_WA_IPC, 510/3, IPC_SETVOLUME);
									break;
								}
								case CMD_Louder:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									for(uint32 i = 0 ; i < 10 ; ++i)
									{
										SendMessage(hWinAMP, WM_COMMAND, WINAMP_VOLUMEUP, 0);
									}
									break;
								}
								case CMD_Quieter:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									
									for(uint32 i = 0 ; i < 10 ; ++i)
									{
										SendMessage(hWinAMP, WM_COMMAND, WINAMP_VOLUMEDOWN, 0);
									}
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
				CVCSystem::GetSingleton().logger.Log(LMT_Debug, "VC: dropping");
				CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_RestateCommand);
				//PlayNotifySound(S_Exit);
			}

			grammar->SetRuleIdState(MODE_Volume, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE);

		}

		//=====================================================
		//Function: CWinAMPController::PlaybackMenu()
		//Last Revised: 02.12.2006
		//	Listen for WinAMP commands.
		//=====================================================
		void CWinAMPController::PlaybackMenu()
		{
			grammar->SetRuleIdState(MODE_Select, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Playback, SPRS_ACTIVE);

			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			if(SUCCEEDED(CVCSystem::GetSingleton().BlockForResult(CVCSystem::GetSingleton().recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
			{
				CSpDynamicString dstrText;
				if (SUCCEEDED(result->GetPhrase(&pElements)))
				{        

					switch ( pElements->Rule.ulId )
					{
						case MODE_Playback:
						{
							switch( pElements->pProperties->vValue.ulVal )
							{
								case CMD_Stop:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_COMMAND, WINAMP_BUTTON4, 0);
									break;
								}
								case CMD_Pause:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_COMMAND, WINAMP_BUTTON3, 0);
									break;
								}
								case CMD_Resume:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_COMMAND, WINAMP_BUTTON2, 0);
									break;
								}
								case CMD_NextSong:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_COMMAND, WINAMP_BUTTON5, 0);
									break;
								}
								case CMD_PreviousSong:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									SendMessage(hWinAMP, WM_COMMAND, WINAMP_BUTTON1, 0);
									break;
								}
								case CMD_Shuffle:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									uint32 res = SendMessage(hWinAMP, WM_WA_IPC, 1, IPC_GET_SHUFFLE);
									SendMessage(hWinAMP, WM_WA_IPC, (res==0)?1:0, IPC_SET_SHUFFLE);
									break;
								}
								case CMD_Repeat:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									uint32 res = SendMessage(hWinAMP, WM_WA_IPC, 1, IPC_GET_REPEAT);
									SendMessage(hWinAMP, WM_WA_IPC, (res==0)?1:0, IPC_SET_REPEAT);
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
				CVCSystem::GetSingleton().logger.Log(LMT_Debug, "PC: dropping");
				CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_RestateCommand);

				//PlayNotifySound(S_Exit);
			}

			grammar->SetRuleIdState(MODE_Playback, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE);

		}

		
		//=====================================================
		//Function: CWinAMPController::PlaylistMenu()
		//Last Revised: 02.12.2006
		//	Listen for WinAMP commands.
		//=====================================================
		void CWinAMPController::PlaylistMenu()
		{
			grammar->SetRuleIdState(MODE_Select, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Playlist, SPRS_ACTIVE);

			CComPtr<ISpRecoResult> result;
			SPPHRASE *pElements;

			if(SUCCEEDED(CVCSystem::GetSingleton().BlockForResult(CVCSystem::GetSingleton().recoContext, &result, MODULE_COMMAND_LISTEN_TIME)))
			{
				CSpDynamicString dstrText;
				if (SUCCEEDED(result->GetPhrase(&pElements)))
				{        

					switch ( pElements->Rule.ulId )
					{
						case MODE_Playlist:
						{
							switch( pElements->pProperties->vValue.ulVal )
							{
								case PLAYLIST_Alpha:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									ShellExecute(NULL, "open", "playlist/alpha.m3u", NULL, NULL, SW_SHOWNORMAL);
									break;
								}
								case PLAYLIST_Beta:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									ShellExecute(NULL, "open", "playlist/beta.m3u", NULL, NULL, SW_SHOWNORMAL);
									break;
								}
								case PLAYLIST_Gamma:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									ShellExecute(NULL, "open", "playlist/gamma.m3u", NULL, NULL, SW_SHOWNORMAL);
									break;
								}
								case PLAYLIST_Delta:
								{
									CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_Executing);
									ShellExecute(NULL, "open", "playlist/delta.m3u", NULL, NULL, SW_SHOWNORMAL);
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
				CVCSystem::GetSingleton().logger.Log(LMT_Debug, "PL: dropping");
				CVCSystem::GetSingleton().PlayNotifySound(CVCSystem::S_RestateCommand);

				//PlayNotifySound(S_Exit);
			}

			grammar->SetRuleIdState(MODE_Playlist, SPRS_INACTIVE);
			grammar->SetRuleIdState(MODE_Select, SPRS_ACTIVE);

		}
	} //end of namespace VCS
} //end of namespace TRC
