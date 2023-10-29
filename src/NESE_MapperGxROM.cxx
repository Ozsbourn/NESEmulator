#include "NESE_MapperGxROM.hxx"


namespace NESE {

    MapperGxROM::MapperGxROM(Cartridge& cart, std::function<void(void)> mirroring_cb)
    : Mapper(cart, Mapper::GxROM),
      m_Mirroring(Vertical),
      m_MirroringCallback(mirroring_cb) {}

    BYTE MapperGxROM::ReadPRG(Address addr) {
        if (addr >= 0x8000) {
            return m_Cartridge.GetROM()[(m_PRGBank * 0x8000) + (addr & 0x7FFF)];        
        }

        return 0;
    }

    void MapperGxROM::WritePRG(Address addr, BYTE val) {
        if (addr >= 0x8000) {
            m_PRGBank = ((val & 0x30) >> 4);
            m_CHRBank = (val & 0x3);

            m_Mirroring = Vertical;
        }

        m_MirroringCallback();
    }

    BYTE MapperGxROM::ReadCHR(Address addr) {
        if (addr <= 0x1FFF) {
            return m_Cartridge.GetVROM()[m_CHRBank * 0x2000 + addr];
        }

        return 0;
    }

    NameTableMirroring MapperGxROM::GetNameTableMirroring(void) {
        return m_Mirroring;
    }

    void MapperGxROM::WriteCHR(Address addr, BYTE val) {
        LOG(INFO) << "Not expecting writes here" ;
    }

};