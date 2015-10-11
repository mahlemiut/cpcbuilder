/*
 * imgconvert.cpp - class to convert images to CPC graphical data
 *
 *  Created on: 15/01/2014
 */

#include "imgconvert.h"

// Graphic screen importation
imageconvert::imageconvert(QString fname, int mode)
{
	QImage orig_img;

	m_loaded = false;

	// load image, exit if it fails
	if(!orig_img.load(fname))
	{
		QMessageBox msg(QMessageBox::Critical,"Import error","Cannot load or find image file",QMessageBox::Ok);
		msg.exec();
		m_loaded = false;
		return;
	}

	// convert to an indexed colour image
	m_image = orig_img.convertToFormat(QImage::Format_Indexed8);

	if(mode < 0 || mode > 2)  // maybe add mode 3?
		return;

	// setting colour count may or may not work right
	switch(mode)
	{
	case 0:
		m_image.setColorCount(16);
		break;
	case 3:
	case 1:
		m_image.setColorCount(4);
		break;
	case 2:
		m_image.setColorCount(2);
		break;
	}

	m_loaded = true;
}

bool imageconvert::convert(unsigned char* buffer, unsigned int buffer_size, int mode)
{
	QImage temp_img;

	if(!m_loaded)
		return false;
	if(mode < 0 || mode > 2)  // maybe add mode 3?
		return false;

	switch(mode)
	{
	case 0:
		temp_img = m_image.scaled(160,200,Qt::IgnoreAspectRatio);
		m_image = temp_img;
		return convert_mode0(buffer, buffer_size);
	case 1:
		temp_img = m_image.scaled(320,200,Qt::IgnoreAspectRatio);
		m_image = temp_img;
		return convert_mode1(buffer, buffer_size);
	case 2:
		temp_img = m_image.scaled(640,200,Qt::IgnoreAspectRatio);
		m_image = temp_img;
		return convert_mode2(buffer, buffer_size);
	}
	return false;
}

bool imageconvert::convert_mode0(unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 2;
	m_height = m_image.height();

	// determine if we have a buffer large enough
	size = m_width * m_height;  // 2 pixels per byte

	// for testing
	//m_image.save("test.png","PNG");

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(y=0;y<m_image.height();y++)  // scanline
	{
		for(x=0;x<m_image.width();x+=2)  // image pixel
		{
			unsigned char byte;
			unsigned char res;
			buffer[ptr] = 0;
			byte = m_image.pixelIndex(x,y);
			res = ((byte & 0x08) << 4) | ((byte & 0x04) << 3) | ((byte & 0x02) << 2) | ((byte & 0x01) << 1);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+1,y);
			res = ((byte & 0x08) << 3) | ((byte & 0x04) << 2) | ((byte & 0x02) << 1) | ((byte & 0x01));
			buffer[ptr] |= res;
			ptr++;
		}
		ptr = (y / 8) * m_width;
		ptr += 0x800 * (y % 8);
		ptr &= 0x3fff;
	}
	return true;
}

bool imageconvert::convert_mode1(unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 4;
	m_height = m_image.height();

	// determine if we have a buffer large enough
	size = m_width * m_height;  // 2 pixels per byte

	// for testing
	//m_image.save("test.png","PNG");

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(y=0;y<m_image.height();y++)  // scanline
	{
		for(x=0;x<m_image.width();x+=4)  // image pixel
		{
			unsigned char byte;
			unsigned char res;
			buffer[ptr] = 0;
			byte = m_image.pixelIndex(x,y);
			res = ((byte & 0x02) << 2) | ((byte & 0x01) << 7);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+1,y);
			res = ((byte & 0x02) << 1) | ((byte & 0x01) << 6);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+2,y);
			res = ((byte & 0x02)) | ((byte & 0x01) << 5);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+3,y);
			res = ((byte & 0x02) >> 1) | ((byte & 0x01) << 4);
			buffer[ptr] |= res;
			ptr++;
		}
		ptr = (y / 8) * m_width;
		ptr += 0x800 * (y % 8);
		ptr &= 0x3fff;
	}
	return true;
}

bool imageconvert::convert_mode2(unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 8;
	m_height = m_image.height();

	// determine if we have a buffer large enough
	size = m_width * m_height;  // 2 pixels per byte

	// for testing
	//m_image.save("test.png","PNG");

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(y=0;y<m_image.height();y++)  // scanline
	{
		for(x=0;x<m_image.width();x+=8)  // image pixel
		{
			unsigned char byte;
			unsigned char res;
			buffer[ptr] = 0;
			byte = m_image.pixelIndex(x,y);
			res = ((byte & 0x01) << 7);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+1,y);
			res = ((byte & 0x01) << 6);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+2,y);
			res = ((byte & 0x01) << 5);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+3,y);
			res = ((byte & 0x01) << 4);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+4,y);
			res = ((byte & 0x01) << 3);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+5,y);
			res = ((byte & 0x01) << 2);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+6,y);
			res = ((byte & 0x01) << 1);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+7,y);
			res = ((byte & 0x01));
			buffer[ptr] |= res;
			ptr++;
		}
		ptr = (y / 8) * m_width;
		ptr += 0x800 * (y % 8);
		ptr &= 0x3fff;
	}
	return true;
}

