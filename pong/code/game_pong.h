#if !defined(GAME_PONG_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jeffrey Goodall $
   ======================================================================== */

#define PI 3.14159265f
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#include "game_math.h"

struct game_sound_data
{
    uint32_t SampleCount;
    int SamplesPerSecond;
    int16_t *Samples;
};

struct tile_map
{
    int *map;
    vect2 TileDimensions;
    intvect2 TileCount;
    
};
struct game_screen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct game_controller_state
 {
    bool EndedDown;
    int transitions;
    
};
struct mouse_input
{
    int MouseX;
    int MouseY;
};
struct relative_position
{
    vect2 relcoord;
    intvect2 Tilecoord;
    
    
};

struct player
{
    relative_position Position;
    vect2 Dimensions;
    vect2 Velocity;
};
struct game_controller_input
{
    union
    {
        game_controller_state Buttons[4];
        struct
        {
            game_controller_state MoveUp;
            game_controller_state MoveDown;
            game_controller_state MoveLeft;
            game_controller_state MoveRight;
            mouse_input Mouse;
        };
    };
};
struct game_state
{
    player Player;
};
struct game_memory
{
    bool IsInitialized;
    int PermanentStorageSize;
    void *PermanentStorage;
};



#define GAME_PONG_H
#endif
