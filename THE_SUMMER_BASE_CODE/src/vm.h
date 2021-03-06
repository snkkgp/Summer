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
// The back-end | VM:
//
// FILE:
//   vn.h
//
// SUMMER LANGUAGE START DATE ( 27/08/2017 - 08:35 ):
//   rewrite: 20/07/2018 - 11:10
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#ifndef _VM_H_
#define _VM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_VM

#define LIBIMPORT         extern
#define ASM_DEFAULT_SIZE  50000
#define UCHAR             unsigned char

enum {
    OP_PUSH_INT,
    OP_PUSH_VAR,
    OP_POP_VAR,

    OP_MUL_INT,
    OP_DIV_INT,
    OP_ADD_INT,
    OP_SUB_INT,

    OP_POP_EAX,
    OP_PRINT_EAX,

    OP_MOV_EAX_VAR,

    OP_PUSH_STRING,

    OP_CALL,

    OP_HALT
};

//-----------------------------------------------
//-------------------  STRUCT  ------------------
//-----------------------------------------------
//
typedef struct ASM        ASM;
typedef struct ASM_label  ASM_label;
typedef struct ASM_jump   ASM_jump;

struct ASM {
    UCHAR     *p;
    UCHAR     *code;
    ASM_label *label;
    ASM_jump  *jump;
    int       size;
    int       ip;
};
struct ASM_label {
    char      *name;
    int       pos;
    ASM_label *next;
};
struct ASM_jump {
    char      *name;
    int       pos;
    ASM_jump  *next;
};

//-----------------------------------------------
//-----------------  PROTOTYPES  ----------------
//-----------------------------------------------
//
LIBIMPORT void    Run               (ASM *a); // back-end in file: vm.c
LIBIMPORT void    vm_run            (ASM *a);
LIBIMPORT ASM   * asm_new           (unsigned int size);
LIBIMPORT void    asm_reset         (ASM *a);
LIBIMPORT void    asm_begin         (ASM *a);
LIBIMPORT void    asm_end           (ASM *a);
// emit:
LIBIMPORT void    emit_push_int     (ASM *a, int i);
LIBIMPORT void    emit_push_var     (ASM *a, UCHAR i);
LIBIMPORT void    emit_pop_var      (ASM *a, UCHAR i);
//-----------------------------------------------
// int expression math:
//-----------------------------------------------
LIBIMPORT void    emit_mul_int      (ASM *a);
LIBIMPORT void    emit_div_int      (ASM *a);
LIBIMPORT void    emit_add_int      (ASM *a);
LIBIMPORT void    emit_sub_int      (ASM *a);
//-----------------------------------------------
LIBIMPORT void    emit_pop_eax      (ASM *a);
LIBIMPORT void    emit_print_eax    (ASM *a, UCHAR type);
LIBIMPORT void    emit_mov_eax_var  (ASM *a, UCHAR index);
LIBIMPORT void    emit_push_string  (ASM *a, char *s);
LIBIMPORT void    emit_call         (ASM *a, void *func, UCHAR arg_count, UCHAR return_type);
LIBIMPORT void    emit_halt         (ASM *a);

#endif // ! USE_VM

#ifdef __cplusplus
}
#endif

#endif // ! _VM_H
