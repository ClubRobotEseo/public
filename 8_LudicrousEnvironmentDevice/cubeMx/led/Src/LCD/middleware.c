#include <stdarg.h>
#include "middleware.h"
#include "low layer/lcd_display.h"

typedef enum{
	BUTTON_STATE_ON,
	BUTTON_STATE_OFF
}buttonState_e;

typedef enum{
	BUTTON_STATE_TOUCH,
	BUTTON_STATE_NO_TOUCH
}buttonStateTouch_e;

typedef enum{
	TEXT,
	BUTTON_IMG,
	BUTTON_BASE,
	PROGRESS_BAR,
	SLIDER,
	IMAGE,
	ANIMATED_IMAGE,
	MULTI_TEXT,
	RECTANGLE,
	CIRCLE,
	LINE
}objectType_e;

typedef struct{
	bool_e use;
	objectType_e type;
	bool_e toDisplay;
	bool_e toErase;
	bool_e toDestroy;

	struct{
		Sint16 x;
		Sint16 y;
		Uint16 width;
		Uint16 height;
	}surfaceToErase;

	union{

		struct{
			Sint16 x;
			Sint16 y;
			Uint16 width;
			Uint16 height;
			char text[OBJECT_TEXT_MAX_SIZE];
			textFonts_e fonts;
			objectColor_e colorText;
			objectColor_e colorBackground;
		}text;

		struct{
			buttonState_e state;
			buttonState_e lastState;
			buttonStateTouch_e lastStateTouch;
			Sint16 x;
			Sint16 y;
			Uint16 widthButton;
			Uint16 heightButton;
			Uint16 widthText;
			Uint16 heightText;
			char text[OBJECT_TEXT_MAX_SIZE];
			textFonts_e fonts;
			Uint16 colorText;
			objectColor_e colorButton;
			objectColor_e colorTouch;
			objectColor_e colorBorder;
			bool_e lockTouch;
			bool_e *touch;
		}buttonBase;

		struct{
			buttonState_e state;
			buttonState_e lastState;
			buttonStateTouch_e lastStateTouch;
			Sint16 x;
			Sint16 y;
			const imageInfo_s *imageNormal;
			const imageInfo_s *imageLock;
			bool_e lockTouch;
			bool_e *touch;
		}buttonImg;

		struct{
			Sint16 x;
			Sint16 y;
			Uint16 width;
			Uint16 height;
			objectOrientation_e orientation;
			bool_e refreshBack;
			Uint8 *value;
			Uint8 lastValue;
		}progressBar;

		struct{
			Sint16 x;
			Sint16 y;
			Uint16 width;
			Uint16 height;
			Sint32 minValue;
			Sint32 maxValue;
			Sint32 *value;
			objectOrientation_e orientation;
			Sint32 realValue;
			Sint32 lastRealValue;
		}slider;

		struct{
			Sint16 x;
			Sint16 y;
			const imageInfo_s *image;
		}image;

		struct{
			Sint16 x;
			Sint16 y;
			Uint8 actualFrame;
			Uint8 lastFrame;
			time32_t timeToRefresh;
			const animatedImageInfo_s *animatedImageInfo;
		}animatedImage;

		struct{
			Sint16 x;
			Sint16 y;
			Uint16 width;
			Uint16 height;
			objectColor_e colorBorder;
			objectColor_e colorCenter;
		}rectangle;

		struct{
			Sint16 x;
			Sint16 y;
			Uint16 r;
			objectColor_e colorBorder;
			objectColor_e colorCenter;
		}circle;

		struct{
			Sint16 x0;
			Sint16 y0;
			Sint16 x1;
			Sint16 y1;
			objectColor_e color;
		}line;

	}objectData;
}object_s;

typedef struct{
	bool_e toDisplay;
	Uint16 color;
}background_s;

static volatile object_s objectTab[LCD_DISPLAY_Layer_nb][LCD_NB_MAX_OBJECT] = {0};
static volatile background_s background[LCD_DISPLAY_Layer_nb];

#ifdef TACTILE_ACTIVATED
	static bool_e MIDDLEWARE_objectTouch(Uint16 xT, Uint16 yT, Uint16 x, Uint16 y, Uint16 width, Uint16 height);
	static void MIDDLEWARE_checkObjectTouch(bool_e touch, Sint16 x, Sint16 y);
#endif
static void MIDDLEWARE_checkRebuildObject();
static void MIDDLEWARE_rebuildObject();
static void MIDDLEWARE_checkEraseObject();
static void MIDDLEWARE_checkDestroyObject();
static objectId_t MIDDLEWARE_newObject(LCD_DISPLAY_Layer_t layer);
static FontDef_t* MIDDLEWARE_getFont(textFonts_e fonts);

void MIDDLEWARE_init(){
	LCD_DISPLAY_init();

	MIDDLEWARE_resetScreen();
}

void MIDDLEWARE_processMain(){

//	// Check des zones touchés
#ifdef TACTILE_ACTIVATED
//	MIDDLEWARE_checkObjectTouch(touch, x, y);
#endif

	LCD_DISPLAY_setConfig();

	// Check effacement d'objets
	MIDDLEWARE_checkEraseObject();

	// Check destruction d'objets
	MIDDLEWARE_checkDestroyObject();

	// Check des objets à réafficher
	MIDDLEWARE_checkRebuildObject();

	// Refresh background
	for(Uint8 layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		if(background[layer].toDisplay){
			LCD_DISPLAY_fill(layer, background[layer].color);
			background[layer].toDisplay = FALSE;
		}
	}

	// Mise à jours graphique des objets
	MIDDLEWARE_rebuildObject();
}

