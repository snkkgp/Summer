//-------------------------------------------------------------------
//
// TANKS TO:
// ----------------------------------------------
//
//   01: God the creator of the heavens and the earth in the name of Jesus Christ.
//
//   02 - Fabrice Bellard: www.bellard.org
//
// ----------------------------------------------
//
// THIS FILE IS PART OF SUMMER LANGUAGE:
//
// The Lexical Analyzer:
//
// FILE:
//   lex.h
//
// SUMMER LANGUAGE START DATE ( 27/08/2017 - 08:35 ):
//   rewrite: 20/07/2018 - 11:10
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#ifndef _LEX_H
#define _LEX_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // malloc()

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
//---------------  DEFINE / ENUM  ---------------
//-----------------------------------------------
//
#define LEXER_NAME_SIZE   255
#define LEXER_TOKEN_SIZE  1024 * 4

enum {
    TOK_INT = 255,
    //-----------------------
    TOK_ID,
    TOK_STRING,
    TOK_NUMBER,
    //-------------
    TOK_PLUS_PLUS,    // ++
    TOK_MINUS_MINUS,  // --
    TOK_EQUAL_EQUAL,  // ==
    TOK_NOT_EQUAL,    // !=
    TOK_AND_AND,      // &&
    TOK_PTR           // ->
};

typedef struct LEXER LEXER;

struct LEXER {
    char  *text;
    char  name  [LEXER_NAME_SIZE];
    char  token [LEXER_TOKEN_SIZE];
    //
    int   pos; // text [ pos ]
    int   tok;
    int   line;
};

extern int  lex         (LEXER *lexer);

extern void lex_set     (LEXER *lexer, char *text, char *name);

extern void lex_save    (LEXER *l);

extern void lex_restore (LEXER *l);

#ifdef __cplusplus
}
#endif

#endif // ! _LEX_H
