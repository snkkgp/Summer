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
// Main Core Language:
//
// FILE:
//   core.c
//
// SUMMER LANGUAGE START DATE ( 27/08/2017 - 08:35 ):
//   rewrite: 20/07/2018 - 11:10
//
// BY:  Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#include "summer.h"
#include <stdarg.h>

struct DATA { // test SDL_Surface ( w, h )
    int   flags;  // not used
    void  *none;  // not used
    int   w;
    int   h;
};

//-----------------------------------------------
//-----------------  PROTOTYPES  ----------------
//-----------------------------------------------
//
static void   word_int      (LEXER *l, ASM *a);
static void   word_module   (LEXER *l, ASM *a);
static void   word_import   (LEXER *l, ASM *a);
static void   word_function (LEXER *l, ASM *a);
//
static void   expression    (LEXER *l, ASM *a);
//--------------  Expression:  --------------
static int    expr0         (LEXER *l, ASM *a);
static void   expr1         (LEXER *l, ASM *a);
static void   expr2         (LEXER *l, ASM *a);
static void   expr3         (LEXER *l, ASM *a);
static void   atom          (LEXER *l, ASM *a);
//-------------------------------------------
static void   do_block      (LEXER *l, ASM *a);
static int    stmt          (LEXER *l, ASM *a);
static int    see           (LEXER *l);
static void   module_add    (char *module_name, char *func_name, char *proto, UCHAR *code);
F_STRING    * fs_new        (char *s);
// stdlib:
void  lib_info    (int arg);
int   lib_add     (int a, int b);
int   lib_arg     (int a, int b, int c, int d, int e);
void  lib_printf  (char *format, ...);
void  lib_log (char *s); // console.log();
TFunc *lib_get_func (char *name);

struct DATA *new_data (int x, int y, int w, int h);
void display (struct DATA *data);
void disasm (UCHAR *code);

//-----------------------------------------------
//------------------  VARIABLES  ----------------
//-----------------------------------------------
//
TVar              Gvar [GVAR_SIZE]; // global:
int               gvar_len;
//
static MODULE   * Gmodule = NULL; // ... console.log(); ...
static TFunc    * Gfunc = NULL;  // store the user functions
static ASM      * asm_function;
static char       strErro [STR_ERRO_SIZE];
static char       func_name [100];
static char       array_break [20][20];   // used to word break
static ARG        argument [20];
static F_STRING * fs = NULL;

//
static int
    erro,
    loop_level,
    is_function,
    is_recursive,
    main_variable_type,
    var_type,
    argument_count,
    local_count
    ;

static TFunc stdlib[]={
  //-----------------------------------------------------------------
  // char*        char*   UCHAR*/ASM*             int   int   FUNC*
  // name         proto   code                    type  len   next
  //-----------------------------------------------------------------
  { "info",       "0i",     (UCHAR*)lib_info,       0,    0,    NULL },
  { "add",        "iii",    (UCHAR*)lib_add,        0,    0,    NULL },
  { "arg",        "iiiiii", (UCHAR*)lib_arg,        0,    0,    NULL },
  { "printf",     "0s",     (UCHAR*)lib_printf,     0,    0,    NULL },
  { "get_func",   "ps",     (UCHAR*)lib_get_func,   0,    0,    NULL },

  { "new_data",   "piiii",  (UCHAR*)new_data,       0,    0,    NULL },
  { "display",    "0p",     (UCHAR*)display,        0,    0,    NULL },

  { "disasm",     "0s",     (UCHAR*)disasm,        0,    0,    NULL },

  { NULL,         NULL,     NULL,                   0,    0,    NULL }
};


void func_null (void) { printf ("FUNCTION: func_null\n"); }
TFunc func_null_default = { "func_null", "00", (UCHAR*)func_null, 0, 0, NULL };


//-----------------------------------------------
//------------------  FUNCTIONS  ----------------
//-----------------------------------------------
//
ASM * core_Init (unsigned int size) {
    static int init = 0;
    ASM *a;
    if (!init) {
        init = 1;
        if ((a            = asm_new (size))==NULL) return NULL;
        if ((asm_function = asm_new (size))==NULL) return NULL;

        module_add ("console", "log", "0s", (UCHAR*)lib_printf);
/*
        module_add ("console", "data", "0s", (UCHAR*)lib_log);
        module_add ("console", "type", "0s", (UCHAR*)lib_log);

        module_add ("sdl", "SDL_Init", "0s", (UCHAR*)lib_log);
        module_add ("sdl", "SDL_Quit", "0s", (UCHAR*)lib_log);
        module_add ("sdl", "SDL_Sleep", "0s", (UCHAR*)lib_log);
*/
        gvar_len = 12345;
        return a;
    }
    return NULL;
}