//////////////////////////////////////////////////////////////////
//-------------------------Fonction Main------------------------//
//////////////////////////////////////////////////////////////////
#ifdef TACTILE_ACTIVATED
static void MIDDLEWARE_checkObjectTouch(bool_e touch, Sint16 x, Sint16 y){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			if(objectTab[layer][i].use){
				switch(objectTab[layer][i].type){

					case BUTTON_BASE:
						if(objectTab[layer][i].objectData.buttonBase.lockTouch){
							if(touch && MIDDLEWARE_objectTouch(x, y, objectTab[layer][i].objectData.buttonBase.x, objectTab[layer][i].objectData.buttonBase.y, objectTab[layer][i].objectData.buttonBase.widthButton, objectTab[layer][i].objectData.buttonBase.heightButton)){

								if(objectTab[layer][i].objectData.buttonBase.lastStateTouch != BUTTON_STATE_TOUCH){

									if(objectTab[layer][i].objectData.buttonBase.state == BUTTON_STATE_ON){
										objectTab[layer][i].objectData.buttonBase.state = BUTTON_STATE_OFF;
										*(objectTab[layer][i].objectData.buttonBase.touch) = FALSE;
									}else{
										objectTab[layer][i].objectData.buttonBase.state = BUTTON_STATE_ON;
										*(objectTab[layer][i].objectData.buttonBase.touch) = TRUE;
									}
									objectTab[layer][i].objectData.buttonBase.lastStateTouch = BUTTON_STATE_TOUCH;
								}
							}else{
								objectTab[layer][i].objectData.buttonBase.lastStateTouch = BUTTON_STATE_NO_TOUCH;
							}
						}else{
							if(touch && MIDDLEWARE_objectTouch(x, y, objectTab[layer][i].objectData.buttonBase.x, objectTab[layer][i].objectData.buttonBase.y, objectTab[layer][i].objectData.buttonBase.widthButton, objectTab[layer][i].objectData.buttonBase.heightButton)){

								objectTab[layer][i].objectData.buttonBase.state = BUTTON_STATE_ON;
								*(objectTab[layer][i].objectData.buttonBase.touch) = TRUE;

							}else{

								objectTab[layer][i].objectData.buttonBase.state = BUTTON_STATE_OFF;
								*(objectTab[layer][i].objectData.buttonBase.touch) = FALSE;

							}
						}
						break;

					case BUTTON_IMG:
						if(objectTab[layer][i].objectData.buttonImg.lockTouch){
							if(touch && MIDDLEWARE_objectTouch(x, y, objectTab[layer][i].objectData.buttonImg.x, objectTab[layer][i].objectData.buttonImg.y, objectTab[layer][i].objectData.buttonImg.imageNormal->width, objectTab[layer][i].objectData.buttonImg.imageNormal->height)){

								if(objectTab[layer][i].objectData.buttonImg.lastStateTouch != BUTTON_STATE_TOUCH){

									if(objectTab[layer][i].objectData.buttonImg.state == BUTTON_STATE_ON){
										objectTab[layer][i].objectData.buttonImg.state = BUTTON_STATE_OFF;
										*(objectTab[layer][i].objectData.buttonImg.touch) = FALSE;
									}else{
										objectTab[layer][i].objectData.buttonImg.state = BUTTON_STATE_ON;
										*(objectTab[layer][i].objectData.buttonImg.touch) = TRUE;
									}
									objectTab[layer][i].objectData.buttonImg.lastStateTouch = BUTTON_STATE_TOUCH;
								}
							}else{
								objectTab[layer][i].objectData.buttonImg.lastStateTouch = BUTTON_STATE_NO_TOUCH;
							}
						}else{
							if(touch && MIDDLEWARE_objectTouch(x, y, objectTab[layer][i].objectData.buttonImg.x, objectTab[layer][i].objectData.buttonImg.y, objectTab[layer][i].objectData.buttonImg.imageNormal->width, objectTab[layer][i].objectData.buttonImg.imageNormal->height)){

								objectTab[layer][i].objectData.buttonImg.state = BUTTON_STATE_ON;
								*(objectTab[layer][i].objectData.buttonImg.touch) = TRUE;

							}else{

								objectTab[layer][i].objectData.buttonImg.state = BUTTON_STATE_OFF;
								*(objectTab[layer][i].objectData.buttonImg.touch) = FALSE;

							}
						}
						break;

					case SLIDER:
						if(touch && MIDDLEWARE_objectTouch(x, y, objectTab[layer][i].objectData.slider.x, objectTab[layer][i].objectData.slider.y, objectTab[layer][i].objectData.slider.width, objectTab[layer][i].objectData.slider.height)){

							if(objectTab[layer][i].objectData.slider.orientation == OBJECT_HORIZONTAL_L_2_R || objectTab[layer][i].objectData.slider.orientation == OBJECT_HORIZONTAL_R_2_L)
								objectTab[layer][i].objectData.slider.realValue = (x - objectTab[layer][i].objectData.slider.x) * (objectTab[layer][i].objectData.slider.maxValue - objectTab[layer][i].objectData.slider.minValue) / (objectTab[layer][i].objectData.slider.width);
							else
								objectTab[layer][i].objectData.slider.realValue = (y - objectTab[layer][i].objectData.slider.y) * (objectTab[layer][i].objectData.slider.maxValue - objectTab[layer][i].objectData.slider.minValue) / (objectTab[layer][i].objectData.slider.height);

							if(objectTab[layer][i].objectData.slider.orientation == OBJECT_HORIZONTAL_R_2_L || objectTab[layer][i].objectData.slider.orientation == OBJECT_VERTICAL_B_2_T)
								*(objectTab[layer][i].objectData.slider.value) = objectTab[layer][i].objectData.slider.maxValue - objectTab[layer][i].objectData.slider.realValue;
							else
								*(objectTab[layer][i].objectData.slider.value) = objectTab[layer][i].objectData.slider.realValue;
						}
						break;

					default:
						break;
				}
			}
		}
	}
}
#endif

static void MIDDLEWARE_checkEraseObject(){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			if(objectTab[layer][i].use && objectTab[layer][i].toErase){
				switch(objectTab[layer][i].type){

					case TEXT:
					case BUTTON_BASE:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].surfaceToErase.x,
													objectTab[layer][i].surfaceToErase.y,
													objectTab[layer][i].surfaceToErase.x + objectTab[layer][i].surfaceToErase.width,
													objectTab[layer][i].surfaceToErase.y + objectTab[layer][i].surfaceToErase.height,
													background[layer].color);
						break;

					default:
						break;
				}
				objectTab[layer][i].toErase = FALSE;
			}
		}
	}
}

