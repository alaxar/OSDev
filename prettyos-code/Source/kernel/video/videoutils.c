/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "videoutils.h"
#include "util/util.h"
#include "video.h"
#include "font.h"
#include "kheap.h"
#include "console.h"

const BGRA_t black = {0, 0, 0, 0xFF};
const BGRA_t white = {0xFF, 0xFF, 0xFF, 0xFF};

static position_t curPos = {0, 0};


// Advanced and formatted drawing functionality
void video_drawLine(videoDevice_t* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, BGRA_t color)
{
    uint16_t dxabs = x2>x1 ? (x2-x1):(x1-x2); // the absolute horizontal distance of the line
    uint16_t dyabs = y2>y1 ? (y2 - y1) : (y1 - y2); // the absolute vertical distance of the line
    int32_t sdx = x2>x1 ? 1 : -1; // sign of the horizontal distance of the line
    int32_t sdy = y2>y1 ? 1 : -1; // sign of the horizontal distance of the line
    uint16_t x = dyabs / 2;
    uint16_t y = dxabs / 2;

    video_setPixel(buffer, x1, y1, color);

    if (dxabs >= dyabs) // the line is more horizontal than vertical
    {
        for (uint16_t i = 0; i < dxabs; i++)
        {
            y+=dyabs;
            if (y>=dxabs)
            {
                y-=dxabs;
                y1+=sdy;
            }
            x1+=sdx;
            video_setPixel(buffer, x1, y1, color);
        }
    }
    else // the line is more vertical than horizontal
    {
        for (uint16_t i = 0; i < dyabs; i++)
        {
            x+=dxabs;
            if (x>=dyabs)
            {
                x-=dyabs;
                x1+=sdx;
            }
            y1+=sdy;
            video_setPixel(buffer, x1, y1, color);
        }
    }
}

void video_drawRect(videoDevice_t* buffer, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, BGRA_t color)
{
    // Sort edges
    if (top>bottom)
    {
        uint16_t temp = top;
        top=bottom;
        bottom=temp;
    }
    if (left>right)
    {
        uint16_t temp = left;
        left=right;
        right=temp;
    }

    video_fillPixels(buffer, left, top, color, right-left);
    video_fillPixels(buffer, left, bottom, color, right-left);
    for (uint16_t i = top; i <= bottom; i++)
    {
        video_setPixel(buffer, left, i, color);
        video_setPixel(buffer, right, i, color);
    }
}

void video_drawRectFilled(videoDevice_t* buffer, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, BGRA_t color)
{
    for (uint16_t j = top; j < bottom; j++)
    {
        video_fillPixels(buffer, left, j, color, right-left);
    }
}

void video_drawCartesianCircle(videoDevice_t* buffer, uint16_t xm, uint16_t ym, uint16_t radius, BGRA_t color)
{
    for (uint16_t i = 0; i <= radius; i++)
    {
        uint16_t ydiff = sqrt((uint32_t)radius*(uint32_t)radius - (uint32_t)i*(uint32_t)i);
        video_setPixel(buffer, xm - i, ym + ydiff, color);
        video_setPixel(buffer, xm - i, ym - ydiff, color);
        video_setPixel(buffer, xm + i, ym + ydiff, color);
        video_setPixel(buffer, xm + i, ym - ydiff, color);
    }
}

void video_drawChar(videoDevice_t* buffer, char c)
{
    uint8_t uc = AsciiToCP437((uint8_t)c); // no negative values

    switch (uc)
    {
        case 0x08: // backspace: move the cursor one space backwards and delete
            /*move_cursor_left();
            putch(' ');
            move_cursor_left();*/
            break;
        case 0x09: // tab: increment cursor.x (divisible by 8)
            curPos.x = (curPos.x + fontWidth*8) & ~(fontWidth*8 - 1);
            break;
        case '\r': // r: cursor back to the margin
            curPos.x = 0;
            break;
        case '\n': // newline: like 'cr': cursor to the margin and increment cursor.y
            curPos.x = 0;
            curPos.y += fontHeight;
            break;
        default:
            if (uc >= fontFirstChar && uc <= fontLastChar)
            {
                uc -= fontFirstChar;
                for (uint16_t y = 0; y < fontHeight; y++)
                {
                    for (uint16_t x = 0; x < fontWidth; x++)
                    {
                        uint8_t value = font[(x + fontWidth*uc) + (fontHeight - y - 1) * (fontLastChar - fontFirstChar) * 8];
                        BGRA_t temp = { value, value, value, 0};
                        video_setPixel(buffer, curPos.x+x, curPos.y+y, temp);
                    }
                }
            }
            curPos.x += 8;
            break;
    }
}

