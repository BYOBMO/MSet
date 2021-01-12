// MSet.cpp : Defines the entry point for the console application.
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <stdio.h>
#include "CMSet.h"
#include "CTexture.h"
#include "CTimer.h"


int poff;
//int scaleOffset;

int scaleFactor = 1;

int mWidth, mHeight;

double gX1 = -2.0, gX2 = 1.0;
double gY1 = -1.5, gY2 = 1.5;

int maxiter = 200;
int dimX = 200;
int dimY = 150;
int winX = 800;
int winY = 600;
int texW, texH;

int xoff = 0, yoff = 0;
int header = 35;
double zfactor = 2;

CMSet mMsetHi;
CMSet mMsetLo;

SDL_Window *gWindow;
SDL_Renderer *gRenderer;
Uint32 colors[2048];

Uint32 gFormat;

SDL_Texture *gPlotTexHi = NULL;

Uint32 black;

int curX, curY;

double cursorVelX;
double cursorVelY;

CTexture *cursor;
SDL_Joystick* gGameController = NULL;

CTimer timer;

SDL_Thread *gThread;

void close()
{
	SDL_DestroyWindow(gWindow);
	SDL_Quit();
}
//
// Thread for high resolution calulations.
//
int CalcThread(void* data)
{
	mMsetHi.mEnd = false;
	while (!mMsetHi.mEnd)
	{
		if (mMsetHi.mStart)
		{
			mMsetHi.Go();
		}
	}

	return(0);
}

//
// Build the color gradient for the final rendered images.
//
void MakeColors(Uint32 pixelFormat)
{
	int i, k;
	k = 0;
	int ii;

	//Allocate format from window
	SDL_PixelFormat* mappingFormat = SDL_AllocFormat(pixelFormat);

	black = SDL_MapRGBA(mappingFormat, 0, 0, 0, 255);

	for (i = 0; i <= 255; i++)
	{

		int offset = (0.4 * 255);
		double range = (255.0 * 0.6);

		ii = ((double)i/255.0) * range + offset;
		colors[k] = SDL_MapRGBA(mappingFormat, ii, 0, 0, 255);
		k = k + 1;
	}
	for (i = 0; i <= 255; i++)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, 255, i, 0, 255);
		k = k + 1;
	}
	for (i = 255; i >= 0; i--)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, i, 255, 0, 255);
		k = k + 1;
	}
	for (i = 0; i <= 255; i++)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, 0, 255, i, 255);
		k = k + 1;
	}
	for (i = 255; i >= 0; i--)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, 0, i, 255, 255);
		k = k + 1;
	}
	for (i = 0; i <= 255; i++)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, i, 0, 255, 255);
		k = k + 1;
	}
	for (i = 0; i <= 255; i++)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, i, 255, 255, 255);
		k = k + 1;
	}
	for (i = 255; i >= 0; i--)
	{
		colors[k] = SDL_MapRGBA(mappingFormat, i, i, i, 255);
		k = k + 1;
	}

	//Free format
	SDL_FreeFormat(mappingFormat);
}


bool init()
{
	//Initialization flag
	bool success = true;



	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Load joystick
		gGameController = SDL_JoystickOpen(0);
		if (gGameController == NULL)
		{
			printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
		}

		//Create window
		gWindow = SDL_CreateWindow("Flocking", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winX, winY, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			gFormat = SDL_GetWindowPixelFormat(gWindow);

			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);


				//Create blank streamable texture
				gPlotTexHi = SDL_CreateTexture(gRenderer, gFormat, SDL_TEXTUREACCESS_STREAMING, winX, winY);
				if (gPlotTexHi == NULL)
				{
					printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
				}
			}
		}
	}

	return success;
}

//
// Render teh final calcualtions to a texture.
//
void Draw(CMSet *mset)
{
	void *pixels;
	int pitch;
	int x, y;

	int index;
	int iterations;
	int bpp;

	Uint32 c;
	int k;

	//Lock texture for manipulation
	SDL_LockTexture(gPlotTexHi, NULL, &pixels, &pitch);

	// Calcualte the bytes per pixel. Should be 4 (RGBA). But some screens could be 3(RGB).
	bpp = pitch / winX;

	for (x = 0; x < mset->mDimX; x++)
	{
		for (y = 0; y < mset->mDimY; y++)
		{
			iterations = mset->points[x][y];

			// Get a color value from teh gradient based on the number of iterations.
			k = 2047 * ((double)iterations / (double)maxiter);
			if (k < 0)
				k = 0;
			else if (k > 2047)
				k = 2047;

			c = colors[k];
			if (iterations == 0)
			{
				c = black;
			}
			index = (x) + (y * pitch/bpp);
			*((Uint32*)pixels + index) = c; // Update the texture's pixel data.
		}
	}

	//Unlock texture to update
	SDL_UnlockTexture(gPlotTexHi);

	pixels = NULL;

	// Set the rendering dimensions based on whether it was a high or low resolution image.
	texW = mset->mDimX;
	texH = mset->mDimY;
}


