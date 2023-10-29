#include "NESE_CPUOpCodex.hxx"
#include "NESE_MainBus.hxx"


#ifndef NESE_CPU_HXX_
#define NESE_CPU_HXX_

namespace NESE {
    
    class CPU {
    private:
        void InterruptSequence(InterruptType intype);

        bool ExecuteImplied(BYTE opcode);
        bool ExecuteBranch(BYTE  opcode);
        bool ExecuteType0(BYTE   opcode);
        bool ExecuteType1(BYTE   opcode);
        bool ExecuteType2(BYTE   opcode);

        Address ReadAddress(Address addr);

        void PushStack(BYTE val);
        BYTE PullStack(void);

        void SetPageCrossed(Address addr1, Address addr2, int inc = 1);
        void SetZN(BYTE val);

        int m_SkipCycles;
        int m_Cycles;

        // Registers
        Address m_rPC
        BYTE m_rSP;
        BYTE m_rA;
        BYTE m_rX; 
        BYTE m_rY;

        // Status flags
        bool m_fC;
        bool m_fZ;
        bool m_fI;
        bool m_fD;
        bool m_fV;
        bool m_fN;
        
        bool m_PendingNMI;
        bool m_PendingIRQ;

        MainBus& m_Bus;
    public:
        explicit CPU(MainBus& mem);

        void Step(void);
        void Reset(void);
        void Reset(Address start_addr);
        void Log(void);

        Address GetPC(void) {
            return m_rPC;
        }
        void SkipDMACycles(void);

        void Interrupt(InterruptType intype);

        ~CPU(void);
    };

};

#endif // NESE_CPU_HXX_