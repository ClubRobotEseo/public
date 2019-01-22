
#include <stdio.h>
#include <stdarg.h>
#include "lcd_display.h"

Uint16 frame0[F0_HEIGHT][F0_WIDTH]; // __attribute__((section(".ramd1")));
Uint16 frame1[F1_HEIGHT][F1_WIDTH]; // __attribute__((section(".ramd1")));
static LTDC_HandleTypeDef hltdc;
static LTDC_LayerCfgTypeDef pLayerCfg[LCD_DISPLAY_Layer_nb];
static volatile LCD_DISPLAY_point_t cursor[LCD_DISPLAY_Layer_nb];
static volatile LCD_DISPLAY_Options_t options[LCD_DISPLAY_Layer_nb];
static volatile bool_e initialised = FALSE;

/* Private functions */
//static void LCD_DISPLAY_delay(volatile Uint32 delay);
static void LCD_DISPLAY_INT_fill(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color);

void LCD_DISPLAY_init(){
	if(initialised)
		return;
	initialised = TRUE;

	/* Initialize default options */
	options[0] = (LCD_DISPLAY_Options_t){.width = F0_WIDTH, .height = F0_HEIGHT, .orientation = LCD_DISPLAY_Orientation_Landscape_1};
	options[1] = (LCD_DISPLAY_Options_t){.width = F1_WIDTH, .height = F1_HEIGHT, .orientation = LCD_DISPLAY_Orientation_Landscape_1};

	/* Initialized hltdc configuration */
	  hltdc.Instance = LTDC;
	  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IIPC;
	  hltdc.Init.HorizontalSync = 95;
	  hltdc.Init.VerticalSync = 1;
	  hltdc.Init.AccumulatedHBP = 143;
	  hltdc.Init.AccumulatedVBP = 33;
	  hltdc.Init.AccumulatedActiveW = 783;
	  hltdc.Init.AccumulatedActiveH = 513;
	  hltdc.Init.TotalWidth = 799;
	  hltdc.Init.TotalHeigh = 524;
	  hltdc.Init.Backcolor.Blue = 255;
	  hltdc.Init.Backcolor.Green = 255;
	  hltdc.Init.Backcolor.Red = 255;
	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	/* Initialized player configuration */
	pLayerCfg[0].WindowX0 = F0_OFFSET_WINDOW_X;
	pLayerCfg[0].WindowX1 = F0_OFFSET_WINDOW_X + F0_WIDTH - 1;
	pLayerCfg[0].WindowY0 = F0_OFFSET_WINDOW_Y;
	pLayerCfg[0].WindowY1 = F0_OFFSET_WINDOW_Y + F0_HEIGHT - 1;
	pLayerCfg[0].PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	pLayerCfg[0].Alpha = 255;
	pLayerCfg[0].Alpha0 = 0;
	pLayerCfg[0].BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg[0].BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg[0].FBStartAdress = (Uint32)frame0;
	pLayerCfg[0].ImageWidth = F0_WIDTH;
	pLayerCfg[0].ImageHeight = F0_HEIGHT;
	pLayerCfg[0].Backcolor.Blue = 255;
	pLayerCfg[0].Backcolor.Green = 255;
	pLayerCfg[0].Backcolor.Red = 255;
	if (HAL_LTDC_ConfigLayer(&hltdc, &(pLayerCfg[0]), 0) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	pLayerCfg[1].WindowX0 = F1_OFFSET_WINDOW_X;
	pLayerCfg[1].WindowX1 = F1_OFFSET_WINDOW_X + F1_WIDTH - 1;
	pLayerCfg[1].WindowY0 = F1_OFFSET_WINDOW_Y;
	pLayerCfg[1].WindowY1 = F1_OFFSET_WINDOW_Y + F1_HEIGHT - 1;
	pLayerCfg[1].PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	pLayerCfg[1].Alpha = 0;
	pLayerCfg[1].Alpha0 = 0;
	pLayerCfg[1].BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg[1].BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg[1].FBStartAdress = (Uint32)frame1;
	pLayerCfg[1].ImageWidth = F1_WIDTH;
	pLayerCfg[1].ImageHeight = F1_HEIGHT;
	pLayerCfg[1].Backcolor.Blue = 255;
	pLayerCfg[1].Backcolor.Green = 255;
	pLayerCfg[1].Backcolor.Red = 255;
	if (HAL_LTDC_ConfigLayer(&hltdc, &(pLayerCfg[1]), 1) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	/* Fill with white color */
	LCD_DISPLAY_fill(LCD_DISPLAY_Layer_0, LCD_DISPLAY_COLOR_BLUE);
	LCD_DISPLAY_fill(LCD_DISPLAY_Layer_1, LCD_DISPLAY_COLOR_BLUE);
}

void LCD_DISPLAY_setConfig(void){
	// Nothing to do here
}

void LCD_DISPLAY_displayOn(LCD_DISPLAY_Layer_t layer) {
	pLayerCfg[layer].Alpha = 255;
	if (HAL_LTDC_ConfigLayer(&hltdc, &(pLayerCfg[layer]), layer) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}
}

void LCD_DISPLAY_displayOff(LCD_DISPLAY_Layer_t layer) {
	pLayerCfg[layer].Alpha = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &(pLayerCfg[layer]), layer) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}
}

