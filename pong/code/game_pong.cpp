/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jeffrey Goodall $
   ======================================================================== */

#include "game_pong.h"


static int RoundToInt(float X)
{
    int Result = (int)round(X);
    return Result;
}
static int FloorFloat(float X)
{
    int Result = (int)floor(X);
    return Result;
}

static vect2 GetPixelPositionFromRelativePosition(relative_position Position, tile_map TileMap)
{
    vect2 PixelPosition;
    PixelPosition.X = Position.Tilecoord.X*TileMap.TileDimensions.X + Position.relcoord.X;
    PixelPosition.Y = Position.Tilecoord.Y*TileMap.TileDimensions.Y + Position.relcoord.Y;
    return PixelPosition;
}

static relative_position PixelToRelativePosition(vect2 PixelPosition, tile_map TileMap)
{
    relative_position RelativePosition;
    RelativePosition.Tilecoord.X = (int32_t)(PixelPosition.X/TileMap.TileDimensions.X);
    RelativePosition.Tilecoord.Y = (int32_t)(PixelPosition.Y/TileMap.TileDimensions.Y);
    RelativePosition.relcoord.X =  PixelPosition.X - RelativePosition.Tilecoord.X*TileMap.TileDimensions.X;
    RelativePosition.relcoord.Y = PixelPosition.Y - RelativePosition.Tilecoord.Y*TileMap.TileDimensions.Y;
    return RelativePosition;
}
static void SineWave(game_sound_data *GameSoundData, game_controller_input KeyboardController)
{
    if(KeyboardController.MoveUp.EndedDown)
    {
    }
    
    int Frequency = 512;
    int Volume = 3000;
    int WavePeriod = GameSoundData->SamplesPerSecond/Frequency;
    int16_t *SampleToBeWritten = GameSoundData->Samples;
    static float tSine = 0;

    for(DWORD SampleIndex = 0; SampleIndex < GameSoundData->SampleCount; ++SampleIndex)
    {
        
       
        int16_t SampleValue = (int16_t)(sinf(tSine)*Volume);
        *SampleToBeWritten++ = SampleValue;
        *SampleToBeWritten++ = SampleValue;
        if(tSine > 2.0f*PI)
        {
            tSine = tSine - 2*(float)PI;
        }
        tSine += 2.0f*PI/(float)WavePeriod;
             
    };
}

static void RenderGradient(game_screen_buffer *GameScreenBuffer, int BlueOffset, int GreenOffset, game_controller_input KeyboardController)
{
    int shift = 8;
    if(KeyboardController.MoveUp.EndedDown && KeyboardController.MoveUp.transitions)
    {
        shift = 16;
    }
    uint8_t *Row = (uint8_t *)GameScreenBuffer->Memory;    
    for(int Y = 0;
        Y < GameScreenBuffer->Height;
        ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0;
            X < GameScreenBuffer->Width;
            ++X)
        {
            uint8_t Blue =(uint8_t) (X + BlueOffset);
            uint8_t Green = (uint8_t)(Y + GreenOffset);
            
            *Pixel++ = ((Green << shift ) | Blue);
            
        }
        Row += GameScreenBuffer->Pitch;
    }
}