void core_Finalize (void) {
    MODULE *p = Gmodule;
    while (p) {
        TFunc *f = p->func;
        printf ("MODULE: '%s'\n", p->name);
        while (f) {
            printf ("  '%s'\n", f->name);
            f = f->next;
        }
        p = p->next;
    }
}

char * FileOpen (const char *FileName) {
    FILE *fp;

    if ((fp = fopen (FileName, "rb")) != NULL) {
        char *str;
        int size, i;

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        str = (char *)malloc (size + 5);
        if(!str){
            fclose (fp);
            return NULL;
        }
        i = fread(str, 1, size, fp);
        fclose(fp);
        str[i] = 0;
        str[i+1] = 0;

        return str;
    }
    else printf ("File Not Found: '%s'\n", FileName);

    return NULL;
}
void CreateVarInt (char *name, int value) {
    TVar *v = Gvar;
    int i = 0;
    while (v->name) {
        if (!strcmp(v->name, name))
      return;
        v++;
        i++;
    }
    if (i < GVAR_SIZE) {
        v->name = strdup(name);
        v->type = TYPE_INT;
        v->value.i = value;
        v->info = NULL;
    }
}
TFunc *FuncFind (char *name) {
    // array:
    TFunc *lib = stdlib;
    while (lib->name) {
        if ((lib->name[0]==name[0]) && !strcmp(lib->name, name))
      return lib;
        lib++;
    }
    // linked list:
    TFunc *func = Gfunc;
    while (func) {
        if ((func->name[0]==name[0]) && !strcmp(func->name, name))
      return func;
        func = func->next;
    }
    return NULL;
}

TFunc *ModuleFind (char *LibName, char *FuncName) {
    MODULE *p = Gmodule;
    while (p) {
        if (!strcmp(p->name, LibName)) {
            TFunc *f = p->func;
            while (f) {
                if (!strcmp(f->name, FuncName)) {
              return f;
                }
                f = f->next;
            }
        }
        p = p->next;
    }
    return NULL;
}

int ModuleIsLib (char *LibName) {
    MODULE *p = Gmodule;
    while (p) {
        if (!strcmp(p->name, LibName))
      return 1;
        p = p->next;
    }
    return 0;
}

int VarFind (char *name) {
    TVar *v = Gvar;
    int i = 0;
    while(v->name) {
        if (!strcmp(v->name, name))
      return i;
        v++;
        i++;
    }
    return -1;
}

static void word_int (LEXER *l, ASM *a) {
    while (lex(l)) {
        if (l->tok == TOK_ID) {
            char name[255];
            int value = 0;

            strcpy (name, l->token); // save

            if (lex(l) == '=') {
                if (lex(l) == TOK_NUMBER)
                    value = atoi (l->token);
            }
            CreateVarInt (name, value);

        }
        if (l->tok == ';') break;
    }
    if (l->tok != ';') Erro ("ERRO: The word(float) need the char(;) on the end\n");
}

//
// library ("SDL", "sdl");
//
static void word_module (LEXER *l, ASM *a) {
    char FileName [100] = { 0 };
    char LibName  [100] = { 0 };
    void *lib = NULL;
    int count = 0;

    while (lex(l)) {
        if (l->tok==TOK_STRING) {
            if (count==0) {
                #ifdef WIN32
                sprintf (FileName, "%s.dll", l->token);
                #endif
                #ifdef __linux__
                sprintf (FileName, "%s.so", l->token);
                #endif
            } else if (count==1) {
                sprintf (LibName, "%s", l->token);
            }
            count++;
        }
        if (l->tok==')' || l->tok==';') break;
    }

    //
    // is module exist then return
    //
    MODULE *mod, *p = Gmodule;
    while (p) { // ! if exist
        if (!strcmp(p->name, LibName))
      return;
        p = p->next;
    }

    #ifdef WIN32
    lib = (void *)LoadLibrary (FileName);
    #endif
    #ifdef __linux__
    lib = dlopen (FileName, RTLD_LAZY); // RTLD_NOW
    #endif
    if (lib && (mod = (MODULE*) malloc(sizeof(MODULE))) != NULL) {
        mod->name = strdup(LibName);
        mod->lib = lib;
        mod->func = NULL;
        mod->next = NULL;
        // add on top
        mod->next = Gmodule;
        Gmodule = mod;
        printf ("------------ LOADED LIBRARY----------------(%s)\n", FileName);
    } else {
        //Erro("%s: %d: File Not Found: '%s'\n", l->name, l->line, FileName);
        printf ("%s: %d: File Not Found: '%s'\n", l->name, l->line, FileName);
    }
}

