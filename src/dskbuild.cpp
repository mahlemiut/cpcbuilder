/*
 * dskbuild.cpp
 *
 *  Created on: 25/08/2011
 *      Author: bsr
 */

#include "dskbuild.h"

unsigned char sector_order[9] =
{
	0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5
};

dsk_builder::dsk_builder(QString filename) :
	m_filename(filename),
	m_block_counter(2),
	m_alloc_counter(0),
	m_record_counter(0),
	m_directory_counter(0),
	m_sector_counter(4)
{
	memset(m_sector_data,0xe5,sizeof(m_sector_data));  // 0xe5 == unformatted data
}

dsk_builder::~dsk_builder()
{

}

// Generates a DSK image of a disk containing the files stored in path
bool dsk_builder::generate_dsk()
{
	int x,y;

	// copy raw sector data to DSK image
	QFile out(m_filename);
	if(out.open(QFile::WriteOnly) == false)
	{
		QMessageBox msg;
		msg.setText(QString("Cannot create DSK image file '%1'").arg(m_filename));
		msg.setIcon(QMessageBox::Critical);
		msg.setStandardButtons(QMessageBox::Ok);
		msg.exec();
		return false;
	}

	//	sector_counter = 0;

	// write disk info header
	dsk_header header;
	memset(&header,0,sizeof(dsk_header));
	strcpy(header.header,"MV - CPCEMU Disk-File\r\nDisk-Info\r");
	header.header[33] = '\n';  // to avoid buffer overflow
	strcpy(header.creator,"CPC Builder");
	header.tracks = 40;
	header.sides = 1;
	header.track_size_low = 0x00;
	header.track_size_high = 0x13;  // 0x1300 bytes per track (includes track header)
	out.write((const char*)&header,sizeof(dsk_header));

	// write tracks - header then interleaved sector data
	dsk_track track;
	for(x=0;x<40;x++)
	{
		memset(&track,0,sizeof(dsk_track));
		strcpy(track.header,"Track-Info\r\n");
		track.track_num = x;
		track.side_num = 0;  // one-sided image, so always side 0
		track.sector_size = 2;  // N=2 or 512-byte sectors
		track.sectors = 9;  // 9 sectors per track
		track.gap3 = 0x4e;  // GAP#3 size
		track.filler = 0xe5;
		// add sector info
		for(y=0;y<9;y++)
		{
			track.sector_list[y].c = x;
			track.sector_list[y].h = 0;
			track.sector_list[y].r = sector_order[y];
			track.sector_list[y].n = 2;
		}
		out.write((const char*)&track,sizeof(dsk_track));
		for(y=0;y<9;y++)
		{
			int offset = 0x200*((sector_order[y]&0x0f)-1) + (x*0x1200);
			out.write((const char*)m_sector_data+offset, 0x200);
		}
	}
	out.close();
	return true;
}