LCD_DISPLAY_Options_t LCD_DISPLAY_getOptions(LCD_DISPLAY_Layer_t layer){
	return options[layer];
}

void LCD_DISPLAY_drawPixel(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 color) {
	assert(x < options[layer].width && y < options[layer].height);

	Uint16 xp = 0, yp = 0;

	switch(options[layer].orientation) {
		case LCD_DISPLAY_Orientation_Portrait_1:
			xp = y;
			yp = (options[layer].height - 1) - x;
			break;
		case LCD_DISPLAY_Orientation_Portrait_2:
			xp = (options[layer].width - 1) - y;
			yp = x;
			break;
		case LCD_DISPLAY_Orientation_Landscape_1:	// default orientation
			xp = x;
			yp = y;
			break;
		case LCD_DISPLAY_Orientation_Landscape_2:
			xp = (options[layer].width - 1) - x;
			yp = (options[layer].height - 1) - y;
			break;
	}

	if(xp >= 0 && xp < options[layer].width && yp >= 0 && yp < options[layer].height) {
		switch(layer) {
			case LCD_DISPLAY_Layer_0:
				frame0[yp][xp] = color;
				break;
			case LCD_DISPLAY_Layer_1:
				frame1[yp][xp] = color;
				break;
			default:
				break;
		}
	}
}

void LCD_DISPLAY_fill(LCD_DISPLAY_Layer_t layer, Uint16 color) {
	/* Fill entire screen */
	LCD_DISPLAY_INT_fill(layer, 0, 0, options[layer].width - 1, options[layer].height, color);
}

void LCD_DISPLAY_rotate(LCD_DISPLAY_Layer_t layer, LCD_DISPLAY_Orientation_t orientation) {
	options[layer].orientation = orientation;

//	if (orientation == LCD_DISPLAY_Orientation_Portrait_1 || orientation == LCD_DISPLAY_Orientation_Portrait_2) {
//		options[layer].width = pLayerCfg[layer].ImageHeight;
//		options[layer].height = pLayerCfg[layer].ImageWidth;
//	} else {
//		options[layer].width = pLayerCfg[layer].ImageWidth;
//		options[layer].height = pLayerCfg[layer].ImageHeight;
//	}
}