//
// import ("sdl", "SDL_init", "0i");
//
static void word_import (LEXER *l, ASM *a) {
    char LibName[100];
    char FuncName[100];
    char proto[100];
    MODULE *p = Gmodule;
    int count = 0;

    while (lex(l)) {
        if (l->tok==TOK_STRING) {
            if (count==0) {
                sprintf (LibName, "%s", l->token);
            } else if (count==1) {
                sprintf (FuncName, "%s", l->token);
            } else if (count==2) {
                sprintf(proto, "%s", l->token);
            }
            count++;
        }
        if (l->tok==')' || l->tok==';') break;
    }

    while (p) {
        if (!strcmp(p->name, LibName)) {
            #ifdef WIN32
            void *fp = (void*)GetProcAddress ((HMODULE)p->lib, FuncName);
            #endif
            #ifdef __linux__
            void *fp = dlsym (p->lib, FuncName);
            #endif
            if (fp) {
                TFunc *func;
                if ((func = (TFunc*) malloc (sizeof(TFunc))) != NULL) {
                    func->name = strdup (FuncName);
                    func->proto = strdup (proto);
                    func->type = FUNC_TYPE_NATIVE_C;
                    func->len = 0;
                    func->code = fp;

                    // add function on top
                    func->next = p->func;
                    p->func = func;
                }
            }
            else printf ("Function Not Found: '%s'\n", FuncName);//Erro("%s: %d: USAGE: import(%csdl%c, %SDL_Init%c, %c0i%c\n", '"', '"', '"');
      return;
        }
        p = p->next;
    }//: while (p)
    //else Erro("%s: %d: USAGE: import(%csdl%c, %SDL_Init%c, %c0i%c\n", '"', '"', '"');
}

