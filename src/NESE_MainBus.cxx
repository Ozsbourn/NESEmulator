#include <cstring>

#include "NESE_MainBus.hxx"
#include "NESE_Util.hxx"


namespace NESE {

    MainBus::MainBus()
    : m_RAM(0x800, 0),
      m_Mapper(nullptr) {}

    BYTE MainBus::Read(Address addr) {
        if (addr < 0x2000) {
            return m_RAM[addr & 0x7ff];
        } else if (addr < 0x4020) {

            //PPU registers, mirrored 
            if (addr < 0x4000) { 
                auto it = m_ReadCallbacks.find(static_cast<IORegisters>(addr & 0x2007));
                if (it != m_ReadCallbacks.end()) {
                    //Second object is the pointer to the function object
                    //Dereference the function pointer and call it
                    return (it->second)();
                } else {
                    LOG(INFO_VERBOSE) << "No read callback registered for I/O register at: " << std::hex << +addr << std::endl;
                }

            } else if (addr < 0x4018 && addr >= 0x4014)  { // Only *some* IO registers
                auto it = m_ReadCallbacks.find(static_cast<IORegisters>(addr));
                if (it != m_ReadCallbacks.end()) {
                    //Second object is the pointer to the function object
                    //Dereference the function pointer and call it
                    return (it->second)();
                } else {
                    LOG(INFO_VERBOSE) << "No read callback registered for I/O register at: " << std::hex << +addr << std::endl;
                }

            } else {
                LOG(INFO_VERBOSE) << "Read access attempt at: " << std::hex << +addr << std::endl;
            }

        } else if (addr < 0x6000) {
            LOG(INFO_VERBOSE) << "Expansion ROM read attempted. This is currently unsupported" << std::endl;
        } else if (addr < 0x8000) {
            if (m_Mapper->HasExtendedRAM()) {
                return m_ExtRAM[addr - 0x6000];
            }
        } else {
            return m_Mapper->ReadPRG(addr);
        }


        return 0;
    }

    // write

    // GetPagePtr

    // SetMapper


    bool MainBus::SetWriteCallback(IORegisters reg, std::function<void(BYTE)> callback) {
        if (!callback) {
            LOG(ERROR) << "Callback argument is nullptr" << std::endl;
            return false;
        } 

        return m_WriteCallbacks.emplace(reg, callback).second;
    }

    bool MainBus::SetReadCallback(IORegisters reg, std::function<BYTE(void)> callback) {
        if (!callback) {
            LOG(ERROR) << "Callback argument is nullptr" << std::endl;
            return false;
        } 

        return m_ReadCallbacks.emplace(reg, callback).second;
    }

};