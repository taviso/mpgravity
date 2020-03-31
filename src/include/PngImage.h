// ----------
// PngImage.h
// ----------
/**
* @file
* @brief Re: PngImage
* @author Achim Klein
* @version 0.2
*/

#pragma once

#include "png/png.h"

class PngImage
{
public:
	PngImage();
	virtual ~PngImage();

	bool load(const char* Path);			// reads a png file
	bool load(UINT uiResourceID);			// reads a png file
	bool good() const; // returns true if the object contains valid data
	bool GetBitmap(CBitmap &bitmap, CWnd* pCompatibleWindow);
	int getWidth() const;					// returns the image's width
	int getHeight() const;					// returns the image's height
	// returns the image's RGB+Alpha values (8 bit each)
	unsigned char* getBGRA() const;

protected:
	bool createCompatibleBitmap(int Width, int Height, unsigned char* BGRA, CBitmap &bitmap, CWnd* pCompatibleWindow);
	// returns true if the specified file is a png file
	bool doCheckFileHeader(const char* Path) const;
	// converts the png bytes to BGRA
	bool doExtractCanonicData(png_structp& PngPtr, png_infop& InfoPtr);
	// gets the data from an 8-bit rgb image
	unsigned char* doConvertRGB8(png_structp& PngPtr, png_infop& InfoPtr) const;
	// gets the data from an 8-bit rgb image with alpha values
	unsigned char* doConvertRGBA8(png_structp& PngPtr, png_infop& InfoPtr) const;
	// gets the data from an 8-bit monochrome image
	unsigned char* doConvertGrey8(png_structp& PngPtr, png_infop& InfoPtr) const;
	// gets the data from an 8-bit monochrome image with alpha values
	unsigned char* doConvertGreyA8(png_structp& PngPtr, png_infop& InfoPtr) const;

private:
	bool m_good;						// health flag
	int m_width;						// the image's width
	int m_height;						// the image's height
	unsigned char* m_bgra;				// the color values (blue, green, red, alpha)
};