bool dsk_builder::add_file(QString filename, unsigned int load, unsigned int entry)
{
	fat_entry fat;
	amsdos_header amsdos;
	int bytes;
	unsigned char* ptr;
	int x;

	QString fname = filename;
	QStringList split_path = fname.split(QDir::separator());
	QString fullfilename = split_path.last();
	QStringList split = fullfilename.split(".");
	QString filemain = split.first();
	QString fileext = split.last();
	QFile f(fname);
	char buffer[512];
	unsigned short check=0;

	f.open(QFile::ReadOnly);
	// set up fat entry
	memset(&fat,0x00,sizeof(fat_entry));
	for(x=0;x<filemain.length();x++)
        fat.filename[x] = filemain.at(x).toUpper().toLatin1();
	for(x=x;x<8;x++)
		fat.filename[x] = 0x20;
	for(x=0;x<fileext.length();x++)
        fat.extension[x] = fileext.at(x).toUpper().toLatin1();
	for(x=x;x<3;x++)
		fat.extension[x] = 0x20;
	// first extent number is zero, memset has set this for us

	// all files begin at the start of a block, so move to the start of the next block
	if(m_sector_counter & 1)
		m_sector_counter++;

	// insert AMSDOS header
	memset(buffer,0x00,sizeof(buffer));
	memset(&amsdos,0x00,sizeof(amsdos_header));
	for(x=0;x<8;x++)
		amsdos.filename[x] = fat.filename[x];
	for(x=0;x<3;x++)
		amsdos.extension[x] = fat.extension[x];
	amsdos.first_block = 0xff;
	amsdos.file_type = 0x02;  // for now, only support binary files
	amsdos.logical_length_low = f.size() & 0xff;
	amsdos.logical_length_high = (f.size() >> 8) & 0xff;
	amsdos.data_length_low = f.size() & 0xff;
	amsdos.data_length_high = (f.size() >> 8) & 0xff;
	amsdos.length_bytes_low = f.size() & 0xff;
	amsdos.length_bytes_mid = (f.size() >> 8) & 0xff;
	amsdos.length_bytes_high = (f.size() >> 16) & 0xff;
	amsdos.data_location_low = load & 0xff;
	amsdos.data_location_high = (load >> 8) & 0xff;
	amsdos.entry_address_low = entry & 0xff;
	amsdos.entry_address_high = (entry >> 8) & 0xff;

	// checksum
	ptr = (unsigned char*)&amsdos;
	for(x=0;x<66;x++)
		check += *(ptr+x);

	amsdos.checksum_low = check & 0xff;
	amsdos.checksum_high = (check >> 8) & 0xff;

	m_record_counter = 0;
	m_alloc_counter = 0;
	memcpy(m_sector_data[m_sector_counter],(unsigned char*)&amsdos,128);
	fat.allocation[m_alloc_counter] = m_block_counter;
	m_block_counter++;
	m_alloc_counter++;
	m_record_counter++;
	bytes = f.read(buffer,512-128);  // fill rest of sector
	m_record_counter += ((bytes-1)/128)+1;
	memcpy(&m_sector_data[m_sector_counter][128],buffer,512-128);
	m_sector_counter++;

	while(!f.atEnd())
	{
		if(!(m_sector_counter & 1))
		{
			fat.allocation[m_alloc_counter] = m_block_counter;
			m_block_counter++;
			m_alloc_counter++;
		}
		if(m_alloc_counter >= 16)  // after every 32 sectors (16 blocks/128 records), make an extra fat entry
		{
			fat.record_num = 0x80;
			m_record_counter = 0;
			m_alloc_counter = 0;
			memcpy(m_sector_data[0]+(m_directory_counter*32),&fat,32);
			memset(fat.allocation,0,16);
			m_directory_counter++;
			fat.extent_low++;
		}
		memset(buffer,0x00,sizeof(buffer));
		bytes = f.read(buffer,512);  // read one sector worth at a time
		m_record_counter += ((bytes-1)/128)+1;
		memcpy(m_sector_data[m_sector_counter],buffer,512);
		m_sector_counter++;
	}
	// and write last FAT entry
	fat.record_num = m_record_counter;
	memcpy(m_sector_data[0]+(m_directory_counter*32),&fat,32);
	memset(fat.allocation,0,16);
	m_directory_counter++;
	f.close();
	return true;  // TODO: handle possible errors
}

bool dsk_builder::add_ascii_file(QString filename)
{
	fat_entry fat;
	int bytes;
	int x;

	QString fname = filename;
	QStringList split_path = fname.split(QDir::separator());
	QString fullfilename = split_path.last();
	QStringList split = fullfilename.split(".");
	QString filemain = split.first();
	QString fileext = split.last();
	QFile f(fname);
	char buffer[512];

	f.open(QFile::ReadOnly);
	// set up fat entry
	memset(&fat,0x00,sizeof(fat_entry));
	for(x=0;x<filemain.length();x++)
        fat.filename[x] = filemain.at(x).toUpper().toLatin1();
	for(x=x;x<8;x++)
		fat.filename[x] = 0x20;
	for(x=0;x<fileext.length();x++)
        fat.extension[x] = fileext.at(x).toUpper().toLatin1();
	for(x=x;x<3;x++)
		fat.extension[x] = 0x20;
	// first extent number is zero, memset has set this for us

	// all files begin at the start of a block, so move to the start of the next block
	if(m_sector_counter & 1)
	{
		m_sector_counter++;
		m_block_counter++;
	}

	m_record_counter = 0;
	m_alloc_counter = 0;
	fat.allocation[m_alloc_counter] = m_block_counter;
	m_record_counter++;
	bytes = f.read(buffer,512);  // fill rest of sector
	m_record_counter += ((bytes-1)/128)+1;
	memcpy(&m_sector_data[m_sector_counter][128],buffer,512);
	m_sector_counter++;

	while(!f.atEnd())
	{
		if(!(m_sector_counter & 1))
		{
			fat.allocation[m_alloc_counter] = m_block_counter;
			m_block_counter++;
			m_alloc_counter++;
		}
		if(m_alloc_counter >= 16)  // after every 32 sectors (16 blocks/128 records), make an extra fat entry
		{
			fat.record_num = 0x80;
			m_record_counter = 0;
			m_alloc_counter = 0;
			memcpy(m_sector_data[0]+(m_directory_counter*32),&fat,32);
			memset(fat.allocation,0,16);
			m_directory_counter++;
			fat.extent_low++;
		}
		memset(buffer,0x00,sizeof(buffer));
		bytes = f.read(buffer,512);  // read one sector worth at a time
		m_record_counter += ((bytes-1)/128)+1;
		memcpy(m_sector_data[m_sector_counter],buffer,512);
		m_sector_counter++;
	}
	// and write last FAT entry
	fat.record_num = m_record_counter;
	memcpy(m_sector_data[0]+(m_directory_counter*32),&fat,32);
	memset(fat.allocation,0,16);
	m_directory_counter++;
	f.close();
	return true;  // TODO: handle possible errors
}
