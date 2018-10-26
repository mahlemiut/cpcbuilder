/*
 * gfxeditor.h
 *
 *  Created on: 5/09/2011
 *      Author: bsr
 */

#ifndef GFXEDITOR_H_
#define GFXEDITOR_H_

#include <QtGui>
#include <QtWidgets>

enum
{
	GFX_DRAW_POINT = 0,
	GFX_DRAW_LINE,
	GFX_DRAW_BOX,
	GFX_DRAW_END  // end of list
};

enum
{
	SCR_FORMAT_LINEAR = 0,
	SCR_FORMAT_SCREEN,
	SCR_FORMAT_SPRITES
};

// Standard CPC palette is 27 (technically 32) colours, with 16 colours at one time
// CPC+ palette is 4096 colours, but still with 16 at one time
// This does not count hardware sprites or changing the palette mid-frame
#define PAL_SIZE_CPC   1*16
#define PAL_SIZE_PLUS  2*16

// gfx file header format (TODO)
struct gfxfile
{
	unsigned char id[7];  // should be set to "CPCGFX\0"
	unsigned int width;
	unsigned int height;
	unsigned char mode;
	unsigned int tilenum;  // number for tiles in the file
};

// Advanced OCP Art Studio palette file (TODO)
struct ocppalette
{
	unsigned char mode;
	unsigned char colour_anim;
	unsigned char colour_anim_delay;
	unsigned char palette_entry[17][12];  // stored as bytes written to gate array
	unsigned char excluded_inks[16];
	unsigned char protected_inks[16];
};

class paletteeditor;

// this class is a test to see if making a custom QFrame that display CPC graphics is faster and easier than QGraphicsView/Scene
class gfxdisplay : public QFrame
{
	Q_OBJECT

public:
	gfxdisplay(QWidget* parent = nullptr, paletteeditor* pal = nullptr);
	~gfxdisplay() { /* if(m_data != NULL) delete m_data;*/  }
	void set_data(unsigned char* data, int x, int y);
	void realloc_data(unsigned char* data) { m_data = data; }  // used after realloc()'ing data, as the old pointer doesn't become NULL, so can't be checked
	int get_mode() { return m_mode; }
	int get_draw_mode() { return m_draw_mode; }
	int get_zoom() { return m_zoom; }
	int get_height() { return m_height; }
	int get_width() { return m_width; }
	int get_height_pixel() { return (m_width * 8) * m_zoom; }
	int get_width_pixel() { return (m_height * 2) * m_zoom; }
	int get_format() { return m_format; }
	void set_size(unsigned int width, unsigned int height) { m_width = width; m_height = height; resize((m_width * 8) * m_zoom,(m_height * m_zoom) * 2); }
	void set_tile(unsigned int tile) { m_tile_current = tile; }
	void line_guide_active(bool state) { m_line_click = state; }
	void box_guide_active(bool state) { m_box_click = state; }
	bool line_guide() { return m_line_click; }
	bool box_guide() { return m_box_click; }
	void set_guide_begin(int x, int y) { m_guide_x0 = x; m_guide_y0 = y; }
	void set_guide_end(int x, int y) { m_guide_x1 = x; m_guide_y1 = y; }
	void set_guide(int x0, int y0, int x1, int y1) { set_guide_begin(x0,y0); set_guide_end(x1,y1); }
public slots:
	void set_zoom(int level) { if(level >= 0) m_zoom = level+1; resize((m_width * 8) * m_zoom,(m_height * m_zoom) * 2);}
	void set_mode(int mode) { m_mode = mode; }
	void draw_mode_point() { m_draw_mode = GFX_DRAW_POINT; }
	void draw_mode_line() { m_draw_mode = GFX_DRAW_LINE; }
	void draw_mode_box() { m_draw_mode = GFX_DRAW_BOX; }
	void set_format_linear() { m_format = SCR_FORMAT_LINEAR; repaint(); }
	void set_format_screen() { m_format = SCR_FORMAT_SCREEN; repaint(); }
protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);
private:
	unsigned char* m_data;  // graphics data
	paletteeditor* m_palette;  // palette
	int m_x;
	int m_y;  //  display size
	int m_zoom;  // display zoom level
	int m_width;  // width in bytes (1 byte = 4 mode 1 pixels)
	int m_height;
	int m_mode;  // CPC video mode (0, 1 or 2)
	int m_draw_mode;  // currently selected drawing tool
	bool m_line_click;  // true if waiting for second click to finish line drawing
	bool m_box_click;  // true if waiting for second click to finish box drawing
	int m_guide_x0;
	int m_guide_y0;
	int m_guide_x1;
	int m_guide_y1;  // co-ords when drawing box/line guides
	int m_hold_x;  // x-coord of last mouse button press
	int m_hold_y;  // y-coord of last mouse button press
	unsigned int m_tile_current;
	int m_format;
	unsigned int get_pen(unsigned int x, unsigned char byte);
	void convert_coords(int *x, int *y);
	void convert_pixel(int *x, int *y);
};

