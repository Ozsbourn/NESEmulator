#include "Mapper.hxx"

#include "NESE_CPUOpCodes.hxx"

// to-do: specify mappers

namespace NESE {
	
	NameTableMirroring Mapper::GetNameTableMirroring(void) {
		return static_cast<NameTableMirroring>(m_Cartridge.GetNameTableMirroring());
	}

	std::unique_ptr<Mapper> Mapper::CreateMapper(Mapper::Type t, Cartridge& cart, std::function<void()> interrupt_cb, std::function<void(void)> mirroring_cb) {
		std::unique_ptr<Mapper> ret(nullptr);

		switch (t) {
			case NROM:
				//ret.reset(new MapperNROM(cart));
				break;
			case SxROM:
				break;
			case UxROM:
				break;
			case CNROM:
				break;
			case MMC3:
				break;
			case AxROM:
				break;
			case ColorDreams:
				break;
			case GxROM:
				break;
			default:
				break;
		}

		return ret;
	}

};