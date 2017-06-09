#ifndef __TRC_VCS_LOG_OUTPUT_TEXT_FILE_H__
#define __TRC_VCS_LOG_OUTPUT_TEXT_FILE_H__

/*!
\file LogOutput_TextFile.h
\brief Implementation of a simple text file log output.


Project:	TRC Voice Control System

Programmer:	Jacek "TeMPOraL" Zlydach	[temporal@v-lo.krakow.pl]

Created: 29.11.2006
Last Revised:	29.11.2006

Implementation File: none.

Notes:


*/

#include "Defines.h"
#include "BaseTypes.h"

#include <string>
#include <sstream>
#include <time.h>
#include <windows.h>

#include "Logger.h"

namespace TRC
{
	namespace VCS
	{
		//! \brief Simple text file log output, that uses WinAPI for file I/O.
		class CLogOutput_TextFile : public ILogOutput
		{
		protected:
			std::string fileName;	//!< Output file name.
			bool bReady;	//!< Is the output ready to save data?
			HANDLE hFile;	//!< WinAPI handle to output file.

		public:
			//! \brief Constructor.
			//! \param _fileName: Name of the log file to open.
			CLogOutput_TextFile(const std::string& _fileName):fileName(_fileName), bReady(false){}
			virtual ~CLogOutput_TextFile(){}	//!< Default d-tor.

			virtual bool Init()
			{
				if(fileName.empty())
				{
					return false;
				}
				return OpenFile(fileName.c_str());
			}
			virtual bool DeInit()
			{
				if(bReady)
				{
					std::stringstream buffer;	//temporary buffer
					char timesign [128];	//temporary buffer for holding date / time stuff
					DWORD dwTemp;	//temporary dword

					_strdate(timesign);	//load date

					buffer <<"-[Log file closed at: " << timesign;	//first half of date/time info

					_strtime(timesign);	//load current time

					buffer <<" " << timesign <<"]\r\n";	//... and the second half of date/time stuff

					WriteFile(hFile, buffer.str().c_str(), buffer.str().size(), &dwTemp, NULL);

					CloseHandle(hFile);	//close log file handle

					return true;
				}

				//don't de-init twice
				return false;
			}

			//! \brief Opens output file.
			//! \param name - File name.
			//! \return Returns true if file is open and ready to receive data; false otherwise.
			bool OpenFile(const std::string& name)
			{
				fileName = name;
				hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ , NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if(hFile == INVALID_HANDLE_VALUE)
				{
					bReady = false;
					return false;
				}

				std::stringstream buffer;	//temporary buffer
				char timesign [128];	//temporary buffer for holding date / time stuff
				DWORD dwTemp;	//temporary dword

				_strdate(timesign);	//load date

				buffer <<"-[Log file opened at: " << timesign;	//first half of date/time info

				_strtime(timesign);	//load current time

				buffer <<" " << timesign <<"]-\r\n";	//... and the second half of date/time stuff

				buffer <<"-[======================================================]-\r\n";
				buffer <<"-[Logging unit created for Project: Anything            ]-\r\n";
				buffer <<"-[By Jacek \"TeMPOraL\" Zlydach                           ]-\r\n";
				buffer <<"-[======================================================]-\r\n";

				WriteFile(hFile, buffer.str().c_str(), buffer.str().size(), &dwTemp, NULL);
				bReady = true;
				return true;
			}

			virtual void Write(uint32 msgType, const std::string& time, const std::string& message)
			{
				char8 typeChar;	//here we'll store character representing message type
				if(msgType & LMT_Info)
				{
					typeChar = 'I';
				}
				else if(msgType & LMT_Success)
				{
					typeChar = 'S';
				}
				else if(msgType & LMT_Warning)
				{
					typeChar = 'W';
				}
				else if(msgType & LMT_Error)
				{
					typeChar = 'E';
				}
				else if(msgType & LMT_Debug)
				{
					typeChar = 'D';
				}
				else if(msgType == LMT_Fatal)
				{
					typeChar = 'F';
				}
				else
				{
					typeChar = '?';
				}

				//format message
				std::stringstream buffer;
				buffer << '[' << typeChar << "] [" << time << "] " << message << "\r\n";

				//prepare to write
				DWORD dwTemp;
				std::string text(buffer.str());

				//output message
				WriteFile(hFile, text.c_str(), text.size(), &dwTemp, NULL);
			}
		};
	} //end of namespace VCS
} //end of namespace TRC

#endif //__TRC_VCS_LOG_OUTPUT_TEXT_FILE_H__
