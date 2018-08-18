//-------------------------------------------------------------------
//
// SUMMER LANGUAGE:
//
// FILE:
//   def.h
//
//-------------------------------------------------------------------
//

//-----------------------------------------------
//---------------  DEFINE / ENUM  ---------------
//-----------------------------------------------
//
#define UCHAR           unsigned char
#define ASM_LEN(arg)    (arg->p - arg->code)
#define GVAR_SIZE       255

#define VAR_RET         2

#define TYPE_NO_RETURN  255

enum { // variable type:
    TYPE_LONG = 0,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_POINTER,
    TYPE_STRUCT,
    TYPE_PSTRUCT  // struct data *p;
};

//-----------------------------------------------
//-------------------  STRUCT  ------------------
//-----------------------------------------------
//
typedef union  TValue     TValue;
typedef struct TVar       TVar;
typedef struct DEFINE     DEFINE;

union TValue {
    long    l;  //: type long integer
    float   f;  //: type float
    char    *s; //: type pointer of char
    void    *p; //: type pointer
};
struct TVar {
    char    *name;
    int     type;
    TValue  value;
    void    *info;  // any information ... struct type use this
};
struct DEFINE {
    char    *name;
    int     value;
    DEFINE  *next;
};

//-----------------------------------------------
//---------------  GLOBAL VARIABLE  -------------
//-----------------------------------------------
//
extern TVar Gvar[GVAR_SIZE];