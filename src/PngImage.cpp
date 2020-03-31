// ------------
// PngImage.cpp
// ------------
/**
* @file
* @brief Re: PngImage
* @author Achim Klein
* @version 0.2
*/


// --------
// Includes
// --------
#include "stdafx.h"
#include "PngImage.h"

#include<fstream>

// --------
// PngImage
// --------
/**
 * The standard-constructor.
 */
PngImage::PngImage()
{
	m_good = false;
	m_width = 0;
	m_height = 0;
	m_bgra = 0;
}


// ---------
// ~PngImage
// ---------
/**
 * The destructor frees the allocated memory.
 */
PngImage::~PngImage()
{
	if (m_bgra)
	{
		delete[]  m_bgra;
	}
}


// ----
// load
// ----
/**
 * Reads a png file.
 *
 * @return true if the file operation succeeded
 * @return false if the file operation failed
 */
bool PngImage::load(const char* Path)
{
	// reset health flag
	m_good = false;

	// check filetype
	if (!doCheckFileHeader(Path)) return false;

	// try to open file
	FILE* file = NULL;
	
	if (fopen_s(&file, Path, "rb") != 0) return false;

	// unable to open
	if (file == 0) return false;

	// create read struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	// check pointer
	if (png_ptr == 0)
	{
		fclose(file);
		return false;
	}

	// create info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);

	// check pointer
    if (info_ptr == 0)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(file);
        return false;
    }

	// set error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(file);
		return false;
	}

	// I/O initialization using standard C streams
	png_init_io(png_ptr, file);

	// read entire image (high level)
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	// convert the png bytes to BGRA
	if (!doExtractCanonicData(png_ptr, info_ptr))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);

		fclose(file);
		return false;
	}

	// free memory
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	// close file
	fclose(file);

	// reset health flag
	return (m_good = true);
}

struct PngResourceInfo
{
	char *pData;
	DWORD dwMaxDataSize;
	DWORD dwDataPos;
};

void ReadPNGFromResource(png_structp pPngStruct, png_bytep pBuff, png_size_t bytesToRead);

bool PngImage::load(UINT uiResourceID)
{
	HGLOBAL hResourceLoaded;  // handle to loaded resource
    HRSRC   hRes;             // handle/ptr to res. info.
	PngResourceInfo *pPngInfo = new PngResourceInfo;

    hRes = FindResource(NULL, MAKEINTRESOURCE(uiResourceID), "BINARY");
    hResourceLoaded = LoadResource(NULL, hRes);
	pPngInfo->pData = (char *) LockResource(hResourceLoaded);
    pPngInfo->dwMaxDataSize = SizeofResource(NULL, hRes);
	pPngInfo->dwDataPos = 0;

    // pPngInfo->pData points to the binary data of the PNG
	// The size of the data is pPngInfo->dwMaxDataSize bytes.

	// create read struct
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	// check pointer
	if (png_ptr == 0)
	{
		UnlockResource(hResourceLoaded);
		return false;
	}

	// create info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);

	// check pointer
    if (info_ptr == 0)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
		UnlockResource(hResourceLoaded);
        return false;
    }

	// set error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		UnlockResource(hResourceLoaded);
		return false;
	}

	// Set up the custom load function for the png library
	png_set_read_fn(png_ptr, pPngInfo, ReadPNGFromResource);

	// read entire image (high level)
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	// convert the png bytes to BGRA
	if (!doExtractCanonicData(png_ptr, info_ptr))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		UnlockResource(hResourceLoaded);
		return false;
	}

	// free memory
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	// Unlock the resource
	UnlockResource(hResourceLoaded);

	// reset health flag
	return (m_good = true);
}

void ReadPNGFromResource(png_structp pPngStruct, png_bytep pBuff, png_size_t bytesToRead)
{
	if (!pPngStruct)
	{
		png_error(pPngStruct, "NULL png_struct input pointer");
		return;
	}

	if (!pBuff)
	{
		png_error(pPngStruct, "NULL output buffer pointer");
		return;
	}

	PngResourceInfo *pPngInfo = (PngResourceInfo *)pPngStruct->io_ptr;

	if (!pPngInfo)
	{
		png_error(pPngStruct, "NULL io_ptr pointer");
		return;
	}

	// Read bytesToRead from the input and copy them to pBuff
	char *pDest = (char*)pBuff;

	// Check we have enough bytes left in the stream
	if (pPngInfo->dwDataPos + bytesToRead <= pPngInfo->dwMaxDataSize)
	{
		memcpy(pBuff, pPngInfo->pData + pPngInfo->dwDataPos, bytesToRead);
		pPngInfo->dwDataPos += bytesToRead;
	}
	else
	{
		// Copy what we can
		memcpy(pBuff, pPngInfo->pData + pPngInfo->dwDataPos, pPngInfo->dwMaxDataSize - pPngInfo->dwDataPos);
		// Then report error
		png_error(pPngStruct, "End Of File");
	}
}

