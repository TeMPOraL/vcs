#ifndef __TRC_VCS_WINAMP_CONTROLLER_H__
#define __TRC_VCS_WINAMP_CONTROLLER_H__

/*!
\file WinAMPController.h
\brief Header class for WinAMP Controll Tool.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: WinAMPController.cpp

Notes:

*/

/*
   
*/

#include "Defines.h"

#include <windows.h>
namespace TRC
{
	namespace VCS
	{
		class CWinAMPController
		{
		protected:
			CComPtr<ISpRecoGrammar> grammar; //!< WinAMP controll grammar.
			bool bPreserve;	//!< Keep control ;)

			HWND hWinAMP;
		public:
			void Init();
			void DeInit();
			
			void TakeControll();
			void VolumeMenu();
			void PlaybackMenu();
			void PlaylistMenu();
		};
	};
};

#endif //__TRC_VCS_WINAMP_CONTROLLER_H__