static void MIDDLEWARE_checkDestroyObject(){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			if(objectTab[layer][i].use && objectTab[layer][i].toDestroy){
				switch(objectTab[layer][i].type){

					case TEXT:{
						Uint16 width, height;
						LCD_DISPLAY_getStringSize((char *)objectTab[layer][i].objectData.buttonBase.text, &Font_7x10, &width, &height);
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.buttonBase.x,
													objectTab[layer][i].objectData.buttonBase.y,
													objectTab[layer][i].objectData.buttonBase.x + width,
													objectTab[layer][i].objectData.buttonBase.y + height,
													background[layer].color);
						}break;

					case BUTTON_BASE:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.buttonBase.x,
													objectTab[layer][i].objectData.buttonBase.y,
													objectTab[layer][i].objectData.buttonBase.x + objectTab[layer][i].objectData.buttonBase.widthButton,
													objectTab[layer][i].objectData.buttonBase.y + objectTab[layer][i].objectData.buttonBase.heightButton,
													background[layer].color);
						break;

					case BUTTON_IMG:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.buttonImg.x,
													objectTab[layer][i].objectData.buttonImg.y,
													objectTab[layer][i].objectData.buttonImg.x + objectTab[layer][i].objectData.buttonImg.imageNormal->width,
													objectTab[layer][i].objectData.buttonImg.y + objectTab[layer][i].objectData.buttonImg.imageNormal->height,
													background[layer].color);
						break;

					case PROGRESS_BAR:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x,
													objectTab[layer][i].objectData.progressBar.y,
													objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width,
													objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height,
													background[layer].color);
						break;

					case SLIDER:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.slider.x,
													objectTab[layer][i].objectData.slider.y,
													objectTab[layer][i].objectData.slider.x + objectTab[layer][i].objectData.slider.width,
													objectTab[layer][i].objectData.slider.y + objectTab[layer][i].objectData.slider.height,
													background[layer].color);
						break;

					case IMAGE:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.image.x,
													objectTab[layer][i].objectData.image.y,
													objectTab[layer][i].objectData.image.x + objectTab[layer][i].objectData.image.image->width,
													objectTab[layer][i].objectData.image.y + objectTab[layer][i].objectData.image.image->height,
													background[layer].color);
						break;

					case ANIMATED_IMAGE:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.animatedImage.x,
													objectTab[layer][i].objectData.animatedImage.y,
													objectTab[layer][i].objectData.animatedImage.x + objectTab[layer][i].objectData.animatedImage.animatedImageInfo->frame[objectTab[layer][i].objectData.animatedImage.actualFrame].width,
													objectTab[layer][i].objectData.animatedImage.y + objectTab[layer][i].objectData.animatedImage.animatedImageInfo->frame[objectTab[layer][i].objectData.animatedImage.actualFrame].height,
													background[layer].color);
						break;

					case RECTANGLE:
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.rectangle.x,
																		objectTab[layer][i].objectData.rectangle.y,
																		objectTab[layer][i].objectData.rectangle.x + objectTab[layer][i].objectData.rectangle.width,
																		objectTab[layer][i].objectData.rectangle.y + objectTab[layer][i].objectData.rectangle.height,
																		background[layer].color);
						break;

					case CIRCLE:
						LCD_DISPLAY_drawCircle(layer, objectTab[layer][i].objectData.circle.x,
											objectTab[layer][i].objectData.circle.y,
											objectTab[layer][i].objectData.circle.r,
											background[layer].color);
						break;

					case LINE:
						LCD_DISPLAY_drawLine(layer, objectTab[layer][i].objectData.line.x0,
										objectTab[layer][i].objectData.line.y0,
										objectTab[layer][i].objectData.line.x1,
										objectTab[layer][i].objectData.line.y1,
										background[layer].color);
						break;

					default:
						break;
				}
				objectTab[layer][i].use = FALSE;
			}
		}
	}
}

static void MIDDLEWARE_checkRebuildObject(){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			if(objectTab[layer][i].use){
				if(background[layer].toDisplay){
					switch(objectTab[layer][i].type){
						case PROGRESS_BAR:
							objectTab[layer][i].objectData.progressBar.refreshBack = TRUE;
							break;

						default:
							break;
					}
					objectTab[layer][i].toDisplay = TRUE;
				}else{
					switch(objectTab[layer][i].type){

						case BUTTON_BASE:
							if(objectTab[layer][i].objectData.buttonBase.lastState != objectTab[layer][i].objectData.buttonBase.state){
								objectTab[layer][i].toDisplay = TRUE;
							}
							objectTab[layer][i].objectData.buttonBase.lastState = objectTab[layer][i].objectData.buttonBase.state;
							break;

						case BUTTON_IMG:
							if(objectTab[layer][i].objectData.buttonImg.lastState != objectTab[layer][i].objectData.buttonImg.state){
								objectTab[layer][i].toDisplay = TRUE;
							}
							objectTab[layer][i].objectData.buttonImg.lastState = objectTab[layer][i].objectData.buttonImg.state;
							break;

						case PROGRESS_BAR:
							if(objectTab[layer][i].objectData.progressBar.lastValue != *(objectTab[layer][i].objectData.progressBar.value)){
								if(objectTab[layer][i].objectData.progressBar.lastValue > *(objectTab[layer][i].objectData.progressBar.value))
										objectTab[layer][i].objectData.progressBar.refreshBack = TRUE;
								objectTab[layer][i].objectData.progressBar.lastValue = *(objectTab[layer][i].objectData.progressBar.value);
								objectTab[layer][i].toDisplay = TRUE;
							}
							break;

						case SLIDER:
							if(objectTab[layer][i].objectData.slider.realValue != objectTab[layer][i].objectData.slider.lastRealValue){
								objectTab[layer][i].objectData.slider.lastRealValue = objectTab[layer][i].objectData.slider.realValue;
								objectTab[layer][i].toDisplay = TRUE;
							}
							break;

						case ANIMATED_IMAGE:
							if(global.absolute_time >= objectTab[layer][i].objectData.animatedImage.timeToRefresh)
								objectTab[layer][i].toDisplay = TRUE;
							break;

						default:
							break;
					}
				}
			}
		}
	}
}