static void DrawRectangle(float left, float right, float top, float bottom, game_screen_buffer *GameScreenBuffer, int color)
{
    
    int MinX = RoundToInt(left);
    int MinY = RoundToInt(top);
    int MaxX = RoundToInt(right);
    int MaxY = RoundToInt(bottom);
 

    if(MinX < 0)
    {
        MinX = 0;
    }
    if(MaxX > GameScreenBuffer->Width)
    {
        MaxX = GameScreenBuffer->Width;
    }
    if(MinY < 0)
    {
        MinY = 0;
    }
    if(MaxY > GameScreenBuffer->Height)
    {
        MaxY = GameScreenBuffer->Height;
    }
    
     uint8_t *Row = ((uint8_t *)GameScreenBuffer->Memory +
                          MinX*GameScreenBuffer->BytesPerPixel +
                          MinY*GameScreenBuffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
       
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            
            
            *Pixel++ = color;
            
        }
        Row +=  GameScreenBuffer->Pitch;
    }
    
        
}
static void DrawTileMap (tile_map TileMap, game_screen_buffer *GameScreenBuffer)
{
    for(int Y=0;
        Y < TileMap.TileCount.Y;
        ++Y)
    {
    
        for(int X = 0;
            X < TileMap.TileCount.X;
            ++X)
        {
            if(TileMap.map[X + Y*TileMap.TileCount.X] == 1)
            {
                float left = X*TileMap.TileDimensions.X;
                float right = left + TileMap.TileDimensions.X;
                float bottom = GameScreenBuffer->Height - Y*TileMap.TileDimensions.Y;
                float top = bottom - TileMap.TileDimensions.Y;
                
                DrawRectangle(left,
                              right,
                              top,
                              bottom,                       
                              GameScreenBuffer, 0x00FF0000);
            }
        }
        
    }
}
static relative_position DetermineNewTile(relative_position RelativePosition, tile_map TileMap)
{
 
    int TileChangeX = RoundToInt(RelativePosition.relcoord.X/TileMap.TileDimensions.X);
    RelativePosition.Tilecoord.X += TileChangeX;
    RelativePosition. relcoord.X -= TileChangeX*(int)TileMap.TileDimensions.X;

    int TileChangeY = RoundToInt(RelativePosition.relcoord.Y/TileMap.TileDimensions.Y);
    RelativePosition.Tilecoord.Y += TileChangeY;
    RelativePosition. relcoord.Y -= TileChangeY*TileMap.TileDimensions.Y;
    return RelativePosition;
    
}
static bool TileIsEmpty(tile_map TileMap, relative_position RelativePosition)
{
    
    if(TileMap.map[RelativePosition.Tilecoord.X +
                   TileMap.TileCount.X*RelativePosition.Tilecoord.Y ] == 1)
    {
        return false;
    }
    else
    {
        return true;
    }
    
}

