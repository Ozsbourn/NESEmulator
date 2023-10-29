#include "NESE_MapperMMC3.hxx"



namespace NESE {

    MapperMMC3::MapperMMC3(Cartridge& cart, std::function<void(void)> interrupt_cb, std::function<void(void)> mirroring_cb) 
    : Mapper(cart, Mapper::MMC3), 
      m_TargetRegister(0),
      m_PRGBankMode(false),
      m_CHRInversion(false),
      m_BankRegister{},
      m_IRQEnabled(false),
      m_IRQCounter(0),
      m_IRQLatch(0),
      m_IRQReloadPending(false),
      m_PRGRAM(32 * 1024),
      m_MirroringRAM(4 * 1024),
      m_Mirroring(Horizontal),
      m_MirroringCallback(mirroring_cb),
      m_InterruptCallback(interrupt_cb) {
          m_PRGBank = &cart.GetROM()[cart.GetROM().size() - 0x4000];
          m_PRGBank = &cart.GetROM()[cart.GetROM().size() - 0x2000];
          m_PRGBank = &cart.GetROM()[cart.GetROM().size() - 0x4000];
          m_PRGBank = &cart.GetROM()[cart.GetROM().size() - 0x2000];

          for (auto& bank : m_CHRBanks) {
              bank = cart.GetVROM().size() - 0x400;
          }

          m_CHRBanks[0] = cart.GetVROM().size() - 0x800;
          m_CHRBanks[3] = cart.GetVROM().size() - 0x800;
    }


    BYTE MapperMMC3::ReadPRG(Address addr) {
        if (addr >= 0x6000 && addr <= 0x7FFF) {
            return m_PRGRAM[addr & 0x1FFF];
        }

        if (addr >= 0x8000 && addr <= 0x9FFF) {
            return *(m_PRGBank0 + (addr & 0x1FFF));
        }

        if (addr >= 0xA000 && addr <= 0xBFFF) {
            return *(m_PRGBank1 + (addr & 0x1FFF));
        }

        if (addr >= 0xC000 && addr <= 0xDFFF) {
            return *(m_PRGBank2 + (addr & 0x1FFF));
        }

        if (addr >= 0xE000) {
            return *(m_PRGBank3 + (addr & 0x1FFF));
        }

        return 0;
    }


    BYTE MapperMMC3::ReadCHR(Address addr) {
        if (addr < 0x1FFF) {
            const auto bank_select  = addr >> 10;
            const auto base_address = m_CHRBanks[bank_select];
            const auto offset       = addr & 0x3FF;

            return m_Cartridge.GetVROM()[base_address + offset];
        } else if (addr <= 0x2FFF) {
            return m_MirroringRAM[addr - 0x2000];
        }

        return 0;
    }

