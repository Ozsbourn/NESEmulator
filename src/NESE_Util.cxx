#include "NESE_Util.hxx"

namespace NESE {

	Log& Log::GetInstance(void) {
		static Log instance;
		return instance;
	}



	std::ostream& Log::GetCPUTraceStream(void) {
		return *m_CPUTrace;
	}

	std::ostream& Log::GetStream(void) {
		return *m_LogStream;
	}



	void Log::SetLogStream(std::ostream& stream) {
		m_LogStream = &stream;
	}

	void Log::SetCPUTraceStream(std::ostream& stream) {
		m_COUTrace = &stream;
	}



	Log& Log::SetLevel(Level level) {
		m_LogLevel = level;
		
		return *this;
	}

	Level Log::GetLevel(void) {
		return m_LogLevel;
	}



	TeeBuf::TeeBuf(std::streambuf* sb1, std::streambuf* sb2)
	: m_strbuf1(sb1),
	  m_strbuf2(sb2) {}



	int TeeBuf::Overflow(int c) {
		if (c == EOF) {
			return !EOF;
		} else {
			const int r1 = m_strbuf1->sputc(c);
			const int r2 = m_strbuf2->sputc(c);

			return r1 == EOF || r2 == EOF ? EOF : c;
		}
	}

	int TeeBuf::Sync(void) {
        int const r1 = m_strbuf1->pubsync();
        int const r2 = m_strbuf2->pubsync();
        
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }



	Log::~Log(void){}

};