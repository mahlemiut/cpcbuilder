/*
 * imgconvert.cpp - class to convert images to CPC graphical data
 *
 *  Created on: 15/01/2014
 */

#include "imgconvert.h"

imageconvert::imageconvert(QString fname)
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

	m_loaded = true;
}

bool imageconvert::convert(unsigned char* buffer, int buffer_size, int mode)
{
	if(!m_loaded)
		return false;
	if(mode < 0 || mode > 2)  // maybe add mode 3?
		return false;

	switch(mode)
	{
	case 0:
		return convert_mode0(buffer, buffer_size);
	case 1:
		return convert_mode1(buffer, buffer_size);
	case 2:
		return convert_mode2(buffer, buffer_size);
	}
	return false;
}

bool imageconvert::convert_mode0(unsigned char* buffer, int buffer_size)
{
	int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 2;
	m_height = m_image.height();

	// this may or may not work right
	m_image.setColorCount(16);

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
	}
	return true;
}

bool imageconvert::convert_mode1(unsigned char* buffer, int buffer_size)
{
	int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 4;
	m_height = m_image.height();

	// this may or may not work right
	m_image.setColorCount(4);

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
	}
	return true;
}

bool imageconvert::convert_mode2(unsigned char* buffer, int buffer_size)
{
	int size;
	int x,y;
	unsigned int ptr = 0;

	// set width/height
	m_width = m_image.width() / 8;
	m_height = m_image.height();

	// this may or may not work right
	m_image.setColorCount(2);

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
