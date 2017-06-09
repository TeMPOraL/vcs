#ifndef __TRC_VCS_BASE_TYPES_H__
#define __TRC_VCS_BASE_TYPES_H__

/*!
\file BaseTypes.h
\brief Typedefs for fixed-size types.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: None

Notes:

*/

/*
   This header contains typedefs for fixed-size types.
   They are used throughout the code to make it portable to other platform.
*/

#include <boost/logic/tribool.hpp>
namespace TRC
{
	namespace VCS
	{
		//! tri-state boolean
		typedef boost::logic::tribool tribool;

		//! Unsigned Char - 8 bits
		typedef unsigned char uchar8;

		//! Unsigned Char - 8 bits
		typedef unsigned char uint8;

		//! Signed Char - 8 bits
		typedef signed char schar8;

		//! Signed Char - 8 bits
		typedef signed char sint8;

		//! typical char
		typedef char char8;

		//! Unsigned Short - 2 bytes
		typedef unsigned short uint16;

		//! Signed Short - 2 bytes
		typedef signed short sint16;

		//! Unsigned Int - 4 bytes
		typedef unsigned int uint32;

		//! Signed Int - 4 bytes
		typedef signed int sint32;

		//! 4 bytes Floating Point
		typedef float float32;

		//! 8 bytes Floating Point
		typedef double float64;

		//! 64-bit integer
		#ifdef WIN32	//Widows Platform
			#ifdef _MSC_VER	//MS VC++ Specific
				typedef unsigned __int64 uint64;
				typedef signed __int64 sint64;
			#else //_MSC_VER
				//non MS VC++
				typedef unsigned long long uint64;
				typedef signed long long sint64;

			#endif //_MSC_VER
		#endif //WIN32
		};
};

#endif //__TRC_VCS_BASE_TYPES_H__