static void word_function (LEXER *l, ASM *a) {
    TFunc *func;
    char name[255], proto[255] = { '0', 0, 0, 0, 0, 0, 0, 0 };
    int i;

    lex(l);

    strcpy (name, l->token);

    // if exist ... return
    //
    if (FuncFind(name)!=NULL) {
        int brace = 0;

        printf ("Function exist: ... REBOBINANDO '%s'\n", name);

        while (lex(l) && l->tok != ')');

        if (see(l)=='{') { } else Erro ("word(if) need start block: '{'\n");

        while (lex(l)){
            if (l->tok == '{') brace++;
            if (l->tok == '}') brace--;
            if (brace <= 0) break;
        }

  return;
    }

    // PASSA PARAMETROS ... IMPLEMENTADA APENAS ( int ) ... AGUARDE
    //
    // O analizador de expressao vai usar esses depois...
    //
    // VEJA EM ( expr3() ):
    // ---------------------
    // Funcoes usadas:
    //     ArgumentFind();
    //     asm_push_argument();
    // ---------------------
    //
    argument_count = 0;
    while (lex(l)) {

        if (l->tok==TOK_INT) {
            argument[argument_count].type[0] = TYPE_INT; // 0
            //strcpy (argument[argument_count].type, "int");
            if (lex(l)==TOK_ID) {
                strcpy (argument[argument_count].name, l->token);
                strcat (proto, "i");
                argument_count++;
            }
        }
/*
        else if (l->tok==TOK_FLOAT) {
            argument[argument_count].type[0] = TYPE_FLOAT; // 1
            //strcpy (argument[argument_count].type, "int");
            if (lex(l)==TOK_ID) {
                strcpy (argument[argument_count].name, l->token);
                strcat (proto, "f");
                argument_count++;
            }
        }
*/
        else if (l->tok==TOK_ID) {
            argument[argument_count].type[0] = TYPE_INT;
            strcpy (argument[argument_count].name, l->token);
            strcat (proto, "i");
            argument_count++;
        }

        if (l->tok=='{') break;
    }
    if (argument_count==0) {
        proto[1] = '0';
        proto[2] = 0;
    }
    if (l->tok=='{') l->pos--; else { Erro("Word Function need char: '{'"); return; }

    is_function = 1;
    local_count = 0;
    strcpy (func_name, name);

    // compiling to buffer ( f ):
    //
    // compiling to buffer ( f ):
    //
    asm_reset (asm_function);
    asm_begin (asm_function);
    //stmt (l,a); // here start from char: '{'
    stmt (l,asm_function); // here start from char: '{'
    asm_end (asm_function);

    if (erro) return;

    int len = asm_get_len (asm_function);

#ifdef USE_JIT
    // new function:
    //
    func = (TFunc*) malloc (sizeof(TFunc));
    func->name = strdup (func_name);
    func->proto = strdup (proto);
    func->type = FUNC_TYPE_COMPILED;
    func->len = len;
    func->code = (UCHAR*) malloc (func->len);
    // NOW: copy the buffer ( f ):
    for (i=0;i<func->len;i++) {
        func->code[i] = asm_function->code[i];
    }

    //-------------------------------------------
    // HACKING ... ;)
    // Resolve Recursive:
    // change 4 bytes ( func_null ) to this
    //-------------------------------------------
    if (is_recursive)
    for (i=0;i<func->len;i++) {
        // b8     7a 13 40 00       mov    $0x40137a,%eax
        //
        // unsigned char: 184
        //
        if (func->code[i] == 0xb8 && *(void**)(func->code+i+1) == func_null) {

            i++; // ... HERE: 4 bytes ...

            // ff d0    call   *%eax
            //
            if (*(func->code+i+4) == 0xff && *(func->code+i+5) == 0xd0) {
                if (*(void**)(func->code+i) == func_null) {
                    *(void**)(func->code+i) = func->code; //<<< change here
                    i += 5;
                }
            }
        }
    }

    set_executable (func->code, len);

#endif
#ifdef USE_VM
    ASM *vm;
    if ((vm = asm_new (len + 5)) != NULL) {
        // new function:
        //
        func = (TFunc*) malloc (sizeof(TFunc));
        func->name = strdup (func_name);
        func->proto = strdup (proto);
        func->type = FUNC_TYPE_VM;
        func->len = len;
        // NOW: copy the buffer ( f ):
        for (i=0;i<func->len;i++) {
            vm->code[i] = asm_function->code[i];
        }
        vm->code[func->len  ] = 0;
        vm->code[func->len+1] = 0;
        vm->code[func->len+2] = 0;

//printf ("FUNCAO SIZE: %d\n", func->len);

        //-------------------------------------------
        // HACKING ... ;)
        // Resolve Recursive:
        // change 4 bytes ( func_null ) to this
        //-------------------------------------------
        if (is_recursive)
        for (i=0;i<func->len;i++) {
            if (vm->code[i]==OP_CALL && *(void**)(vm->code+i+1) == func_null) {
printf ("FUNCAO POSITION: %d\n", i);
                vm->code[i] = OP_CALL_VM;      //<<<<<<<  change here  >>>>>>>
                *(void**)(vm->code+i+1) = vm; //<<<<<<<  change here  >>>>>>>
                i += 5;
            }
        }

        func->code = (UCHAR*)vm;

    } else {
        is_function = is_recursive = argument_count = *func_name = 0;
        return;
    }
#endif

    // add on top:
    func->next = Gfunc;
    Gfunc = func;

    is_function = is_recursive = argument_count = *func_name = 0;
}

