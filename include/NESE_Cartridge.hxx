#include <vector>
#include <string>

#include "NESE_Util.hxx"


#ifndef NESE_CARTRIDGE_HXX_
#define NESE_CARTRIDGE_HXX_

namespace NESE {
    class Cartridge {
    private:
        std::vector<BYTE> m_PRG_ROM;
        std::vector<BYTE> m_CHR_ROM;

        BYTE m_NameTableMirroring;
        BYTE m_MapperNumber;

        bool m_ExtendedRAM;
        bool m_CHRRAM;
    public:
        explicit Cartridge();

        bool LoadFromFile(std::string path);

        const std::vector<BYTE>& GetROM();
        const std::vector<BYTE>& GetVROM();

        BYTE GetMapper();
        BYTE GetNameTableMirroring();

        bool HasExtendedRAM();

        ~Cartridge() {}
    };
};

#endif // NESE_CARTRIDGE_HXX_