#include "NESE_MapperAxROM.hxx"
#include "NESE_Util.hxx"


namespace NESE {

    MapperAxROM::MapperAxROM(Cartridge& cart, std::function<void(void)> mirroring_cb)
    : Mapper(cart, Type::MapperAxROM),
      m_Mirroring(OneScreenLower),
      m_MirroringCallback(mirroring_cb),
      m_PRGBank(0) {
          if (cart.GetROM().size() >= 0x8000) {
              LOG(Info) << "Using PRG-ROM OK" << std::endl;
          }

          if (cart.GetVROM().size() == 0) {
              m_CharacterRam.resize(0x2000);
              LOG(Info) << "Uses Character RAM OK" << std::endl;
          }
    }



    void MapperAxROM::WritePRG(Address addr, BYTE value) {
        if (addr >= 0x8000) {
            m_PRGBank   = value  & 0x07;
            m_Mirroring = (value & 0x10) ? OneScreenHigher : OneScreenLower;
            m_MirroringCallback();
        }
    }

    BYTE MapperAxROM::ReadPRG(Address addr) {
        if (addr >= 0x8000) {
            return m_Cartridge.GetROM()[m_PRGBank * 0x8000 + (addr & 0x7FFF)];
        }

        return 0;
    }



    BYTE MapperAxROM::ReadCHR(Address addr) {
        if (addr < 0x2000) {
            return m_CharacterRAM[addr];
        }

        return 0;
    }

    void MapperAxROM::WriteCHR(Address addr, BYTE value) {
        if (addr < 0x2000) {
            m_CharacterRAM[addr] = value;
        }
    }



    NameTableMirroring MapperAxROM::GetNameTableMirroring() {
        return m_Mirroring;
    }

};