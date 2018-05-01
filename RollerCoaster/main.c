#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "SDL2/SDL.h"
#include "GLES/gl.h"

#include "roller.h"

struct rollercoaster iRollerCoaster;

void AppInit(int width, int height)
{
	static const point array[] = {
		{4.0f, -2.0f, 0.2f}, {0.0f, 0.0f, 0.0f},
		{3.0f, 0.0f, 0.21f}, {-0.5f, 1.0f, 0.0f},
		{2.0f, 2.0f, 3.5f}, {0.0f, 0.0f, 0.0f},
		{0.0f, 3.0f, 3.4f}, {0.0f, 0.0f, 0.0f},
		{-3.0f, 0.0f, 0.2f}, {0.0f, 0.0f, 0.0f},
		{-3.0f, -2.0f, 2.5f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -3.0f, 0.2f}, {1.0f, 0.0f, 0.0f},
		{1.0f, -3.1f, 1.2f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -3.2f, 2.2f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, -3.3f, 1.2f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -3.4f, 0.2f}, {1.0f, 0.0f, 0.0f},
		{0.5f, -3.4f, 0.2f}, {1.0f, 0.0f, 0.0f},
		{1.5f, -3.5f, 1.2f}, {0.0f, 0.0f, 0.0f},
		{0.5f, -3.6f, 2.2f}, {0.0f, 0.0f, 0.0f},
		{-0.5f, -3.7f, 1.2f}, {0.0f, 0.0f, 0.0f},
		{0.5f, -3.8f, 0.2f}, {1.0f, 0.0f, 0.0f},
		{4.0f, -4.0f, 2.5f}, {0.0f, 0.0f, 0.0f},
		{4.0f, -1.0f, 3.0f}, {0.0f, 0.0f, 0.0f},
		{2.0f, 2.0f, 1.0f}, {0.0f, 0.0f, 0.0f},
		{-3.0f, -2.0f, 1.0f}, {0.0f, 0.0f, 0.0f},
		{-3.0f, 3.0f, 3.3f}, {0.0f, 0.0f, 0.0f},
		{0.0f, 3.3f, 0.7f}, {0.0f, 0.0f, 0.0f},
		{2.0f, 3.5f, 0.2f}, {0.0f, 0.0f, 0.0f},
		{3.5f, 3.4f, 1.7f}, {0.0f, 0.0f, 0.0f},
		{2.0f, 3.3f, 3.2f}, {0.0f, 0.0f, 0.0f},
		{0.5f, 3.2f, 1.7f}, {0.0f, 0.0f, 0.0f},
		{2.0f, 3.1f, 0.3f}, {1.0f, 0.0f, 0.0f},
		{4.0f, 2.0f, 0.3f}, {0.0f, 0.0f, 0.0f},
		{3.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -1.0f, 3.0f}, {0.0f, 0.0f, 0.0f},
		{-3.0f, -3.0f, 1.0f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -5.0f, 0.2f}, {4.0f, -0.5f, 0.0f},
		{5.0f, -4.0f, 0.2f}, {-0.5f, 1.0f, 0.0f}
	};
	iRollerCoaster.trkd.nbControlPoint = 33;
	iRollerCoaster.trkd.control = (point*)array;
	iRollerCoaster.trkd.startSegment = 2;
	iRollerCoaster.trkd.brakeSegment = -12;
	iRollerCoaster.trkd.twistFactor = 1.5f;
	iRollerCoaster.trkd.averageSegmentLength = 0.35f;

	//iStartTime.UniversalTime();
	iRollerCoaster.eng.doIntro = 0;
	//iRollerCoaster.eng.wndWidth = iScreenWidth;
	//iRollerCoaster.eng.wndHeight = iScreenHeight;
        iRollerCoaster.eng.wndWidth = width;
	iRollerCoaster.eng.wndHeight = height;

	static const point coord[] = {
		{0.98f, -3.05f, 1.0f}, {1.0f, 0.0f, 0.0f},
		{0.0f, -3.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
		{0.25f, -3.4f, 1.0f}, {1.0f, 0.0f, 0.0f},
		{0.5f, -3.8f, 1.0f}, {1.0f, 0.0f, 0.0f},
		{4.44f, -2.88f, 0.0f}, {-0.5f, 1.0f, -0.18f},
		{2.0f, 3.5f, 1.0f}, {1.0f, 0.0f, 0.0f},
		{3.4f, 3.4f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{1.0f, 3.05f, 2.0f}, {1.0f, 0.0f, 0.0f},
		{2.5f, 3.05f, 1.0f}, {1.0f, 0.0f, 0.0f}
	};
	iRollerCoaster.supd.pillar_coord = (point*)coord;
	iRollerCoaster.supd.nbPillarCoord = 9;
	static const int absc[] = {
		9,
		15,
		22,
		30,
		40,
		50,
		60,
		115, //after double loop
		120,
		135,
		140,
		150,
		155,
		165,
		180,
		252,
		258,
		265,
		275,
		280,
		285,
		290,
		295
	};
	iRollerCoaster.supd.pillar_absc = (int*)absc;
	iRollerCoaster.supd.nbPillarAbsc = 23;

	InitializeRoller(&iRollerCoaster, "");
        SetRollerShadeMode(&iRollerCoaster, ROLLER_SHADE_SMOOTH);
}


void AppExit()
{
	ExitRoller(&iRollerCoaster);
}

void draw_string(SDL_Renderer *renderer, SDL_Texture *font, const char *s, int x, int y) {
    unsigned char *p = (unsigned char *)s;
    char c;
    SDL_Rect src_rect;
    SDL_Rect dst_rect;

    src_rect.w = 16;
    src_rect.h = 16;

    dst_rect.w = 16;
    dst_rect.h = 16;

    dst_rect.x = x;
    dst_rect.y = y;

    while((c = *(p++))) {
        src_rect.x = (c & 0x0F) << 4;
        src_rect.y = (c & 0xF0);

        SDL_RenderCopy(renderer, font, &src_rect, &dst_rect);

        dst_rect.x += 16;
    }
}

void cut_path(char *path) {
    char *p = path + strlen(path);
    char d;
    while (*p != '/' && *p != '\\' && p >= path) p--;
    d = *p;
    *(p + 1) = 0;
    while (p >= path) {
        if (*p == '\\') *p = '/';
        p--;
    }
}

int main(int argc, char *argv[]) {
    Uint32 videoflags;
    int width, height, bpp;
    int fullscreen;

    char path[256];
    char fps_string[256];
    int done = 0;
    long t;
    long new_t;
    long start_time;
    int frames = 0;
    int fps;

    width = 640;
    height = 480;
    bpp = 16;
    fullscreen = 0;



    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        char s[256];
        sprintf(s, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    videoflags = 0;
    if (fullscreen) videoflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    SDL_Window *window = SDL_CreateWindow(
        "RollerCoaster",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        videoflags
    );

    if (window == NULL) {
        char s[256];
        sprintf(s, "Couldn't initialize window %ix%i\n", width, height);
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(renderer, width, height);

    SDL_Texture *sdlTexture = SDL_CreateTexture(renderer,
                               SDL_PIXELFORMAT_RGB565,
                               SDL_TEXTUREACCESS_STREAMING,
                               width, height);

    uint16_t *pixels = malloc(width * height * sizeof(uint16_t));
    int pitch = width * sizeof(uint16_t);
    memset(pixels, 0x1f, width * height * sizeof(uint16_t));
    SDL_UpdateTexture(sdlTexture, NULL, pixels, pitch);

    gl_CreateContext(pixels, width, height, pitch);

    strcpy(path, argv[0]);
    cut_path(path);
    strcat(path, "arial.bmp");

    SDL_Surface *fontImage = SDL_LoadBMP(path);
    SDL_Texture *font = SDL_CreateTextureFromSurface(renderer, fontImage);

    //SDL_SetColorKey(font, SDL_SRCCOLORKEY, 0x000000);

    AppInit(width, height);

    t = start_time = SDL_GetTicks();
    DrawRoller(0, &iRollerCoaster);
    while (!done) {
        SDL_PumpEvents();

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_RETURN] || keystate[SDL_SCANCODE_ESCAPE]) done = 1;

        new_t = SDL_GetTicks();
        if (new_t - t >= 1000) {
        	fps = (int) ((frames * 1000) / (new_t - t));
        	frames = 0;
        	t = new_t;
        }
        sprintf(fps_string, "%i FPS", fps);
        //if (fps > 0 && fps <= 30) sprintf(fps_string, "%i FPS ", fps);
        //else sprintf(fps_string, "30+ FPS");

        frames++;

        DrawRoller(new_t - start_time, &iRollerCoaster);
        //DrawRoller(1014000, &iRollerCoaster);

        SDL_UpdateTexture(sdlTexture, NULL, pixels, pitch);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
        draw_string(renderer, font, fps_string, 0, 0);
        SDL_RenderPresent(renderer);
    }

    AppExit();

    SDL_Quit();

    return EXIT_SUCCESS;
}