static void MIDDLEWARE_rebuildObject(){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			if(objectTab[layer][i].use && objectTab[layer][i].toDisplay){
				switch(objectTab[layer][i].type){
					case TEXT:{
						LCD_DISPLAY_printf(layer, objectTab[layer][i].objectData.text.x,
										objectTab[layer][i].objectData.text.y,
										MIDDLEWARE_getFont(objectTab[layer][i].objectData.text.fonts),
										objectTab[layer][i].objectData.text.colorText,
										((objectTab[layer][i].objectData.text.colorBackground == LCD_DISPLAY_TRANSPARENT)? background[layer].color : objectTab[layer][i].objectData.text.colorBackground),
										"%s", objectTab[layer][i].objectData.text.text);
					}break;

					case BUTTON_BASE:{

						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.buttonBase.x,
													objectTab[layer][i].objectData.buttonBase.y,
													objectTab[layer][i].objectData.buttonBase.x + objectTab[layer][i].objectData.buttonBase.widthButton,
													objectTab[layer][i].objectData.buttonBase.y + objectTab[layer][i].objectData.buttonBase.heightButton,
													(objectTab[layer][i].objectData.buttonBase.state == BUTTON_STATE_ON)? objectTab[layer][i].objectData.buttonBase.colorTouch : objectTab[layer][i].objectData.buttonBase.colorButton
													);

						LCD_DISPLAY_printf(layer, objectTab[layer][i].objectData.buttonBase.x + objectTab[layer][i].objectData.buttonBase.widthButton/2 - objectTab[layer][i].objectData.buttonBase.widthText/2,
										objectTab[layer][i].objectData.buttonBase.y + objectTab[layer][i].objectData.buttonBase.heightButton/2 - objectTab[layer][i].objectData.buttonBase.heightText/2,
										MIDDLEWARE_getFont(objectTab[layer][i].objectData.buttonBase.fonts),
										objectTab[layer][i].objectData.buttonBase.colorText,
										LCD_DISPLAY_TRANSPARENT,
										"%s", objectTab[layer][i].objectData.buttonBase.text);

						if(objectTab[layer][i].objectData.buttonBase.colorBorder != LCD_DISPLAY_TRANSPARENT){
							LCD_DISPLAY_drawRectangle(layer, objectTab[layer][i].objectData.buttonBase.x,
														objectTab[layer][i].objectData.buttonBase.y,
														objectTab[layer][i].objectData.buttonBase.x + objectTab[layer][i].objectData.buttonBase.widthButton,
														objectTab[layer][i].objectData.buttonBase.y + objectTab[layer][i].objectData.buttonBase.heightButton,
														objectTab[layer][i].objectData.buttonBase.colorBorder
														);
						}

						}break;

					case BUTTON_IMG:{
						const imageInfo_s *imageInfo = (objectTab[layer][i].objectData.buttonImg.state == BUTTON_STATE_ON)? objectTab[layer][i].objectData.buttonImg.imageLock : objectTab[layer][i].objectData.buttonImg.imageNormal;

						if(objectTab[layer][i].objectData.buttonImg.imageNormal->transparence){
							LCD_DISPLAY_putImageWithTransparence(layer, objectTab[layer][i].objectData.buttonImg.x,
															objectTab[layer][i].objectData.buttonImg.y,
															imageInfo->width,
															imageInfo->height,
															imageInfo->image,
															imageInfo->colorTransparence,
															imageInfo->size);
						}else{
							LCD_DISPLAY_putImage(layer, objectTab[layer][i].objectData.buttonImg.x,
															objectTab[layer][i].objectData.buttonImg.y,
															imageInfo->width,
															imageInfo->height,
															imageInfo->image,
															imageInfo->size);
						}

						}break;

					case PROGRESS_BAR:{
						if(objectTab[layer][i].objectData.progressBar.refreshBack){
							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x,
														objectTab[layer][i].objectData.progressBar.y,
														objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width,
														objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height,
														LCD_DISPLAY_COLOR_GRAY);
							objectTab[layer][i].objectData.progressBar.refreshBack = FALSE;
						}

						Uint8 lastValue = (objectTab[layer][i].objectData.progressBar.lastValue <= 100) ? objectTab[layer][i].objectData.progressBar.lastValue: 100;

						if(objectTab[layer][i].objectData.progressBar.orientation == OBJECT_HORIZONTAL_L_2_R){

							Uint16 width = lastValue * (objectTab[layer][i].objectData.progressBar.width - 4) / 100;

							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x + 2,
														objectTab[layer][i].objectData.progressBar.y + 2,
														objectTab[layer][i].objectData.progressBar.x + 2 + width,
														objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height - 2,
														LCD_DISPLAY_COLOR_GREEN);

						}else if(objectTab[layer][i].objectData.progressBar.orientation == OBJECT_HORIZONTAL_R_2_L){

							Uint16 width = lastValue * (objectTab[layer][i].objectData.progressBar.width - 4) / 100;

							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width - 2 - width,
														objectTab[layer][i].objectData.progressBar.y + 2,
														objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width - 2,
														objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height - 2,
														LCD_DISPLAY_COLOR_GREEN);

						}else if(objectTab[layer][i].objectData.progressBar.orientation == OBJECT_VERTICAL_T_2_B){

							Uint16 height = lastValue * (objectTab[layer][i].objectData.progressBar.height - 4) / 100;

							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x + 2,
														objectTab[layer][i].objectData.progressBar.y + 2,
														objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width - 2,
														objectTab[layer][i].objectData.progressBar.y + 2 + height,
														LCD_DISPLAY_COLOR_GREEN);
						}else if(objectTab[layer][i].objectData.progressBar.orientation == OBJECT_VERTICAL_B_2_T){

							Uint16 height = lastValue * (objectTab[layer][i].objectData.progressBar.height - 4) / 100;

							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.progressBar.x + 2,
														objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height - 2 - height,
														objectTab[layer][i].objectData.progressBar.x + objectTab[layer][i].objectData.progressBar.width - 2,
														objectTab[layer][i].objectData.progressBar.y + objectTab[layer][i].objectData.progressBar.height - 2,
														LCD_DISPLAY_COLOR_GREEN);
						}
						}break;

					case SLIDER:{

						Uint16 positionX = 0, positionY = 0;
						LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.slider.x,
													objectTab[layer][i].objectData.slider.y,
													objectTab[layer][i].objectData.slider.x + objectTab[layer][i].objectData.slider.width,
													objectTab[layer][i].objectData.slider.y + objectTab[layer][i].objectData.slider.height,
													LCD_DISPLAY_COLOR_GRAY);

						if(objectTab[layer][i].objectData.slider.orientation == OBJECT_HORIZONTAL_L_2_R || objectTab[layer][i].objectData.slider.orientation == OBJECT_HORIZONTAL_R_2_L){

							positionX = objectTab[layer][i].objectData.slider.x + (objectTab[layer][i].objectData.slider.realValue * objectTab[layer][i].objectData.slider.width) / (objectTab[layer][i].objectData.slider.maxValue - objectTab[layer][i].objectData.slider.minValue);
							positionY = objectTab[layer][i].objectData.slider.y + objectTab[layer][i].objectData.slider.height / 2;

						}else if(objectTab[layer][i].objectData.progressBar.orientation == OBJECT_VERTICAL_T_2_B || objectTab[layer][i].objectData.slider.orientation == OBJECT_VERTICAL_B_2_T){

							positionX = objectTab[layer][i].objectData.slider.x + objectTab[layer][i].objectData.slider.width / 2;
							positionY = objectTab[layer][i].objectData.slider.y + (objectTab[layer][i].objectData.slider.realValue * objectTab[layer][i].objectData.slider.height) / (objectTab[layer][i].objectData.slider.maxValue - objectTab[layer][i].objectData.slider.minValue);
						}

						LCD_DISPLAY_drawFilledCircle(layer, positionX-2, positionY, 5, LCD_DISPLAY_COLOR_RED);
						}break;

					case IMAGE:
						if(objectTab[layer][i].objectData.image.image->transparence){
							LCD_DISPLAY_putImageWithTransparence(layer, objectTab[layer][i].objectData.image.x,
															objectTab[layer][i].objectData.image.y,
															objectTab[layer][i].objectData.image.image->width,
															objectTab[layer][i].objectData.image.image->height,
															objectTab[layer][i].objectData.image.image->image,
															objectTab[layer][i].objectData.image.image->colorTransparence,
															objectTab[layer][i].objectData.image.image->size);
						}else{
							LCD_DISPLAY_putImage(layer, objectTab[layer][i].objectData.image.x,
															objectTab[layer][i].objectData.image.y,
															objectTab[layer][i].objectData.image.image->width,
															objectTab[layer][i].objectData.image.image->height,
															objectTab[layer][i].objectData.image.image->image,
															objectTab[layer][i].objectData.image.image->size);
						}

						break;

					case ANIMATED_IMAGE:{
						const animatedImageInfo_s *animatedImageInfo = objectTab[layer][i].objectData.animatedImage.animatedImageInfo;
						Uint8 actualFrame = objectTab[layer][i].objectData.animatedImage.actualFrame;
						Uint8 lastFrame = objectTab[layer][i].objectData.animatedImage.lastFrame;

						if(animatedImageInfo->frame[actualFrame].transparence){

							if(lastFrame != 255){
								LCD_DISPLAY_putColorInvertedImage(layer, objectTab[layer][i].objectData.animatedImage.x,
																objectTab[layer][i].objectData.animatedImage.y,
																animatedImageInfo->frame[lastFrame].width,
																animatedImageInfo->frame[lastFrame].height,
																background[layer].color,
																animatedImageInfo->frame[lastFrame].image,
																animatedImageInfo->frame[lastFrame].colorTransparence,
																animatedImageInfo->frame[lastFrame].size);
							}

							LCD_DISPLAY_putImageWithTransparence(layer, objectTab[layer][i].objectData.animatedImage.x,
															objectTab[layer][i].objectData.animatedImage.y,
															animatedImageInfo->frame[actualFrame].width,
															animatedImageInfo->frame[actualFrame].height,
															animatedImageInfo->frame[actualFrame].image,
															animatedImageInfo->frame[actualFrame].colorTransparence,
															animatedImageInfo->frame[actualFrame].size);
						}else{
							LCD_DISPLAY_putImage(layer, objectTab[layer][i].objectData.animatedImage.x,
															objectTab[layer][i].objectData.animatedImage.y,
															animatedImageInfo->frame[actualFrame].width,
															animatedImageInfo->frame[actualFrame].height,
															animatedImageInfo->frame[actualFrame].image,
															animatedImageInfo->frame[actualFrame].size);
						}

						objectTab[layer][i].objectData.animatedImage.lastFrame = objectTab[layer][i].objectData.animatedImage.actualFrame;

						if(actualFrame >= animatedImageInfo->nbFrame - 1  || actualFrame == ANIMATED_IMAGE_MAX_FRAME)
							objectTab[layer][i].objectData.animatedImage.actualFrame = 0;
						else
							(objectTab[layer][i].objectData.animatedImage.actualFrame)++;

						objectTab[layer][i].objectData.animatedImage.timeToRefresh = global.absolute_time + objectTab[layer][i].objectData.animatedImage.animatedImageInfo->speedFrame;

						}break;

					case RECTANGLE:
						if(objectTab[layer][i].objectData.rectangle.colorCenter != LCD_DISPLAY_TRANSPARENT){
							LCD_DISPLAY_drawFilledRectangle(layer, objectTab[layer][i].objectData.rectangle.x,
														objectTab[layer][i].objectData.rectangle.y,
														objectTab[layer][i].objectData.rectangle.x + objectTab[layer][i].objectData.rectangle.width,
														objectTab[layer][i].objectData.rectangle.y + objectTab[layer][i].objectData.rectangle.height,
														objectTab[layer][i].objectData.rectangle.colorCenter);
						}

						if(objectTab[layer][i].objectData.rectangle.colorBorder != LCD_DISPLAY_TRANSPARENT){
							LCD_DISPLAY_drawRectangle(layer, objectTab[layer][i].objectData.rectangle.x,
														objectTab[layer][i].objectData.rectangle.y,
														objectTab[layer][i].objectData.rectangle.x + objectTab[layer][i].objectData.rectangle.width,
														objectTab[layer][i].objectData.rectangle.y + objectTab[layer][i].objectData.rectangle.height,
														objectTab[layer][i].objectData.rectangle.colorBorder);
						}
						break;

					case CIRCLE:
						if(objectTab[layer][i].objectData.circle.colorCenter != LCD_DISPLAY_TRANSPARENT){
							LCD_DISPLAY_drawFilledCircle(layer, objectTab[layer][i].objectData.circle.x,
														objectTab[layer][i].objectData.circle.y,
														objectTab[layer][i].objectData.circle.r,
														objectTab[layer][i].objectData.circle.colorCenter);
						}

						if(objectTab[layer][i].objectData.circle.colorBorder != LCD_DISPLAY_TRANSPARENT){
							LCD_DISPLAY_drawCircle(layer, objectTab[layer][i].objectData.circle.x,
												objectTab[layer][i].objectData.circle.y,
												objectTab[layer][i].objectData.circle.r,
												objectTab[layer][i].objectData.circle.colorBorder);
						}
						break;

					case LINE:
						LCD_DISPLAY_drawLine(layer, objectTab[layer][i].objectData.line.x0,
										objectTab[layer][i].objectData.line.y0,
										objectTab[layer][i].objectData.line.x1,
										objectTab[layer][i].objectData.line.y1,
										objectTab[layer][i].objectData.line.color);
						break;

					default:
						break;
				}

				objectTab[layer][i].toDisplay = FALSE;
			}
		}
	}
}

