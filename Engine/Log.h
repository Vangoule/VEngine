
#pragma once
#ifndef LOG_HPP
#define LOG_HPP

#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <memory>
#include <string>
#include <mutex>

namespace Logger {
	enum SeverityLevel
	{
		Engine = 1,
		Game,
		Error,
		Warning
	};

	class ILogPolicy
	{
	public:
		virtual void Open(const std::string& name) = 0;
		virtual void Close() = 0;
		virtual void Write(const std::string& msg) = 0;
	};

	class FileLogPolicy : public ILogPolicy
	{
		std::unique_ptr< std::ofstream > m_out;
	public:
		FileLogPolicy() : m_out(new std::ofstream) {}
		void Open(const std::string& name);
		void Close();
		void Write(const std::string& msg);
		~FileLogPolicy();
	};

	template< typename LogPolicy >
	class Logger
	{
		unsigned m_lineNumber;
		std::string getTime();
		std::string getHeader();
		std::stringstream m_logStream;
		LogPolicy* m_policy;
		std::mutex m_writeMutex;

		//Core printing functionality
		void PrintMessage();
		template<typename First, typename...Rest>
		void PrintMessage(First parm1, Rest...parm);
	public:
		Logger(const std::string& name);

		template< SeverityLevel severity, typename...Args >
		void Print(Args...args);

		template< SeverityLevel severity, typename...Args >
		void PrintThreadSafe(Args...args);

		~Logger();
	};

	template< typename LogPolicy >
	inline Logger< LogPolicy >::Logger(const std::string& name)
	{
		m_lineNumber = 0;
		m_policy = new LogPolicy;
		if (!m_policy)
		{
			throw std::runtime_error("LOGGER: Unable to create the logger instance");
		}
		m_policy->Open(name);
	}

	template< typename LogPolicy >
	inline Logger< LogPolicy >::~Logger()
	{
		if (m_policy)
		{
			m_policy->Close();
			delete m_policy;
		}
	}

	template< typename LogPolicy >
	inline void Logger< LogPolicy >::PrintMessage()
	{
		m_policy->Write(getHeader() + m_logStream.str());
		std::cout << m_logStream.str() << std::endl;
		m_logStream.str("");
	}

	template< typename LogPolicy >
	template<typename First, typename...Rest >
	inline void Logger< LogPolicy >::PrintMessage(First parm1, Rest...parm)
	{
		m_logStream << parm1;
		PrintMessage(parm...);
	}

	template< typename LogPolicy >
	template< SeverityLevel severity, typename...Args >
	inline void Logger< LogPolicy >::Print(Args...args)
	{

		switch (severity)
		{
		case SeverityLevel::Engine:
			m_logStream << "<ENGINE> : ";
			break;
		case SeverityLevel::Game:
			m_logStream << "<GAME> : ";
			break;
		case SeverityLevel::Warning:
			m_logStream << "<WARNING> : ";
			break;
		case SeverityLevel::Error:
			m_logStream << "<ERROR> : ";
			break;
		};
		PrintMessage(args...);

	}

	template< typename LogPolicy >
	template< SeverityLevel severity, typename...Args >
	inline void Logger< LogPolicy >::PrintThreadSafe(Args...args)
	{
		m_writeMutex.lock();

		switch (severity)
		{
		case SeverityLevel::Engine:
			m_logStream << "<ENGINE> : ";
			break;
		case SeverityLevel::Game:
			m_logStream << "<GAME> : ";
			break;
		case SeverityLevel::Warning:
			m_logStream << "<WARNING> : ";
			break;
		case SeverityLevel::Error:
			m_logStream << "<ERROR> : ";
			break;
		};
		PrintMessage(args...);

		m_writeMutex.unlock();
	}


	template<typename LogPolicy>
	inline std::string Logger< LogPolicy >::getTime()
	{
		time_t     now = time(NULL);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d : %X", &tstruct);
		return buf;
	}

	template<typename LogPolicy>
	inline std::string Logger< LogPolicy >::getHeader()
	{
		std::stringstream header;
		header.str("");
		header.fill('0');
		header.width(3);
		header << m_lineNumber++ << ": <" << getTime() << " - ";
		header << clock() << "> ~ ";
		return header.str();
	}

	inline void FileLogPolicy::Open(const std::string& name)
	{
		m_out->open(name.c_str(), std::ios_base::binary | std::ios_base::out);
		if (!m_out->is_open())
		{
			throw(std::runtime_error("LOGGER: Unable to open an output stream"));
		}
	}

	inline void FileLogPolicy::Close()
	{
		if (m_out)
		{
			m_out->close();
		}
	}

	inline void FileLogPolicy::Write(const std::string& msg)
	{
		(*m_out) << msg << std::endl;
	}

	inline FileLogPolicy::~FileLogPolicy()
	{
		if (m_out)
		{
			Close();
		}
	}



}

#endif
