#include <cstdint>
#include <function>
#include <vector>


#ifndef NESE_MAPPERAXROM_HXX_
#define NESE_MAPPERAXROM_HXX_

namespace NESE {

    class MapperAxROM : public Mapper {
    public:
        MapperAxROM(Cartridge& cart, std::function<void(void)> mirroring_cb);

        void WritePRG(Address addr, BYTE value);
        BYTE ReadPRG(Address addr);

        BYTE ReadCHR(Address addr);
        void WriteCHR(Address addr, BYTE value);

        NameTableMirroring GetNameTableMirroring();
    private:
        NameTableMirroring        m_Mirroring;

        std::function<void(void)> m_MirroringCallback;
        std::uint32_t             m_PRGBank;
        std::vector<BYTE>         m_CharacterRAM;
    };

};

#endif // NESE_MAPPERAXROM_HXX_