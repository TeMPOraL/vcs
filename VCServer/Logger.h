#ifndef __TRC_VCS_LOGGER_H__
#define __TRC_VCS_LOGGER_H__

/*!
\file Logger.h
\brief The Logger Tool.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: none.

Notes:

*/

#include "Defines.h"
#include "BaseTypes.h"

#include <boost/format.hpp>

namespace TRC
{
	namespace VCS
	{
		//! \brief Log message types.
		enum E_LogMessageTypes
		{
			LMT_Fatal = 0x00,	//!< Fatal error.

			LMT_Info = 0x01,	//!< Basic information message.
			LMT_Success = 0x02,	//!< Success message.
			LMT_Warning = 0x04,	//!< Warning message.
			LMT_Error = 0x08,	//!< Error message.

			LMT_Debug = 0x40,	//!< Debug message.

			LMT_TTS = 0x80,	//!< Text To Speech message.

			//! Default filter for an output.
			LMTF_Default = (LMT_Info | LMT_Success | LMT_Warning | LMT_Error | LMT_Debug),
			LMTF_TTSDefault = (LMTF_Default | LMT_TTS)
		};

		class ILogOutput
		{
		public:
			ILogOutput(){}	//!< Default c-tor.
			virtual ~ILogOutput(){}	//!< Virtual d-tor.

			//! \brief Initialize log output.
			//! \return Returns true after successful init; false otherwise.
			virtual bool Init() = 0;

			//! \brief Deinitialize log output.
			//! \return Returns true after successful deinit; false otherwise.
			virtual bool DeInit() = 0;

			//! \brief Write log message.
			//! \param msgType: Type of log message, *should* be one of E_LogMessageType constants.
			//! \param time: Text containing time when this message was generated.
			//! \param message: Logged message.
			//!
			//! A log output may not always use all of those params; ie. some outputs may not need
			//! to store time data. Also, an output may employ it's own internal filtering.
			virtual void Write(uint32 msgType, const std::string& time, const std::string& message) = 0;

		};

		class CLogger
		{
		private:
		protected:
			typedef std::vector<std::pair<uint32, ILogOutput*> > outputList_t;	//!< Type of
												//!< log output list.
			outputList_t logOutputs;	//!< List of all log outputs.

			//! \brief Checks current time using some external functions.
			//! \return Returns text containing current time.
			//! I know - it's not thread-safe ;)
			std::string CurrentTime()
			{
				char8 temp[80];	//temporary array for storing the time
				_strtime(temp);	//store time
				return std::string(temp);
			}
		public:
			CLogger(){}	//!< Default c-tor.
			virtual ~CLogger(){}	//! Virtual d-tor.

			virtual bool Init(){ return true; }
			virtual bool DeInit(){ logOutputs.clear(); return true; }
			virtual void SafeShutdown() throw() { try { DeInit(); } catch(...) {} };

			//! \brief Adds an log output to the logger.
			//! \param output: Pointer to log output.
			//! \param filter: Describes output filter. Use | operator [binary OR] to
			//!			specify which message types should be *passed* to the output.
			//! \return Returns true after successful output registering; false otherwise.
			//!
			//! Pointer to log output should not be NULL - otherwise it will be ignored.
			//! No checks for duplicates are made, the same output may be registered more than once.
			virtual bool AddLogOutput(ILogOutput* output, uint32 filter)
			{
				if(output == NULL)	//ignore NULL log output
				{
					return false;
				}

				//add output
				logOutputs.push_back(outputList_t::value_type(filter, output));
				output->Write(LMT_Info, CurrentTime(), "CLogger::AddLogOutput() - Attached to Logger.");
				return true;
			}

			//! \brief Removes an log output from the logger.
			//! \param output: Pointer to log output.
			//! \return Returns true if output was found and unregistered; false otherwise.
			//!
			//! This function does not destroy the log output; it's up to it's owner.
			//! This function removes first output maching address, so if an output is registered
			//! more than once, then more than one call of this function is needed.
			virtual bool RemoveLogOutput(ILogOutput* output)
			{
				//try to find our output
				for(outputList_t::iterator itor = logOutputs.begin() ; itor != logOutputs.end() ; ++itor)
				{
					//check pointer [second part of std::pair]
					if( (*itor).second == output)
					{
						//we've found our output; remove it
						if(output)	//make sure we won't try to write to a NULL pointer
						{
							output->Write(LMT_Info, CurrentTime(), "CLogger::RemoveLogOutput() - Detached from Logger.");
						}
						logOutputs.erase(itor);
						return true;
					}
				}

				//not found
				return false;
			}

			//! \brief Log message.
			//! \param msgType: Type of log message, *should* be one of E_LogMessageType constants.
			//! \param message: Message to be logged.
			void Log(uint32 msgType, const std::string& msg)
			{
				std::string currentTime(CurrentTime());	//output time
				for(outputList_t::iterator itor = logOutputs.begin() ; itor != logOutputs.end() ; ++itor)
				{
					if( (*itor).second && ( ( (*itor).first & msgType ) || msgType == LMT_Fatal ) )
					{
						(*itor).second->Write(msgType, currentTime, msg);
					}
				}
			}
			void Log(uint32 msgType, const boost::format& msg)
			{
				Log(msgType, msg.str());
			}
		};
	} //end of namespace VCS
} //end of namespace TRC

#endif //__TRC_VCS_LOGGER_H__
