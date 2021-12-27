#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <string>


typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   uint;


class Format
{
	std::ostringstream ss;
public:
	Format& operator()()  {ss.str(""); return *this;}
	template<typename T> Format& operator<<(const T& t)  {ss<<t; return *this;}
	const std::string str() const {return ss.str();}
} fmt;



#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif
struct BootSector
{
	byte ignore1[3];
	char oem_name[8];
	word bytes_per_sector;
	byte sectors_per_cluster;
	word reserved_sectors;
	byte fat_copies;
	word max_rootdir_entries;
	word sector_count;
	byte media_destriptor_byte;
	word sectors_per_fat;
	word sectors_per_track;
	word head_count;
	uint hidden_sectors;
	uint sector_count_ex;
	byte drive_number;
	byte ignore2;
	byte boot_signature;
	uint volume_serial;
	char fs_name[11];
	char fat_version[8];
	byte bootloader[448];
	word bootloader_sig;
}
#ifdef _MSC_VER
;
#pragma pack(pop)
#else
__attribute__((packed));
#endif





#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif
struct RootDirEntry
{
	char filename[8];
	char extension[3];
	byte attributes;
	byte reserved1;
	byte creation_ms;
	word creation_time;
	word creation_date;
	word lastaccess_date;
	word reserved2;
	word lastchange_time;
	word lastchange_date;
	word start_cluster_plus2;
	uint file_size;

	RootDirEntry()
	{
		memset( this, 0, sizeof(*this) );
	}

	RootDirEntry( const std::string& name )
		: attributes(0x08), reserved1(0), creation_ms(0), creation_time(0)
		, creation_date(0), lastaccess_date(0), reserved2(0), lastchange_time(0xA2AA)
		, lastchange_date(0x3B1E), start_cluster_plus2(0), file_size(0)
	{
		memcpy( filename, name.c_str(), name.size() );
		memset( filename+name.size(), ' ', sizeof(filename)-name.size() );
		memset( extension, ' ', 3 );
	}

	RootDirEntry( const std::string& name, const std::string& ext, uint filesize, uint first_cluster )
		: attributes(0x20), reserved1(0), creation_ms(1), creation_time(0xA2BA)
		, creation_date(0x3B1E), lastaccess_date(0x3B5A), reserved2(0), lastchange_time(0x863C)
		, lastchange_date(0x3B5A), start_cluster_plus2(first_cluster+2), file_size(filesize)
	{
		memcpy( filename, name.c_str(), name.size() );
		memset( filename+name.size(), ' ', sizeof(filename)-name.size() );
		memcpy( extension, ext.c_str(), ext.size() );
		memset( filename+ext.size(), ' ', sizeof(extension)-ext.size() );
	}
}
#ifdef WIN32
;
#pragma pack(pop)
#else
__attribute__((packed));
#endif




std::vector<byte> compactFAT( const std::vector<word>& fat )
{
	std::vector<byte> ret( fat.size()*3/2 );
	byte* p = &ret[0];
	for ( uint i=0; i<fat.size()-1; i+=2 )
	{
		*(p++) = fat[i] & 0xFF;
		*(p++) = (fat[i]>>8) | ((fat[i+1]&0xF)<<4);
		*(p++) = fat[i+1]>>4;
	}
	return ret;
}




