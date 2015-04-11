#if !defined(WIN32_PONG_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jeffrey Goodall $
   ======================================================================== */
struct ScreenBuffer
{
    BITMAPINFO BitmapInfo;
    void *BitmapMemory;
    int BitmapWidth;
    int BitmapHeight;
    int Pitch;
    int BytesPerPixel;
   
};

struct RectDim
{
    int Width;
    int Height;
};

    
struct Sound_Data
{
    
    int BytesPerSample;
    int RunningSampleIndex;
    LPDIRECTSOUNDBUFFER SecondaryBuffer;
    int SamplesPerSecond;
    DWORD LatencySampleCount;
    int BufferSize;
    int SafetyBytes;
    
};
#define WIN32_PONG_H
#endif