//
// function_name (a, b, c + d);
//
void execute_call (LEXER *l, ASM *a, TFunc *func) {
    int count = 0;
    int pos = 0, size = 4; // Used in: JIT 32 bits
    int return_type = TYPE_INT;

    // no argument
    if (func->proto && func->proto[1] == '0') {
        while (lex(l))
            if (l->tok == ')' || l->tok == ';') break;
    } else {
        // get next: '('
        if (lex(l)!='(') { Erro ("Function need char: '('\n"); return; }

        while (lex(l)) {

            if (l->tok==TOK_ID || l->tok==TOK_NUMBER || l->tok==TOK_STRING || l->tok=='(') {

                //
                // The result of expression is store in the "stack".
                //
                expr0 (l,a);

                #ifdef USE_VM
                // ... none ...
                #endif

                #ifdef USE_JIT

                emit_pop_eax (a); // ... pop the "stack" and store in %eax ... := expression result

                #if defined(__x86_64__)
                //-----------------------------------
                // JIT 64 bits
                //
                // %edi, %esi, %edx, %ecx, %r8 and %r9
                //
                if (count==0) {     // argument 1:
                    g2(a,G2_MOV_EAX_EDI);
                }
                else if (count==1) { // argument 2
                    g2(a,G2_MOV_EAX_ESI);
                }
                else if (count==2) { // argument 3
                    g2(a,G2_MOV_EAX_EDX);
                }
                else if (count==3) { // argument 4
                    g2(a,G2_MOV_EAX_ECX);
                }
                else if (count==4) { // argument 5
                    g3(a,G3_MOV_EAX_r8d);
                }
                #else
                //-----------------------------------
                // JIT 32 bits
                emit_mov_eax_ESP (a, pos);
                pos += size;
                #endif
                #endif // ! USE_JIT

                if (count++ > 15) break;
            }
            if (l->tok == ')' || l->tok == ';') break;
        }
    }

    if (count > 5) {
        Erro ("%s:%d: - Call Function(%s) the max arguments is: 5\n", l->name, l->line, func->name);
  return;
    }
    if (func->proto) {
        if (func->proto[0] == '0') return_type = TYPE_NO_RETURN;
        if (func->proto[0] == 'f') return_type = TYPE_FLOAT;
    }
    if (func->type == FUNC_TYPE_VM) {
        //
        // THE_SUMMER_BASE_CODE: <<<<<<<<<<  Not implemented  >>>>>>>>>>
        //
        // here: fi->code ==  ASM*
        //
        // emit_call_vm (a, (ASM*)(func->code), (UCHAR)count, return_type);
    } else {
        emit_call (a, (void*)func->code, (UCHAR)count, return_type);
    }
}