static relative_position GetPlayerTopRight(player Player, tile_map TileMap)
{
    
    relative_position Result = Player.Position;
    Result.relcoord += Player.Dimensions;
    Result = DetermineNewTile(Result, TileMap);
    return(Result);
    
    

}
static void DEBUGDrawContainerTile(relative_position RelativePosition, tile_map TileMap, game_screen_buffer *GameScreenBuffer)
{
    float left = RelativePosition.Tilecoord.X*TileMap.TileDimensions.X;
    float right = left + TileMap.TileDimensions.X;
    float bottom = GameScreenBuffer->Height - RelativePosition.Tilecoord.Y*TileMap.TileDimensions.Y;
    float top = bottom - TileMap.TileDimensions.Y;
    
    DrawRectangle(left,
                  right,
                  top,
                  bottom,                       
                  GameScreenBuffer, 0x0000FFFF);
   
}
inline void DrawPlayer(player Player,tile_map TileMap, game_screen_buffer *GameScreenBuffer)
{
    float left = Player.Position.Tilecoord.X*TileMap.TileDimensions.X +
        Player.Position.relcoord.X + 0.5f*TileMap.TileDimensions.X;
    float right = left + Player.Dimensions.X;
    float bottom = GameScreenBuffer->Height - Player.Position.Tilecoord.Y*TileMap.TileDimensions.Y
        - Player.Position.relcoord.Y - 0.5f*TileMap.TileDimensions.Y;
    float top = bottom - Player.Dimensions.Y;
  

    DrawRectangle(left,
                  right,
                  top,
                  bottom,                       
                  GameScreenBuffer, Player.color);

}
static vect2 GetPositionDiff(vect2 ddPos, float TargetSPF, vect2 Vzero)
{
    vect2 PosDiff = {};
    PosDiff = 0.5f*TargetSPF*TargetSPF*ddPos + TargetSPF*Vzero;
    return(PosDiff);
}
static vect2 GetVelocity(vect2 ddPos, float TargetSPF, vect2 Vzero)
{
    vect2 Velocity = {};
    Velocity = TargetSPF*ddPos + Vzero;
    return(Velocity);
}
static vect2 GetPlayerAcceleration(game_controller_input KeyboardController)
{
    vect2 ddPos = {};
    if(KeyboardController.MoveUp.EndedDown)
    {
        ddPos.Y = 1;
    }
    if(KeyboardController.MoveDown.EndedDown)
    {
        ddPos.Y = -1;
    }
    if(KeyboardController.MoveLeft.EndedDown)
    {
        ddPos.X = -1;
    }
    if(KeyboardController.MoveRight.EndedDown)
    {
        ddPos.X = 1;
    }
    float PlayerAcceleration = 100;
    ddPos *= PlayerAcceleration;
    

    return(ddPos);
}
static void MovePlayer(game_controller_input KeyboardController, game_state *GameState, tile_map TileMap, float TargetSPF)
{
    
    vect2 ddPos = GetPlayerAcceleration(KeyboardController);
    vect2 Friction = 1.5*GameState->Player.Velocity;
    ddPos = ddPos - Friction;
    GameState->Player.Velocity = GetVelocity(ddPos, TargetSPF, GameState->Player.Velocity);

    vect2 PosDiff = GetPositionDiff(ddPos, TargetSPF,
                                    GameState->Player.Velocity);
    

    /* vect2 ddPos = GetPlayerAcceleration(KeyboardController);
     
     vect2 PosDiff = GetPositionDiff({0, 0}, TargetSPF,
                                     GameState->Player.Velocity);
     GameState->Player.Velocity = ddPos;
    */
    relative_position NewPosition = GameState->Player.Position;
    NewPosition.relcoord += PosDiff;
    NewPosition = DetermineNewTile(NewPosition, TileMap);
   
    relative_position TopRight = NewPosition;
    TopRight.relcoord += GameState->Player.Dimensions;
    TopRight = DetermineNewTile(TopRight, TileMap);
    
    
    
    
    bool Collided = false;
    relative_position CollideP = {};
    if(!TileIsEmpty(TileMap, NewPosition))
    {
        CollideP = NewPosition;
        Collided = true;
                                                                               
    }
    if(!TileIsEmpty(TileMap, TopRight))
    {
        CollideP = TopRight;
        Collided = true;
    }

    if(Collided)
    {
        vect2 WallVector = {0, 0};
        if(CollideP.Tilecoord.X  < GameState->Player.Position.Tilecoord.X)
        {
            WallVector = {1, 0};
        }
        if(CollideP.Tilecoord.X  > GameState->Player.Position.Tilecoord.X)
        {
            WallVector = {-1, 0};
        }
        if(CollideP.Tilecoord.Y  < GameState->Player.Position.Tilecoord.Y)
        {
            WallVector = {0, 1};
        }
        if(CollideP.Tilecoord.Y  > GameState->Player.Position.Tilecoord.Y)
        {
            WallVector = {0, -1};
        }
        
        GameState->Player.Velocity = GameState->Player.Velocity - 1*DotProduct(GameState->Player.Velocity,
                                                                             WallVector)*WallVector;
       
    }
    else
    {
        GameState->Player.Position = NewPosition;
    }
     
}
static bool ObjectCollision(player Player, relative_position BallPosition, tile_map TileMap)
{
    bool Result = false;
    float BallX = BallPosition.Tilecoord.X*TileMap.TileDimensions.X + BallPosition.relcoord.X + 0.5f*TileMap.TileDimensions.X;
    float PlayerX = Player.Position.Tilecoord.X*TileMap.TileDimensions.X +
        Player.Position.relcoord.X + 0.5f*TileMap.TileDimensions.X + Player.Dimensions.X;
    float BallY = BallPosition.Tilecoord.Y*TileMap.TileDimensions.Y + BallPosition.relcoord.Y + 0.5f*TileMap.TileDimensions.Y;
    float PlayerHighY = Player.Position.Tilecoord.Y*TileMap.TileDimensions.Y +
        Player.Position.relcoord.Y + 0.5f*TileMap.TileDimensions.Y + Player.Dimensions.Y;
    float PlayerLowY = Player.Position.Tilecoord.Y*TileMap.TileDimensions.Y +
        Player.Position.relcoord.Y + 0.5f*TileMap.TileDimensions.Y;

        if((BallX < PlayerX) && (BallY <= PlayerHighY) && (BallY >= PlayerLowY))
    {
        Result = true;
    }
    return Result;
}
static void MoveBall(player *Ball, tile_map TileMap,
                     float TargetSPF, player Player)
{
    vect2 ddPos = {0, 0};
    vect2 Friction = {0, 0};
    ddPos = ddPos - Friction;
    Ball->Velocity = GetVelocity(ddPos, TargetSPF, Ball->Velocity);

    vect2 PosDiff = GetPositionDiff(ddPos, TargetSPF,
                                    Ball->Velocity);
    relative_position NewPosition = Ball->Position;
    NewPosition.relcoord += PosDiff;
    NewPosition = DetermineNewTile(NewPosition, TileMap);
   
    relative_position TopRight = NewPosition;
    TopRight.relcoord += Ball->Dimensions;
    TopRight = DetermineNewTile(TopRight, TileMap);
    
    
    
    
    bool Collided = false;
    bool PlayerCollision = false;
    relative_position CollideP = {};
    if(!TileIsEmpty(TileMap, NewPosition))
    {
        CollideP = NewPosition;
        Collided = true;
                                                                               
    }
    if(!TileIsEmpty(TileMap, TopRight))
    {
        CollideP = TopRight;
        Collided = true;
    }
    if(ObjectCollision(Player, NewPosition, TileMap))
    {
        CollideP = NewPosition;
        Collided = true;
        PlayerCollision = true;
    }
        
    
    

    if(Collided)
    {
        vect2 WallVector = {0, 0};
        if(PlayerCollision)
        {
            WallVector = {1, 0};
        }
            
        if(CollideP.Tilecoord.X  < Ball->Position.Tilecoord.X)
        {
            WallVector = {1, 0};
        }
        if(CollideP.Tilecoord.X  > Ball->Position.Tilecoord.X)
        {
            WallVector = {-1, 0};
        }
        if(CollideP.Tilecoord.Y  < Ball->Position.Tilecoord.Y)
        {
            WallVector = {0, 1};
        }
        if(CollideP.Tilecoord.Y  > Ball->Position.Tilecoord.Y)
        {
            WallVector = {0, -1};
        }
        
        Ball->Velocity = Ball->Velocity - 2*DotProduct(Ball->Velocity,
                                                     WallVector)*WallVector;
       
    }
    else
    {
        Ball->Position = NewPosition;
    }

}