int main(unsigned int argc, char* argv[])
{
	assert( sizeof(BootSector) == 512 );
	assert( sizeof(RootDirEntry) == 32 );

	try
	{
		if ( argc < 3 )
			throw (fmt() << "Usage: CreateFloppyImage VolumeName [ImagePath] [BootsectorPath] [File1] [File2] ...").str();

		// Read and verify the bootsector
		BootSector bs;
		{
			std::ifstream bsf( argv[3], std::ios::binary );
			if ( ! bsf )
				throw (fmt() << "Cannot open the bootsector file '" << argv[3] << "'").str();
			bsf.read( (char*)&bs, sizeof(bs) );
			if ( bsf.gcount() != sizeof(bs) )
				throw (fmt() << "Bootsector file is too small, expected " << sizeof(bs) << " bytes").str();
			if ( strncmp( bs.fat_version, "FAT12   ", 8 ) != 0 )
				throw (fmt() << "Can handle FAT12 only").str();
			if ( bs.bytes_per_sector*bs.sector_count != 1474560 )
				throw (fmt() << "Can handle images of size 1474560 (floppy) only").str();
		}

		uint cluster_size = bs.bytes_per_sector * bs.sectors_per_cluster;
		uint cluster_count = bs.sector_count / bs.sectors_per_cluster;

		// Setup FAT, Root Directory and Data Sectors
		std::vector<word> fat( bs.sectors_per_fat*bs.bytes_per_sector*2/3, 0 );
		std::vector<RootDirEntry> rootdir( bs.max_rootdir_entries, RootDirEntry() );
		std::vector<char> data( cluster_count*cluster_size );

		uint fat_idx = 2;
		fat[0] = 0xFF0;
		fat[1] = 0xFFF;
		uint direntry_idx = 0;

		// Add volume entry
		RootDirEntry volume_entry = RootDirEntry( argv[1] );
		rootdir[direntry_idx++] = volume_entry;

		// Add the files
		for ( int i=4; i<argc; ++i )
		{
			// Open and read the file
			std::ifstream file( argv[i], std::ios::binary );
			if ( ! file )
				throw (fmt() << "Cannot open the file '" << argv[i] << "'").str();
			file.seekg( 0, std::ios::end );
			size_t file_size = file.tellg();
			file.seekg( 0, std::ios::beg );

			// Get the file's name and extension ("C:/foo/dog.txt" -> "dog", "txt")
			std::string name( argv[i] );
			size_t slash_pos = name.find_last_of( "\\/" );
			if ( slash_pos != std::string::npos )
				name.erase( 0, slash_pos+1 );
			std::string ext;
			size_t dot_pos = name.find_last_of( "." );
			if ( dot_pos != std::string::npos )
			{
				ext = name.substr( dot_pos+1 );
				name.erase( dot_pos );
			}

			// Insert file
			{
				assert( bs.sectors_per_cluster == 1 );
				assert( bs.bytes_per_sector == 512 );
				assert( bs.max_rootdir_entries == 224 );

				uint first = fat_idx;
				for ( int fs=file_size; fs>=0; fs-=bs.bytes_per_sector*bs.sectors_per_cluster )
				{
					char* cluster = &data[0] + (fat_idx-2)*cluster_size;
					file.read( cluster, cluster_size );

					fat[fat_idx] = fat_idx+1;
					fat_idx++;
				}
				fat[fat_idx-1] = 0xFFF;
				rootdir[direntry_idx++] = RootDirEntry( name, ext, file_size, first-2 );
			}
		}

		// Save the image to file
		{
			std::ofstream file( argv[2], std::ios::binary );
			if ( ! file )
				throw (fmt() << "Cannot open the file '" << argv[1] << "'").str();

			// Write the bootsector
			file.write( (const char*)&bs, 512 );

			// Write the reserved sectors
			std::vector<char> reserved( bs.bytes_per_sector, 0 );
			for ( uint i=1; i<bs.reserved_sectors; ++i )
				file.write( &reserved[0], reserved.size() );

			// Write the FAT
			std::vector<byte> compacted_fat = compactFAT( fat );
			for ( uint i=0; i<bs.fat_copies; ++i )
				file.write( (const char*)&compacted_fat[0], compacted_fat.size() );

			// Write the root directory
			file.write( (const char*)&rootdir[0], rootdir.size()*sizeof(RootDirEntry) );

			// Write the data sectors
			file.write( &data[0], 1474560-file.tellp() );
		}
	}
	catch ( const std::string& s )
	{
		std::cerr << s << '\n';
		return -1;
	}
}