static void expression (LEXER *l, ASM *a) {
    if (l->tok==TOK_ID || l->tok==TOK_NUMBER) {
        char buf[100];
        TFunc *fi;
        int i;

        main_variable_type = var_type = TYPE_INT; // 0

        if (see(l)=='.') { // console.log();
            lex_save(l);
            sprintf (buf, "%s", l->token);
            lex(l); // .
            if (lex(l)==TOK_ID) { // ! FuncName:
                if ((fi = ModuleFind (buf, l->token)) != NULL) {
                    execute_call (l, a, fi);
              return;
                }
            }
            else lex_restore(l);
        }

        //
        // call a function without return:
        //   function_name (...);
        //
        if ((fi = FuncFind(l->token)) != NULL) {
            execute_call (l, a, fi);
      return;
        }

        if ((i = VarFind (l->token)) != -1) {

            main_variable_type = var_type = Gvar[i].type;

            if (see(l)=='=') {
                lex_save(l); // save the lexer position
                lex(l); // =
                if (lex(l)==TOK_ID) {

                    if (see(l)=='.' && ModuleIsLib(l->token)) {
                        lex_save(l); // save the lexer position
                        sprintf (buf, l->token);
                        lex(l); // .
                        if (lex(l)==TOK_ID) { // Module FuncName
                            if ((fi = ModuleFind(buf, l->token)) != NULL) {

                                execute_call (l, a, fi);

                                #ifdef USE_VM
                                // The function return is stored in variable VALUE( eax ) ... see in file: vm.c
                                emit_mov_eax_var(a,i);
                                #endif

                                #ifdef USE_JIT
                                //
                                // The function return is stored in register %eax ... types ! TYPE_FLOAT
                                //
                                //   HERE: copy the register ( %eax ) to the variable
                                //
                                if (main_variable_type != TYPE_FLOAT) {
                                    emit_mov_reg_var (a, EAX, &Gvar[i].value.i);
                                } else {

                                }
                                #endif //: #ifdef USE_JIT
                          return;
                            }
                        }
                    }
                    //
                    // call a function with return:
                    //   i = function_name (...);
                    //
                    if ((fi = FuncFind(l->token)) != NULL) {
                        execute_call (l, a, fi);

                        #ifdef USE_VM
                        // The function return is stored in variable VALUE( eax ) ... see in file: vm.c
                        emit_mov_eax_var(a,i);
                        #endif

                        #ifdef USE_JIT
                        //
                        // The function return is stored in register %eax ... types ! TYPE_FLOAT
                        //
                        //   HERE: copy the register ( %eax ) to the variable
                        //
                        if (main_variable_type != TYPE_FLOAT) {
                            emit_mov_reg_var (a, EAX, &Gvar[i].value.i);
                        } else {

                        }
                        #endif //: #ifdef USE_JIT
                  return;
                    }//: if ((fi = FuncFind(l->token)) != NULL)

                }
                lex_restore (l); // restore the lexer position

            }//: if (see(l)=='=')

        }//: if ((i = VarFind (l->token)) != -1)

        //---------------------------------------
        // Expression types:
        //   a * b + c * d;
        //   10 * a + 3 * b;
        //---------------------------------------
        //
        if (expr0(l,a) == -1) {
            #ifdef USE_VM
            emit_pop_eax (a); // %eax | eax.i := expression result
            emit_print_eax (a,main_variable_type);
            #endif

            #ifdef USE_JIT
            #if defined(__x86_64__)
            //
            // JIT 64 bits
            //
            if (main_variable_type == TYPE_INT) {
                // argument 1:
                // this is a constans :
                // b8    00 20 40 00       	mov    $0x402000,%eax
                g(a,0xb8); asm_get_addr(a,"%d\n");
                g2(a,G2_MOV_EAX_EDI); // 89 c7   : mov   %eax,%edi

                // argument 2:
                emit_pop_eax (a); // %eax | eax.i := expression result
                g2(a,G2_MOV_EAX_ESI); // 89 c6   : mov   %eax,%esi

            } else if (main_variable_type == TYPE_FLOAT) {

            }
            emit_call(a,printf,0,0);   // call the function
            #else
            //
            // JIT 32 bits
            //

            if (main_variable_type == TYPE_INT) {
                emit_pop_eax (a); // %eax | eax.i := expression result
                emit_movl_ESP (a, (long)("%d\n"), 0); // pass argument 1
                emit_mov_eax_ESP (a, 4);              // pass argument 2
            } else if (main_variable_type == TYPE_FLOAT) {

            }
            emit_call(a,printf,0,0);   // call the function
            #endif
            #endif //: #ifdef USE_JIT
        }//: if (expr0(l,a) == -1)

    }
    else Erro ("%s: Expression ERRO(%d) - Ilegar Word (%s)\n", l->name, l->line, l->token);
}
static int expr0 (LEXER *l, ASM *a) {
    if (l->tok == TOK_ID) {
        int i;
        //---------------------------------------
        //
        // Expression type:
        //
        //   i = a * b + c;
        //
        //---------------------------------------
        if (see(l)=='=') {
            if ((i=VarFind(l->token)) != -1) {
                lex_save (l); // save the lexer position
                if (lex(l) == '=') {
                    lex(l);
                    expr1(l,a);
                    #ifdef USE_VM
                    // Copia o TOPO DA PILHA ( sp ) para a variavel ... e decrementa sp++.
                    emit_pop_var (a,i);
                    #endif
                    #ifdef USE_JIT
                    // Copia o TOPO DA PILHA ( %ESP ) para a variavel ... e estabiliza a PILHA
                    emit_pop_var (a, &Gvar[i].value.i);
                    #endif
              return i;
                } else {
                    lex_restore (l); // restore the lexer position
                }
            }//: if ((i=VarFind(l->token)) != -1)
        }//: if (see(l)=='=')
    }
    expr1(l,a);
    return -1;
}
static void expr1 (LEXER *l, ASM *a) { // '+' '-' : ADDITION | SUBTRACTION
    int op;
    expr2(l,a);
    while ((op=l->tok) == '+' || op == '-') {
        lex(l);
        expr2(l,a);
        if (main_variable_type==TYPE_FLOAT) {
//            if (op=='+') emit_add_float (a);
        } else { // INT
            if (op=='+') emit_add_int (a);
            if (op=='-') emit_sub_int (a);
        }

    }
}
static void expr2 (LEXER *l, ASM *a) { // '*' '/' : MULTIPLICATION | DIVISION
    int op;
    expr3(l,a);
    while ((op=l->tok) == '*' || op == '/') {
        lex(l);
        expr3(l,a);
        if (main_variable_type==TYPE_FLOAT) {
//            if (op=='*') emit_mul_float (a);
        } else { // INT
            if (op=='*') emit_mul_int (a);
            if (op=='/') emit_div_int (a);
        }
    }
}
static void expr3 (LEXER *l, ASM *a) { // '('
    if (l->tok=='(') {
        lex(l); expr0(l,a);
        if (l->tok != ')') {
            Erro ("ERRO )\n");
        }
        lex(l);
    }
    else atom(l,a); // atom:
}
static void atom (LEXER *l, ASM *a) { // expres

    if (l->tok==TOK_STRING) {
        F_STRING *s = fs_new (l->token);
        if (s) {
            #ifdef USE_VM
            emit_push_string (a, s->s);
            #endif
            #ifdef USE_JIT
            // this is a constans :
            // 68    00 20 40 00       	push   $0x402000
            //
            g(a, 0x68); asm_get_addr(a,s->s);
            #endif
        }
  return;
    }
    if (l->tok==TOK_ID) {
        int i;
        TFunc *fi;

#ifdef USE_JIT
        if ((fi = FuncFind (l->token)) != NULL) {
            emit_mov_var_reg (a, &fi->code, EAX);
            g (a,0x50);// 50   push   %eax
            lex (l);
        }
        else
#endif
        if ((i=VarFind(l->token))!=-1) {
            var_type = Gvar[i].type;

            #ifdef USE_JIT
            emit_push_var (a, &Gvar[i].value.i);
            #endif
            #ifdef USE_VM
            emit_push_var (a,i);
            #endif

            lex(l);

        }
        else Erro ("%s: %d: - Var Not Found '%s'", l->name, l->line, l->token);
    }
    else if (l->tok==TOK_NUMBER) {
        if (strchr(l->token, '.'))
            var_type = TYPE_FLOAT;

        if (var_type==TYPE_FLOAT) {
//            emit_push_float (a, atof(l->token));
        } else {
            emit_push_int (a, atoi(l->token));
        }

        lex(l);
    }
    else Erro("%s: %d Expression atom - Ilegal Word (%s)\n", l->line, l->token);

}//: atom ()


