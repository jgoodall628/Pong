/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jeffrey Goodall $
   ======================================================================== */

#include <windows.h>
#include <stdint.h>
#include <Dsound.h>
#include <math.h>
#include <stdio.h>
#include <windowsx.h>
#include "win32_pong.h"
#include "game_pong.cpp"



static int xoffset;

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)



#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);



static ScreenBuffer MainScreenBuffer;
static bool Running;
static bool Playing;

static void InitGameSound(Sound_Data *SoundData, game_sound_data *GameSoundData, int BytesToWrite)
{
    if(GameSoundData->Samples)
    {
        VirtualFree(GameSoundData->Samples, 0, MEM_RELEASE);
    }
    
    GameSoundData->SamplesPerSecond = SoundData->SamplesPerSecond;
    GameSoundData->SampleCount = BytesToWrite / SoundData->BytesPerSample;
    GameSoundData->Samples = (int16_t *)VirtualAlloc(0,
                                                    SoundData->BufferSize,
                                                    MEM_RESERVE|MEM_COMMIT,
                                                    PAGE_READWRITE);
    
    
}

static void ProcessMessageQueue(game_controller_input *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                Running = false;
            } break;
            case WM_MOUSEMOVE:
            {
                KeyboardController->Mouse.MouseY = GET_Y_LPARAM(Message.lParam);
                
                
                KeyboardController->Mouse.MouseX = GET_X_LPARAM(Message.lParam); 
            }
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
        
            {
                uint32_t VKCode = (uint32_t)Message.wParam;
                bool WasDown = (Message.lParam & (1 << 30)) != 0;
                bool IsDown = (Message.lParam & (1 << 31)) == 0;
                if(WasDown != IsDown)
                {
                                        
                    if(VKCode == 'W')
                    {
                           
                        KeyboardController->MoveUp.EndedDown = IsDown;
                        KeyboardController->MoveUp.transitions++;
                        if(KeyboardController->MoveUp.EndedDown)
                        {
                            OutputDebugStringA("DownW\n");
                        }
                        else
                        {
                            OutputDebugStringA("UpW\n");

                        }
                            
                    }
                    if(VKCode == 'S')
                    {
                        KeyboardController->MoveDown.EndedDown = IsDown;
                        KeyboardController->MoveDown.transitions++;
                        if(KeyboardController->MoveDown.EndedDown)
                        {
                            OutputDebugStringA("DownA\n");
                        }
                        else
                        {
                            OutputDebugStringA("UpA\n");

                        }
                    }
                    if(VKCode == 'A')
                    {
                        KeyboardController->MoveLeft.EndedDown = IsDown;
                        KeyboardController->MoveLeft.transitions++;
                        if(KeyboardController->MoveLeft.EndedDown)
                        {
                            OutputDebugStringA("DownA\n");
                        }
                        else
                        {
                            OutputDebugStringA("UpA\n");

                        }
                    }
                    if(VKCode == 'D')
                    {
                        KeyboardController->MoveRight.EndedDown = IsDown;
                        KeyboardController->MoveRight.transitions++;
                        if(KeyboardController->MoveRight.EndedDown)
                        {
                            OutputDebugStringA("DownA\n");
                        }
                        else
                        {
                            OutputDebugStringA("UpA\n");

                        }
                    }
                  
                }
                                 
            }
         
        }
        
                    
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
}

static LARGE_INTEGER GetTimeCount()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

static uint64_t GetPerformanceFrequency()
{
    LARGE_INTEGER QueryPerfFrequency;
    QueryPerformanceFrequency(&QueryPerfFrequency);
    return QueryPerfFrequency.QuadPart;

}

static float GetTimeElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    uint64_t CountDiff = End.QuadPart - Start.QuadPart;
    uint64_t PerfFrequency = GetPerformanceFrequency();
    float Result = CountDiff/(float)PerfFrequency;
    return Result;
}



    