bool PngImage::GetBitmap(CBitmap &bitmap, CWnd* pCompatibleWindow)
{
	if (!good())
		return false;

	// get size
	int width  = getWidth();
	int height = getHeight();

	// get blue, green, red and alpha
	unsigned char* data = getBGRA();

	// create a CBitmap to display
	return (createCompatibleBitmap(width, height, data, bitmap, pCompatibleWindow));
}

// ------------------------
// createCompatibleBitmap
// ------------------------
/**
 * Creates a new bitmap.
 */
bool PngImage::createCompatibleBitmap(int Width, int Height, unsigned char* BGRA, CBitmap &bitmap, CWnd* pCompatibleWindow)
{
	bool bRV = true;

	// RGB + Alpha
	int size = Width * Height * 4;

	// get device context
	CDC* pDC = NULL;

	if (pCompatibleWindow)
		pDC = pCompatibleWindow->GetDC();
	else
		pDC = ::AfxGetMainWnd()->GetDC();

	ASSERT(bitmap.m_hObject == NULL);

	// initialize
	if (bitmap.CreateCompatibleBitmap(pDC, Width, Height) == TRUE)
	{
		// set pixels
		if (bitmap.SetBitmapBits(size, BGRA) == 0)
			bRV = false;
	}
	else
		bRV = false;

	// release device context
	if (pCompatibleWindow)
		pCompatibleWindow->ReleaseDC(pDC);
	else
		::AfxGetMainWnd()->ReleaseDC(pDC);

	return bRV;
}

// ----
// good
// ----
/**
 * Returns true if the object contains valid data.
 */
bool PngImage::good() const
{
	return m_good;
}


// --------
// getWidth
// --------
/**
 * Returns the image's width.
 */
int PngImage::getWidth() const
{
	return m_width;
}


// ---------
// getHeight
// ---------
/**
 * Returns the image's height.
 */
int PngImage::getHeight() const
{
	return m_height;
}


// -------
// getBGRA
// -------
/**
 * Returns the image's RGB+Alpha values (8 bit each).
 *
 * @par Memory
 * - Do not delete the returned pointer yourself. It will be deleted internally.
 *
 * @par Structure
 * - The color sequence is Blue-Green-Red-Alpha (8 bit each).
 * - The first 4 values (BGRA) are located in the top-left corner of the image.
 * - The last 4 values (BGRA) are located in the bottom-right corner of the image.
 * - The number of elements is always: getWidth() * getHeight() * 4
 *
 * @par Customizing the structure
 * If you constantly need another color sequence than BGRA (for example RGB),
 * it should be better (faster) to change the internal conversion routines
 * than to change the byte order in the returned array.
 *
 * @see
 * doConvertRGB8()
 * doConvertRGBA8()
 * doConvertGrey8()
 * doConvertGreyA8()
 */
unsigned char* PngImage::getBGRA() const
{
	return m_bgra;
}


// -----------------
// doCheckFileHeader
// -----------------
/**
 * Returns true if the specified file is a png file.
 */
bool PngImage::doCheckFileHeader(const char* Path) const
{
	// try to open file
	FILE* file = NULL;
	
	if (fopen_s(&file, Path, "rb") != 0) return false;

	// unable to open
	if (file == 0) return false;

	// the number of bytes to read
	const int nread = 8;

	unsigned char buffer[nread];

	// try to read header
	if (fread(buffer, 1, nread, file) != nread)
	{
		fclose(file);
		return 0;
	}

	// close file
	fclose(file);

	// compare headers
	int result = png_sig_cmp(buffer, 0, nread);

	return(result == 0);
}


// --------------------
// doExtractCanonicData
// --------------------
/**
 * Converts the png bytes to BGRA.
 *
 * @par Assumptions
 * - png_read_png() has been called with the PNG_TRANSFORM_EXPAND flag
 */
bool PngImage::doExtractCanonicData(png_structp& PngPtr, png_infop& InfoPtr)
{
	// check pointer
	if (m_bgra)
	{
		// free memory
		delete m_bgra;

		// reset pointer
		m_bgra = 0;
	}

	// get dimensions
	m_width = png_get_image_width(PngPtr, InfoPtr);
    m_height = png_get_image_height(PngPtr, InfoPtr);

	// get color information
	int color_type = png_get_color_type(PngPtr, InfoPtr);

	// rgb
	if (color_type == PNG_COLOR_TYPE_RGB)
	{
		m_bgra = doConvertRGB8(PngPtr, InfoPtr);
	}

	// rgb with opacity
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		m_bgra = doConvertRGBA8(PngPtr, InfoPtr);
	}

	// 256 grey values
	else if (color_type == PNG_COLOR_TYPE_GRAY)
	{
		m_bgra = doConvertGrey8(PngPtr, InfoPtr);
	}

	// 256 grey values with opacity
	else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		m_bgra = doConvertGreyA8(PngPtr, InfoPtr);
	}

	// check pointer
	return (m_bgra != 0);
}


