#include <cstdio>
#include "cprbuild.h"

cprbuilder::cprbuilder(QString fname) :
    m_filename(fname)
{
    init();
}

void cprbuilder::init()
{
    std::string data_hdr = "AMS!fmt \0\0\0\0";

    // create main RIFF header
    strcpy(main_header.header,std::string("RIFF").c_str());
	main_header.data_length = 12;  // no data added yet
    std::copy(data_hdr.begin(),data_hdr.end(),std::back_inserter(main_header.data));

    m_blocks.clear();  // clear all blocks
    for(int x=0;x<32;x++)
        m_blocks_used[x] = false;  // mark all blocks as available
}

bool cprbuilder::generate_cpr()
{
    QFile out(m_filename);
	//riff_header* blocks;

    if(out.open(QFile::WriteOnly) == false)
    {
    	QMessageBox msg;
		msg.setText(QString("Cannot create CPR image file '%1'").arg(m_filename));
		msg.setIcon(QMessageBox::Critical);
		msg.setStandardButtons(QMessageBox::Ok);
		msg.exec();
		return false;
	}

	QDataStream stream(&out);

	stream.setByteOrder(QDataStream::LittleEndian);

    // write header to file
	out.write(reinterpret_cast<const char*>(main_header.header),4);
	stream << main_header.data_length;
	out.write(reinterpret_cast<const char*>(main_header.data.data()),12);

	for(riff_header blocks : m_blocks)
	{
		out.write(reinterpret_cast<const char*>(blocks.header),4);
		stream << blocks.data_length;
		stream.writeRawData(reinterpret_cast<const char*>(&blocks.data[0]),blocks.data_length);
	}

    out.close();
    return true;
}

// returns number of blocks used by file
long cprbuilder::add_block(QString file, char blocknum)
{
    QFile input(file);
    char* input_data;
	char blockhdr[5];
	long blockcount;

	input.open(QFile::ReadOnly);
	input_data = new char[static_cast<unsigned long>(input.size())];
	input.read(input_data,16384*32);
    input.close();

	blockcount = ((input.size()-1) / 16384) + 1;

	for(int x=0;x<input.size()-1;x+=16384)
    {
        riff_header* hdr = new riff_header();

        if(m_blocks_used[blocknum] == true)
            return 0;  // block already used
		snprintf(blockhdr,5,"cb%02uhh",blocknum);
		strncpy(hdr->header,blockhdr,4);
		printf("Block: %s\n",hdr->header);
        hdr->data_length = 16384;
		hdr->data.resize(hdr->data_length);
        memset(hdr->data.data()+x,0,hdr->data_length);  // reset all bytes of the block(s) to 0
		memcpy(hdr->data.data()+x,input_data,input.size());
        m_blocks_used[blocknum] = true;

        m_blocks.push_back(*hdr);
        blocknum++;
		main_header.data_length += 8;  // account for the headers in main header length
		main_header.data_length += hdr->data_length;  // add data length to main chunk
    }
    return blockcount;
}