void LCD_DISPLAY_puts(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, char *str, FontDef_t *font, Uint16 foreground, Uint32 background) {
	uint16_t startX = x;

	/* Set X and Y coordinates */
	cursor[layer].x = x;
	cursor[layer].y = y;

	while (*str) {
		/* New line */
		if (*str == '\n') {
			cursor[layer].y += font->FontHeight + 1;
			/* if after \n is also \r, than go to the left of the screen */
			if (*(str + 1) == '\r') {
				cursor[layer].x = 0;
				str++;
			} else {
				cursor[layer].x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}

		/* Put character to LCD */
		LCD_DISPLAY_putc(layer, cursor[layer].x, cursor[layer].y, *str++, font, foreground, background);
	}
}



void LCD_DISPLAY_getStringSize(char *str, FontDef_t *font, Uint16 *width, Uint16 *height) {
	uint16_t w = 0;
	*height = font->FontHeight;
	while (*str++) {
		w += font->FontWidth;
	}
	*width = w;
}

void LCD_DISPLAY_putc(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, char c, FontDef_t *font, Uint16 foreground, Uint32 background) {
	uint32_t i, b, j;
	/* Set coordinates */
	cursor[layer].x = x;
	cursor[layer].y = y;

	if ((cursor[layer].x + font->FontWidth) > options[layer].width) {
		/* If at the end of a line of display, go to new line and set x to 0 position */
		cursor[layer].y += font->FontHeight;
		cursor[layer].x = 0;
	}

	/* Draw rectangle for background */
	if(background != LCD_DISPLAY_TRANSPARENT)
		LCD_DISPLAY_INT_fill(layer, cursor[layer].x, cursor[layer].y, cursor[layer].x + font->FontWidth, cursor[layer].y + font->FontHeight, background);  // TODO

	/* Draw font data */
	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];
		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				LCD_DISPLAY_drawPixel(layer, cursor[layer].x + j, (cursor[layer].y + i), foreground);
			}
		}
	}

	/* Set new pointer */
	cursor[layer].x += font->FontWidth;
}

