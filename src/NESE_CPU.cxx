#include <iomanip>

#include "NESE_CPU.hxx"
#include "NESE_CPUOpCodes.hxx"
#include "NESE_Util.hxx"


namespace NESE {
    CPU::CPU(MainBus &mem) :
        m_PendingNMI(false),
        m_PendingIRQ(false),
        m_Bus(mem) {}

    void CPU::Reset(void) {
        Reset(ReadAddress(ResetVector));
    }

    void CPU::Reset(Address start_addr) {
        m_SkipCycles = m_Cycles = 0;

        m_rA = m_rX = m_rY = 0;
        m_fI = true;
        m_fC = m_fD = m_fN = m_fV = m_fZ = false;
        m_rPC = start_addr;
        m_rSP = 0xfd; //documented startup state
    }

    void CPU::Interrupt(InterruptType type) {
        switch (type) {
            case InterruptType::NMI:
                m_PendingNMI = true;
                break;
            case InterruptType::IRQ:
                m_PendingIRQ = true;
                break;
            default:
                break;
        }
    }

    void CPU::InterruptSequence(InterruptType type) {
        if (m_fI && type != NMI && type != BRK_) {
            return;
        }


        //Add one if BRK, a quirk of 6502
        if (type == BRK_) {
            ++m_rPC;
        }

        PushStack(m_rPC >> 8);
        PushStack(m_rPC);

        BYTE flags = m_fN << 7 |
                     m_fV << 6 |
                        1 << 5 | //unused bit, supposed to be always 1
           (type == BRK_) << 4 | //B flag set if BRK
                     m_fD << 3 |
                     m_fI << 2 |
                     m_fZ << 1 |
                     m_fC;
        PushStack(flags);

        m_fI = true;

        switch (type) {
            case IRQ:
            case BRK_:
                m_rPC = ReadAddress(IRQVector);
                break;
            case NMI:
                m_rPC = ReadAddress(NMIVector);
                break;
        }

        // Interrupt sequence takes 7, but one cycle was actually spent on this.
        // So skip 6
        m_SkipCycles += 6;
    }

    void CPU::PushStack(BYTE value) {
        m_Bus.Write(0x100 | m_rSP, value);
        --m_rSP; //Hardware stacks grow downward!
    }

    BYTE CPU::PullStack(void) {
        return m_Bus.Read(0x100 | ++m_rSP);
    }

    void CPU::SetZN(BYTE value) {
        m_fZ = !value;
        m_fN = value & 0x80;
    }

    void CPU::SetPageCrossed(Address a, Address b, int inc) {
        // Page is determined by the high BYTE
        if ((a & 0xFF00) != (b & 0xFF00)) {
            m_SkipCycles += inc;
        }
    }

    void CPU::SkipDMACycles(void) {
        m_SkipCycles += 513; // 256 read + 256 write + 1 dummy read
        m_SkipCycles += (m_Cycles & 1); //+1 if on odd cycle
    }