static void FillSoundBuffer(Sound_Data *SoundData, game_sound_data *GameSoundData, int ByteToLock, int BytesToWrite)
{
    LPVOID AudioPtr1;
    LPVOID AudioPtr2;
    DWORD AudioRegion1;
    DWORD AudioRegion2;
    if(SUCCEEDED(SoundData->SecondaryBuffer->Lock(ByteToLock,
                                                  BytesToWrite,
                                                  &AudioPtr1,
                                                  &AudioRegion1,
                                                  &AudioPtr2,
                                                  &AudioRegion2,
                                                  0)))
    {
        DWORD Region1SampleCount =AudioRegion1/SoundData->BytesPerSample;
        DWORD Region2SampleCount = AudioRegion2/SoundData->BytesPerSample;
        int16_t *DestinationSample = (int16_t *)AudioPtr1;
        int16_t *SourceSample = (int16_t *)GameSoundData->Samples;

        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            *DestinationSample++ = *SourceSample++;
            *DestinationSample++ = *SourceSample++;
            ++SoundData->RunningSampleIndex;
        }

        DestinationSample = (int16_t *)AudioPtr2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            *DestinationSample++ = *SourceSample++;
            *DestinationSample++ = *SourceSample++;
            ++SoundData->RunningSampleIndex;
        }

        SoundData->SecondaryBuffer->Unlock(AudioPtr1,
                                           AudioRegion1,
                                           AudioPtr2,
                                           AudioRegion2);
    }

}


   
    



static RectDim GetRectWidthHeight(RECT Rectangle)
{
    RectDim Dimensions;
    Dimensions.Width = Rectangle.right -  Rectangle.left;
    Dimensions.Height = Rectangle.bottom - Rectangle.top;
    return(Dimensions);
}
void InitDSound(HWND Window, int32_t BufferSize, Sound_Data *SoundData)
{
    HMODULE DsoundLibrary = LoadLibrary("Dsound.dll");

    if(DsoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DsoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDesc = {};
                BufferDesc.dwSize = sizeof(BufferDesc);
                BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                      
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                
                WAVEFORMATEX WaveFormat = {};
                WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
                WaveFormat.nChannels = 2;
                WaveFormat.nSamplesPerSec = SoundData->SamplesPerSecond;
                WaveFormat.wBitsPerSample = 16;
                WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample)/8 ;
                WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign*WaveFormat.nSamplesPerSec;
            
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDesc, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {                        
                    }
                    else
                    {
                    }                    
                }
                else
                {
                } 
                
                DSBUFFERDESC SecBufferDesc = {};
                SecBufferDesc.dwSize = sizeof(SecBufferDesc);
                SecBufferDesc.dwFlags = 0;
                SecBufferDesc.dwBufferBytes= BufferSize;
                SecBufferDesc.lpwfxFormat = &WaveFormat;
                            
                if(SUCCEEDED( DirectSound->CreateSoundBuffer(&SecBufferDesc, &SoundData->SecondaryBuffer, 0)))
                {
                }
                else
                {
                }               
            }
            else
            {
            }            
        }
        else
        {
        }        
    }        
}


