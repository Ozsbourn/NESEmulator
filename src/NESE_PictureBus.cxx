#include "NESE_PictureBus.hxx"
#include "NESE_Util.hxx"


namespace NESE {

    PictureBus::PictureBus(void) 
    : m_Palette(0x20),
      m_RAM(0x800),
      m_Mapper(nullptr) {}

    BYTE PictureBus::read(Address addr) {
        if (addr < 0x2000) {
            return m_Mapper->readCHR(addr);
        } else if (addr < 0x3eff) {
            const auto index = addr & 0x3ff;

            // Name tables upto 0x3000, then mirrored upto 3eff
            auto normalizedAddr = addr;
            if (addr >= 0x3000) {
                normalizedAddr -= 0x1000;
            }

            if (NameTable0 >= m_RAM.size()) {
                return m_Mapper->readCHR(normalizedAddr);
            } else if (normalizedAddr < 0x2400) {
                //NT0
                return m_RAM[NameTable0 + index];
            } else if (normalizedAddr < 0x2800) {
            	// NT1
                return m_RAM[NameTable1 + index];
            } else if (normalizedAddr < 0x2c00) {
            	// NT2
            	return m_RAM[NameTable2 + index];
            } else {
            	/* if (normalizedAddr < 0x3000)*/ 
            	// NT3
                return m_RAM[NameTable3 + index];
            }
        } else if (addr < 0x3fff) {
            auto paletteAddr = addr & 0x1f;
            return ReadPalette(paletteAddr);
        }

        return 0;
    }

    BYTE PictureBus::ReadPalette(BYTE paletteAddr) {
        // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C

        if (paletteAddr >= 0x10 && paletteAddr % 4 == 0) {
            paletteAddr = paletteAddr & 0xf;
        }

        return m_Palette[paletteAddr];
    }

    void PictureBus::write(Address addr, BYTE value) {
        if (addr < 0x2000) {
            m_Mapper->writeCHR(addr, value);
        } else if (addr < 0x3eff) {
            const auto index = addr & 0x3ff;

            // Name tables upto 0x3000, then mirrored upto 3eff
            auto normalizedAddr = addr;
            if (addr >= 0x3000) {
                normalizedAddr -= 0x1000;
            }

            if (NameTable0 >= m_RAM.size()) {
                m_Mapper->writeCHR(normalizedAddr, value);
            } else if (normalizedAddr < 0x2400) {
            	//NT0
                m_RAM[NameTable0 + index] = value;
            } else if (normalizedAddr < 0x2800) {
            	//NT1
                m_RAM[NameTable1 + index] = value;
            } else if (normalizedAddr < 0x2c00) {
            	//NT2
                m_RAM[NameTable2 + index] = value;
            } else {
            	//NT3
                m_RAM[NameTable3 + index] = value;
            }
        } else if (addr < 0x3fff) {
            auto palette = addr & 0x1f;

            // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
            if (palette >= 0x10 && addr % 4 == 0) {
                palette = palette & 0xf;
            }

            m_Palette[palette] = value;
       }
    }

    void PictureBus::UpdateMirroring(void) {
        switch (m_Mapper->GetNameTableMirroring()) {
            case Horizontal:
                m_NameTable0 = m_NameTable1 = 0;
                m_NameTable2 = m_NameTable3 = 0x400;
                LOG(INFO_VERBOSE) << "Horizontal Name Table mirroring set. (Vertical Scrolling)" << std::endl;
                break;
            case Vertical:
                m_NameTable0 = m_NameTable2 = 0;
                m_NameTable1 = m_NameTable3 = 0x400;
                LOG(INFO_VERBOSE) << "Vertical Name Table mirroring set. (Horizontal Scrolling)" << std::endl;
                break;
            case OneScreenLower:
                m_NameTable0 = m_NameTable1 = m_NameTable2 = m_NameTable3 = 0;
                LOG(INFO_VERBOSE) << "Single Screen mirroring set with lower bank." << std::endl;
                break;
            case OneScreenHigher:
                m_NameTable0 = m_NameTable1 = m_NameTable2 = m_NameTable3 = 0x400;
                LOG(INFO_VERBOSE) << "Single Screen mirroring set with higher bank." << std::endl;
                break;
            case FourScreen:
                m_NameTable0 = m_RAM.size();
                LOG(INFO_VERBOSE) << "FourScreen mirroring." << std::endl;
                break;
            default:
                m_NameTable0 = m_NameTable1 = m_NameTable2 = m_NameTable3 = 0;
                LOG(ERROR) << "Unsupported Name Table mirroring : " << m_Mapper->GetNameTableMirroring() << std::endl;
        }
    }

    bool PictureBus::SetMapper(Mapper *mapper) {
        if (!mapper) {
            LOG(ERROR) << "Mapper argument is nullptr" << std::endl;
            return false;
        }

        m_Mapper = mapper;
        UpdateMirroring();

        return true;
    }

    void PictureBus::ScanLineIRQ(void){
        m_Mapper->ScanLineIRQ();
    }

};