    void CPU::Step(void) {
        ++m_Cycles;

        if (m_SkipCycles-- > 1) {
            return;
        }

        m_SkipCycles = 0;

        // NMI has higher priority, check for it first
        if (m_PendingNMI) {
            InterruptSequence(NMI);
            m_PendingNMI = m_PendingIRQ = false;
            
            return;
        } else if (m_PendingIRQ) {
            InterruptSequence(IRQ);
            m_PendingNMI = m_PendingIRQ = false;

            return;
        }

        int psw =    m_fN << 7 |
                     m_fV << 6 |
                        1 << 5 |
                     m_fD << 3 |
                     m_fI << 2 |
                     m_fZ << 1 |
                     m_fC;
        LOG_CPU << std::hex << std::setfill('0') << std::uppercase
                  << std::setw(4) << +m_rPC
                  << "  "
                  << std::setw(2) << +m_Bus.Read(m_rPC)
                  << "  "
                  << "A:"   << std::setw(2) << +m_rA << " "
                  << "X:"   << std::setw(2) << +m_rX << " "
                  << "Y:"   << std::setw(2) << +m_rY << " "
                  << "P:"   << std::setw(2) << psw << " "
                  << "SP:"  << std::setw(2) << +m_rSP  << /*std::endl;*/" "
                  << "CYC:" << std::setw(3) << std::setfill(' ') << std::dec << ((m_Cycles - 1) * 3) % 341
                  << std::endl;

        BYTE opcode = m_Bus.Read(m_rPC++);

        auto CycleLength = OperationCycles[opcode];

        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (ExecuteImplied(opcode) || ExecuteBranch(opcode) ||
                        ExecuteType1(opcode) || ExecuteType2(opcode) || ExecuteType0(opcode))) {
            m_SkipCycles += CycleLength;
            
            //m_Cycles %= 340; //compatibility with Nintendulator log
            //m_SkipCycles = 0; //for TESTING
        } else {
            LOG(ERROR) << "Unrecognized opcode: " << std::hex << +opcode << std::endl;
        }
    }

    bool CPU::ExecuteImplied(BYTE opcode) {
        switch (static_cast<OperationImplied>(opcode)) {
            case NOP:
                break;
            case BRK:
                InterruptSequence(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus m_rPC + 1 instead of m_rPC + 2
                //since m_rPC and m_rPC + 1 are address of subroutine
                PushStack(static_cast<BYTE>((m_rPC + 1) >> 8));
                PushStack(static_cast<BYTE>(m_rPC + 1));

                m_rPC = ReadAddress(m_rPC);
                break;
            case RTS:
                m_rPC = PullStack();
                m_rPC |= PullStack() << 8;

                ++m_rPC;
                break;
            case RTI:
                {
                    BYTE flags = PullStack();
                    m_fN = flags & 0x80;
                    m_fV = flags & 0x40;
                    m_fD = flags & 0x8;
                    m_fI = flags & 0x4;
                    m_fZ = flags & 0x2;
                    m_fC = flags & 0x1;
                }

                m_rPC = PullStack();
                m_rPC |= PullStack() << 8;
                break;
            case JMP:
                m_rPC = ReadAddress(m_rPC);
                break;
            case JMPI:
                {
                    Address location = ReadAddress(m_rPC);
                    
                    //6502 has a bug such that the when the vector of anindirect address begins at the last BYTE of a page,
                    //the second BYTE is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    Address Page = location & 0xFF00;
                    m_rPC = m_Bus.Read(location) |
                           m_Bus.Read(Page | ((location + 1) & 0xFF)) << 8;
                }
                break;
            case PHP:
                {
                    BYTE flags = m_fN << 7 |
                                 m_fV << 6 |
                                    1 << 5 | //supposed to always be 1
                                    1 << 4 | //PHP pushes with the B flag as 1, no matter what
                                 m_fD << 3 |
                                 m_fI << 2 |
                                 m_fZ << 1 |
                                 m_fC;
                    PushStack(flags);
                }
                break;
            case PLP:
                {
                    BYTE flags = PullStack();
                    m_fN = flags & 0x80;
                    m_fV = flags & 0x40;
                    m_fD = flags & 0x8;
                    m_fI = flags & 0x4;
                    m_fZ = flags & 0x2;
                    m_fC = flags & 0x1;
                }
                break;
            case PHA:
                PushStack(m_rA);
                break;
            case PLA:
                m_rA = PullStack();
                SetZN(m_rA);
                break;
            case DEY:
                --m_rY;
                SetZN(m_rY);
                break;
            case DEX:
                --m_rX;
                SetZN(m_rX);
                break;
            case TAY:
                m_rY = m_rA;
                SetZN(m_rY);
                break;
            case INY:
                ++m_rY;
                SetZN(m_rY);
                break;
            case INX:
                ++m_rX;
                SetZN(m_rX);
                break;
            case CLC:
                m_fC = false;
                break;
            case SEC:
                m_fC = true;
                break;
            case CLI:
                m_fI = false;
                break;
            case SEI:
                m_fI = true;
                break;
            case CLD:
                m_fD = false;
                break;
            case SED:
                m_fD = true;
                break;
            case TYA:
                m_rA = m_rY;
                SetZN(m_rA);
                break;
            case CLV:
                m_fV = false;
                break;
            case TXA:
                m_rA = m_rX;
                SetZN(m_rA);
                break;
            case TXS:
                m_rSP = m_rX;
                break;
            case TAX:
                m_rX = m_rA;
                SetZN(m_rX);
                break;
            case TSX:
                m_rX = m_rSP;
                SetZN(m_rX);
                break;
            default:
                return false;
        };

        return true;
    }

    bool CPU::ExecuteBranch(BYTE opcode) {
        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult) {
            //branch is initialized to the condition required (for the flag specified later)
            bool branch = opcode & BranchConditionMask;

            //set branch to true if the given condition is met by the given flag
            //We use xnor here, it is true if either both operands are true or false
            switch (opcode >> BranchOnFlagShift) {
                case Negative:
                    branch = !(branch ^ m_fN);
                    break;
                case Overflow:
                    branch = !(branch ^ m_fV);
                    break;
                case Carry:
                    branch = !(branch ^ m_fC);
                    break;
                case Zero:
                    branch = !(branch ^ m_fZ);
                    break;
                default:
                    return false;
            }

            if (branch) {
                int8_t offset = m_Bus.Read(m_rPC++);
                ++m_SkipCycles;
                auto newPC = static_cast<Address>(m_rPC + offset);
                SetPageCrossed(m_rPC, newPC, 2);
                m_rPC = newPC;
            } else {
                ++m_rPC;
            }

            return true;
        }

        return false;
    }

    bool CPU::ExecuteType1(BYTE opcode) {
        if ((opcode & InstructionModeMask) == 0x1) {
            Address location = 0; //Location of the operand, could be in RAM
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);
            switch (static_cast<AddrMode1>(
                    (opcode & AddrModeMask) >> AddrModeShift)) {
                case IndexedIndirectX:
                    {
                        BYTE zero_addr = m_rX + m_Bus.Read(m_rPC++);
                        //Addresses wrap in zero page mode, thus pass through a mask
                        location = m_Bus.Read(zero_addr & 0xFF) | m_Bus.Read((zero_addr + 1) & 0xFF) << 8;
                    }
                    break;
                case ZeroPage:
                    location = m_Bus.Read(m_rPC++);
                    break;
                case Immediate:
                    location = m_rPC++;
                    break;
                case Absolute:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;
                    break;
                case IndirectY:
                    {
                        BYTE zero_addr = m_Bus.Read(m_rPC++);

                        location = m_Bus.Read(zero_addr & 0xFF) | m_Bus.Read((zero_addr + 1) & 0xFF) << 8;
                        if (op != STA) {
                            SetPageCrossed(location, location + m_rY);
                        }

                        location += m_rY;
                    }
                    break;
                case IndexedX:
                    // Address wraps around in the zero page
                    location = (m_Bus.Read(m_rPC++) + m_rX) & 0xFF;
                    break;
                case AbsoluteY:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;

                    if (op != STA) {
                        SetPageCrossed(location, location + m_rY);
                    }

                    location += m_rY;
                    break;
                case AbsoluteX:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;

                    if (op != STA) {
                        SetPageCrossed(location, location + m_rX);
                    }

                    location += m_rX;
                    break;
                default:
                    return false;
            }

            switch (op) {
                case ORA:
                    m_rA |= m_Bus.Read(location);
                    SetZN(m_rA);
                    break;
                case AND:
                    m_rA &= m_Bus.Read(location);
                    SetZN(m_rA);
                    break;
                case EOR:
                    m_rA ^= m_Bus.Read(location);
                    SetZN(m_rA);
                    break;
                case ADC:
                    {
                        BYTE operand = m_Bus.Read(location);
                        std::uint16_t sum = m_rA + operand + m_fC;
                        //Carry forward or UNSIGNED overflow
                        m_fC = sum & 0x100;
                        //SIGNED overflow, would only happen if the sign of sum is
                        //different from BOTH the operands
                        m_fV = (m_rA ^ sum) & (operand ^ sum) & 0x80;
                        m_rA = static_cast<BYTE>(sum);
                        SetZN(m_rA);
                    }
                    break;
                case STA:
                    m_Bus.Write(location, m_rA);
                    break;
                case LDA:
                    m_rA = m_Bus.Read(location);
                    SetZN(m_rA);
                    break;
                case SBC:
                    {
                        //High carry means "no borrow", thus negate and subtract
                        std::uint16_t subtrahend = m_Bus.Read(location),
                                 diff = m_rA - subtrahend - !m_fC;
                        
                        //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                        m_fC = !(diff & 0x100);
                        
                        //Same as ADC, except instead of the subtrahend,
                        //substitute with it's one complement
                        m_fV = (m_rA ^ diff) & (~subtrahend ^ diff) & 0x80;
                        m_rA = diff;
                        SetZN(diff);
                    }
                    break;
                case CMP:
                    {
                        std::uint16_t diff = m_rA - m_Bus.Read(location);
                        m_fC = !(diff & 0x100);
                        SetZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }

        return false;
    }

    bool CPU::ExecuteType2(BYTE opcode) {
        if ((opcode & InstructionModeMask) == 2) {
            Address location = 0;
            auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);
            auto addr_mode = static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            
            switch (addr_mode) {
                case Immediate_:
                    location = m_rPC++;
                    break;
                case ZeroPage_:
                    location = m_Bus.Read(m_rPC++);
                    break;
                case Accumulator:
                    break;
                case Absolute_:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;
                    break;
                case Indexed:
                    {
                        location = m_Bus.Read(m_rPC++);
                        BYTE index;
                        if (op == LDX || op == STX) {
                            index = m_rY;
                        }
                        else {
                            index = m_rX;
                        }
                        //The mask wraps address around zero page
                        location = (location + index) & 0xFF;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        location = ReadAddress(m_rPC);
                        m_rPC += 2;
                        BYTE index;
                        if (op == LDX || op == STX) {
                            index = m_rY;
                        } else {
                            index = m_rX;
                        }

                        SetPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                default:
                    return false;
            }

            std::uint16_t operand = 0;
            switch (op) {
                case ASL:
                case ROL:
                    if (addr_mode == Accumulator) {
                        auto prev_C = m_fC;
                        m_fC = m_rA & 0x80;
                        m_rA <<= 1;
                        //If Rotating, set the bit-0 to the the previous carry
                        m_rA = m_rA | (prev_C && (op == ROL));
                        SetZN(m_rA);
                    } else {
                        auto prev_C = m_fC;
                        operand = m_Bus.Read(location);
                        m_fC = operand & 0x80;
                        operand = operand << 1 | (prev_C && (op == ROL));
                        SetZN(operand);
                        m_Bus.Write(location, operand);
                    }
                    break;
                case LSR:
                case ROR:
                    if (addr_mode == Accumulator) {
                        auto prev_C = m_fC;
                        m_fC = m_rA & 1;
                        m_rA >>= 1;
                        //If Rotating, set the bit-7 to the previous carry
                        m_rA = m_rA | (prev_C && (op == ROR)) << 7;
                        SetZN(m_rA);
                    } else {
                        auto prev_C = m_fC;
                        operand = m_Bus.Read(location);
                        m_fC = operand & 1;
                        operand = operand >> 1 | (prev_C && (op == ROR)) << 7;
                        SetZN(operand);
                        m_Bus.Write(location, operand);
                    }
                    break;
                case STX:
                    m_Bus.Write(location, m_rX);
                    break;
                case LDX:
                    m_rX = m_Bus.Read(location);
                    SetZN(m_rX);
                    break;
                case DEC:
                    {
                        auto tmp = m_Bus.Read(location) - 1;
                        SetZN(tmp);
                        m_Bus.Write(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = m_Bus.Read(location) + 1;
                        SetZN(tmp);
                        m_Bus.Write(location, tmp);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }

        return false;
    }

    bool CPU::ExecuteType0(BYTE opcode) {
        if ((opcode & InstructionModeMask) == 0x0) {
            Address location = 0;
            switch (static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift)) {
                case Immediate_:
                    location = m_rPC++;
                    break;
                case ZeroPage_:
                    location = m_Bus.Read(m_rPC++);
                    break;
                case Absolute_:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;
                    break;
                case Indexed:
                    // Address wraps around in the zero page
                    location = (m_Bus.Read(m_rPC++) + m_rX) & 0xFF;
                    break;
                case AbsoluteIndexed:
                    location = ReadAddress(m_rPC);
                    m_rPC += 2;
                    SetPageCrossed(location, location + m_rX);
                    location += m_rX;
                    break;
                default:
                    return false;
            }

            std::uint16_t operand = 0;
            switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift)) {
                case BIT:
                    operand = m_Bus.Read(location);
                    m_fZ = !(m_rA & operand);
                    m_fV = operand & 0x40;
                    m_fN = operand & 0x80;
                    break;
                case STY:
                    m_Bus.Write(location, m_rY);
                    break;
                case LDY:
                    m_rY = m_Bus.Read(location);
                    SetZN(m_rY);
                    break;
                case CPY:
                    {
                        std::uint16_t diff = m_rY - m_Bus.Read(location);
                        m_fC = !(diff & 0x100);
                        SetZN(diff);
                    }
                    break;
                case CPX:
                    {
                        std::uint16_t diff = m_rX - m_Bus.Read(location);
                        m_fC = !(diff & 0x100);
                        SetZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }

        return false;
    }

    Address CPU::ReadAddress(Address addr) {
        return m_Bus.Read(addr) | m_Bus.Read(addr + 1) << 8;
    }

};