//
// Run a low resolution plot.
//
 void RunPlot(bool hi)
 {
	mMsetHi.SetPlot(gX1, gX2, gY1, gY2, scaleFactor, maxiter);
	mMsetHi.mCancel = true;
	mMsetHi.mStart = true;

	// Increase teh iterations based on how far we are zoomed in.
	// High zoom values can handle more iterations.
	int scale_iterations = 100;
	double delta = gX2 - gX1;
	if (delta < 0.01)
		delta = 0.01;

	scale_iterations = 100 + 1.0 / (2.0*delta / 4.0);
	if (scale_iterations > maxiter)
		scale_iterations = maxiter;

	// Run teh calcualtions.
	mMsetLo.RunPlot(gX1, gX2, gY1, gY2, scaleFactor, scale_iterations);

	// Render the result to a texture.
	Draw(&mMsetLo);
 }



void Setup()
{
	mMsetHi.SetDim(gRenderer, gFormat, winX, winY);
	mMsetLo.SetDim(gRenderer, gFormat, dimX, dimY);

	mMsetHi.maxIter = maxiter;
	mMsetHi.maxIter = 150;

	SDL_ShowCursor(false);
	cursor = new CTexture();
	cursor->LoadFromFile(gRenderer, "dot.png");

	curX = winX / 2;
	curY = winY / 2;

	gX1 = -2.5;
	gX2 = 1.5;
	gY1 = -1.5;
	gY2 = 1.5;

	texW = winX;
	texH = winY;

	timer.start();
}


//
// Runs a calculation centered on the specified point.
//
void DoCenter(int px, int py)
{
	double dx, dy;
	double y, x;

	curX = px;
	curY = py;

	dx = gX2 - gX1;
	dy = gY2 - gY1;

	x = gX1 + dx * ((float)px / (float)winX);
	y = gY2 - dy * ((float)py / (float)winY);

	gX1 = x - dx / 2;
	gY1 = y - dy / 2;

	gX2 = x + dx / 2;
	gY2 = y + dy / 2;

	RunPlot(true);
}

//
// Zoom in or out around the specified point.
//
void DoMouse(int px, int py, double z)
{
	double dx, dy;
	double y,x;
	double gXoffset, gYoffset;
	double centerX, centerY;

	double xpercent, ypercent;

	//printf("%d %d\n", px, py);

	// Figure out the current X and Y ranges.
	dx = gX2 - gX1;
	dy = gY2 - gY1;

	// Find the center point of the current plot.
	centerX = gX1 + dx / 2.0;
	centerY = gY1 + dy / 2.0;

	// Find out where the poitn is in the current plot.
	xpercent = ((double)px / (double)winX);
	ypercent = ((double)py / (double)winY);

	x = gX1 + dx * xpercent;
	y = gY2 - dy * ypercent;

	// The zoom is done around the specified point. We need to translate the entire plot
	// so teh selected point remains in the same spot on the screen. This could probably
	// all be done with one clever matrix calcualtion...
	gXoffset = x - centerX;
	gYoffset = y - centerY;

	double sx = dx * xpercent;
	double sy = dy * ypercent;

	// FIgure out the new X and Y ranges based on the zoom.
	gX1 = x - dx / z / 2;
	gY1 = y - dy / z / 2;

	gX2 = x + dx / z / 2;
	gY2 = y + dy / z / 2;

	// Now translate so the selected point remains stationary.
	dx = gX2 - gX1;
	dy = gY2 - gY1;
	x = gXoffset / z;
	y = gYoffset / z;

	gX1 -= x;
	gX2 -= x;
	gY1 -= y;
	gY2 -= y;

	// Recalculate points.
	RunPlot(true);
}


