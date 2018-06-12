#include "CTexture.h"

#include <stdio.h>
#include <sstream>

CTexture::CTexture()
{
	mTexture = NULL;
	Angle = 0.0;
}


CTexture::~CTexture()
{
}

void CTexture::Draw(SDL_Renderer *renderer, int x, int y, int w, int h, float angle)
{
	if (mTexture != NULL)
	{
		//Set rendering space and render to screen
		SDL_Rect renderQuad = { x, y, w, h };

		//Set clip rendering dimensions
		//if (clip != NULL)
		//{
		//	renderQuad.w = clip->w;
		//	renderQuad.h = clip->h;
		//}

		//Render to screen
		SDL_RenderCopyEx(renderer, mTexture, NULL, &renderQuad, angle, NULL, SDL_FLIP_NONE);
	}
}

void CTexture::Draw(SDL_Renderer *renderer, int x, int y, float angle)
{
	if (mTexture != NULL)
	{
		//Set rendering space and render to screen
		SDL_Rect renderQuad = { x, y, mTexW, mTexH };

		//Set clip rendering dimensions
		//if (clip != NULL)
		//{
		//	renderQuad.w = clip->w;
		//	renderQuad.h = clip->h;
		//}

		//Render to screen
		SDL_RenderCopyEx(renderer, mTexture, NULL, &renderQuad, angle, NULL, SDL_FLIP_NONE);
	}
}

void CTexture::Draw(SDL_Renderer *renderer)
{
	if (mTexture != NULL)
	{
		//Set rendering space and render to screen
		SDL_Rect renderQuad = { X, Y, mTexW, mTexH };

		//Set clip rendering dimensions
		//if (clip != NULL)
		//{
		//	renderQuad.w = clip->w;
		//	renderQuad.h = clip->h;
		//}

		//Render to screen
		SDL_RenderCopyEx(renderer, mTexture, NULL, &renderQuad, 0.0, NULL, SDL_FLIP_NONE);
	}
}

void CTexture::SetTexture(SDL_Texture *tex)
{
	mTexture = tex;
}

void CTexture::SetPosition(int x, int y)
{
	X = x;
	Y = y;
}

bool CTexture::LoadFromFile(SDL_Renderer *renderer, std::string path)
{
	//Get rid of preexisting texture
	//free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mTexW = loadedSurface->w;
			mTexH = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}