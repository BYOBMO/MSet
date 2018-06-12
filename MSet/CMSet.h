#pragma once
#include <SDL2/SDL.h>
#include <stdio.h>

class CMSet
{
public:
	CMSet();
	~CMSet();

	SDL_Texture *mPlotTexture = NULL;
	double gX1, gX2, gY1, gY2;

	int points[800][600];
	int mDimX;
	int mDimY;
	int scaleOffsetX;
	int scaleOffsetY;
	int maxIter;
	//int calcDimX;
	//int calcDimY;
	bool mReady;
	bool mCancel;
	bool mStart;
	bool mEnd;

	void RunPlot(double x1, double x2, double y1, double y2, int scaleFactor, int max);
	void SetPlot(double x1, double x2, double y1, double y2, int scaleFactor, int max);
	bool StartMSet();
	int DoCalc(double x, double y);
	void SetDim(SDL_Renderer *renderer, Uint32 format, int x, int y);
	void Go();

};