static void UpdateScreen(ScreenBuffer *MainScreenBuffer,
                         HDC DeviceContext)
{
    StretchDIBits(DeviceContext,
                  0, 0,
                  MainScreenBuffer->BitmapWidth,
                  MainScreenBuffer->BitmapHeight,
                  0, 0,
                  MainScreenBuffer->BitmapWidth,
                  MainScreenBuffer->BitmapHeight,
                  MainScreenBuffer->BitmapMemory,
                  &MainScreenBuffer->BitmapInfo,  
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

void ResizeClientBuffer(ScreenBuffer *MainScreenBuffer, LONG Width, LONG Height)
{
    if(MainScreenBuffer->BitmapMemory)
    {
        VirtualFree(MainScreenBuffer->BitmapMemory, 0, MEM_RELEASE);
    }

    MainScreenBuffer->BitmapWidth = Width;
    MainScreenBuffer->BitmapHeight = Height;

    MainScreenBuffer->BytesPerPixel = 4;
    MainScreenBuffer->BitmapInfo.bmiHeader.biSize = sizeof(MainScreenBuffer->BitmapInfo.bmiHeader);
    MainScreenBuffer->BitmapInfo.bmiHeader.biWidth = MainScreenBuffer->BitmapWidth;
    MainScreenBuffer->BitmapInfo.bmiHeader.biHeight = -MainScreenBuffer->BitmapHeight;
    MainScreenBuffer->BitmapInfo.bmiHeader.biPlanes = 1;
    MainScreenBuffer->BitmapInfo.bmiHeader.biBitCount = 32;
    MainScreenBuffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = (MainScreenBuffer->BitmapWidth*MainScreenBuffer->BitmapHeight)*MainScreenBuffer->BytesPerPixel;
    MainScreenBuffer->BitmapMemory = VirtualAlloc(0,
                                                  BitmapMemorySize,
                                                  MEM_RESERVE|MEM_COMMIT,
                                                  PAGE_READWRITE);
    MainScreenBuffer->Pitch = Width*MainScreenBuffer->BytesPerPixel;
}

LRESULT CALLBACK MainWindowCallback(HWND hwnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    LRESULT Result = 0;
    
    switch(uMsg)
    {       
        case WM_CLOSE:
        {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DisplayDevice = BeginPaint(hwnd, &Paint);
      
            RECT ScreenRect;
            GetClientRect(hwnd, &ScreenRect);
            
            UpdateScreen(&MainScreenBuffer, DisplayDevice);    
            EndPaint(hwnd, &Paint);
        }

        default:
        {
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
    
    WNDCLASS WindowClass = {};
    
    ResizeClientBuffer(&MainScreenBuffer, 512, 512);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "PongWindowClass";
 
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Pong",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                hInstance,
                0);
        if(Window)
        {
            float TargetFPS = 30;
            float TargetSPF = 1/TargetFPS;
            
         

            Sound_Data SoundData= {};
            SoundData.SamplesPerSecond = 48000;
            SoundData.BytesPerSample = sizeof(int16_t)*2;
            SoundData.BufferSize = SoundData.SamplesPerSecond*SoundData.BytesPerSample;
            SoundData.SafetyBytes = (int)(SoundData.SamplesPerSecond*SoundData.BytesPerSample*TargetSPF/2.0f);
            InitDSound(Window, SoundData.BufferSize, &SoundData);

            game_sound_data GameSoundData = {};
            game_memory Memory = {};
            Memory.PermanentStorageSize = Megabytes(64);
            Memory.PermanentStorage = VirtualAlloc(0,
                                                   Memory.PermanentStorageSize,
                                                   MEM_RESERVE|MEM_COMMIT,
                                                   PAGE_READWRITE);
            
                       
            Running = true;
            Playing = false;
            bool SoundInitialized = false;
            LARGE_INTEGER PrevTime = GetTimeCount();
            LARGE_INTEGER EndTime;
            int SleepResolution = 1;
            bool FineSleep = false;
            if(SUCCEEDED(timeBeginPeriod(SleepResolution)))
            {
                FineSleep = true;
            }
            
            game_controller_input KeyboardController = {};
            while(Running)
            {
                              
                
                ProcessMessageQueue(&KeyboardController);
                
                
                
                
                game_screen_buffer GameScreenBuffer = {};
                GameScreenBuffer.Memory = MainScreenBuffer.BitmapMemory;
                GameScreenBuffer.Height = MainScreenBuffer.BitmapHeight;
                GameScreenBuffer.Width = MainScreenBuffer.BitmapWidth;
                GameScreenBuffer.Pitch = MainScreenBuffer.Pitch;
                GameScreenBuffer.BytesPerPixel = MainScreenBuffer.BytesPerPixel;

                int BytesToWrite = 0;
                DWORD ByteToLock = 0;
                DWORD TargetCursor = 0;
                DWORD PlayCursor= 0;
                DWORD WriteCursor= 0;
                DWORD SafeWriteCursor = 0;
                LARGE_INTEGER TimeBeforeAudio = GetTimeCount();
                if(SUCCEEDED(SoundData.SecondaryBuffer-> GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    if(!SoundInitialized)
                    {
                        SoundData.RunningSampleIndex = WriteCursor;
                        SoundInitialized = true;
                    }
                        
                    ByteToLock = SoundData.BytesPerSample*SoundData.RunningSampleIndex % SoundData.BufferSize;
                    SafeWriteCursor = WriteCursor;
                    DWORD SoundBytesPerFrame = (DWORD)(TargetSPF*SoundData.SamplesPerSecond*SoundData.BytesPerSample);
                    float SecondsToFlip = TargetSPF - GetTimeElapsed(PrevTime, TimeBeforeAudio);
                    DWORD BytesToFlip = (DWORD)(SecondsToFlip*SoundData.SamplesPerSecond*SoundData.BytesPerSample);

                    DWORD FrameBoundaryByte = PlayCursor + BytesToFlip;

                    if(SafeWriteCursor < PlayCursor)
                    {
                        SafeWriteCursor += SoundData.BufferSize;
                    }

                    bool IsLowLatency = (SafeWriteCursor < FrameBoundaryByte);
                        
                    if(IsLowLatency)
                    {
                        TargetCursor = (FrameBoundaryByte + SoundBytesPerFrame);
                    }
                    else
                    {
                        TargetCursor = (WriteCursor + SoundBytesPerFrame + SoundData.SafetyBytes);
                    }

                    TargetCursor = TargetCursor % SoundData.BufferSize;
                    if(ByteToLock > TargetCursor)
                    {
                        BytesToWrite = SoundData.BufferSize - ByteToLock;
                        int BytesToWriteMod = BytesToWrite % SoundData.BytesPerSample;
                        Assert(BytesToWriteMod == 0);
                        BytesToWrite += TargetCursor;
                    }
                 
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }
                    InitGameSound(&SoundData, &GameSoundData, BytesToWrite);
                    
                    GameUpdateRender(GameSoundData, GameScreenBuffer, KeyboardController, &Memory, TargetSPF);
                   
                    FillSoundBuffer(&SoundData, &GameSoundData, ByteToLock, BytesToWrite);
                    
                    if(!Playing)
                    {
                        SoundData.SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
                        Playing = true;
                    }
                   
                }
               
                
                HDC DeviceContext = GetDC(Window);
                RECT ScreenRect;
                GetClientRect(Window, &ScreenRect);
                UpdateScreen(&MainScreenBuffer, DeviceContext);
                ReleaseDC(Window, DeviceContext);                  
                EndTime = GetTimeCount();
                float TimeElapsedSeconds = GetTimeElapsed(PrevTime, EndTime);

                
                if(TimeElapsedSeconds < TargetSPF)
                {
                    if(FineSleep)
                    {
                        DWORD TimeToSleep = (DWORD)((TargetSPF - TimeElapsedSeconds)*1000 - 1);
                        if(TimeToSleep > 0)
                        {
                            Sleep(TimeToSleep);
                        }
                    }
                    
                    while(TimeElapsedSeconds < TargetSPF)
                    {
                        TimeElapsedSeconds = GetTimeElapsed(PrevTime, GetTimeCount());
                    }
                  
                }
                else
                {
                    // OutputDebugStringA("Missed Target\n"); 
                }
                

            
                    
                
                char FPSBuffer[256];
                _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                            "%.02f  mouse/f\n", KeyboardController.Mouse.MouseX);
                //OutputDebugStringA(FPSBuffer);
                PrevTime = EndTime;

                
            }
            if(FineSleep)
            {
                timeEndPeriod(SleepResolution);
            }
        }
        else
        {
        }
        
    }
    else
    {
    }
    
    return(0);
}
