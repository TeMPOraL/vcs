#ifndef __TRC_VCS_VOICE_CONTROL_SYSTEM_H__
#define __TRC_VCS_VOICE_CONTROL_SYSTEM_H__

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
#include "BaseTypes.h"
#include "Singleton.h"

#include "Logger.h"

#include <sapi.h>
#include <sphelper.h>

#include "WinAMPController.h"

namespace TRC
{
	namespace VCS
	{
		enum
		{
			CORE_GRAMMAR_ID = 1,	//!< ID of Core Grammar Object.
			MODULE_COMMAND_LISTEN_TIME = 8000
		};
		class CVCSystem : public CSingleton<CVCSystem>
		{
		public:
			CLogger logger;	//!< Logger system.
			CComPtr<ISpRecoContext> recoContext;	//!< Core recognition context.
			ISpVoice* TTSVoice;	//!< Default TTS Voice.
		protected:
			bool bShouldQuit;	//!< Should the application stop?

			//---vv modules
			CWinAMPController winAmpController;	//!< WinAMP Controll Module
			//---^^ modules

			//log outputs
			ILogOutput* textOutput;	//!< Text output for logger.

			//SAPI Objects
			CComPtr<ISpRecognizer> recoEngine;	//!< Recognition engine.
			CComPtr<ISpRecoGrammar> recoGrammar; //!< Core recognition grammar.

		public:
			enum E_Sounds
			{
				S_Exit = 0,
				S_Error,
				S_Accepted,
				S_Executing,
				S_NotYetImplemented,
				S_RestateCommand,
				S_Deny,
				S_Activate,
				S_MaxSounds	//!< Count of sounds
			};

			std::vector<std::string> soundList;

			CVCSystem(){ TTSVoice = NULL; textOutput = NULL;}	//!< Constructor.
			virtual ~CVCSystem(){}	//!< Destructor.
			//! \brief Initialize VC System.
			void Init();

			//! \brief Deinitialize VC System.
			void DeInit();

			//! \brief Main function of VC System.
			void Run(uint32 argc, char8** argv);

			//utility functions
			void PlayNotifySound(E_Sounds sound, bool bAsync = true)
			{
				PlaySoundA((std::string("sounds/") + soundList[sound]).c_str(), NULL, SND_FILENAME | ((bAsync)? SND_ASYNC : SND_SYNC));
			}

			void SelectModule();

			HRESULT CVCSystem::BlockForResult(CComPtr<ISpRecoContext> pRecoCtxt, ISpRecoResult ** ppResult, DWORD dwHowLong = INFINITE);
		};
	};
};

#endif //__TRC_VCS_VOICE_CONTROL_SYSTEM_H__