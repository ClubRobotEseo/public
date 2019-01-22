/*
 *	Club Robot ESEO 2016 - 2017
 *
 *	Fichier : lcd_display.h
 *	Package : IHM
 *	Description : Driver du LCD_DISPLAY
 *	Auteur : reprise Arnaud Guilmet de la base de Tilen Majerle (http://stm32f4-discovery.com/2014/04/library-08-LCD_DISPLAY-lcd-on-stm32f429-discovery-board/)
 *  Licence : CeCILL-C (voir LICENCE.txt)
 *	Version 20100929
 */

#ifndef LCD_DISPLAY_H
	#define LCD_DISPLAY_H

	#include "../../QS/QS_all.h"
	#include "fonts.h"

	/* LCD settings */
	#define F0_HEIGHT		(420)
	#define F0_WIDTH		(600)
	#define F0_OFFSET_WINDOW_X	(20)
	#define F0_OFFSET_WINDOW_Y	(30)
	extern Uint16 frame0[F0_HEIGHT][F0_WIDTH] __attribute__((section(".ramd1")));

	#define F1_HEIGHT		(20)
	#define F1_WIDTH		(20)
	#define F1_OFFSET_WINDOW_X	(10)
	#define F1_OFFSET_WINDOW_Y	(30)
	extern Uint16 frame1[F1_HEIGHT][F1_WIDTH] __attribute__((section(".ramd1")));


	/* Colors 565 */
	#define LCD_DISPLAY_COLOR_WHITE			0xFFFF
	#define LCD_DISPLAY_COLOR_BLACK			0x0000
	#define LCD_DISPLAY_COLOR_RED			0xF800
	#define LCD_DISPLAY_COLOR_GREEN			0x07E0
	#define LCD_DISPLAY_COLOR_GREEN2		0xB723
	#define LCD_DISPLAY_COLOR_BLUE			0x001F
	#define LCD_DISPLAY_COLOR_BLUE2			0x051D
	#define LCD_DISPLAY_COLOR_YELLOW		0xFFE0
	#define LCD_DISPLAY_COLOR_ORANGE		0xFBE4
	#define LCD_DISPLAY_COLOR_CYAN			0x07FF
	#define LCD_DISPLAY_COLOR_MAGENTA		0xA254
	#define LCD_DISPLAY_COLOR_GRAY			0x7BEF
	#define LCD_DISPLAY_COLOR_LIGHT_GRAY	0xBDF7
	#define LCD_DISPLAY_COLOR_BROWN			0xBBCA

	/* Transparent background, only for strings and chars */
	#define LCD_DISPLAY_TRANSPARENT			0x80000000


	/**
	 * @brief  The point type for LCD
	 */
	typedef struct {
		Uint16 x;  /*!< La coordonnée x */
		Uint16 y;  /*!< La coordonnée y */
	}LCD_DISPLAY_point_t;


	/**
	 * @brief  Possible layers for LCD
	 */
	typedef enum{
		LCD_DISPLAY_Layer_0,  /*!< Couche d'affichage 0 */
		LCD_DISPLAY_Layer_1,  /*!< Couche d'affichage 1 */
		LCD_DISPLAY_Layer_nb
	}LCD_DISPLAY_Layer_t;

	/**
	 * @brief  Possible orientations for LCD
	 */
	typedef enum{
		LCD_DISPLAY_Orientation_Portrait_1,  /*!< Portrait orientation mode 1 */
		LCD_DISPLAY_Orientation_Portrait_2,  /*!< Portrait orientation mode 2 */
		LCD_DISPLAY_Orientation_Landscape_1, /*!< Landscape orientation mode 1 */
		LCD_DISPLAY_Orientation_Landscape_2  /*!< Landscape orientation mode 2 */
	}LCD_DISPLAY_Orientation_t;

	/**
	 * @brief  Screen options structure
	 */
	typedef struct{
		Uint16 width;							/*!< Largeur de l'écran */
		Uint16 height;							/*!< Hauteur de l'écran */
		LCD_DISPLAY_Orientation_t orientation;	/*!< Orientation de l'écran */
	}LCD_DISPLAY_Options_t;

	/**
	 * @brief  Initializes LCD_DISPLAY LCD with LTDC peripheral
	 *         It also initializes external SDRAM
	 * @param  None
	 * @retval None
	 */
	void LCD_DISPLAY_init(void);

	/**
	 * @brief  Reconfigure config.
	 */
	void LCD_DISPLAY_setConfig(void);

	/**
	 * @brief  Get option screen like actual orientation, width, height
	 * @param  layer the layer of the screen concerned.
	 * @retval screen option structure
	 */
	LCD_DISPLAY_Options_t LCD_DISPLAY_getOptions(LCD_DISPLAY_Layer_t layer);

	/**
	 * @brief  Draws single pixel to LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x: X position for pixel
	 * @param  y: Y position for pixel
	 * @param  color: Color of pixel
	 * @retval None
	 */
	void LCD_DISPLAY_drawPixel(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 color);

	/**
	 * @brief  Fills entire LCD with color
	 * @param  layer the layer of the screen concerned
	 * @param  color: Color to be used in fill
	 * @retval None
	 */
	void LCD_DISPLAY_fill(LCD_DISPLAY_Layer_t layer, Uint16 color);

	/**
	 * @brief  Rotates LCD to specific orientation
	 * @param  layer the layer of the screen concerned
	 * @param  orientation: LCD orientation. This parameter can be a value of @ref LCD_DISPLAY_Orientation_t enumeration
	 * @retval None
	 */
	void LCD_DISPLAY_rotate(LCD_DISPLAY_Layer_t layer, LCD_DISPLAY_Orientation_t orientation);

	/**
	 * @brief  Puts single character to LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x: X position of top left corner
	 * @param  y: Y position of top left corner
	 * @param  c: Character to be displayed
	 * @param  *font: Pointer to @ref FontDef_t used font
	 * @param  foreground: Color for char
	 * @param  background: Color for char background
	 * @retval None
	 */
	void LCD_DISPLAY_putc(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, char c, FontDef_t* font, Uint16 foreground, Uint32 background);

	/**
	 * @brief  Puts string to LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x: X position of top left corner of first character in string
	 * @param  y: Y position of top left corner of first character in string
	 * @param  *str: Pointer to first character
	 * @param  *font: Pointer to @ref FontDef_t used font
	 * @param  foreground: Color for string
	 * @param  background: Color for string background
	 * @retval None
	 */
	void LCD_DISPLAY_puts(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, char* str, FontDef_t *font, Uint16 foreground, Uint32 background);

	/**
	 * @brief  Puts formated string to LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x: X position of top left corner of first character in string
	 * @param  y: Y position of top left corner of first character in string
	 * @param  *font: Pointer to @ref FontDef_t used font
	 * @param  foreground: Color for string
	 * @param  background: Color for string background
	 * @param  format : Formated string like printf
	 * @retval None
	 */
	void LCD_DISPLAY_printf(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, FontDef_t *font, Uint16 foreground, Uint32 background, const char *format, ...)  __attribute__((format (printf, 7, 8)));

	/**
	 * @brief  Gets width and height of box with text
	 * @param  *str: Pointer to first character
	 * @param  *font: Pointer to @ref FontDef_t used font
	 * @param  *width: Pointer to variable to store width
	 * @param  *height: Pointer to variable to store height
	 * @retval None
	 */
	void LCD_DISPLAY_getStringSize(char* str, FontDef_t* font, Uint16* width, Uint16* height);

	/**
	 * @brief  Draws line to LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of starting point
	 * @param  y0: Y coordinate of starting point
	 * @param  x1: X coordinate of ending point
	 * @param  y1: Y coordinate of ending point
	 * @param  color: Line color
	 * @retval None
	 */
	void LCD_DISPLAY_drawLine(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color);

	/**
	 * @brief  Draws rectangle on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of top left point
	 * @param  y0: Y coordinate of top left point
	 * @param  x1: X coordinate of bottom right point
	 * @param  y1: Y coordinate of bottom right point
	 * @param  color: Rectangle color
	 * @retval None
	 */
	void LCD_DISPLAY_drawRectangle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color);

	/**
	 * @brief  Draws filled rectangle on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of top left point
	 * @param  y0: Y coordinate of top left point
	 * @param  x1: X coordinate of bottom right point
	 * @param  y1: Y coordinate of bottom right point
	 * @param  color: Rectangle color
	 * @retval None
	 */
	void LCD_DISPLAY_drawFilledRectangle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color);

	/**
	 * @brief  Draws circle on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of center circle point
	 * @param  y0: Y coordinate of center circle point
	 * @param  r: Circle radius
	 * @param  color: Circle color
	 * @retval None
	 */
	void LCD_DISPLAY_drawCircle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 r, Uint16 color);

	/**
	 * @brief  Draws filled circle on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of center circle point
	 * @param  y0: Y coordinate of center circle point
	 * @param  r: Circle radius
	 * @param  color: Circle color
	 * @retval None
	 */
	void LCD_DISPLAY_drawFilledCircle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 r, Uint16 color);

	/**
	 * @brief  Put Image on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x: X coordinate of starting point
	 * @param  y: Y coordinate of starting point
	 * @param  width: width of image
	 * @param  height: height of image
	 * @param  img: Pointeur sur le tableau des pixels de l'image en BMP 565
	 * @param  size: Nombre d'élément dans le tableau
	 * @retval None
	 */
	void LCD_DISPLAY_putImage(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, const Uint16 *img, Uint32 size);

	/**
	 * @brief  Put Image with transparence on LCD
	 * @param  layer the layer of the screen concerned
	 * @param  x0: X coordinate of starting point
	 * @param  y0: Y coordinate of starting point
	 * @param  width: width of image
	 * @param  height: height of image
	 * @param  img: Pointeur sur le tableau des pixels de l'image en BMP 565
	 * @param  colorTransparence: color of transparence in BMP 565
	 * @param  size: Nombre d'élément dans le tableau
	 * @retval None
	 */
	void LCD_DISPLAY_putImageWithTransparence(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, const Uint16 *img, Uint16 colorTransparence, Uint32 size);

	/**
	 * @brief  Put color on non transparence of image
	 * @param  layer the layer of the screen concerned
	 * @param  x: X coordinate of starting point
	 * @param  y: Y coordinate of starting point
	 * @param  width: width of image
	 * @param  height: height of image
	 * @param  replaceColor: Couleur de remplacement
	 * @param  img: Pointeur sur le tableau des pixels de l'image en BMP 565
	 * @param  colorTransparence: color of transparence in BMP 565
	 * @param  size: Nombre d'élément dans le tableau
	 * @retval None
	 */
	void LCD_DISPLAY_putColorInvertedImage(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, Uint16 replaceColor, const Uint16 *img, Uint16 colorTransparence, Uint32 size);

	/**
	 * @brief   Enables display
	 * @note    After initialization, LCD is enabled and you don't need to call this function
	 * @param  layer the layer of the screen concerned
	 * @retval  None
	 */
	void LCD_DISPLAY_displayOn(LCD_DISPLAY_Layer_t layer);

	/**
	 * @brief   Disables display
	 * @param  layer the layer of the screen concerned
	 * @retval  None
	 */
	void LCD_DISPLAY_displayOff(LCD_DISPLAY_Layer_t layer);

#endif