void video_drawString(videoDevice_t* buffer, const char* text, uint16_t xpos, uint16_t ypos)
{
    curPos.x = xpos;
    curPos.y = ypos;
    for (; *text; video_drawChar(buffer, *text), ++text);
}

static uint8_t* getUncompressedPixelData(uint32_t width, uint32_t height, const uint8_t* data, size_t datasize)
{
    uint8_t* pixel = malloc(width*height, 0, "pixel");
    memset(pixel, 0, width*height);
    size_t pixel_idx = 0;
    size_t data_idx = 0;
    while (data_idx < datasize)
    {
        if (data[data_idx] != 0)
        {
            size_t num = data[data_idx];
            if (pixel_idx + num > width*height)
                break; // Error
            memset(pixel + pixel_idx, data[data_idx + 1], num);
            pixel_idx += num;
            data_idx += 2;
        }
        else
        {
            data_idx++;

            if (data[data_idx] == 0)
                data_idx++;
            else if (data[data_idx] == 1)
                break;
            else if (data[data_idx] == 2)
            {
                pixel_idx += data[data_idx + 1];
                pixel_idx += data[data_idx + 2]*width;
                data_idx += 3;
            }
            else
            {
                uint8_t num = data[data_idx];
                if (pixel_idx + num > width*height)
                    break; // Error
                data_idx++;
                memcpy(pixel + pixel_idx, data+data_idx, num);
                pixel_idx += num;
                data_idx += num;
                if (data_idx % 2 == 1)
                    data_idx++;
            }
        }
    }
    return pixel;
}

void video_drawBitmap(videoDevice_t* buffer, uint16_t xpos, uint16_t ypos, const BMPHeader_t* bitmap)
{
    uint8_t* pixel;
    if (bitmap->Compression == 0)      // uncompressed
        pixel = ((uint8_t*)bitmap) + bitmap->Offset;
    else if (bitmap->Compression == 1) // 8 bit RLE
        pixel = getUncompressedPixelData(bitmap->Width, bitmap->Height, ((uint8_t*)bitmap) + bitmap->Offset, bitmap->SizeImage);
    else                               // unsupported
        return;

    uint8_t* temp = pixel + bitmap->Width * bitmap->Height - 1;
    BGRA_t* palette = (void*)bitmap + 14 + bitmap->headerSize;

    for (uint32_t y = 0; y < bitmap->Height; y++)
    {
        for (uint32_t x = bitmap->Width; x > 0; x--)
        {
            video_setPixel(buffer, xpos + x, ypos + y, palette[*temp]);
            temp -= bitmap->BitsPerPixel / 8;
        }
    }

    if (bitmap->Compression == 1)
        free(pixel);
}

void video_drawBitmapTransparent(videoDevice_t* buffer, uint16_t xpos, uint16_t ypos, const BMPHeader_t* bitmap, BGRA_t colorKey)
{
    uint8_t* pixel;
    if (bitmap->Compression == 0)      // uncompressed
        pixel = ((uint8_t*)bitmap) + bitmap->Offset;
    else if (bitmap->Compression == 1) // 8 bit RLE
        pixel = getUncompressedPixelData(bitmap->Width, bitmap->Height, ((uint8_t*)bitmap) + bitmap->Offset, bitmap->SizeImage);
    else                               // unsupported
        return;

    uint8_t* temp = pixel + bitmap->Width * bitmap->Height - 1;
    BGRA_t* palette = (void*)bitmap + 14 + bitmap->headerSize;

    for (uint32_t y=0; y<bitmap->Height; y++)
    {
        for (uint32_t x=bitmap->Width; x>0; x--)
        {
            if (palette[*temp].red != colorKey.red || palette[*temp].green != colorKey.green || palette[*temp].blue != colorKey.blue)
            {
                video_setPixel(buffer, xpos+x, ypos+y, palette[*temp]);
            }
            temp -= bitmap->BitsPerPixel/8;
        }
    }

    if (bitmap->Compression == 1)
        free(pixel);
}