#ifdef TACTILE_ACTIVATED
static bool_e MIDDLEWARE_objectTouch(Uint16 xT, Uint16 yT, Uint16 x, Uint16 y, Uint16 width, Uint16 height){
	return 	(xT >= x && xT <= (x + width) && yT >= y && yT <= (y + height));
}
#endif

//////////////////////////////////////////////////////////////////
//---------------------Fonction Mutation------------------------//
//////////////////////////////////////////////////////////////////

void MIDDLEWARE_setBackground(LCD_DISPLAY_Layer_t layer, objectColor_e color){
	if(!(IS_OBJECT_COLOR_REAL(color)))
		return;

	background[layer].color = color;
	background[layer].toDisplay = TRUE;
}

void MIDDLEWARE_setText(LCD_DISPLAY_Layer_t layer, objectId_t id, const char *text, ...){
	if(!(id < LCD_NB_MAX_OBJECT))
		return;

	if(!(objectTab[layer][id].use))
		return;

	switch(objectTab[layer][id].type){
		case TEXT :{

			objectTab[layer][id].surfaceToErase.x = objectTab[layer][id].objectData.text.x;
			objectTab[layer][id].surfaceToErase.y = objectTab[layer][id].objectData.text.y;
			objectTab[layer][id].surfaceToErase.width = objectTab[layer][id].objectData.text.width;
			objectTab[layer][id].surfaceToErase.height = objectTab[layer][id].objectData.text.height;
			objectTab[layer][id].toErase = TRUE;

			va_list args_list;
			va_start(args_list, text);
			vsnprintf((char *)objectTab[layer][id].objectData.text.text, OBJECT_TEXT_MAX_SIZE, text, args_list);
			va_end(args_list);

			Uint16 widthText, heightText;
			LCD_DISPLAY_getStringSize((char *)objectTab[layer][id].objectData.text.text, MIDDLEWARE_getFont(objectTab[layer][id].objectData.text.fonts), &widthText, &heightText);
			objectTab[layer][id].objectData.text.width = widthText;
			objectTab[layer][id].objectData.text.height = heightText;

			}break;

		case BUTTON_IMG :
			break;

		case BUTTON_BASE :{

				objectTab[layer][id].surfaceToErase.x = objectTab[layer][id].objectData.buttonBase.x;
				objectTab[layer][id].surfaceToErase.y = objectTab[layer][id].objectData.buttonBase.y;
				objectTab[layer][id].surfaceToErase.width = objectTab[layer][id].objectData.buttonBase.widthButton;
				objectTab[layer][id].surfaceToErase.height = objectTab[layer][id].objectData.buttonBase.heightButton;
				objectTab[layer][id].toErase = TRUE;

				va_list args_list;
				va_start(args_list, text);
				vsnprintf((char *)objectTab[layer][id].objectData.buttonBase.text, OBJECT_TEXT_MAX_SIZE, text, args_list);
				va_end(args_list);

				Uint16 widthText, heightText;
				LCD_DISPLAY_getStringSize((char *)objectTab[layer][id].objectData.buttonBase.text, MIDDLEWARE_getFont(objectTab[layer][id].objectData.buttonBase.fonts), &widthText, &heightText);
				objectTab[layer][id].objectData.buttonBase.widthText = widthText;
				objectTab[layer][id].objectData.buttonBase.heightText = heightText;
				objectTab[layer][id].objectData.buttonBase.widthButton = widthText + 6;
				objectTab[layer][id].objectData.buttonBase.heightButton = heightText + 6;
			}break;

		case PROGRESS_BAR :
			break;

		case SLIDER :
			break;

		case IMAGE :
			break;

		case ANIMATED_IMAGE :
			break;

		default :
			break;
	}
	objectTab[layer][id].toDisplay = TRUE;
}


