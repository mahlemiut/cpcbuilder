/*
 * imgconvert.h
 *
 *  Created on: 15/01/2014
 */

#ifndef IMGCONVERT_H_
#define IMGCONVERT_H_

#include <stdio.h>
#include <QtWidgets>
#include <QString>
#include <QImage>

class imageconvert
{
public:
	imageconvert(QString fname, int mode);
	~imageconvert() {}
	bool convert(unsigned char* buffer, unsigned int buffer_size, int mode);
	//bool convert_scr(unsigned char* buffer, int buffer_size) {}  // TODO
	int get_width() { return m_width; }
	int get_height() { return m_height; }
	QColor get_colour(int col);
	virtual int calculate_size(int mode)
	{
		if(!m_loaded)
			return 0;

		// For now, we'll stick with the default screen size
		return 16384;

		switch(mode)
		{
		default:
		case 0:
			return (m_image.width()/2) * m_image.height();
		case 1:
			return (m_image.width()/4) * m_image.height();
		case 2:
			return (m_image.width()/8) * m_image.height();
		case 3:
			return (m_image.width()/2) * m_image.height();
		}
	}
protected:
	bool m_loaded;  // true if image is loaded and ready, false if loading failed
	QImage m_image;
	int m_width;  // set after conversion
	int m_height;  // set after conversion
private:
	bool convert_mode0(unsigned char* buffer, unsigned int buffer_size);
	bool convert_mode1(unsigned char* buffer, unsigned int buffer_size);
	bool convert_mode2(unsigned char* buffer, unsigned int buffer_size);
};

class tileconvert : public imageconvert
{
public:
	tileconvert(QString fname, int mode);
	~tileconvert() {}

	bool convert(unsigned char* buffer, unsigned int buffer_size, int mode, int width, int height);
	int get_tile_width() { return m_tilewidth; }
	int get_tile_height() { return m_tileheight; }

	virtual int calculate_size(int mode, int width, int height) // TODO
	{
		int tilesize;  // in bytes
		int tilenum;

		if(!m_loaded)
			return 0;

		switch(mode)
		{
		default:
		case 0:
			tilesize = width/2 * height;
			break;
		case 1:
			tilesize = width/4 * height;
			break;
		case 2:
			tilesize = width/8 * height;
			break;
		case 3:
			tilesize = width/2 * height;
			break;
		}
		tilenum = (m_image.width() / width) * (m_image.height() / height);

		QMessageBox msg(QMessageBox::Critical,"DEBUG",QString("Size: %1 x %2.  Tilesize: %3 bytes.  Buffer: %4 bytes for %5 tiles.").arg(width).arg(height).arg(tilesize).arg(tilesize*tilenum).arg(tilenum),QMessageBox::Ok);
		msg.exec();

		return tilesize * tilenum;
	}

private:
	bool convert_mode0(unsigned char* buffer, unsigned int buffer_size, int width, int height);
	bool convert_mode1(unsigned char* buffer, unsigned int buffer_size, int width, int height);
	bool convert_mode2(unsigned char* buffer, unsigned int buffer_size, int width, int height);
	int m_tilewidth;  // set after conversion
	int m_tileheight;  // set after conversion
};

#endif /* IMGCONVERT_H_ */
