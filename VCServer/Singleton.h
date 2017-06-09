#ifndef __TRC_VCS_SINGLETON_H__
#define __TRC_VCS_SINGLETON_H__

/*!
\file Singleton.h
\brief Singleton template.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: none.

Notes:

	This singleton pattern was created by Scott Bilas and was presented
	in "Game Programming Gems" Volume I.
	
*/

/*

*/

//#include <windows.h>
#include "Defines.h"
#include "BaseTypes.h"

namespace TRC
{
	namespace VCS
	{

		template <typename T>
		class CSingleton
		{
			static T * ms_instance;	//our class instance

		public:

			//constructor
			CSingleton()
			{
				//assert( !ms_instance );
				sint32 offset = (sint32)(T*)1 - (sint32)(CSingleton<T>*)(T*)1;	//some trick with pointers that I still
						//don`t understand; it is used to find memory offset where pointer to dervived class is
				ms_instance = (T*)((sint32) this + offset );	//set up the pointer
			}
			//destructor
			~CSingleton()
			{
				//TSW_ASSERT( ms_instance ,"CSingleton::~CSingleton - ms_instance == NULL");
				ms_instance = 0;	//reset pointer
			}
			//get reference to singleton class
			static T& GetSingleton()
			{
				//TSW_ASSERT( ms_instance , "CSingleton::GetSingleton() - attempting to dereference a NULL pointer ms_instance");
				return ( *ms_instance );	//return reference
			}
			//get pointer to singleton class
			static T* GetSingletonPtr()
			{
				return ms_instance;	//return pointer
			}

		};	//end of class CSingleton

		template <typename T> T* CSingleton<T>::ms_instance = 0;	//set static member to 0

	} //end of namespace VCS
} //end of namespace TRC


#endif //__TRC_VCS_SINGLETON_H__