void MIDDLEWARE_setTextColor(LCD_DISPLAY_Layer_t layer, objectId_t id, objectColor_e colorText){
	if(!(id < LCD_NB_MAX_OBJECT))
		return;

	if(!(objectTab[layer][id].use))
		return;

	switch(objectTab[layer][id].type){
		case TEXT :{

			objectTab[layer][id].surfaceToErase.x = objectTab[layer][id].objectData.text.x;
			objectTab[layer][id].surfaceToErase.y = objectTab[layer][id].objectData.text.y;
			objectTab[layer][id].surfaceToErase.width = objectTab[layer][id].objectData.text.width;
			objectTab[layer][id].surfaceToErase.height = objectTab[layer][id].objectData.text.height;
			objectTab[layer][id].toErase = TRUE;

			objectTab[layer][id].objectData.text.colorText = colorText;
			}break;

		case BUTTON_IMG :
			break;

		case BUTTON_BASE :{

				objectTab[layer][id].surfaceToErase.x = objectTab[layer][id].objectData.buttonBase.x;
				objectTab[layer][id].surfaceToErase.y = objectTab[layer][id].objectData.buttonBase.y;
				objectTab[layer][id].surfaceToErase.width = objectTab[layer][id].objectData.buttonBase.widthButton;
				objectTab[layer][id].surfaceToErase.height = objectTab[layer][id].objectData.buttonBase.heightButton;
				objectTab[layer][id].toErase = TRUE;

				objectTab[layer][id].objectData.buttonBase.colorText = colorText;
			}break;

		case PROGRESS_BAR :
			break;

		case SLIDER :
			break;

		case IMAGE :
			break;

		case ANIMATED_IMAGE :
			break;

		default :
			break;
	}
	objectTab[layer][id].toDisplay = TRUE;
}

//////////////////////////////////////////////////////////////////
//---------------------Fonction Création------------------------//
//////////////////////////////////////////////////////////////////