QColor imageconvert::get_colour(int col)
{
	if(!m_loaded)
		return QColor(0,0,0);

	QColor colour(m_image.color(col));

	// convert to nearest CPC colour
	if(colour.blue() < 60)
		colour.setBlue(0);
	else if(colour.blue() > 146)
		colour.setBlue(255);
	else colour.setBlue(96);
	if(colour.red() < 60)
		colour.setRed(0);
	else if(colour.red() > 146)
		colour.setRed(255);
	else colour.setRed(96);
	if(colour.green() < 60)
		colour.setGreen(0);
	else if(colour.green() > 146)
		colour.setGreen(255);
	else colour.setGreen(96);

	return colour;
}

// Tile importation
tileconvert::tileconvert(QString fname, int mode) :
		imageconvert(fname,mode)
{
}

bool tileconvert::convert(unsigned char* buffer, unsigned int buffer_size, int mode, int width, int height)
{
	QImage temp_img;

	if(!m_loaded)
		return false;
	if(mode < 0 || mode > 2)  // maybe add mode 3?
		return false;

	switch(mode)
	{
	case 0:
		return convert_mode0(buffer, buffer_size, width, height);
	case 1:
		return convert_mode1(buffer, buffer_size, width, height);
	case 2:
		return convert_mode2(buffer, buffer_size, width, height);
	}
	return false;
}

bool tileconvert::convert_mode0(unsigned char* buffer, unsigned int buffer_size, int width, int height)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;
	int tilenum_x;
	int tilenum_y;

	// set width/height
	m_width = m_image.width() / 2;
	m_height = m_image.height();
	m_tilewidth = width / 2;
	m_tileheight = height;

	// determine if we have a buffer large enough
	size = calculate_size(0,width,height);

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(tilenum_y=0;tilenum_y<m_height;tilenum_y+=height)
	{
		for(tilenum_x=0;tilenum_x<m_width;tilenum_x+=width)
		{
			// per tile
			for(y=0;y<height;y++)  // tile scanline
			{
				for(x=0;x<width;x+=2)  // image pixel
				{
					unsigned char byte;
					unsigned char res;
					buffer[ptr] = 0;
					byte = m_image.pixelIndex(tilenum_x+x,tilenum_y+y);
					res = ((byte & 0x08) << 4) | ((byte & 0x04) << 3) | ((byte & 0x02) << 2) | ((byte & 0x01) << 1);
					buffer[ptr] |= res;
					byte = m_image.pixelIndex(tilenum_x+x+1,tilenum_y+y);
					res = ((byte & 0x08) << 3) | ((byte & 0x04) << 2) | ((byte & 0x02) << 1) | ((byte & 0x01));
					buffer[ptr] |= res;
					ptr++;
					if(ptr >= buffer_size)
					{
						QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer overflow!"),QMessageBox::Ok);
						msg.exec();
						return false;  // buffer overflow
					}
				}
			}
		}
	}
	return true;
}

bool tileconvert::convert_mode1(unsigned char* buffer, unsigned int buffer_size, int width, int height)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 4;
	m_height = m_image.height();

	// determine if we have a buffer large enough
	size = m_width * m_height;  // 2 pixels per byte

	// for testing
	//m_image.save("test.png","PNG");

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(y=0;y<m_image.height();y++)  // scanline
	{
		for(x=0;x<m_image.width();x+=4)  // image pixel
		{
			unsigned char byte;
			unsigned char res;
			buffer[ptr] = 0;
			byte = m_image.pixelIndex(x,y);
			res = ((byte & 0x02) << 2) | ((byte & 0x01) << 7);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+1,y);
			res = ((byte & 0x02) << 1) | ((byte & 0x01) << 6);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+2,y);
			res = ((byte & 0x02)) | ((byte & 0x01) << 5);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+3,y);
			res = ((byte & 0x02) >> 1) | ((byte & 0x01) << 4);
			buffer[ptr] |= res;
			ptr++;
		}
		ptr = (y / 8) * m_width;
		ptr += 0x800 * (y % 8);
		ptr &= 0x3fff;
	}
	return true;
}

bool tileconvert::convert_mode2(unsigned char* buffer, unsigned int buffer_size, int width, int height)
{
	unsigned int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 8;
	m_height = m_image.height();

	// determine if we have a buffer large enough
	size = m_width * m_height;  // 2 pixels per byte

	// for testing
	//m_image.save("test.png","PNG");

	if(size > buffer_size)
	{
		QMessageBox msg(QMessageBox::Critical,"Import error",QString("Buffer provided is too small - Provided %1 bytes, need %2 bytes").arg(buffer_size).arg(size),QMessageBox::Ok);
		msg.exec();
		return false;  // size is greater than the buffer provided
	}

	for(y=0;y<m_image.height();y++)  // scanline
	{
		for(x=0;x<m_image.width();x+=8)  // image pixel
		{
			unsigned char byte;
			unsigned char res;
			buffer[ptr] = 0;
			byte = m_image.pixelIndex(x,y);
			res = ((byte & 0x01) << 7);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+1,y);
			res = ((byte & 0x01) << 6);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+2,y);
			res = ((byte & 0x01) << 5);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+3,y);
			res = ((byte & 0x01) << 4);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+4,y);
			res = ((byte & 0x01) << 3);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+5,y);
			res = ((byte & 0x01) << 2);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+6,y);
			res = ((byte & 0x01) << 1);
			buffer[ptr] |= res;
			byte = m_image.pixelIndex(x+7,y);
			res = ((byte & 0x01));
			buffer[ptr] |= res;
			ptr++;
		}
		ptr = (y / 8) * m_width;
		ptr += 0x800 * (y % 8);
		ptr &= 0x3fff;
	}
	return true;
}
