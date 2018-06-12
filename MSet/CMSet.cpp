#include "CMSet.h"
#include <SDL2/SDL.h>


CMSet::CMSet()
{
	mStart = false;
}


CMSet::~CMSet()
{
}

void CMSet::SetDim(SDL_Renderer *renderer, Uint32 format, int x, int y)
{
	mDimX = x;
	mDimY = y;

	mPlotTexture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, mDimX, mDimY);
	if (mPlotTexture == NULL)
	{
		printf("Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
	}
}

//
// Run a plot on the specified range.
//
void CMSet::RunPlot(double x1, double x2, double y1, double y2, int scaleFactor, int max)
{
	mReady = false;
	mCancel = false;
	mStart = false;

	maxIter = max;
	gX1 = x1;
	gX2 = x2;
	gY1 = y1;
	gY2 = y2;
	//calcDimX = mDimX / scaleFactor;
	//calcDimY = mDimY / scaleFactor;
	scaleOffsetX = 0;// (mDimX - calcDimX) / 2;
	scaleOffsetY = 0;// (mDimY - calcDimY) / 2;
	mReady = StartMSet();
}

//
// Set the plot range for an asynchronous plot.
//
void CMSet::SetPlot(double x1, double x2, double y1, double y2, int scaleFactor, int max)
{
	maxIter = max;
	gX1 = x1;
	gX2 = x2;
	gY1 = y1;
	gY2 = y2;
	//calcDimX = mDimX / scaleFactor;
	//calcDimY = mDimY / scaleFactor;
	scaleOffsetX = 0;// (mDimX - calcDimX) / 2;
	scaleOffsetY = 0;// (mDimY - calcDimY) / 2;

	mReady = false;
	mCancel = false;
	mStart = true; // Tell the thread a plot is ready for running.
}

//
// Start an asychronous plot.
//
void CMSet::Go()
{
	mReady = false;
	mCancel = false;
	mStart = false;

	mReady = StartMSet();
}

//
// Iterates of the pixels to generate the plot data.
//
bool CMSet::StartMSet()
{
	int i, x, y;
	double x1, y1;

	double yd, xd;

	double dx, dy;

	dx = gX2 - gX1;
	dy = gY2 - gY1;

	// Map the data points to pixel values.
	xd = dx / (float)mDimX;
	yd = dy / (float)mDimY;


	x1 = gX1;

	for (x = 0; x < mDimX; x++)
	{
		x1 += xd;
		x1 = gX1 + ((float)x/(float)mDimX * dx);
		y1 = gY2;
		for (y = 0; y < mDimY; y++)
		{
			if (mCancel)
			{
				// Asynchrnous plot cancelled.
				printf("Cancel\n");
				return(false);
			}
			y1 = y1 - yd;

			y1 = gY2 - ((float)y / (float)mDimY * dy);
			i = DoCalc(x1, y1); // Get the iterations from teh calculation.

			points[x][y] = i;
		}
		printf("%03d\r", x);
	}

	// returns true if not cancelled. This indicates a new set of points is ready for rendering.
	return(true);
}

//
// Calculate the number of iterations for the specified point.
//
int CMSet::DoCalc(double x, double y)
{
	double x1, y1, zx, zy;
	int i;
	bool done = false;
	double d;

	double x2, y2;

	x1 = 0;
	y1 = 0;


	//s = iterField.getText();

	int max;// = Integer.parseInt(s.trim());

	max = maxIter;

	for (i = 1; i <= max; i++)
	{
		if (mCancel)
		{
			printf("Cancel\n");
			return(0);
		}
		done = true;

		x2 = x1 * x1;
		y2 = y1 * y1;

		zx = x2 - y2 + x;
		zy = 2 * x1*y1 + y;

		x1 = zx;
		y1 = zy;

		d = x2 + y2;
		//printf("%f\n", d);
		if (d >= 4.0)
		{
			done = false;
			break;
		}

	}

	if (done)
	{
		return(0);
	}

	return(i);
}


