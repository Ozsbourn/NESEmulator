#include <fstream>
#include <string>

#include "NESE_Cartridge.hxx"
#include "NESE_Util.hxx"
// #include "NESE_Mapper.hxx"


namespace NESE {

	Cartrifge::Cartridge(void) 
	: m_NameTableMirroring(0),
	  m_MapperNumber(0),
	  m_ExtendedRAM(false) {}

	const std::vector<BYTE>& Cartridge::GetROM(void) {
		return m_PRG_ROM;
	}

	const std::vector<BYTE>& Cartridge::GetVROM(void) {
		return m_CHR_ROM;
	}

	BYTE Cartridge::GetMapper(void) {
		return m_MapperNumber;
	}

	BYTE Cartridge::GetNameTableMirroring(void) {
		return m_NameTableMirroring;
	}

	bool Cartridge::LoadFromFile(std::string path) {
		std::ifstream romFile(path, std::ios_base::binary | std::ios_base::in);
		if (!romFile) {
			LOG(ERROR) << "Couldn't open ROM file from path: " << path << std::endl;
			return false;
		}

		std::vector<BYTE> header;
		LOG(INFO) << "Reading ROM file from path: " << path << std::endl;

		// Header 
		header.resize(0x10);
		if (!romFile.read(std::reinterpret_cast<char*>(&header[0]), 0x10)) {
			LOG(ERROR) << "Reading iNES header failed!" << std::endl;
			return false;
		}

		if (std::string{&header[0], &header[4]} != "NES\x1A") {
			LOG(ERROR) << "Not a valid iNES image. Magic number: "
					   << std::hex << header[0] << " " << header[1] << " " << header[2] << " " << header[3] << std::endl
					   << "Valid magic number is N E S 1a" << std:endl;
			return false;
		}



		LOG(INFO) << "Reading header, it dictates:\n";

		BYTE banks = header[4];
		LOG(INFO) << "16KB CHR-ROM Banks: " << +banks << std::endl;
		if (!banks) {
			LOG(ERROR) << "ROM has no PRG-ROM banks. Loading ROM failed!" << std::endl;
			return false;
		}


		BYTE vbanks = header[5];
        LOG(INFO) << "8KB CHR-ROM Banks: " << +vbanks << std::endl;

        if (header[6] & 0x8) {
            m_NameTableMirroring = NameTableMirroring::FourScreen;
            LOG(INFO) << "Name Table Mirroring: " << "FourScreen" << std::endl;
        } else {
            m_nameTableMirroring = header[6] & 0x1;
            LOG(INFO) << "Name Table Mirroring: " << (m_NameTableMirroring == 0 ? "Horizontal" : "Vertical") << std::endl;
        }


        m_MapperNumber = ((header[6] >> 4) & 0xF) | (header[7] & 0xF0);
        LOG(INFO) << "Mapper #: " << +m_MapperNumber << std::endl;

        m_ExtendedRAM = header[6] & 0x2;
        LOG(INFO) << "Extended (CPU) RAM: " << std::boolalpha << m_ExtendedRAM << std::endl;
	

        if (header[6] & 0x4) {
        	LOG(ERROR) << "Trainer isn't supported!" << std::endl;
        	return false;
        }

        if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1)) {
        	LOG(ERROR) << "PAL ROM not supported!" << std::endl;
        	return false;
        } else {
        	LOG(INFO) << "ROM is NTSC compatible." << std::endl;
        }


        // PRG-ROM 16KB banks
        m_PRG_ROM.resize(0x4000 * banks);
        if (!romFile.read(reinterpret_cast<char*>(&m_PRG_ROM[0]), 0x4000 * banks)) {
        	LOG(ERROR) << "Reading PRG-ROM from image file failed!" << std::endl;
        	return false;
        }

        // CHR-ROM 8KB banks
        if (vbanks) {
        	m_CHR_ROM.resize(0x2000 * vbanks);
        	if (!romFile.read(reinterpret_cast<char*>(&m_CHR_ROM[0]), 0x2000 * vbanks)) {
        		LOG(ERROR) << "Reading CHR-ROM from image file failed!" << std::endl;
        		return false;
        	}
        } else {
        	LOG(INFO) << "Cartridge with CHR-RAM." << std::endl;
        }


        return true;
	}
};