void do_block (LEXER *l, ASM *a) {

    while (!erro && l->tok && l->tok != '}') {
        stmt (l,a);
    }
    l->tok = ';';
}

static int stmt (LEXER *l, ASM *a) {

    lex (l);

    switch (l->tok) {
    case '{':
        //----------------------------------------------------
        do_block (l,a); //<<<<<<<<<<  no recursive  >>>>>>>>>>
        //----------------------------------------------------
        return 1;
    case TOK_INT:       word_int      (l,a); return 1;
    case TOK_MODULE:    word_module   (l,a); return 1;
    case TOK_IMPORT:    word_import   (l,a); return 1;
    case TOK_FUNCTION:  word_function (l,a); return 1;
    default:            expression    (l,a); return 1;
    case ';':
    case '}':
        return 1;
    case 0: return 0;
    }
    return 1;
}
static int see (LEXER *l) {
    char *s = l->text+l->pos;
    while (*s) {
        if (*s=='\n' || *s==' ' || *s==9 || *s==13) {
            s++;
        } else {
            if (s[0]=='=' && s[1]=='=') return TOK_EQUAL_EQUAL;
            if (s[0]=='+' && s[1]=='+') return TOK_PLUS_PLUS;
            return *s;
        }
    }
    return 0;
}
F_STRING *fs_new (char *s) {
    static int count = 0;
    F_STRING *p = fs, *n;

    while (p) {
        if (!strcmp(p->s,s)) return p;
        p = p->next;
    }

    if ((n = (F_STRING*)malloc(sizeof(F_STRING)))==NULL) return NULL;
    n->s = strdup(s);

//printf ("FIXED: %p\n", &n->s);

    n->i = count++;
    // add on top
    n->next = fs;
    fs = n;

    return n;
}

void module_add (char *module_name, char *func_name, char *proto, UCHAR *code) {
    MODULE *mod, *p = Gmodule;
    TFunc *func;

    while (p) {
        if (!strcmp(module_name, p->name)) {
            if ((func = (TFunc*) malloc (sizeof(TFunc))) != NULL) {
                func->name = strdup (func_name);
                func->proto = strdup (proto);
                func->type = FUNC_TYPE_NATIVE_C;
                func->len = 0;
                func->code = code;

                // add on top;
                func->next = p->func;
                p->func = func;
          return;
            }
        }
        p = p->next;
    }
    //
    // create: MODULE and FUNCTION.
    //
    if ((mod = (MODULE*) malloc(sizeof(MODULE))) != NULL) {
        if ((func = (TFunc*) malloc (sizeof(TFunc))) != NULL) {
            func->name = strdup (func_name);
            func->proto = strdup (proto);
            func->type = FUNC_TYPE_NATIVE_C;
            func->len = 0;
            func->code = code;

            mod->name = strdup(module_name);

            mod->func = NULL;

            // add function on top
            func->next = mod->func;
            mod->func = func;

            // add module on top
            mod->next = Gmodule;
            Gmodule = mod;
        }
    }
}

