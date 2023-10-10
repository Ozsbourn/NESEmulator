#include "NESE_Mapper.hxx"
#include "NESE_Util.hxx"


namespace NESE {

    MapperCNROM::MapperCNROM(Cartridge& cart)
    : Mapper(cart, Mapper::CNROM),
      m_SelectCHR(0) {
          if (cart.GetROM().size() == 0x4000) {
              m_OneBank = true;
          } else {
              m_OneBank = false;
          }
    }



    BYTE MapperCNROM::ReadPRG(Address addr) {
        if (!m_OneBank) {
            return m_Cartridge.GetROM()[addr - 0x8000];
        } else {
            return m_Cartridge.GetROM()[(addr - 0x8000) & 0x3FFF];
        }
    }

    void MapperCNROM::WritePRG(Address addr, BYTE val) {
        m_SelectCHR = val & 0x3;
    }



    BYTE MapperCNROM::ReadCHR(Address addr) {
        return m_Cartridge.GetVROM()[addr | (m_SelectCHR << 13)];
    }

    void MapperCNROM::WriteCHR(Address addr, BYTE val) {
        LOG(INFO) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
    }

};