// -------------
// doConvertRGB8
// -------------
/**
 * Gets the data from an 8-bit rgb image.
 *
 * @par Memory
 * - The returned pointer is created by using the new[] operator.
 * - You have to free the allocated memory yourself.
 *
 * @par Structure
 * - The color-sequence is Blue-Green-Red-Alpha (8 bit each).
 * - The first 4 values (RGBA) are located in the top-left corner of the image.
 * - The last 4 values (RGBA) are located in the bottom-right corner of the image.
 */
unsigned char* PngImage::doConvertRGB8(png_structp& PngPtr, png_infop& InfoPtr) const
{
	// calculate needed memory
	int size = m_height * m_width * 4;

	// allocate memory
	unsigned char* bgra = new unsigned char[size];

	// get image rows
	png_bytep* row_pointers = png_get_rows(PngPtr, InfoPtr);

	int pos = 0;

	// get color values
	for(int i = 0; i < m_height; i++)
	{
		for(int j = 0; j < (3 * m_width); j += 3)
		{
			bgra[pos++] = row_pointers[i][j + 2];	// blue
			bgra[pos++] = row_pointers[i][j + 1];	// green
			bgra[pos++] = row_pointers[i][j];		// red
			bgra[pos++] = 0;						// alpha
		}
	}

	return bgra;
}


// --------------
// doConvertRGBA8
// --------------
/**
 * Gets the data from an 8-bit rgb image with alpha values.
 *
 * @par Memory
 * - The returned pointer is created by using the new[] operator.
 * - You have to free the allocated memory yourself.
 *
 * @par Structure
 * - The color-sequence is Blue-Green-Red-Alpha (8 bit each).
 * - The first 4 values (RGBA) are located in the top-left corner of the image.
 * - The last 4 values (RGBA) are located in the bottom-right corner of the image.
 */
unsigned char* PngImage::doConvertRGBA8(png_structp& PngPtr, png_infop& InfoPtr) const
{
	// calculate needed memory
	int size = m_height * m_width * 4;

	// allocate memory
	unsigned char* bgra = new unsigned char[size];

	// get image rows
	png_bytep* row_pointers = png_get_rows(PngPtr, InfoPtr);

	int pos = 0;

	// get color values
	for(int i = 0; i < m_height; i++)
	{
		for(int j = 0; j < (4 * m_width); j += 4)
		{
			bgra[pos++] = row_pointers[i][j + 2];	// blue
			bgra[pos++] = row_pointers[i][j + 1];	// green
			bgra[pos++] = row_pointers[i][j];		// red
			bgra[pos++] = row_pointers[i][j + 3];	// alpha
		}
	}

	return bgra;
}


// --------------
// doConvertGrey8
// --------------
/**
 * Gets the data from an 8-bit monochrome image.
 *
 * @par Memory
 * - The returned pointer is created by using the new[] operator.
 * - You have to free the allocated memory yourself.
 *
 * @par Structure
 * - The color-sequence is Blue-Green-Red-Alpha (8 bit each).
 * - The first 4 values (RGBA) are located in the top-left corner of the image.
 * - The last 4 values (RGBA) are located in the bottom-right corner of the image.
 */
unsigned char* PngImage::doConvertGrey8(png_structp& PngPtr, png_infop& InfoPtr) const
{
	// calculate needed memory
	int size = m_height * m_width * 4;

	// allocate memory
	unsigned char* bgra = new unsigned char[size];

	// get image rows
	png_bytep* row_pointers = png_get_rows(PngPtr, InfoPtr);

	int pos = 0;

	// get color values
	for(int i = 0; i < m_height; i++)
	{
		for(int j = 0; j < m_width; j++)
		{
			bgra[pos++] = row_pointers[i][j];	// blue
			bgra[pos++] = row_pointers[i][j];	// green
			bgra[pos++] = row_pointers[i][j];	// red
			bgra[pos++] = 0;					// alpha
		}
	}

	return bgra;
}


// ---------------
// doConvertGreyA8
// ---------------
/**
 * Gets the data from an 8-bit monochrome image with alpha values.
 */
unsigned char* PngImage::doConvertGreyA8(png_structp& PngPtr, png_infop& InfoPtr) const
{
	// calculate needed memory
	int size = m_height * m_width * 4;

	// allocate memory
	unsigned char* bgra = new unsigned char[size];

	// get image rows
	png_bytep* row_pointers = png_get_rows(PngPtr, InfoPtr);

	int pos = 0;

	// get color values
	for(int i = 0; i < m_height; i++)
	{
		for(int j = 0; j < (2 * m_width); j += 2)
		{
			bgra[pos++] = row_pointers[i][j];		// blue
			bgra[pos++] = row_pointers[i][j];		// green
			bgra[pos++] = row_pointers[i][j];		// red
			bgra[pos++] = row_pointers[i][j + 1];	// alpha
		}
	}

	return bgra;
}