void video_drawScaledBitmap(videoDevice_t* buffer, uint16_t newSizeX, uint16_t newSizeY, const BMPHeader_t* bitmap)
{
    uint8_t* pixel;
    if (bitmap->Compression == 0)      // uncompressed
        pixel = ((uint8_t*)bitmap) + bitmap->Offset;
    else if (bitmap->Compression == 1) // 8 bit RLE
        pixel = getUncompressedPixelData(bitmap->Width, bitmap->Height, ((uint8_t*)bitmap) + bitmap->Offset, bitmap->SizeImage);
    else                               // unsupported
        return;

    uint8_t* temp = pixel + bitmap->Width * bitmap->Height - 1;
    BGRA_t* palette = (void*)bitmap + 14 + bitmap->headerSize;

    float FactorX = (float)newSizeX / (float)bitmap->Width;
    float FactorY = (float)newSizeY / (float)bitmap->Height;

    float OverflowX = 0, OverflowY = 0;
    for (uint16_t y = 0; y < newSizeY; y += (uint16_t)FactorY)
    {
        for (int16_t x = newSizeX - 1; x >= 0; x -= (int16_t)FactorX)
        {
            temp -= bitmap->BitsPerPixel/8;

            uint16_t rowsX = (uint16_t)FactorX + (OverflowX >= 1 ? 1:0);
            uint16_t rowsY = (uint16_t)FactorY + (OverflowY >= 1 ? 1:0);
            for (uint16_t i = 0; i < rowsX; i++)
            {
                for (uint16_t j = 0; j < rowsY; j++)
                {
                    if (x-i >= 0)
                    {
                        video_setPixel(buffer, x-i, y+j, palette[*temp]);
                    }
                }
            }

            if (OverflowX >= 1)
            {
                OverflowX--;
                x--;
            }
            OverflowX += FactorX-(int)FactorX;
        }
        if (OverflowY >= 1)
        {
            OverflowY--;
            y++;
        }
        OverflowY += FactorY-(int)FactorY;
    }

    if (bitmap->Compression == 1)
        free(pixel);
}

void video_printPalette(videoDevice_t* device)
{
    if (device->videoMode.colorMode != CM_256COL) return;

    uint16_t xpos = 0;
    uint16_t ypos = 0;
    for (uint16_t j=0; j<256; j++)
    {
        video_drawRectFilled(device, xpos, ypos, xpos+5, ypos+5, device->videoMode.palette[j]);
        xpos +=5;
        if (xpos >= 255)
        {
            ypos += 5;
            xpos = 0;
        }
    }
}


// Renderbuffer
renderBuffer_t* renderBuffer_create(uint16_t width, uint16_t height, uint8_t bitsPerPixel)
{
    renderBuffer_t* buffer = malloc(sizeof(renderBuffer_t), 0, "renderBuffer_t");
    buffer->device = video_createDevice(video_drivers + VDD_RENDERBUFFER);
    buffer->device->videoMode.colorMode = bitsPerPixel;
    buffer->device->videoMode.xRes = width;
    buffer->device->videoMode.yRes = height;
    buffer->device->data = buffer;
    buffer->buffer = malloc(width*height*bitsPerPixel/8, 0, "renderBuffer_t::buffer");
    buffer->buffer2 = 0;
    return (buffer);
}

void renderBuffer_free(renderBuffer_t* buffer)
{
    free(buffer->buffer);
    free(buffer->buffer2);
    video_freeDevice(buffer->device);
    free(buffer);
}

void renderBuffer_render(videoDevice_t* destination, renderBuffer_t* buffer, uint16_t x, uint16_t y)
{
    for (uint16_t i = 0; i < buffer->device->videoMode.yRes; i++)
    {
        video_copyPixels(destination, x, y+i, buffer->buffer + 4*buffer->device->videoMode.xRes*i, buffer->device->videoMode.xRes); // 32-bit hack
    }
}

void renderBuffer_setPixel(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color)
{
    renderBuffer_t* buffer = device->data;

    if (buffer)
    {
        switch (device->videoMode.colorMode)
        {
            case CM_15BIT:
                ((uint16_t*)buffer->buffer)[y * device->videoMode.xRes + x] = BGRAtoBGR15(color);
                break;
            case CM_16BIT:
                ((uint16_t*)buffer->buffer)[y * device->videoMode.xRes + x] = BGRAtoBGR16(color);
                break;
            case CM_24BIT:
                *(uint16_t*)(buffer->buffer + (y * device->videoMode.xRes + x) * 3) = *(uint16_t*)&color; // Performance Hack - copying 16 bits at once should be faster than copying 8 bits twice
                ((uint8_t*)buffer->buffer)[(y * device->videoMode.xRes + x) * 3 + 2] = color.red;
                break;
            case CM_32BIT:
                ((uint32_t*)buffer->buffer)[y * device->videoMode.xRes + x] = *(uint32_t*)&color;
                break;
            case CM_256COL: default:
                ((uint8_t*)buffer->buffer)[y * device->videoMode.xRes + x] = BGRAtoBGR8(color);
                break;
        }
    }
    else
    {
        printfe("\nbuffer error at renderBuffer_setPixel");
    }
}

