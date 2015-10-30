#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

extern unsigned char basicfont[];
extern unsigned int basicfont_size;
extern const unsigned char background_png[];
extern const unsigned char button_pressed_png[];
extern const unsigned char button_unpressed_png[];

#define EXIT_COMBO (SCE_CTRL_START | SCE_CTRL_SELECT)

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

#define BLACK   RGBA8(  0,   0,   0, 255)
#define WHITE   RGBA8(255, 255, 255, 255)

#define SCREEN_W 960
#define SCREEN_H 544

SceCtrlData pad;
SceTouchData touch;

int main()
{
	printf("Starting VitaShell\n");
	vita2d_init();
	vita2d_set_clear_color(BLACK);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);

	vita2d_font *font = vita2d_load_font_mem(basicfont, basicfont_size);

	vita2d_texture *background = vita2d_load_PNG_buffer(background_png);
	vita2d_texture *button_pressed = vita2d_load_PNG_buffer(button_pressed_png);
	vita2d_texture *button_unpressed = vita2d_load_PNG_buffer(button_unpressed_png);
	vita2d_texture *gameIcon = vita2d_create_empty_texture(300, 250);

	// Create homebrew directory
	// TODO: check if directory exists first, instead of sliently throwing an error
	sceIoMkdir("cache0:/homebrews/", 0777);

	int buttonState[4] = {1,0,0,0};
	int i;
	int lastButtonPressed=0;
	int Xtouch = 0;
	int Ytouch = 0;
	int selectedButton = 1;

	printf("Rendering menu\n");

	while (1) {
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);

		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons == EXIT_COMBO) {
			printf("Exit combo pressed\n");
			break;
		}

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_texture(background, 0, 0);

		i=0;
		for(;i<4;i++)
		{
			if(buttonState[i] == 0)
			{
				vita2d_draw_texture(button_unpressed, 138*i, 0);
			}
			else {
				vita2d_draw_texture(button_pressed, 138*i, 0);
			}
		}

		sceTouchPeek(0, &touch, 1);
		if (touch.reportNum > 0)
		{
			i=0;
			Xtouch = lerp(touch.report[0].x, 1920, SCREEN_W);
			Ytouch = lerp(touch.report[0].y, 1088, SCREEN_H);

			// Homebrews button
			if ((Xtouch>=0)&&(Xtouch<138)&&(Ytouch<52))
			{
				buttonState[lastButtonPressed] = 0;
				buttonState[0] = 1;
				lastButtonPressed = 0;
				selectedButton = 1;
			}

			// Emulators button
			if ((Xtouch>=138)&&(Xtouch<2*138)&&(Ytouch<52))
			{
				buttonState[lastButtonPressed] = 0;
				buttonState[1] = 1;
				lastButtonPressed=1;
				selectedButton = 2;
			}

			// Tools button
			if ((Xtouch>=138*2)&&(Xtouch<3*138)&&(Ytouch<52))
			{
				buttonState[lastButtonPressed] = 0;
				buttonState[2] = 1;
				lastButtonPressed = 2;
				selectedButton = 3;
			}

			// Options button
			if ((Xtouch>138*3)&&(Xtouch<4*138)&&(Ytouch<52))
			{
				buttonState[lastButtonPressed] = 0;
				buttonState[3] = 1;
				lastButtonPressed = 3;
				selectedButton = 4;
			}
		}

		vita2d_font_draw_text(font, 5, 10, WHITE, 25, "Homebrews");
		vita2d_font_draw_text(font, 138+15, 10, WHITE, 25, "Emulators");
		vita2d_font_draw_text(font, 2*138+38, 10, WHITE, 25, "Tools");
		vita2d_font_draw_text(font, 3*138+27, 10, WHITE, 25, "Options");

		// Homebrews button
		if (selectedButton == 1)
		{
			vita2d_font_draw_text(font, 170, 170, WHITE, 30, "Homebrews Menu");
			vita2d_font_draw_text(font, 100, 500, WHITE, 25, "Press CROSS to launch homebrew");
		}

		// Emulators button
		if (selectedButton == 2)
		{
			vita2d_font_draw_text(font, 170, 170, WHITE, 30, "Emulators Menu");
			vita2d_font_draw_text(font, 100, 500, WHITE, 25, "Press CROSS to launch homebrew");
		}

		// Tools button
		if (selectedButton == 3)
		{
			vita2d_font_draw_text(font, 170, 170, WHITE, 30, "Tools Menu");
		}

		// Options button
		if (selectedButton == 4)
		{
			vita2d_font_draw_text(font, 170, 170, WHITE, 30, "Options Menu");
		}

		vita2d_end_drawing();
		vita2d_swap_buffers();
		sceDisplayWaitVblankStart();
	}

	vita2d_fini();

	//Cleanup
	printf("Cleaning up\n");
	vita2d_free_font(font);
	vita2d_free_texture(background);
	vita2d_free_texture(button_pressed);
	vita2d_free_texture(button_unpressed);
	vita2d_free_texture(gameIcon);

	printf("Exiting\n");
	sceKernelExitProcess(0);
	return 0;
}