objectId_t MIDDLEWARE_addText(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, objectColor_e colorText, objectColor_e colorBackground,  textFonts_e fonts, const char *text, ...){
	if(!(IS_OBJECT_COLOR_REAL(colorText)))
		return OBJECT_ID_ERROR_FULL;

	if(!(IS_TEXT_FONTS(fonts)))
		return OBJECT_ID_ERROR_FULL;

	if(!(text != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = TEXT;
	objectTab[layer][idFound].objectData.text.x = x;
	objectTab[layer][idFound].objectData.text.y = y;
	objectTab[layer][idFound].objectData.text.fonts = fonts;
	objectTab[layer][idFound].objectData.text.colorText = colorText;
	objectTab[layer][idFound].objectData.text.colorBackground = colorBackground;

	va_list args_list;
	va_start(args_list, text);
	vsnprintf((char *)objectTab[layer][idFound].objectData.text.text, OBJECT_TEXT_MAX_SIZE, text, args_list);
	va_end(args_list);

	Uint16 widthText, heightText;
	LCD_DISPLAY_getStringSize((char *)objectTab[layer][idFound].objectData.text.text, MIDDLEWARE_getFont(objectTab[layer][idFound].objectData.text.fonts), &widthText, &heightText);
	objectTab[layer][idFound].objectData.text.width = widthText;
	objectTab[layer][idFound].objectData.text.height = heightText;

	return idFound;
}

objectId_t MIDDLEWARE_addButtonImg(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, const imageInfo_s *imageNormal, const imageInfo_s *imageLock, bool_e lockTouch, bool_e * touch){
	if(!(imageNormal != NULL))
		return OBJECT_ID_ERROR_FULL;

	if(!(imageLock != NULL))
		return OBJECT_ID_ERROR_FULL;

	if(!(touch != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	*touch = FALSE;

	objectTab[layer][idFound].type = BUTTON_IMG;
	objectTab[layer][idFound].objectData.buttonImg.state = BUTTON_STATE_OFF;
	objectTab[layer][idFound].objectData.buttonImg.lastState = BUTTON_STATE_OFF;
	objectTab[layer][idFound].objectData.buttonImg.lastStateTouch = BUTTON_STATE_NO_TOUCH;
	objectTab[layer][idFound].objectData.buttonImg.x = x;
	objectTab[layer][idFound].objectData.buttonImg.y = y;
	objectTab[layer][idFound].objectData.buttonImg.imageNormal = imageNormal;
	objectTab[layer][idFound].objectData.buttonImg.imageLock = imageLock;
	objectTab[layer][idFound].objectData.buttonImg.lockTouch = lockTouch;
	objectTab[layer][idFound].objectData.buttonImg.touch = touch;

	return idFound;
}

objectId_t MIDDLEWARE_addButton(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, Uint16 width, Uint16 height, bool_e lockTouch, bool_e *touch, objectColor_e colorText, objectColor_e colorButton, objectColor_e colorButtonTouch, objectColor_e colorBorder, textFonts_e fonts, const char *text, ...){
	if(!(touch != NULL))
		return OBJECT_ID_ERROR_FULL;

	if(!(IS_OBJECT_COLOR_REAL(colorText)))
		return OBJECT_ID_ERROR_FULL;

	if(!(IS_OBJECT_COLOR_REAL(colorButton)))
		return OBJECT_ID_ERROR_FULL;

	if(!(IS_OBJECT_COLOR_REAL(colorButtonTouch)))
		return OBJECT_ID_ERROR_FULL;

	if(!(IS_TEXT_FONTS(fonts)))
		return OBJECT_ID_ERROR_FULL;

	if(!(text != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	*touch = FALSE;

	objectTab[layer][idFound].type = BUTTON_BASE;
	objectTab[layer][idFound].objectData.buttonBase.state = BUTTON_STATE_OFF;
	objectTab[layer][idFound].objectData.buttonBase.lastState = BUTTON_STATE_OFF;
	objectTab[layer][idFound].objectData.buttonBase.lastStateTouch = BUTTON_STATE_NO_TOUCH;
	objectTab[layer][idFound].objectData.buttonBase.x = x;
	objectTab[layer][idFound].objectData.buttonBase.y = y;
	objectTab[layer][idFound].objectData.buttonBase.fonts = fonts;
	objectTab[layer][idFound].objectData.buttonBase.lockTouch = lockTouch;
	objectTab[layer][idFound].objectData.buttonBase.colorText = colorText;
	objectTab[layer][idFound].objectData.buttonBase.colorButton = colorButton;
	objectTab[layer][idFound].objectData.buttonBase.colorTouch = colorButtonTouch;
	objectTab[layer][idFound].objectData.buttonBase.colorBorder = colorBorder;
	objectTab[layer][idFound].objectData.buttonBase.touch = touch;

	va_list args_list;
	va_start(args_list, text);
	vsnprintf((char *)objectTab[layer][idFound].objectData.buttonBase.text, OBJECT_TEXT_MAX_SIZE, text, args_list);
	va_end(args_list);

	Uint16 widthText, heightText;
	LCD_DISPLAY_getStringSize((char *)objectTab[layer][idFound].objectData.buttonBase.text, MIDDLEWARE_getFont(fonts), &widthText, &heightText);
	objectTab[layer][idFound].objectData.buttonBase.widthText = widthText;
	objectTab[layer][idFound].objectData.buttonBase.heightText = heightText;

	if(width > 0)
		objectTab[layer][idFound].objectData.buttonBase.widthButton = width;
	else
		objectTab[layer][idFound].objectData.buttonBase.widthButton = widthText + 6;

	if(height > 0)
		objectTab[layer][idFound].objectData.buttonBase.heightButton = height;
	else
		objectTab[layer][idFound].objectData.buttonBase.heightButton = heightText + 6;

	return idFound;
}

objectId_t MIDDLEWARE_addProgressBar(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, Uint16 width, Uint16 height, objectOrientation_e orientation, Uint8 *value){
	if(!(IS_OBJECT_ORIENTATION(orientation)))
		return OBJECT_ID_ERROR_FULL;

	if(!(value != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = PROGRESS_BAR;
	objectTab[layer][idFound].objectData.progressBar.x = x;
	objectTab[layer][idFound].objectData.progressBar.y = y;
	objectTab[layer][idFound].objectData.progressBar.width = width;
	objectTab[layer][idFound].objectData.progressBar.height = height;
	objectTab[layer][idFound].objectData.progressBar.orientation = orientation;
	objectTab[layer][idFound].objectData.progressBar.refreshBack = TRUE;
	objectTab[layer][idFound].objectData.progressBar.value = value;
	objectTab[layer][idFound].objectData.progressBar.lastValue = *value;

	return idFound;
}

objectId_t MIDDLEWARE_addSlider(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, Uint16 width, Uint16 height, Sint32 minValue, Sint32 maxValue, objectOrientation_e orientation, Sint32 *value){
	if(!(IS_OBJECT_ORIENTATION(orientation)))
		return OBJECT_ID_ERROR_FULL;

	if(!(value != NULL))
		return OBJECT_ID_ERROR_FULL;

	if(!(minValue != maxValue))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = SLIDER;
	objectTab[layer][idFound].objectData.slider.x = x;
	objectTab[layer][idFound].objectData.slider.y = y;
	objectTab[layer][idFound].objectData.slider.width = width;
	objectTab[layer][idFound].objectData.slider.height = height;
	objectTab[layer][idFound].objectData.slider.minValue = minValue;
	objectTab[layer][idFound].objectData.slider.maxValue = maxValue;
	objectTab[layer][idFound].objectData.slider.orientation = orientation;
	objectTab[layer][idFound].objectData.slider.value = value;
	if(*value < minValue || *value > maxValue)
		objectTab[layer][idFound].objectData.slider.realValue = (maxValue - minValue)/2;
	else
		objectTab[layer][idFound].objectData.slider.realValue = *value;
	objectTab[layer][idFound].objectData.slider.lastRealValue = objectTab[layer][idFound].objectData.slider.realValue;

	return idFound;
}

objectId_t MIDDLEWARE_addImage(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, const imageInfo_s *image){
	if(!(image != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = IMAGE;
	objectTab[layer][idFound].objectData.image.x = x;
	objectTab[layer][idFound].objectData.image.y = y;
	objectTab[layer][idFound].objectData.image.image = image;

	return idFound;
}

objectId_t MIDDLEWARE_addAnimatedImage(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, const animatedImageInfo_s *animatedImageInfo){
	if(!(animatedImageInfo != NULL))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = ANIMATED_IMAGE;
	objectTab[layer][idFound].objectData.animatedImage.x = x;
	objectTab[layer][idFound].objectData.animatedImage.y = y;
	objectTab[layer][idFound].objectData.animatedImage.animatedImageInfo = animatedImageInfo;
	objectTab[layer][idFound].objectData.animatedImage.actualFrame = 0;
	objectTab[layer][idFound].objectData.animatedImage.lastFrame = 255;
	objectTab[layer][idFound].objectData.animatedImage.timeToRefresh = 0;

	return idFound;
}

objectId_t MIDDLEWARE_addRectangle(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, Uint16 width, Uint16 height, objectColor_e colorBorder, objectColor_e colorCenter){
	if(!(IS_OBJECT_COLOR_REAL(colorBorder) || IS_OBJECT_COLOR_REAL(colorCenter)))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = RECTANGLE;
	objectTab[layer][idFound].objectData.rectangle.x = x;
	objectTab[layer][idFound].objectData.rectangle.y = y;
	objectTab[layer][idFound].objectData.rectangle.width = width;
	objectTab[layer][idFound].objectData.rectangle.height = height;
	objectTab[layer][idFound].objectData.rectangle.colorBorder = colorBorder;
	objectTab[layer][idFound].objectData.rectangle.colorCenter = colorCenter;

	return idFound;
}

objectId_t MIDDLEWARE_addCircle(LCD_DISPLAY_Layer_t layer, Sint16 x, Sint16 y, Uint16 r, objectColor_e colorBorder, objectColor_e colorCenter){
	if(!(IS_OBJECT_COLOR_REAL(colorBorder) || IS_OBJECT_COLOR_REAL(colorCenter)))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = CIRCLE;
	objectTab[layer][idFound].objectData.circle.x = x;
	objectTab[layer][idFound].objectData.circle.y = y;
	objectTab[layer][idFound].objectData.circle.r = r;
	objectTab[layer][idFound].objectData.circle.colorBorder = colorBorder;
	objectTab[layer][idFound].objectData.circle.colorCenter = colorCenter;

	return idFound;
}

objectId_t MIDDLEWARE_addLine(LCD_DISPLAY_Layer_t layer, Sint16 x0, Sint16 y0, Sint16 x1, Sint16 y1, objectColor_e color){
	if(!(IS_OBJECT_COLOR_REAL(color)))
		return OBJECT_ID_ERROR_FULL;

	objectId_t idFound = MIDDLEWARE_newObject(layer);

	if(idFound == OBJECT_ID_ERROR_FULL)
		return OBJECT_ID_ERROR_FULL;

	objectTab[layer][idFound].type = LINE;
	objectTab[layer][idFound].objectData.line.x0 = x0;
	objectTab[layer][idFound].objectData.line.y0 = y0;
	objectTab[layer][idFound].objectData.line.x1 = x1;
	objectTab[layer][idFound].objectData.line.y1 = y1;
	objectTab[layer][idFound].objectData.line.color = color;

	return idFound;
}

static objectId_t MIDDLEWARE_newObject(LCD_DISPLAY_Layer_t layer){
	Uint8 i;
	for(i=0; i<LCD_NB_MAX_OBJECT; i++){
		if(objectTab[layer][i].use == FALSE){
			objectTab[layer][i].toDisplay = TRUE;
			objectTab[layer][i].toErase = FALSE;
			objectTab[layer][i].toDestroy = FALSE;
			objectTab[layer][i].use = TRUE;
			return i;
		}
	}
	return OBJECT_ID_ERROR_FULL;
}

static FontDef_t* MIDDLEWARE_getFont(textFonts_e fonts){
	//assert(IS_TEXT_FONTS(fonts));

	switch(fonts){
		case TEXT_FONTS_7x10 :
			return &Font_7x10;

		case TEXT_FONTS_11x18 :
			return &Font_11x18;

		case TEXT_FONTS_16x26 :
			return &Font_16x26;

		default:
			return &Font_7x10;
	}
}

//////////////////////////////////////////////////////////////////
//------------------Fonction Destruction------------------------//
//////////////////////////////////////////////////////////////////

void MIDDLEWARE_deleteObject(LCD_DISPLAY_Layer_t layer, objectId_t id){
	if(!(id < LCD_NB_MAX_OBJECT))
		return;

	if(!(objectTab[layer][id].use))
		return;

	objectTab[layer][id].toDestroy = TRUE;
}

void MIDDLEWARE_resetScreen(){
	Uint8 layer, i;
	for(layer = 0; layer < LCD_DISPLAY_Layer_nb; layer++) {
		for(i=0;i<LCD_NB_MAX_OBJECT;i++){
			objectTab[layer][i].use = FALSE;
		}
		background[layer].color = 0xFFFF;
		background[layer].toDisplay = TRUE;
	}
}