void LCD_DISPLAY_drawLine(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color) {
	/* Code by dewoller: https://github.com/dewoller */

	int16_t dx, dy, sx, sy, err, e2;
	uint16_t tmp;

	/* Check for overflow */
	if (x0 >= options[layer].width) {
		x0 = options[layer].width - 1;
	}
	if (x1 >= options[layer].width) {
		x1 = options[layer].width - 1;
	}
	if (y0 >= options[layer].height) {
		y0 = options[layer].height - 1;
	}
	if (y1 >= options[layer].height) {
		y1 = options[layer].height - 1;
	}

	/* Check correction */
	if (x0 > x1) {
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	dx = x1 - x0;
	dy = y1 - y0;

	/* Vertical or horizontal line */
	if (dx == 0 || dy == 0) {
		LCD_DISPLAY_INT_fill(layer, x0, y0, x1, y1, color);
		return;
	}

	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	while (1) {
		LCD_DISPLAY_drawPixel(layer, x0, y0, color);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void LCD_DISPLAY_drawRectangle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color) {
	LCD_DISPLAY_drawLine(layer, x0, y0, x1, y0, color); 	//Top
	LCD_DISPLAY_drawLine(layer, x0, y0, x0, y1, color);	//Left
	LCD_DISPLAY_drawLine(layer, x1, y0, x1, y1, color);	//Right
	LCD_DISPLAY_drawLine(layer, x0, y1, x1, y1, color);	//Bottom
}

void LCD_DISPLAY_drawFilledRectangle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color) {
	uint16_t tmp;

	/* Check correction */
	if (x0 > x1) {
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) {
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	/* Fill rectangle */
	LCD_DISPLAY_INT_fill(layer, x0, y0, x1, y1, color);
}

void LCD_DISPLAY_drawCircle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 r, Uint16 color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DISPLAY_drawPixel(layer, x0, y0 + r, color);
	LCD_DISPLAY_drawPixel(layer, x0, y0 - r, color);
	LCD_DISPLAY_drawPixel(layer, x0 + r, y0, color);
	LCD_DISPLAY_drawPixel(layer, x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DISPLAY_drawPixel(layer, x0 + x, y0 + y, color);
		LCD_DISPLAY_drawPixel(layer, x0 - x, y0 + y, color);
		LCD_DISPLAY_drawPixel(layer, x0 + x, y0 - y, color);
		LCD_DISPLAY_drawPixel(layer, x0 - x, y0 - y, color);

		LCD_DISPLAY_drawPixel(layer, x0 + y, y0 + x, color);
		LCD_DISPLAY_drawPixel(layer, x0 - y, y0 + x, color);
		LCD_DISPLAY_drawPixel(layer, x0 + y, y0 - x, color);
		LCD_DISPLAY_drawPixel(layer, x0 - y, y0 - x, color);
	}
}

void LCD_DISPLAY_drawFilledCircle(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 r, Uint16 color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DISPLAY_drawPixel(layer, x0, y0 + r, color);
	LCD_DISPLAY_drawPixel(layer, x0, y0 - r, color);
	LCD_DISPLAY_drawPixel(layer, x0 + r, y0, color);
	LCD_DISPLAY_drawPixel(layer, x0 - r, y0, color);
	LCD_DISPLAY_drawLine(layer, x0 - r, y0, x0 + r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DISPLAY_drawLine(layer, x0 - x, y0 + y, x0 + x, y0 + y, color);
		LCD_DISPLAY_drawLine(layer, x0 + x, y0 - y, x0 - x, y0 - y, color);

		LCD_DISPLAY_drawLine(layer, x0 + y, y0 + x, x0 - y, y0 + x, color);
		LCD_DISPLAY_drawLine(layer, x0 + y, y0 - x, x0 - y, y0 - x, color);
	}
}


void LCD_DISPLAY_putImage(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, const Uint16 *img, Uint32 size){
	Uint32 x0, y0;
	for(y0=0; y0 < height; y0++){
		for(x0=0; x0 < width; x0++){
			Uint32 pixel = img[y0 * width + x0];
			LCD_DISPLAY_drawPixel(layer, x + x0, y + y0, pixel);
		}
	}
}

void LCD_DISPLAY_putImageWithTransparence(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, const Uint16 *img, Uint16 colorTransparence, Uint32 size){
	Uint32 x0, y0;
	for(y0=0; y0 < height; y0++){
		for(x0=0; x0 < width; x0++){
			Uint32 pixel = img[y0 * width + x0];
			if(pixel != colorTransparence)
				LCD_DISPLAY_drawPixel(layer, x + x0, y + y0, pixel);
		}
	}
}

void LCD_DISPLAY_putColorInvertedImage(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, Uint16 width, Uint16 height, Uint16 replaceColor, const Uint16 *img, Uint16 colorTransparence, Uint32 size){
	Uint32 i;
	for(i=0; i < size; i++){
		if(img[i] != colorTransparence)
			LCD_DISPLAY_drawPixel(layer, x + i % width, y + i / width, replaceColor);
	}
}

/***************************************************************
 *                       Fonctions privées
 ***************************************************************/

void LCD_DISPLAY_printf(LCD_DISPLAY_Layer_t layer, Uint16 x, Uint16 y, FontDef_t *font, Uint16 foreground, Uint32 background, const char *format, ...){
	char buffer[256];

	va_list args_list;
	va_start(args_list, format);
	vsnprintf(buffer, 256, format, args_list);
	va_end(args_list);

	LCD_DISPLAY_puts(layer, x, y, buffer, font, foreground, background);
}

static void LCD_DISPLAY_INT_fill(LCD_DISPLAY_Layer_t layer, Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1, Uint16 color) {
	Uint32 x, y;
	for(y=y0; y < y1; y++){
		for(x=x0; x < x1; x++){
			LCD_DISPLAY_drawPixel(layer, x, y, color);
		}
	}
}

//static void LCD_DISPLAY_delay(volatile Uint32 delay) {
//
//	time32_t beginTime = global.absolute_time;
//
//	while(global.absolute_time <= beginTime + delay);
//}