    void MapperMMC3::WritePRG(Address addr, BYTE val) {
        if (addr >= 0x6000 && addr <= 0x7FFF) {
            m_PRGRAM[addr & 0x1FFF] = val;
        } else if (addr >= 0x8000 && addr <= 0x9FFF) {
            // Bank select
            if (!(addr & 0x01)) {
                m_TargetRegister = val & 0x7;
                m_PRGBankMode    = val & 0x40;
                m_CHRInversion   = val & 0x80;
            } else {
                m_BankRegister[m_TargetRegister] = val;

                if (m_CHRInversion == 0) {
                    // add 0xfe mask to ignore lowest bit
                    m_CHRBanks[0] = (m_BankRegister[0] & 0xFE) * 0x0400;
                    m_CHRBanks[1] = (m_BankRegister[0] & 0xFE) * 0x0400 + 0x0400;
                    m_CHRBanks[2] = (m_BankRegister[1] & 0xFE) * 0x0400;
                    m_CHRBanks[3] = (m_BankRegister[1] & 0xFE) * 0x0400 + 0x0400;

                    m_CHRBanks[4] = m_BankRegister[2] * 0x0400;
                    m_CHRBanks[5] = m_BankRegister[3] * 0x0400;
                    m_CHRBanks[6] = m_BankRegister[4] * 0x0400;
                    m_CHRBanks[7] = m_BankRegister[5] * 0x0400;
                } else if (m_CHRInversion == 1) {
                    m_CHRBanks[0] = m_BankRegister[2] * 0x0400;
                    m_CHRBanks[1] = m_BankRegister[3] * 0x0400;
                    m_CHRBanks[2] = m_BankRegister[4] * 0x0400;
                    m_CHRBanks[3] = m_BankRegister[5] * 0x0400;

                    m_CHRBanks[4] = (m_BankRegister[0] & 0xFE) * 0x0400;
                    m_CHRBanks[5] = (m_BankRegister[0] & 0xFE) * 0x0400 + 0x0400;
                    m_CHRBanks[6] = (m_BankRegister[1] & 0xFE) * 0x0400;
                    m_CHRBanks[7] = (m_BankRegister[1] & 0xFE) * 0x0400 + 0x0400;
                }

                if (m_PRGBankMode == 0) {
                    // ignore top two bits for r6 / r7 by using 0x3f
                    m_PRGBank0 = &m_Cartridge.GetROM()[(m_BankRegister[6] & 0x3F) * 0x2000];
                    m_PRGBank1 = &m_Cartridge.GetROM()[(m_BankRegister[7] & 0x3F) * 0x2000];
                    m_PRGBank2 = &m_Cartridge.GetROM()[m_Cartridge.GetROM().size() - 0x4000];
                    m_PRGBank3 = &m_Cartridge.GetROM()[m_Cartridge.GetROM().size() - 0x2000];
                } else if (m_PRGBankMode == 1) {
                    m_PRGBank0 = &m_Cartridge.GetROM()[m_Cartridge.GetROM().size() - 0x4000];
                    m_PRGBank1 = &m_Cartridge.GetROM()[(m_BankRegister[7] & 0x3F) * 0x2000];
                    m_PRGBank2 = &m_Cartridge.GetROM()[(m_BankRegister[6] & 0x3F) * 0x2000];
                    m_PRGBank3 = &m_Cartridge.GetROM()[m_Cartridge.GetROM().size() - 0x2000];
                }
            }
        } else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (!(addr & 0x01)) {
                // Mirroring
                if (m_Cartridge.GetNameTableMirroring() & 0x8) {
                    m_Mirroring = NameTableMirroring::FourScreen;
                }
                else if (val & 0x01) {
                    m_Mirroring = NameTableMirroring::Horizontal;
                } else {
                    m_Mirroring = NameTableMirroring::Vertical;
                }

                m_MirroringCallback();
            } else {
                // PRG Ram Protect
            }
        } else if (addr >= 0xC000 && addr <= 0xDFFF) {
            if (!(addr & 0x01)) {
                m_IRQLatch = value;
            } else {
                m_IRQCounter = 0;
                m_IRQReloadPending = true;
            }
        } else if (addr >= 0xE000) {
            // enabled if odd address
          
            m_irqEnabled = (addr & 0x01) == 0x01;
          
            // TODO acknowledge any pending interrupts?
        }
    }

    void MapperMMC3::WriteCHR(Address addr, BYTE val) {
        if (addr >= 0x2000 && addr <= 0x2FFF) {
            m_MirroringRAM[addr - 0x2000] = val;
        }
    }

    void MapperMMC3::ScanLineIRQ(void) {
        bool zero_transition = false;

        if (m_IRQCounter == 0 || m_IRQReloadPending) {
            m_IRQCounter = m_IRQLatch;
            m_IRQReloadPending = false;
        } else {
            m_IRQCounter--;
            zero_transition = (m_IRQCounter == 0);
        }

        if (zero_transition && m_IRQEnabled) {
            m_InterruptCallback();
        }
    }

    NameTableMirroring MapperMMC3::GetNameTableMirroring(void) {
        return m_Mirroring;
    }

};