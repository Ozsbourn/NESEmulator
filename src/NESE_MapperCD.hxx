#include "NESE_MapperCD.hxx"
#include "NESE_Util.hxx"



namespace NESE {

	MapperCD::MapperCD(Cartridge& cart, std::function<void(void)> mirroring_cb)
	: Mapper(cart, Mapper::ColorDreams),
	  m_Mirroring(Vertical),
	  m_MirroringCallback(mirroring_cb) {}



	NameTableMirroring MapperCD::GetNameTableMirroring() {
		return m_NameTableMirroring;
	}



	void MapperCD::WritePRG(Address addr, BYTE val) {
		if (addr >= 0x8000) {
			m_PRGBank = ((val >> 0x0) & 0x3);
			m_CHRBank = ((val >> 0x4) & 0xF);
		}
	}

	void MapperCD::ReadPRG(Address addr) {
		if (addr >= 0x8000) {
			return m_Cartridge.GetROM()[(m_PRGBank * 0x8000) + (addr & 0x7FFF)];
		} else {
			return 0;
		}
	}



	void MapperCD::WriteCHR(Address addr, BYTE val) {}

	void MapperCD::ReadCHR(Address addr) {
		if (addr <= 0x1FFF) {
			return m_Cartridge.GetVROM()[(m_CHRBank * 0x2000) + addr];
		} else {
			return 0;
		}
	}

};