void renderBuffer_fillPixels(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color, size_t num)
{
    renderBuffer_t* buffer = device->data;
    switch (device->videoMode.colorMode)
    {
        case CM_15BIT:
            memsetw(((uint16_t*)buffer->buffer) + y * device->videoMode.xRes + x, BGRAtoBGR15(color), num);
            break;
        case CM_16BIT:
            memsetw(((uint16_t*)buffer->buffer) + y * device->videoMode.xRes + x, BGRAtoBGR16(color), num);
            break;
        case CM_24BIT:
        {
            void* vidmemBase = buffer->buffer + (y * device->videoMode.xRes + x) * 3;
            for (size_t i = 0; i < num; i++)
            {
                *(uint16_t*)(vidmemBase + i*3) = *(uint16_t*)&color; // Performance Hack - copying 16 bits at once should be faster than copying 8 bits twice
                ((uint8_t*)vidmemBase)[i*3 + 2] = color.red;
            }
            break;
        }
        case CM_32BIT:
            memsetl(((uint32_t*)buffer->buffer) + y * device->videoMode.xRes + x, *(uint32_t*)&color, num);
            break;
        case CM_256COL: default:
            memset(buffer->buffer + y * device->videoMode.xRes + x, BGRAtoBGR8(color), num);
            break;
    }
}

void renderBuffer_copyPixels(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t* colors, size_t num)
{
    renderBuffer_t* buffer = device->data;
    switch (device->videoMode.colorMode)
    {
        case CM_32BIT:
            memcpy(buffer->buffer + 4*device->videoMode.xRes*y + x*4, colors, num);
            break;
        case CM_15BIT: case CM_16BIT: case CM_24BIT: case CM_256COL: default:
            for (size_t i = 0; i < num; i++)
                renderBuffer_setPixel(device, x + i, y, colors[i]);
            break;
    }
}

void renderBuffer_clear(videoDevice_t* device, BGRA_t color)
{
    renderBuffer_fillPixels(device, 0, 0, color, device->videoMode.xRes*device->videoMode.yRes);
}

BGRA_t renderBuffer_getPixel(const videoDevice_t* device, uint16_t x, uint16_t y)
{
    const renderBuffer_t* buffer = device->data;
    return ((BGRA_t*)buffer->buffer)[y * device->videoMode.colorMode + x]; // HACK
}

void renderBuffer_flipScreen(videoDevice_t* device)
{
    renderBuffer_t* buffer = device->data;
    if (buffer->buffer2 == 0)
        buffer->buffer2 = malloc(device->videoMode.xRes*device->videoMode.yRes*device->videoMode.colorMode/8, 0, "renderBuffer_t::buffer2");

    void* temp = buffer->buffer2;
    buffer->buffer2 = buffer->buffer;
    buffer->buffer = temp;
}

// http://de.wikipedia.org/wiki/YUV-Farbmodell#Prinzip_des_Farbmodells
YUV_t RGBtoYUV(BGRA_t rgb)
{
    YUV_t yuv;

    yuv.y = 0.299f * (float)rgb.red + 0.587f * (float)rgb.green + 0.114f * (float)rgb.blue;
    yuv.u = 0.492f * ((float)rgb.blue - yuv.y);
    yuv.v = 0.877f * ((float)rgb.red  - yuv.y);

    return yuv;
}

BGRA_t YUVtoRGB(YUV_t yuv)
{
    BGRA_t rgb;

    // Calculate using integers:
    int c = yuv.y - 16;
    int d = yuv.u - 128;
    int e = yuv.v - 128;

    rgb.red   = clamp(0, (298 * c + 409 * e + 128)           >> 8, 255);
    rgb.green = clamp(0, (298 * c - 100 * d - 208 * e + 128) >> 8, 255);
    rgb.blue  = clamp(0, (298 * c + 516 * d + 128)           >> 8, 255);
    rgb.alpha = 255;

    return rgb;
}

void showYUYV(videoDevice_t* device, uint16_t height, uint16_t width, uint8_t* data, size_t dataSize)
{
    size_t i = 0;
    for (uint16_t y = 0; y < height; y++)
    {
        for (uint16_t x = 0; x < (width-1); x+=2)
        {
            YUV_t yuv1, yuv2;
            // each 4 bytes is two Y's, a U and a V.
            // each Y goes to one of the pixels, and the U and V belong to both pixels.

            yuv1.y =          data[i+0];
            yuv1.u = yuv2.u = data[i+1];
            yuv2.y =          data[i+2];
            yuv1.v = yuv2.v = data[i+3];

            renderBuffer_setPixel(device, x,   y, YUVtoRGB(yuv1));
            renderBuffer_setPixel(device, x+1, y, YUVtoRGB(yuv2));

            i+=4;
            if (i>dataSize)
            {
                break;
            }
        }
    }
}


/*
* Copyright (c) 2011-2016 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