class paletteeditor : public QFrame
{
	Q_OBJECT

public:
	paletteeditor(QWidget* parent = 0);
	~paletteeditor() { /*if(m_data != NULL) delete m_data;*/ }
	void set_data(unsigned char* data, int size);
	void set_colour(int colour, int index) { if(index < m_palette_size) m_data[index] = colour; }
	void set_colour_12(int colour, int index) { if(index < m_palette_size) m_12bit_data[index] = colour; }
	bool set_pen(int index, QColor colour);
	QColor get_pen(int index);
	int get_selected() { return m_selected; }
	bool toggle_pal();
	bool is_12bit_pal() { return m_12bit_palette; }
	void export_palette_to_clipboard();
protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
private:
	QColor get_colour(unsigned int pen);
	QWidget* m_parent;
	unsigned char m_data[PAL_SIZE_CPC];  // palette data
	unsigned short m_12bit_data[PAL_SIZE_CPC];
	int m_palette_size;  // 16, 4 or 2, depending on CPC video mode
	int m_mode;  // CPC video mode (0, 1 or 2)
	int m_selected;  // currently selected pen
	bool m_12bit_palette;  // if true, allow 12-bit CPC+ colours
};

class gfxeditor : public QWidget
{
	Q_OBJECT

public:
	gfxeditor(QWidget* parent = nullptr);
	~gfxeditor();
	QWidget* parent() { return m_parent; }
	QGridLayout* layout() { return m_layout; }
	virtual void set_data(unsigned char* data, int size);
	void set_size(unsigned int width, unsigned int height) { m_width = width; m_height = height; m_frame_gfx->set_size(width,height); /*calculate_tiles();*/ }
	int get_width() { return m_frame_gfx->get_width(); }
	int get_height() { return m_frame_gfx->get_height(); }
	void set_mode(unsigned char mode) { m_mode = mode; m_frame_gfx->set_mode(mode); }
	int get_datasize() { return m_datasize; }
	QScrollArea* get_scrollarea() { return m_scrollarea; }
	unsigned char* get_data() const { return m_data; }
	QColor get_colour(unsigned int pen);
	virtual void plot(int x, int y);  // plot a point in the graphic using the currently selected pen
	virtual void line(int x0, int y0, int x1, int y1); // draw a line in the currently selected pen
	virtual void box(int x0, int y0, int x1, int y1);  // draw a box in the currently selected pen
	bool load_pal_normal(unsigned char* data);
	bool load_pal_12bit(unsigned short* data);
	bool toggle_pal();
	void export_palette_to_clipboard();
	void show_toolbar(bool show) { if(show) m_toolbar->show(); else m_toolbar->hide(); }
	bool set_pen(int index, QColor colour) { return m_frame_palette->set_pen(index,colour); }
public slots:
	void draw_scene() { m_frame_gfx->repaint(); }//m_frame_gfx->draw_scene(m_data,m_frame_palette,m_tile_current); }
	void set_format_linear() { m_frame_gfx->set_format_linear(); }
	void set_format_screen() { m_frame_gfx->set_format_screen(); }
protected:
	void mousePressEvent(QMouseEvent* event);
	QWidget* m_parent;
	unsigned char* m_data;
	int m_datasize;
	unsigned int m_width;  // width is in bytes, so 8 pixels in mode 2, 4 in mode 1, and 2 in mode 0 (and 3)
	unsigned int m_height;
	unsigned char m_mode;
	unsigned char* m_palette;
	unsigned short* m_plus_palette;  // for CPC Plus palette, when we get around to that.

	// child widgets used in this widget
	QGridLayout* m_layout;
	QToolBar* m_toolbar;
	paletteeditor* m_frame_palette;  // the palette will be drawn/accessed here
	gfxdisplay* m_frame_gfx;  // shows visual representation of CPC gfx data
	QComboBox* m_combo_mode;  // used to select the CPC video mode to render as
	QScrollArea* m_scrollarea;  // scroll area for the display of gfx
//	QGraphicsScene m_scene;
	QComboBox* m_combo_zoom;
	QActionGroup* m_actgroup_drawing;
};

class tileeditor : public gfxeditor
{
	Q_OBJECT

public:
	tileeditor(QWidget* parent = nullptr);
	~tileeditor();
	virtual void set_data(unsigned char* data, int size);
	virtual void plot(int x, int y);  // plot a point in the graphic using the currently selected pen
	void set_tile_num(int count) { m_tile_total = count; }
	int get_tile_num() { return m_tile_total; }
	void calculate_tiles();  // calculate the number of tiles that the data represents

public slots:
	void current_tile_changed(int val) { m_tile_current = val; m_frame_gfx->set_tile(val); draw_scene(); }
	void add_gfx_tile();
	void remove_gfx_tile();

private:
	unsigned int m_tile_current;  // currently displayed graphic
	unsigned int m_tile_total;  // total number of graphics

	QSpinBox* m_spin_tile;
	QLabel* m_label_tile_left;
	QLabel* m_label_tile_right;
	QAction* m_act_addtile;
	QAction* m_act_deltile;
};

#endif /* GFXEDITOR_H_ */