int Parse (LEXER *l, ASM *a, char *text, char *name) {
    lex_set (l, text, name);
    ErroReset ();
    asm_reset (a);
    asm_begin (a);
    while (!erro && stmt(l,a)) {
        // ... compiling ...
    }
    asm_end (a);
    return erro;
}

void lib_log (char *s) {
    printf ("%s\n", s);
}

void lib_info (int arg) {
    switch (arg) {
    case 1: {
        TVar *v = Gvar;
        int i = 0;
        printf ("VARIABLES:\n---------------\n");
        while (v->name) {
            if (v->type==TYPE_INT)   printf ("Gvar[%d](%s) = %d\n", i, v->name, v->value.i);
            else
            if (v->type==TYPE_FLOAT) printf ("Gvar[%d](%s) = %f\n", i, v->name, v->value.f);
            v++; i++;
        }
        } break;

    case 2: {
        TFunc *fi = stdlib;
        printf ("FUNCTIONS:\n---------------\n");
        while (fi->name) {
            if(fi->proto){
                char *s=fi->proto;
                if (*s=='0') printf ("void  ");
                else
                if (*s=='i') printf ("int   ");
                else
                if (*s=='f') printf ("float ");
                else
                if (*s=='s') printf ("char  *");
                else
                if (*s=='p') printf ("void * ");
                printf ("%s (", fi->name);
                s++;
                while(*s){
                    if (*s=='i') printf ("int");
                    else
                    if (*s=='f') printf ("float");
                    else
                    if (*s=='s') printf ("char *");
                    s++;
                    if(*s) printf (", ");
                }
                printf (");\n");
            }
            fi++;
        }
        }
        break;

    default:
        printf ("USAGE(%d): info(1);\n\nInfo Options:\n 1: Variables\n 2: Functions\n 3: Defines\n 4: Words\n",arg);
    }
}
int lib_add (int a, int b) {
    return a + b;
}

int lib_arg (int a, int b, int c, int d, int e) {
  printf ("arg1: %d\n", a);
  printf ("arg2: %d\n", b);
  printf ("arg3: %d\n", c);
  printf ("arg4: %d\n", d);
  printf ("arg5: %d\n", e);
  return a + b + c + d + e;
}
void lib_printf (char *format, ...) {
    char msg[1024] = { 0 };
    register unsigned int i;
    va_list ap;

    va_start (ap,format);
    vsprintf (msg, format, ap);
    va_end (ap);

    for (i = 0; i < strlen(msg); i++) {
        if (msg[i] == '\\' && msg[i+1] == 'n') { // new line
            putc (10, stdout); // new line
            i++;
        }
        else if (msg[i] == '\\' && msg[i+1] == 't') { // tab
            putc ('\t', stdout); // tab
            i++;
        } else {
            putc (msg[i], stdout);
        }
    }
}

TFunc *lib_get_func (char *name) {
    TFunc *f;
    f = FuncFind (name);
    if (f && f->type == FUNC_TYPE_COMPILED) {
printf ("f existe\n");
        return f;
    }
    return NULL;
}

//void Erro (char *s) {
void Erro (char *format, ...) {
    char msg[1024] = { 0 };
    va_list ap;

    va_start (ap,format);
    vsprintf (msg, format, ap);
    va_end (ap);

    erro++;
    if ((strlen(strErro) + strlen(msg)) < STR_ERRO_SIZE)
        strcat (strErro, msg);
}

char *ErroGet (void) {
    if (strErro[0])
        return strErro;
    else
        return NULL;
}

void ErroReset (void) {
    erro = 0;
    strErro[0] = 0;
}

struct DATA *new_data (int x, int y, int w, int h) {
    struct DATA *d = (struct DATA*)malloc(sizeof(struct DATA));

    if (d) {
        d->w = w;
        d->h = h;
    }
    return d;
}
void display (struct DATA *data) {
    printf ("\nFunction: void display (struct DATA *data);\n");
    printf ("------------------------------------------------\n");
//    printf ("data->x: %d\n", data->x);
//    printf ("data->y: %d\n", data->y);
    printf ("data->w: %d\n", data->w);
    printf ("data->h: %d\n", data->h);
    printf ("------------------------------------------------\n");
}

void disasm (UCHAR *code) {

printf ("\nFunction DISASM\n");

    if (code) {
        printf ("code[0]: 0x%x\n", (UCHAR)code[0]);
    }
}

