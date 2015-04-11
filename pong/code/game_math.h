#if !defined(GAME_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Jeffrey Goodall $
   ======================================================================== */


union vect2
{
    struct
    {
        
        float X;
        float Y;
        
    };
    float Elements[2];
};


vect2 operator+(vect2 A, vect2 B)
{
    vect2 Result;

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return(Result);
};
vect2 & operator+=(vect2 &A, vect2 B)
{
    A = A + B;
    return(A);
};

vect2 operator-(vect2 A, vect2 B)
{
    vect2 Result;

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;

    return(Result);
};

vect2 operator*(float A, vect2 B )
{
    vect2 Result;

    Result.X = A*B.X;
    Result.Y = A*B.Y;

    return(Result);
};
vect2 & operator*=(vect2 &A, float B)
{
    A = B*A;
    

    return(A);
};
float DotProduct (vect2 A, vect2 B)
{
    float Result;
    Result = A.X* B.X + A.Y* B.Y;
    return Result;
}
union intvect2
{
    struct
    {
        
        int X;
        int Y;
        
    };
    float Elements[2];
};



#define GAME_MATH_H
#endif
