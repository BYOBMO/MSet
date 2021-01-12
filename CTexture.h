#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <sstream>

using namespace std;

class CTexture
{
public:
	CTexture();
	~CTexture();
	void Draw(SDL_Renderer *renderer, int x, int y, int w, int h, float angle);
	void Draw(SDL_Renderer *renderer, int x, int y, float angle);
	void Draw(SDL_Renderer *renderer);
	void SetTexture(SDL_Texture *tex);
	void SetPosition(int x, int y);
	bool LoadFromFile(SDL_Renderer *renderer, std::string path);
	int GetWidth() { return(mTexW); }
	int GetHeight() { return(mTexH); }
	void setAngle(float a) { Angle = a; }
	float getAngle() { return(Angle); }

private:
	int X, Y;
	SDL_Texture *mTexture;
	int mTexW;
	int mTexH;
	float Angle;
};

