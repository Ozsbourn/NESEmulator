#include <cstdint>

#ifndef NESE_UTIL_HXX_
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>

#define NESE_UTIL_HXX_

#ifndef __FILENAME__
	#define __FILENAME__ __FILE__
#endif // __FILENAME__

namespace NESE {
	using BYTE    = std::uint8_t;
	using Address = std::uint16_t; 



	enum class Level {
		NONE,
		INFO,
		INFO_VERBOSE,
		CPU_TRACE,
		ERROR
	};



	class Log() {
	private:
		Level         m_LogLevel;
		std::ostream* m_LogStream;
		std::ostream* m_CPUTrace;
	public:
		explicit Log(void);

		void SetLogStream(std::ostream& stream);
		void SetCPUTraceStream(std::ostream& stream);

		Log& SetLevel(Level level);
		Log& GetLevel(void);

		std::ostream& GetStream(void);
		std::ostream& GetCPUTraceStream(void);

		static Log& GetInstance(void);

		~Log(void);
	};



	class TeeBuf : public std::streambuf {
	private:
		std::streambuf* m_strbuf1;
		std::streambuf* m_strbuf2;
	private:
		virtual int overflow(int c);

		virtual int sync(void);
	public:
		explicit TeeBuf(std::streambuf* strbuf1, std::streambuf* strbuf2);
	}

	class TeeStream : public std::ostream {
	private:
		TeeBuf m_TeeBuf;
	public:
		TeeStream(std::ostream& ostream1, std::ostream& ostream2);
	}



	#define LOG(level) \
			if (level > NESE::Log::get().GetLevel()); \
			else NESE::Log::GetInstance().getStream << '[' << __FILENAME__ << ":" << std::dec << __LINE__ << "]"

	#define LOG_CPU    \
			if (NESE::CPUTrace != NESE::Log::GetInstance().GetLevel()) ; \
			else NESE::Log::GetInstance().GetCPUTraceStream()
};

#endif // NESE_UTIL_HXX_