#undef main
int main()
{
	//Event handler
	SDL_Event e;
	bool quit = false;

	init();
	MakeColors(gFormat);
	Setup();

	// Start with a high rez plot.
	mMsetHi.SetPlot(gX1, gX2, gY1, gY2, scaleFactor, maxiter);
	gThread = SDL_CreateThread(CalcThread, "LazyThread", NULL);
	mMsetHi.Go();

	while (!quit)
	{
		const Uint8 *keys = SDL_GetKeyboardState(NULL);


		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			if ((keys[SDL_SCANCODE_RCTRL] || keys[SDL_SCANCODE_LCTRL]))
			{
				DoMouse(curX, curY, 2.0);
			}
			else if ((keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_LSHIFT]))
			{
				DoMouse(curX, curY, 0.5);
			}
			else if ((keys[SDL_SCANCODE_RETURN]))
			{
				DoCenter(curX, curY);
			}

			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				//Select surfaces based on key press
				switch (e.key.keysym.sym)
				{

				case SDLK_UP:
					cursorVelY = -1;
					break;

				case SDLK_DOWN:
					cursorVelY = 1;
					break;

				case SDLK_LEFT:
					cursorVelX = -1;
					break;

				case SDLK_RIGHT:
					cursorVelX = 1;
					break;

				case SDLK_ESCAPE:
					quit = true;
					break;

				default:
					break;
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				//Select surfaces based on key press
				switch (e.key.keysym.sym)
				{

				case SDLK_UP:
				case SDLK_DOWN:
					cursorVelY = 0;
					break;

				case SDLK_LEFT:
				case SDLK_RIGHT:
					cursorVelX = 0;
					break;


				default:
					break;
				}
			}
			else if (e.type == SDL_MOUSEMOTION)
			{

					//Get mouse position
					int x, y;
					SDL_GetMouseState(&x, &y);
					curX = x;
					curY = y;
			
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				//Get mouse position
				int x, y;
				SDL_GetMouseState(&x, &y);


				curX = x;
				curY = y;
				if (e.button.button == 1)
				{
					DoMouse(x, y, 2.0);
				}
				else if (e.button.button == 3)
				{
					DoMouse(x, y, 0.5);
				}
				else if (e.button.button == 2)
				{
					DoCenter(x, y);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				// Zoom using the mouse wheel.
				int x, y;
				SDL_GetMouseState(&x, &y);
				if (e.wheel.y > 0)
				{
					DoMouse(x, y, zfactor);
				}
				else if (e.wheel.y < 0)
				{
					DoMouse(x, y, 0.5);
				}
			}
			else if (e.type == SDL_JOYBUTTONDOWN)
			{

				switch (e.jbutton.button)
				{
				case 0:
					DoMouse(curX, curY, 2.0);
					break;
				case 1:
					DoCenter(curX, curY);
					
					break;
				case 2:
					
					break;
				case 3:
					DoMouse(curX, curY, 0.5);
					break;

				case 5:
					if (cursorVelX == 1)
						cursorVelX = 3;

					if (cursorVelX == -1)
						cursorVelX = -3;

					if (cursorVelY == 1)
						cursorVelY = 3;

					if (cursorVelY == -1)
						cursorVelY = -3;

					break;
				}
			}
			else if (e.type == SDL_JOYBUTTONUP)
			{
				switch (e.jbutton.button)
				{
				case 0:
					break;
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;

				case 5:
					if (cursorVelX == -3)
						cursorVelX = -1;

					if (cursorVelY == -3)
						cursorVelY = -1;

					if (cursorVelX == 3)
						cursorVelX = 1;

					if (cursorVelY == 3)
						cursorVelY = 1;
				}
			}
			else if (e.type == SDL_JOYAXISMOTION)
			{
				int speed = 1;
				if (SDL_JoystickGetButton(gGameController, 5))
					speed =  3;
				//Motion on controller 0
				if (e.jaxis.which == 0)
				{
					//X axis motion
					if (e.jaxis.axis == 0)
					{
						//Left of dead zone
						if (e.jaxis.value < -8000)
						{
							// Map joystick left to a keyboard stroke.
							cursorVelX = -1*speed;
						}
						//Right of dead zone
						else if (e.jaxis.value > 8000)
						{
							// Map jouystick right to keyboard stroke.
							cursorVelX = 1*speed;
						}
						else
						{
							// Zero out the joystick when centered.
							cursorVelX = 0;
						}
					}
					//Y axis motion
					else if (e.jaxis.axis == 1)
					{
						//Below of dead zone
						if (e.jaxis.value < -8000)
						{
							cursorVelY = -1*speed;
						}
						//Above of dead zone
						else if (e.jaxis.value > 8000)
						{
							cursorVelY = 1*speed;
						}
						else
						{
							cursorVelY = 0;
						}
					}
				}
			}
		}

		// Timer for updating the cursor.
		if (timer.getTicks() > 8)
		{
			timer.stop();
			curX += cursorVelX;
			curY += cursorVelY;

			if (curX < 0)
				curX = 0;
			else if (curX > winX)
				curX = winX;

			if (curY < 0)
				curY = 0;
			else if (curY > winY)
				curY = winY;

			timer.start();
		}

	
		// Check the hi rez thread to see if a new render is ready.
		// If it is then save it to the rendering texture.
		if (mMsetHi.mReady)
		{
			mMsetHi.mReady = false;
			Draw(&mMsetHi);
		}
		cursor->SetPosition(curX - cursor->GetWidth() / 2, curY - cursor->GetHeight() / 2);
		SDL_RenderClear(gRenderer);

		// Figure out the dimension to render. if it's a hi rez image then the window and texture
		// should be equal sizes. If it's a low rez image then it will need to be scaled up.
		SDL_Rect src = { 0,0,texW,texH };
		SDL_Rect dst = { 0,0,winX,winY };

		SDL_RenderCopyEx(gRenderer, gPlotTexHi, &src, &dst, 0.0, NULL, SDL_FLIP_NONE);
		cursor->Draw(gRenderer);

		SDL_RenderPresent(gRenderer);
	}
	
	close();
	printf("\ndone\n");

    return 0;
}