static void GameUpdateRender(game_sound_data GameSoundData, game_screen_buffer GameScreenBuffer, game_controller_input KeyboardController, game_memory *Memory, float TargetSPF)
{

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    DrawRectangle(0,
                  (float)GameScreenBuffer.Width,
                  0,
                  (float)GameScreenBuffer.Height,
                  &GameScreenBuffer, 0xFF000000);
   
    int map[10][10]= {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};                          
                    
 
    
    
    tile_map TileMap = {};

    TileMap.TileDimensions = {50, 50};
    TileMap.TileCount = {10, 10};
    
    TileMap.map = (int *)map;
    
     
    if(!Memory->IsInitialized)
    {
        GameState->Player = {};
        GameState->Player.Position.Tilecoord = {1, 1};
        GameState->Player.Position.relcoord = {0,0};
        GameState->Player.Dimensions = {10, 100};
        GameState->Player.color = 0x0000FF00;
        GameState->Ball = {};
        GameState->Ball.Position.Tilecoord = {5, 5};
        GameState->Ball.Position.relcoord = {0, 0};
        GameState->Ball.Dimensions = {10, 10};
        GameState->Ball.color = 0x00FFFF00;
        GameState->Ball.Velocity = {50, 40};
        
        Memory->IsInitialized = true;
    }
    MovePlayer(KeyboardController, GameState, TileMap, TargetSPF);
    MoveBall(&GameState->Ball, TileMap, TargetSPF, GameState->Player);
  
    DrawTileMap(TileMap, &GameScreenBuffer);
    DEBUGDrawContainerTile(GameState->Player.Position, TileMap, &GameScreenBuffer);
    //DEBUGDrawContainerTile(GameState->Ball.Position, TileMap, &GameScreenBuffer);

   
    DrawPlayer(GameState->Player,TileMap, &GameScreenBuffer);
    DrawPlayer(GameState->Ball,TileMap, &GameScreenBuffer);
    
    

  
    
}
