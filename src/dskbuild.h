/*
 * dskbuild.h
 *
 *	DSK image builder - adds files to a CPCEMU DSK image
 *
 *  Created on: 25/08/2011
 *      Author: bsr
 */

#ifndef DSKBUILD_H_
#define DSKBUILD_H_

#include <QtWidgets>

enum
{
	FILETYPE_INTERNAL_BASIC = 0,
	FILETYPE_BINARY,
	FILETYPE_SCREEN_IMAGE,
	FILETYPE_ASCII,
	FILETYPE_TYPE4,  // type 4-7 are unused
	FILETYPE_TYPE5,
	FILETYPE_TYPE6,
	FILETYPE_TYPE7
};

// AMSDOS header, used with any AMSDOS disk file, except unprotected ASCII files.
struct amsdos_header
{
	unsigned char user;  // user number (0-15,0xe5 for deleted files)
	unsigned char filename[8];  // file name
	unsigned char extension[3];  // file extenstion
	unsigned char unused1[4];
	unsigned char block_num;  // not used
	unsigned char last_block;  // not used
	unsigned char file_type;  // bit 0 = protected, bits 1-3 = file type, bits 4-7 version (1 for ASCII, 0 for everything else)
	unsigned char data_length_low;
	unsigned char data_length_high;  // number bytes in the data record
	unsigned char data_location_low;
	unsigned char data_location_high;  // where the data was written from originally
	unsigned char first_block;  // used for output files only, usually 0xff
	unsigned char logical_length_low;
	unsigned char logical_length_high;  // file length in bytes
	unsigned char entry_address_low;
	unsigned char entry_address_high;  // execution address for binary files
	unsigned char unused2[36];
	unsigned char length_bytes_low;
	unsigned char length_bytes_mid;
	unsigned char length_bytes_high;  // file length in bytes
	unsigned char checksum_low;
	unsigned char checksum_high;  // 16-bit checksum, total of all bytes from 0-66
	unsigned char unused3[58];
};

struct fat_entry
{
	unsigned char user;  // user number (0-15,0xe5 for deleted files)
	unsigned char filename[8];  // file name
	unsigned char extension[3];  // file extenstion
	unsigned char extent_low;
	unsigned char extent_reserved;  // not used
	unsigned char extent_high;
	unsigned char record_num;  // number of records
	unsigned char allocation[16]; // each byte or word refers to a block of the disc.  Is 8-bit if less than 256
	                              // blocks on the disc, otherwise 16-bit (low byte first)
};

/* DSK image format - file data in Data Format disks start at block 2 (track 0, sector C5)
 * First 4 sectors (C1-C4) are directory entries, max 64.
 * Files will take up an extra directory entry for each 16kB of the file
 * Tracks are interleaved - C1,C6,C2,C7,C3,C8,C4,C9,C5 (Data Format disk)
 */
struct dsk_header
{
	char header[34];  // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
	char creator[14];
	unsigned char tracks;  // number of tracks
	unsigned char sides;  // number of sides
	unsigned char track_size_low;  // track size in bytes, including this header
	unsigned char track_size_high;
	unsigned char unused[204];
};

struct dsk_sector
{
	unsigned char c;
	unsigned char h;
	unsigned char r;
	unsigned char n;
	unsigned char st1;
	unsigned char st2;
	unsigned char unused[2];
};

struct dsk_track
{
	char header[13];  // "Track-Info\r\n"
	unsigned char unused1[3];
	unsigned char track_num;  // track number
	unsigned char side_num;  // side number (0 or 1)
	unsigned char unused2[2];
	unsigned char sector_size;
	unsigned char sectors;  // number of sectors
	unsigned char gap3;  // GAP#3 length
	unsigned char filler;  // filler byte;
	dsk_sector sector_list[29];
};

class dsk_builder
{
public:
	dsk_builder(QString filename);
	~dsk_builder();
	QString get_filename() { return m_filename; }
	void set_filename(QString filename) { m_filename = filename; }
	bool generate_dsk();
	bool add_file(QString filename, unsigned int load, unsigned int entry);
	bool add_ascii_file(QString filename);
private:
	QString m_filename;
	unsigned char m_sector_data[360][512];  // 9 sectors per track, 40 tracks = 360 512-byte sectors
	unsigned int m_block_counter;  // block = 1kB (2 sectors)
	unsigned int m_alloc_counter;  //
	unsigned int m_record_counter;  // record = 128 bytes
	unsigned int m_directory_counter;
	unsigned int m_sector_counter;  // first 4 sectors are used for the directory table
};

#endif /* DSKBUILD_H_ */
