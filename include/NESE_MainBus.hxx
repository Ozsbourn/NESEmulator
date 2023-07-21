#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include "NESE_Cartridge.hxx"
#include "NESE_Mapper.hxx"

#ifndef NESE_MAINBUS_HXX_
#define NESE_MAINBUS_HXX_

namespace NESE {
    
    enum class IORegisters {
        PPUCTRL    = 0x2000,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROL,
        PPUADDR,
        PPUDATA,
        OAMDMA    = 0x4014,
        JOY1      = 0x4016,
        JOY2      = 0x4017,
    };

    struct IORegisterHasher {
        std::size_t operator()(NESE::IORegisters& const reg) const noexcept {
            return std::hash<std::uint32_t>{}(reg);
        }
    };



    class MainBus {
    private:
        std::vector<BYTE> m_RAM;
        std::vector<BYTE> m_ExtRAM;

        Mapper*           m_Mapper;

        std::unordered_map<IORegisters, std::function<void(BYTE)>, IORegisterHasher> m_WriteCallbacks;
        std::unordered_map<IORegisters, std::function<BYTE(void)>, IORegisterHasher> m_ReadCallbacks;
    public:
        explicit MainBus(void);

        BYTE Read(Address addr);
        void Write(Address addr, BYTE value);

        bool SetMapper(Mapper* mapper);

        bool SetWriteCallback(IORegisters reg, std::function<void(BYTE)> callback);
        bool SetReadCallback(IORegisters  reg, std::function<BYTE(void)> callback);

        const BYTE* GetPagePtr(BYTE page);

        ~MainBus(void);
    };

};

#endif // NESE_MAINBUS_HXX_