/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/zimpl/mmlparse2.y"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*   File....: mmlparse2.y                                                   */
/*   Name....: MML Parser                                                    */
/*   Author..: Thorsten Koch                                                 */
/*   Copyright by Author, All rights reserved                                */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Copyright (C) 2001-2026 by Thorsten Koch <koch@zib.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic ignored "-Wdeprecated-non-prototype"
#endif
   
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#pragma GCC   diagnostic ignored "-Wstrict-prototypes"
#endif
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
   
#include "zimpl/lint.h"
#include "zimpl/attribute.h"
#include "zimpl/mshell.h"
#include "zimpl/ratlptypes.h"
#include "zimpl/numb.h"
#include "zimpl/elem.h"
#include "zimpl/tuple.h"
#include "zimpl/mme.h"
#include "zimpl/set.h"
#include "zimpl/symbol.h"
#include "zimpl/entry.h"
#include "zimpl/idxset.h"
#include "zimpl/rdefpar.h"
#include "zimpl/bound.h"
#include "zimpl/define.h"
#include "zimpl/mono.h"
#include "zimpl/term.h"
#include "zimpl/list.h"
#include "zimpl/stmt.h"
#include "zimpl/local.h"
#include "zimpl/code.h"
#include "zimpl/inst.h"
        
#define YYERROR_VERBOSE 1

/* the function is actually getting a YYSTYPE* as argument, but the
 * type isn't available here, so it is decalred to accept any number of
 * arguments, i.e. yylex() and not yylex(void).
 */
extern int yylex();

/*lint -sem(yyerror, 1p, r_no) */ 
extern void yyerror(const char* s) is_NORETURN;
 

#line 155 "src/zimpl/mmlparse2.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "mmlparse2.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DECLSET = 3,                    /* DECLSET  */
  YYSYMBOL_DECLPAR = 4,                    /* DECLPAR  */
  YYSYMBOL_DECLVAR = 5,                    /* DECLVAR  */
  YYSYMBOL_DECLMIN = 6,                    /* DECLMIN  */
  YYSYMBOL_DECLMAX = 7,                    /* DECLMAX  */
  YYSYMBOL_DECLSUB = 8,                    /* DECLSUB  */
  YYSYMBOL_DECLSOS = 9,                    /* DECLSOS  */
  YYSYMBOL_DEFNUMB = 10,                   /* DEFNUMB  */
  YYSYMBOL_DEFSTRG = 11,                   /* DEFSTRG  */
  YYSYMBOL_DEFBOOL = 12,                   /* DEFBOOL  */
  YYSYMBOL_DEFSET = 13,                    /* DEFSET  */
  YYSYMBOL_PRINT = 14,                     /* PRINT  */
  YYSYMBOL_CHECK = 15,                     /* CHECK  */
  YYSYMBOL_BINARY = 16,                    /* BINARY  */
  YYSYMBOL_INTEGER = 17,                   /* INTEGER  */
  YYSYMBOL_REAL = 18,                      /* REAL  */
  YYSYMBOL_IMPLICIT = 19,                  /* IMPLICIT  */
  YYSYMBOL_ASGN = 20,                      /* ASGN  */
  YYSYMBOL_DO = 21,                        /* DO  */
  YYSYMBOL_WITH = 22,                      /* WITH  */
  YYSYMBOL_IN = 23,                        /* IN  */
  YYSYMBOL_TO = 24,                        /* TO  */
  YYSYMBOL_UNTIL = 25,                     /* UNTIL  */
  YYSYMBOL_BY = 26,                        /* BY  */
  YYSYMBOL_FORALL = 27,                    /* FORALL  */
  YYSYMBOL_EXISTS = 28,                    /* EXISTS  */
  YYSYMBOL_PRIORITY = 29,                  /* PRIORITY  */
  YYSYMBOL_STARTVAL = 30,                  /* STARTVAL  */
  YYSYMBOL_DEFAULT = 31,                   /* DEFAULT  */
  YYSYMBOL_CMP_LE = 32,                    /* CMP_LE  */
  YYSYMBOL_CMP_GE = 33,                    /* CMP_GE  */
  YYSYMBOL_CMP_EQ = 34,                    /* CMP_EQ  */
  YYSYMBOL_CMP_LT = 35,                    /* CMP_LT  */
  YYSYMBOL_CMP_GT = 36,                    /* CMP_GT  */
  YYSYMBOL_CMP_NE = 37,                    /* CMP_NE  */
  YYSYMBOL_INFTY = 38,                     /* INFTY  */
  YYSYMBOL_AND = 39,                       /* AND  */
  YYSYMBOL_OR = 40,                        /* OR  */
  YYSYMBOL_XOR = 41,                       /* XOR  */
  YYSYMBOL_NOT = 42,                       /* NOT  */
  YYSYMBOL_SUM = 43,                       /* SUM  */
  YYSYMBOL_MIN = 44,                       /* MIN  */
  YYSYMBOL_MAX = 45,                       /* MAX  */
  YYSYMBOL_ARGMIN = 46,                    /* ARGMIN  */
  YYSYMBOL_ARGMAX = 47,                    /* ARGMAX  */
  YYSYMBOL_PROD = 48,                      /* PROD  */
  YYSYMBOL_IF = 49,                        /* IF  */
  YYSYMBOL_THEN = 50,                      /* THEN  */
  YYSYMBOL_ELSE = 51,                      /* ELSE  */
  YYSYMBOL_END = 52,                       /* END  */
  YYSYMBOL_INTER = 53,                     /* INTER  */
  YYSYMBOL_UNION = 54,                     /* UNION  */
  YYSYMBOL_CROSS = 55,                     /* CROSS  */
  YYSYMBOL_SYMDIFF = 56,                   /* SYMDIFF  */
  YYSYMBOL_WITHOUT = 57,                   /* WITHOUT  */
  YYSYMBOL_PROJ = 58,                      /* PROJ  */
  YYSYMBOL_MOD = 59,                       /* MOD  */
  YYSYMBOL_DIV = 60,                       /* DIV  */
  YYSYMBOL_POW = 61,                       /* POW  */
  YYSYMBOL_FAC = 62,                       /* FAC  */
  YYSYMBOL_CARD = 63,                      /* CARD  */
  YYSYMBOL_ROUND = 64,                     /* ROUND  */
  YYSYMBOL_FLOOR = 65,                     /* FLOOR  */
  YYSYMBOL_CEIL = 66,                      /* CEIL  */
  YYSYMBOL_RANDOM = 67,                    /* RANDOM  */
  YYSYMBOL_ORD = 68,                       /* ORD  */
  YYSYMBOL_ABS = 69,                       /* ABS  */
  YYSYMBOL_SGN = 70,                       /* SGN  */
  YYSYMBOL_LOG = 71,                       /* LOG  */
  YYSYMBOL_LN = 72,                        /* LN  */
  YYSYMBOL_EXP = 73,                       /* EXP  */
  YYSYMBOL_SQRT = 74,                      /* SQRT  */
  YYSYMBOL_SIN = 75,                       /* SIN  */
  YYSYMBOL_COS = 76,                       /* COS  */
  YYSYMBOL_TAN = 77,                       /* TAN  */
  YYSYMBOL_ASIN = 78,                      /* ASIN  */
  YYSYMBOL_ACOS = 79,                      /* ACOS  */
  YYSYMBOL_ATAN = 80,                      /* ATAN  */
  YYSYMBOL_POWER = 81,                     /* POWER  */
  YYSYMBOL_SGNPOW = 82,                    /* SGNPOW  */
  YYSYMBOL_READ = 83,                      /* READ  */
  YYSYMBOL_AS = 84,                        /* AS  */
  YYSYMBOL_SKIP = 85,                      /* SKIP  */
  YYSYMBOL_USE = 86,                       /* USE  */
  YYSYMBOL_COMMENT = 87,                   /* COMMENT  */
  YYSYMBOL_MATCH = 88,                     /* MATCH  */
  YYSYMBOL_SUBSETS = 89,                   /* SUBSETS  */
  YYSYMBOL_INDEXSET = 90,                  /* INDEXSET  */
  YYSYMBOL_POWERSET = 91,                  /* POWERSET  */
  YYSYMBOL_PERMUTE = 92,                   /* PERMUTE  */
  YYSYMBOL_VIF = 93,                       /* VIF  */
  YYSYMBOL_VABS = 94,                      /* VABS  */
  YYSYMBOL_TYPE1 = 95,                     /* TYPE1  */
  YYSYMBOL_TYPE2 = 96,                     /* TYPE2  */
  YYSYMBOL_LENGTH = 97,                    /* LENGTH  */
  YYSYMBOL_SUBSTR = 98,                    /* SUBSTR  */
  YYSYMBOL_NUMBSYM = 99,                   /* NUMBSYM  */
  YYSYMBOL_STRGSYM = 100,                  /* STRGSYM  */
  YYSYMBOL_VARSYM = 101,                   /* VARSYM  */
  YYSYMBOL_SETSYM = 102,                   /* SETSYM  */
  YYSYMBOL_NUMBDEF = 103,                  /* NUMBDEF  */
  YYSYMBOL_STRGDEF = 104,                  /* STRGDEF  */
  YYSYMBOL_BOOLDEF = 105,                  /* BOOLDEF  */
  YYSYMBOL_SETDEF = 106,                   /* SETDEF  */
  YYSYMBOL_DEFNAME = 107,                  /* DEFNAME  */
  YYSYMBOL_NAME = 108,                     /* NAME  */
  YYSYMBOL_STRG = 109,                     /* STRG  */
  YYSYMBOL_NUMB = 110,                     /* NUMB  */
  YYSYMBOL_SCALE = 111,                    /* SCALE  */
  YYSYMBOL_SEPARATE = 112,                 /* SEPARATE  */
  YYSYMBOL_CHECKONLY = 113,                /* CHECKONLY  */
  YYSYMBOL_INDICATOR = 114,                /* INDICATOR  */
  YYSYMBOL_QUBO = 115,                     /* QUBO  */
  YYSYMBOL_PENALTY1 = 116,                 /* PENALTY1  */
  YYSYMBOL_PENALTY2 = 117,                 /* PENALTY2  */
  YYSYMBOL_PENALTY3 = 118,                 /* PENALTY3  */
  YYSYMBOL_PENALTY4 = 119,                 /* PENALTY4  */
  YYSYMBOL_PENALTY5 = 120,                 /* PENALTY5  */
  YYSYMBOL_PENALTY6 = 121,                 /* PENALTY6  */
  YYSYMBOL_PENALTY7 = 122,                 /* PENALTY7  */
  YYSYMBOL_PENALTY8 = 123,                 /* PENALTY8  */
  YYSYMBOL_124_ = 124,                     /* '+'  */
  YYSYMBOL_125_ = 125,                     /* '-'  */
  YYSYMBOL_126_ = 126,                     /* '*'  */
  YYSYMBOL_127_ = 127,                     /* ';'  */
  YYSYMBOL_128_ = 128,                     /* '['  */
  YYSYMBOL_129_ = 129,                     /* ']'  */
  YYSYMBOL_130_ = 130,                     /* ','  */
  YYSYMBOL_131_ = 131,                     /* '('  */
  YYSYMBOL_132_ = 132,                     /* ')'  */
  YYSYMBOL_133_ = 133,                     /* '/'  */
  YYSYMBOL_134_ = 134,                     /* '{'  */
  YYSYMBOL_135_ = 135,                     /* '}'  */
  YYSYMBOL_YYACCEPT = 136,                 /* $accept  */
  YYSYMBOL_stmt = 137,                     /* stmt  */
  YYSYMBOL_decl_set = 138,                 /* decl_set  */
  YYSYMBOL_set_entry_list = 139,           /* set_entry_list  */
  YYSYMBOL_set_entry = 140,                /* set_entry  */
  YYSYMBOL_def_numb = 141,                 /* def_numb  */
  YYSYMBOL_def_strg = 142,                 /* def_strg  */
  YYSYMBOL_def_bool = 143,                 /* def_bool  */
  YYSYMBOL_def_set = 144,                  /* def_set  */
  YYSYMBOL_name_list = 145,                /* name_list  */
  YYSYMBOL_decl_par = 146,                 /* decl_par  */
  YYSYMBOL_par_singleton = 147,            /* par_singleton  */
  YYSYMBOL_par_default = 148,              /* par_default  */
  YYSYMBOL_decl_var = 149,                 /* decl_var  */
  YYSYMBOL_var_type = 150,                 /* var_type  */
  YYSYMBOL_lower = 151,                    /* lower  */
  YYSYMBOL_upper = 152,                    /* upper  */
  YYSYMBOL_priority = 153,                 /* priority  */
  YYSYMBOL_startval = 154,                 /* startval  */
  YYSYMBOL_cexpr_entry_list = 155,         /* cexpr_entry_list  */
  YYSYMBOL_cexpr_entry = 156,              /* cexpr_entry  */
  YYSYMBOL_matrix_head = 157,              /* matrix_head  */
  YYSYMBOL_matrix_body = 158,              /* matrix_body  */
  YYSYMBOL_decl_obj = 159,                 /* decl_obj  */
  YYSYMBOL_decl_sub = 160,                 /* decl_sub  */
  YYSYMBOL_constraint_list = 161,          /* constraint_list  */
  YYSYMBOL_constraint = 162,               /* constraint  */
  YYSYMBOL_vbool = 163,                    /* vbool  */
  YYSYMBOL_con_attr_list = 164,            /* con_attr_list  */
  YYSYMBOL_con_attr = 165,                 /* con_attr  */
  YYSYMBOL_con_type = 166,                 /* con_type  */
  YYSYMBOL_vexpr = 167,                    /* vexpr  */
  YYSYMBOL_vproduct = 168,                 /* vproduct  */
  YYSYMBOL_vfactor = 169,                  /* vfactor  */
  YYSYMBOL_vexpo = 170,                    /* vexpo  */
  YYSYMBOL_vval = 171,                     /* vval  */
  YYSYMBOL_decl_sos = 172,                 /* decl_sos  */
  YYSYMBOL_soset = 173,                    /* soset  */
  YYSYMBOL_sos_type = 174,                 /* sos_type  */
  YYSYMBOL_exec_do = 175,                  /* exec_do  */
  YYSYMBOL_command = 176,                  /* command  */
  YYSYMBOL_idxset = 177,                   /* idxset  */
  YYSYMBOL_pure_idxset = 178,              /* pure_idxset  */
  YYSYMBOL_sexpr = 179,                    /* sexpr  */
  YYSYMBOL_sunion = 180,                   /* sunion  */
  YYSYMBOL_sproduct = 181,                 /* sproduct  */
  YYSYMBOL_sval = 182,                     /* sval  */
  YYSYMBOL_read = 183,                     /* read  */
  YYSYMBOL_read_par = 184,                 /* read_par  */
  YYSYMBOL_tuple_list = 185,               /* tuple_list  */
  YYSYMBOL_lexpr = 186,                    /* lexpr  */
  YYSYMBOL_tuple = 187,                    /* tuple  */
  YYSYMBOL_symidx = 188,                   /* symidx  */
  YYSYMBOL_cexpr_list = 189,               /* cexpr_list  */
  YYSYMBOL_cexpr = 190,                    /* cexpr  */
  YYSYMBOL_cproduct = 191,                 /* cproduct  */
  YYSYMBOL_cfactor = 192,                  /* cfactor  */
  YYSYMBOL_cexpo = 193,                    /* cexpo  */
  YYSYMBOL_cval = 194                      /* cval  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3509

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  136
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  319
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  938

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   378


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     131,   132,   126,   124,   130,   125,     2,   133,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   127,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   128,     2,   129,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   134,     2,   135,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   158,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   176,   183,   189,   195,   205,   206,   209,
     212,   215,   221,   230,   239,   248,   257,   266,   269,   279,
     282,   285,   288,   295,   299,   300,   307,   308,   316,   323,
     332,   342,   353,   362,   372,   376,   386,   387,   388,   392,
     395,   396,   397,   402,   410,   411,   412,   413,   418,   426,
     427,   431,   432,   440,   441,   444,   445,   449,   453,   457,
     460,   472,   475,   485,   491,   494,   497,   502,   507,   515,
     518,   523,   528,   535,   539,   544,   548,   554,   557,   562,
     567,   572,   576,   583,   590,   596,   602,   608,   613,   621,
     630,   639,   647,   658,   661,   665,   670,   678,   679,   682,
     685,   686,   689,   692,   693,   696,   699,   700,   703,   706,
     707,   710,   713,   714,   717,   720,   721,   722,   723,   724,
     728,   729,   733,   734,   735,   736,   737,   738,   739,   740,
     741,   742,   743,   744,   745,   749,   750,   751,   755,   756,
     757,   758,   759,   762,   763,   771,   772,   773,   777,   778,
     782,   783,   784,   790,   791,   794,   800,   803,   804,   805,
     806,   807,   808,   809,   810,   811,   812,   813,   816,   819,
     822,   830,   836,   839,   845,   846,   847,   855,   859,   860,
     861,   862,   863,   864,   865,   875,   876,   883,   886,   892,
     893,   894,   897,   898,   901,   902,   905,   906,   910,   911,
     912,   915,   919,   922,   927,   928,   929,   932,   935,   938,
     941,   944,   947,   950,   953,   954,   955,   956,   957,   958,
     959,   962,   965,   971,   972,   976,   977,   978,   979,   983,
     986,   989,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1017,  1023,  1024,  1028,  1031,  1037,  1040,  1046,
    1047,  1048,  1052,  1053,  1054,  1055,  1056,  1057,  1063,  1064,
    1065,  1069,  1070,  1071,  1074,  1077,  1080,  1083,  1089,  1090,
    1091,  1094,  1097,  1100,  1105,  1110,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,  1123,  1124,
    1125,  1126,  1128,  1129,  1130,  1133,  1136,  1139,  1142,  1145
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "DECLSET", "DECLPAR",
  "DECLVAR", "DECLMIN", "DECLMAX", "DECLSUB", "DECLSOS", "DEFNUMB",
  "DEFSTRG", "DEFBOOL", "DEFSET", "PRINT", "CHECK", "BINARY", "INTEGER",
  "REAL", "IMPLICIT", "ASGN", "DO", "WITH", "IN", "TO", "UNTIL", "BY",
  "FORALL", "EXISTS", "PRIORITY", "STARTVAL", "DEFAULT", "CMP_LE",
  "CMP_GE", "CMP_EQ", "CMP_LT", "CMP_GT", "CMP_NE", "INFTY", "AND", "OR",
  "XOR", "NOT", "SUM", "MIN", "MAX", "ARGMIN", "ARGMAX", "PROD", "IF",
  "THEN", "ELSE", "END", "INTER", "UNION", "CROSS", "SYMDIFF", "WITHOUT",
  "PROJ", "MOD", "DIV", "POW", "FAC", "CARD", "ROUND", "FLOOR", "CEIL",
  "RANDOM", "ORD", "ABS", "SGN", "LOG", "LN", "EXP", "SQRT", "SIN", "COS",
  "TAN", "ASIN", "ACOS", "ATAN", "POWER", "SGNPOW", "READ", "AS", "SKIP",
  "USE", "COMMENT", "MATCH", "SUBSETS", "INDEXSET", "POWERSET", "PERMUTE",
  "VIF", "VABS", "TYPE1", "TYPE2", "LENGTH", "SUBSTR", "NUMBSYM",
  "STRGSYM", "VARSYM", "SETSYM", "NUMBDEF", "STRGDEF", "BOOLDEF", "SETDEF",
  "DEFNAME", "NAME", "STRG", "NUMB", "SCALE", "SEPARATE", "CHECKONLY",
  "INDICATOR", "QUBO", "PENALTY1", "PENALTY2", "PENALTY3", "PENALTY4",
  "PENALTY5", "PENALTY6", "PENALTY7", "PENALTY8", "'+'", "'-'", "'*'",
  "';'", "'['", "']'", "','", "'('", "')'", "'/'", "'{'", "'}'", "$accept",
  "stmt", "decl_set", "set_entry_list", "set_entry", "def_numb",
  "def_strg", "def_bool", "def_set", "name_list", "decl_par",
  "par_singleton", "par_default", "decl_var", "var_type", "lower", "upper",
  "priority", "startval", "cexpr_entry_list", "cexpr_entry", "matrix_head",
  "matrix_body", "decl_obj", "decl_sub", "constraint_list", "constraint",
  "vbool", "con_attr_list", "con_attr", "con_type", "vexpr", "vproduct",
  "vfactor", "vexpo", "vval", "decl_sos", "soset", "sos_type", "exec_do",
  "command", "idxset", "pure_idxset", "sexpr", "sunion", "sproduct",
  "sval", "read", "read_par", "tuple_list", "lexpr", "tuple", "symidx",
  "cexpr_list", "cexpr", "cproduct", "cfactor", "cexpo", "cval", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-543)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    1361,   -61,   -41,   -12,    15,    32,    42,    61,    66,    74,
      79,    97,   456,   215,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  -543,  -543,  -543,  -543,     6,    17,     2,   202,   211,
     221,   261,   105,   119,   135,   177,  1559,  1652,  1075,   230,
    -543,  3375,   984,  1450,  1075,   294,   312,  -543,   422,  1075,
     312,  2744,  2744,  1726,   110,   263,   263,   263,   263,   243,
    2051,  1652,  1075,     4,     8,  1097,  1346,  1075,  1652,  1075,
    1075,   247,   304,   333,   353,   364,   368,   375,   382,   385,
     412,   466,   509,   513,   537,   551,   563,   591,   610,   625,
     658,   666,   669,   692,   543,   543,  -543,   543,   695,   701,
     704,   728,  -543,  -543,  -543,  3296,  3296,  1652,  1817,   542,
    -543,   -17,  -543,   802,   564,   574,    51,    98,  -543,  -543,
     546,   542,   802,   564,    51,  1652,  3375,   765,  -543,  1002,
     726,  -543,  1117,   844,   754,  2951,  1652,  2951,  2951,   767,
     758,  -543,   888,   895,  2951,   493,   786,  2951,   905,  3020,
     900,   812,  -543,   807,   900,  1075,  1652,   818,   823,   824,
     838,   855,   861,   867,   874,   876,   878,   884,   886,   543,
    2813,  2813,  2744,   459,   -81,  -543,  -543,   930,   561,   229,
     879,  1075,  1652,  2606,    -3,  -543,    28,   245,  1075,  -543,
    -543,   891,   294,  -543,   -26,   347,   361,   438,  1075,  -543,
      -1,   493,  -543,  1007,  1910,  1008,  1910,  1011,  2534,  1014,
    2534,  1022,  1040,   571,  1042,  1045,  3375,  3375,  2951,  2951,
    2951,  2951,  3375,  2951,  2951,  2951,  2951,  2951,  2951,  2951,
    2951,  2951,  2951,  2951,  2951,   948,  3375,  2951,  2951,  2951,
    -543,  -543,  -543,  2951,  2951,  2951,  2951,  -543,  -543,   283,
     173,   219,  1652,  2534,  -543,    -9,   895,   196,   726,   238,
      41,  3375,  3375,  3375,  3375,  3375,  3375,  3375,  3375,  3375,
    3375,  3375,  3375,  1296,  1296,  1652,  1652,  1652,  3375,  2951,
    2951,  2951,  2951,  2951,  2951,  2951,  2951,  2951,  3227,  3227,
    3227,  3227,  3227,  -543,   593,   335,   456,  3375,  -543,   399,
    1003,     1,   707,   379,   797,  -543,  1036,  2951,   888,  2951,
    2951,  2951,  2951,  -543,   493,  1025,   493,  2951,   945,  1652,
    2327,   493,  2120,   294,  -543,   746,   951,  1059,   742,  2744,
    2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,
    2744,  -543,  3338,  3338,  -543,  -543,   863,   902,  2744,  2744,
    -543,  2882,  3227,  3227,  2744,  2744,  2882,  -543,  1062,   769,
    2606,  2606,   790,   505,   703,  2675,  -543,  -543,  -543,  -543,
    2744,  2744,  1070,  -543,  1093,  1009,  1100,  1116,  1125,  1137,
     993,  -543,  2951,  1028,   450,  3227,  1043,   517,  3227,   922,
    2951,   943,  2951,  3227,  1652,  1296,  1155,   828,   671,   987,
    1006,  1015,   577,   860,  1024,  1029,  1034,  1044,  1061,  1105,
    1111,  1124,  1134,  1214,  1216,  1251,  1046,   780,  1282,   589,
     588,   600,   682,   688,   835,  -543,  -543,  -543,   850,  1982,
    -543,  1036,  -543,  -543,  2951,  2951,  1002,  1002,  1002,  1002,
    1002,  1002,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  1145,  1145,  1002,   493,   493,   493,   493,   493,   493,
     493,    98,    98,  -543,  -543,  -543,  -543,  -543,  3375,  -543,
      37,  1057,  1065,   -79,  -543,  3375,   910,  -543,  2951,  2951,
    -543,    18,  2951,   493,   493,   493,   493,  1253,   493,  -543,
     856,  -543,  -543,  1652,   493,   905,   294,   312,   761,   312,
    -543,  2744,  2744,  1284,  1307,  1309,  1318,  1344,  1354,  1357,
    1372,  1376,  1378,  1407,  1410,  1412,  1431,  1437,  1440,  1446,
    1452,   629,   714,  1458,  1075,  1652,  1066,  1069,  1074,  1076,
    1079,  1094,  1096,  1103,  1108,  2744,  -543,   -81,   229,   -81,
     229,  -543,  -543,  -543,  -543,   -81,   229,   -81,   229,  -543,
    1726,  1726,  -543,   218,   348,   499,  2606,  2606,  2606,  2744,
    2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,
    2744,  2744,  -543,   731,   561,    31,   167,   110,  2744,  -543,
    2951,  2951,  1652,  3375,  -543,    98,  -543,  -543,  -543,  -543,
    -543,  -543,  1075,   493,  1075,   493,  -543,   655,   461,   642,
    -543,   -17,  1036,  -543,  -543,  -543,  -543,  2951,  2951,  -543,
    -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  -543,  -543,  -543,  2951,  -543,  -543,  -543,  -543,  -543,
    2534,  1055,   323,  -543,   -10,     3,   957,  1652,  3375,  3375,
    -543,  1036,  1002,   137,  1126,   109,   493,  -543,    19,  2951,
     -18,   924,  3089,   887,  1113,   905,   900,  1119,   900,   -81,
     229,   239,   287,  -543,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  -543,  2951,  2951,  -543,  1233,   921,  2744,  2744,  2744,
    2744,  2744,  2744,  2744,  2744,  2744,  1221,   233,   236,   279,
    -543,  -543,  1225,  1225,    28,   245,   731,   561,   731,   561,
     731,   561,   731,   561,   731,   561,   731,   561,   731,   731,
     731,   731,   731,   731,  1142,  1142,  2951,  2951,  1142,  2951,
    2951,  1142,  -543,   731,  1010,  1087,   138,  1138,  1234,  1246,
    3375,  1652,  2951,  1144,  1461,   777,   794,  -543,  -543,  2951,
    -543,  2951,  -543,   802,   940,   809,  -543,  -543,  -543,  -543,
    1146,  2951,  1147,  -543,  2396,   308,  2189,  -543,  1156,   294,
    -543,  1158,  2744,  1464,  1486,  2744,  2744,  1726,  -543,  2744,
    2744,  1342,   493,   493,   493,   493,  -543,  -543,  -543,  -543,
    2951,  2951,   968,    29,   228,  -543,  -543,  2951,  2951,   362,
     384,  2951,  -543,  -543,   493,  -543,  1236,  3158,  1239,   316,
    -543,   905,  -543,   262,  -543,  -543,   229,    43,   251,   270,
     276,   281,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  -543,  -543,  -543,  -543,  -543,  1142,  1142,  1142,  1142,
     493,   493,  -543,  -543,  -543,  1516,  1518,  -543,  -543,   -27,
    2951,  2465,  2951,  2258,  1168,  -543,  -543,  2744,  -543,  2744,
    -543,  2744,  -543,  2744,  -543,  -543,  -543,  2951,  -543,   289,
    1247,   325,  1254,  -543,    28,   245,  1142,    28,   245,  1142,
      28,   245,  1142,    28,   245,  1142,  1520,  -543,  -543,  -543,
    -543,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  2744,  -543,
     344,   350,   357,   393,   424,   478,   495,   514,   521,   540,
     548,   573,   575,   618,   674,   677,  -543,  -543,  -543,  -543,
    -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,  -543,
    -543,  -543,  1142,  1142,  1142,  1142,  1142,  1142,  1142,  1142,
    1142,  1142,  1142,  1142,  1142,  1142,  1142,  1142
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     8,     9,    10,    11,     3,
       4,     5,     6,     7,    12,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     0,     0,     0,    59,    49,    47,     0,     0,
      49,     0,     0,     0,   184,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   265,   265,   192,   265,     0,     0,
       0,     0,   290,   289,   288,     0,     0,     0,     0,   190,
     199,   206,   208,   191,   189,   188,   267,   269,   272,   278,
     281,     0,   193,     0,     0,     0,     0,     0,   195,   196,
       0,   187,     0,     0,     0,     0,     0,     0,     0,     0,
      34,    63,     0,    65,     0,    35,     0,     0,    61,     0,
      54,     0,    48,     0,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   265,
       0,     0,     0,     0,   148,   155,   160,   163,     0,   269,
       0,     0,     0,     0,     0,    74,     0,     0,     0,   185,
     186,     0,    59,    27,     0,     0,     0,     0,     0,   263,
       0,   267,   257,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     291,   292,   212,     0,     0,     0,     0,   279,   280,     0,
       0,     0,     0,     0,   215,     0,   241,     0,   239,     0,
     267,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   295,     0,     0,     0,     0,    13,     0,
       0,     0,     0,     0,     0,    32,     0,     0,    66,     0,
       0,     0,     0,   234,    67,     0,    60,     0,     0,     0,
       0,    50,     0,    59,    41,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   166,     0,     0,   161,   162,     0,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,    72,     0,     0,
       0,     0,     0,     0,     0,     0,    73,   145,   146,   147,
       0,     0,     0,   181,     0,     0,     0,     0,     0,     0,
       0,   264,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,   258,   312,     0,     0,
     227,     0,   225,   226,     0,     0,   253,   251,   248,   252,
     250,   249,   205,   200,   202,   204,   201,   203,   209,   210,
     254,   255,   256,   259,   268,   247,   245,   242,   246,   244,
     243,   270,   271,   275,   276,   273,   274,   282,     0,   194,
     198,     0,     0,     0,    17,     0,     0,    68,     0,     0,
      64,     0,     0,   235,   236,   237,   238,     0,    62,    43,
       0,    51,    56,     0,    55,    61,    59,    49,     0,    49,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   180,   149,   151,   150,
     152,   159,   156,   157,   164,   153,   270,   154,   271,   158,
       0,     0,   128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,   130,   130,   130,   130,   184,     0,    28,
       0,     0,     0,     0,   260,   283,   286,   318,   284,   287,
     319,   285,     0,   220,     0,   222,   277,     0,     0,     0,
     211,   207,     0,   296,   299,   300,   301,     0,     0,   297,
     298,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   231,   214,   313,     0,   266,   293,   294,   261,   213,
       0,     0,     0,   240,     0,     0,     0,     0,     0,     0,
      16,     0,    22,     0,     0,     0,   233,    69,     0,     0,
      36,     0,     0,     0,     0,    61,    54,     0,    54,   165,
     283,     0,     0,   175,   176,   169,   171,   170,   168,   172,
     173,   174,     0,     0,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,     0,     0,     0,
     129,   125,   126,   127,     0,     0,   113,   115,   116,   118,
     110,   112,   119,   121,   122,   124,   107,   109,   114,   117,
     111,   120,   123,   108,    79,    80,     0,     0,    81,     0,
       0,    82,   183,   182,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   229,   228,     0,
     217,     0,   219,   197,     0,     0,    18,    15,    14,    70,
       0,     0,     0,    30,     0,     0,     0,    45,     0,    59,
      40,     0,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,   130,   130,   130,   130,    23,    24,    25,    26,
       0,     0,     0,     0,     0,   230,   315,     0,     0,     0,
       0,     0,    21,    31,    37,    29,     0,     0,     0,     0,
      42,    61,    38,     0,   177,   178,     0,     0,     0,     0,
       0,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   131,    83,    85,    84,    86,
     221,   223,   232,   262,   316,     0,     0,   216,   218,     0,
       0,     0,     0,     0,     0,   179,    78,     0,   130,     0,
     130,     0,   130,     0,   130,   317,   314,     0,    19,     0,
       0,     0,     0,    44,     0,     0,   103,     0,     0,   105,
       0,     0,   104,     0,     0,   106,     0,    53,    52,    58,
      57,     0,     0,     0,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,    87,    91,    90,    97,    89,    96,    95,   101,
      88,    94,    93,   100,    92,    99,    98,   102
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -543,  -543,  -543,   833,   670,  -543,  -543,  -543,  -543,  1106,
    -543,  -543,  -543,  -543,   985,   -44,  -151,  -190,  -491,   825,
    1031,  -137,  -543,  -543,  -543,  -542,   949,  -330,  -356,  -543,
     -80,    91,  -321,  -121,  -543,  -543,  -543,   738,  -543,  -543,
    1038,   881,   816,  1016,  -194,   953,  -263,  1250,  -543,  -543,
     -15,   329,   -53,   103,   -36,   538,  -234,   -91,  -543
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    13,    14,   473,   474,    15,    16,    17,    18,   194,
      19,   139,   752,    20,    50,   150,   323,   148,   318,   140,
     141,   142,   308,    21,    22,   184,   185,   362,   714,   825,
     370,   363,   174,   175,   176,   177,    23,   191,   192,    24,
      39,   127,   128,   129,   110,   111,   112,   143,   313,   257,
     113,   130,   240,   115,   178,   117,   118,   119,   120
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     116,   124,   374,   326,   654,   307,   154,   145,   686,   687,
     448,   449,   429,   751,   247,   248,   739,   187,    45,    46,
      47,    48,   122,   477,   201,   124,    41,   537,   539,   741,
     552,   553,   124,   545,   547,   381,   365,    43,   273,    60,
     647,   749,   241,    60,   242,   351,   202,    25,   640,   344,
     345,   641,   352,   213,   463,   464,   465,   466,   467,   637,
     367,   368,   369,   716,   717,   434,   435,    26,   275,   276,
     277,   251,   260,   442,   443,   444,   445,   446,   447,   247,
     248,   833,   365,   280,   281,   282,   283,   284,   285,   124,
     267,   268,   250,   269,   270,   846,    27,   286,   287,   201,
     124,   303,   304,   857,   375,   858,   376,   371,   314,   274,
     294,   316,   306,   321,   286,   287,   341,   542,   543,   544,
     124,   302,   465,    28,   366,   740,   430,   286,   287,   279,
      49,   279,   600,   495,    42,   204,   347,   188,   742,   206,
      29,   328,   173,   180,   186,    44,   124,   364,   279,   279,
      30,   588,   348,   349,   591,   348,   349,   288,   289,   596,
     732,   271,   272,   200,   758,   286,   287,   359,   201,    31,
     201,   482,   389,    32,   391,   286,   287,   275,   276,   277,
     659,    33,   399,   400,   401,   402,    34,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,   414,   415,   719,
     720,   418,   419,   201,    35,   189,   190,   201,   201,   201,
     201,   259,   275,   276,   277,    40,   124,   304,   715,   718,
     721,   344,   345,    51,   290,   807,   691,   692,   693,   248,
     541,   291,    52,   286,   287,   549,    55,   428,   301,   124,
     124,   124,    53,   454,   455,   456,   457,   458,   459,   460,
      56,   280,   281,   282,   283,   284,   285,   556,   557,   558,
     450,   451,   452,   346,   747,   778,    57,   641,   367,   368,
     369,   201,   365,   483,   484,   485,   486,   367,   368,   369,
     834,   488,    54,   124,   767,   768,   494,   762,   288,   289,
     762,   354,   355,   504,   506,   508,   510,   512,   514,   516,
     518,   520,   847,   848,   490,   426,   655,   384,    58,   387,
     844,   367,   368,   369,   845,   261,   262,   263,   264,   265,
     266,   849,   850,   147,   364,   555,   431,   851,   852,   187,
     732,   432,   853,   854,   574,   576,   267,   268,   732,   269,
     270,   877,   420,   286,   287,   149,   421,   422,   423,   424,
     690,   427,   286,   287,   593,   356,   595,   131,   599,   797,
     348,   349,   291,   348,   349,   114,   123,   843,   279,   354,
     355,   193,   144,   433,   198,   348,   349,   879,   216,   598,
     560,   561,   562,   563,   564,   565,   348,   349,   267,   268,
     123,   269,   270,   632,   354,   355,   906,   123,   634,   635,
     348,   349,   907,   354,   355,   354,   355,   271,   272,   908,
     481,   354,   355,   286,   287,   425,   826,   827,   828,   829,
     503,   505,   507,   509,   511,   513,   515,   517,   519,   521,
     522,   523,   286,   287,    60,   217,   123,   258,   151,   152,
     286,   287,   645,   646,   659,   909,   201,   286,   287,   286,
     287,   651,   554,   656,   123,   658,   186,   124,   738,   271,
     272,   573,   575,   479,   218,   123,   662,   425,   348,   349,
      36,    37,   348,   349,   354,   355,   910,   375,   653,   377,
     536,   348,   349,    38,   219,   123,   286,   287,   471,   124,
     472,   375,   866,   378,   869,   220,   872,   837,   875,   221,
     275,   276,   277,   286,   287,   759,   222,   761,   286,   287,
     676,   123,   731,   223,   187,   689,   224,   354,   355,   838,
     364,   364,   364,   695,   697,   699,   701,   703,   705,   707,
     911,   566,   567,   568,   569,   570,   571,   560,   561,   562,
     563,   564,   565,   225,   724,   725,   124,   912,   348,   349,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,   933,   934,   935,   936,   937,   913,   726,   375,   801,
     379,   734,   735,   914,   261,   262,   263,   264,   265,   266,
     279,   123,   587,   348,   349,   648,   350,   278,   736,   179,
     179,   179,   915,   661,   645,   267,   268,   226,   269,   270,
     916,   124,   354,   355,   123,   123,   123,   292,   293,   371,
     275,   276,   277,   750,   769,   770,   755,   286,   287,   348,
     349,   394,   743,   354,   355,   917,   346,   918,   475,   348,
     349,   427,   275,   276,   277,   144,   763,   764,   354,   355,
     227,   186,   688,   468,   228,   348,   349,   279,   123,   590,
     694,   696,   698,   700,   702,   704,   706,   708,   709,   710,
     711,   712,   713,   248,   354,   355,   271,   272,   229,   723,
     919,   239,   348,   349,   280,   281,   282,   283,   284,   285,
     772,   773,   230,   774,   775,   354,   355,   261,   262,   263,
     264,   265,   266,   732,   231,   124,   784,   354,   355,   348,
     349,   286,   287,   789,   279,   790,   730,   607,   267,   268,
     179,   269,   270,   286,   287,   794,   783,   625,   279,   624,
     799,   179,   232,   123,   267,   268,   920,   269,   270,   921,
     279,   187,   626,   809,   811,   566,   567,   568,   569,   570,
     571,   233,   354,   355,   830,   831,   275,   276,   277,   297,
     248,   835,   836,   348,   349,   839,   234,   478,   631,   672,
     633,   784,   496,   497,    47,   498,   286,   287,   503,   505,
     507,   509,   511,   513,   515,   517,   519,   657,   152,   271,
     272,   275,   276,   277,   881,   882,   296,   883,   884,   235,
     885,   886,   502,   887,   888,   271,   272,   236,   348,   349,
     237,   354,   355,   603,   859,   475,   861,   784,   275,   276,
     277,   865,   279,   868,   627,   871,   144,   874,   279,   551,
     628,   876,   123,   238,   461,   462,   243,   354,   355,   556,
     557,   558,   244,   267,   268,   245,   269,   270,   348,   349,
     559,   275,   276,   277,   673,   891,   893,   895,   897,   899,
     901,   903,   905,   803,   123,   348,   349,   661,   186,   246,
     808,   810,   267,   268,   299,   269,   270,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   205,
     207,   267,   268,   300,   269,   270,   538,   540,   306,   275,
     276,   277,   546,   548,   305,   275,   276,   277,   179,   179,
     630,   286,   287,   179,   271,   272,   652,   787,   179,   179,
     135,   123,   622,   267,   268,   315,   269,   270,   286,   287,
     585,   286,   287,   134,   788,   146,   275,   276,   277,   427,
     153,   733,   322,   271,   272,   317,   325,   756,   864,   324,
     867,   792,   870,   203,   873,    60,   209,   211,   212,   329,
     214,   215,   271,   272,   330,   331,    65,    66,   602,   125,
     275,   276,   277,    69,    70,   279,   123,   629,    71,   332,
     475,   766,   890,   892,   894,   896,   898,   900,   902,   904,
     309,   310,   311,   312,   271,   272,   333,   348,   349,   255,
     608,   353,   334,   267,   268,   536,   269,   270,   335,   471,
      90,   472,    91,   348,   349,   336,   357,   337,   730,   338,
     267,   268,    97,   269,   270,   339,   101,   340,   373,    60,
     832,   267,   268,   476,   269,   270,   354,   355,   382,   385,
      65,    66,   388,   125,   427,   390,   327,    69,    70,   660,
     179,   126,    71,   392,   108,   487,   286,   287,   286,   287,
     416,   753,   109,   121,   592,   267,   268,   132,   269,   270,
     123,   393,   358,   395,   271,   272,   396,   286,   287,   372,
     791,    60,   489,   179,    90,   594,    91,   121,   500,   380,
     501,   271,   272,   550,   121,   383,    97,   386,   179,   179,
     101,   577,   271,   272,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
      60,   286,   287,   133,   578,   126,   179,   579,   108,   604,
     580,    65,    66,   249,   125,   584,   271,   272,    69,    70,
     286,   287,    60,    71,   286,   287,   581,   776,   605,   286,
     287,   121,   295,    65,    66,   582,   125,   606,   286,   287,
      69,    70,   121,   286,   287,    71,   609,   583,   286,   287,
     586,   610,   195,   196,   197,    90,   611,    91,   286,   287,
     267,   268,   121,   269,   270,   589,   612,    97,   621,   267,
     268,   101,   269,   270,   275,   286,   287,    90,   638,    91,
     737,   267,   268,   613,   269,   270,   639,   677,   121,    97,
     678,    65,    66,   101,   125,   679,   126,   680,    69,   108,
     681,   286,   287,    71,   777,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   295,   682,   295,   683,   208,   286,
     287,   108,   397,   398,   684,   286,   287,   614,   403,   685,
     757,   271,   272,   615,   298,    90,   760,    91,   286,   287,
     271,   272,   417,   748,   765,   780,   616,    97,   286,   287,
     365,   101,   271,   272,   556,   779,   617,   781,   121,   295,
     286,   287,   771,   793,   795,   135,   785,   436,   437,   438,
     439,   440,   441,   800,   649,   802,   126,   840,    60,   108,
     842,   121,   121,   121,   453,   863,    62,    63,    64,   878,
     179,    67,   136,   806,   179,   179,   880,   179,   179,   643,
     499,   746,   650,   470,   572,   722,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,   469,   121,   137,   480,   286,   287,
     286,   287,    65,    66,     0,   125,   618,     0,   619,   601,
      92,    93,    94,    95,    71,     0,    98,    99,   256,     0,
       0,   102,   103,   104,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,   286,   287,   105,   106,     0,
       0,    60,    12,   620,   138,   179,    90,   179,    91,   179,
       0,   179,    65,    66,     0,   125,     0,     0,    97,    69,
      70,     0,   101,     0,    71,   675,   286,   287,   348,   349,
     597,     0,     0,     0,   623,     0,   663,     0,     0,   179,
     179,   179,   179,   179,   179,   179,   179,   126,     0,     0,
     108,   354,   355,   348,   349,     0,    90,     0,    91,   609,
       0,   664,   354,   355,     0,     0,     0,     0,    97,     0,
     610,     0,   101,   812,   813,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   824,     0,     0,   348,   349,
       0,     0,   135,   728,     0,   729,   665,   210,   354,   355,
     108,   348,   349,     0,   636,    60,   611,     0,     0,   666,
       0,   642,   644,    62,    63,    64,   354,   355,    67,   136,
     348,   349,   354,   355,   612,     0,     0,     0,   667,   121,
     613,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,   348,   349,   137,   354,   355,   348,   349,     0,   668,
       0,   121,   614,     0,   669,     0,     0,    92,    93,    94,
      95,     0,     0,    98,    99,   354,   355,     0,   102,   103,
     104,   348,   349,   615,   354,   355,     0,     0,     0,   670,
     348,   349,   616,     0,   105,   106,   354,   355,   671,     0,
       0,   138,   348,   349,   617,   286,   287,    59,   286,   287,
     674,     0,     0,   786,    60,     0,   804,     0,   121,   727,
       0,    61,    62,    63,    64,    65,    66,    67,    68,     0,
     286,   287,    69,    70,     0,     0,     0,    71,   805,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
     286,   287,   286,   287,   286,   287,   636,     0,   855,    90,
     856,    91,   889,   121,   744,   745,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,     0,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,     0,     0,   105,   106,     0,     0,    60,     0,     0,
     107,     0,     0,   108,    61,    62,    63,    64,    65,    66,
      67,    68,     0,     0,     0,    69,    70,     0,     0,     0,
      71,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    90,     0,    91,     0,   782,   121,     0,    92,
      93,    94,    95,   181,    97,    98,    99,   100,   101,     0,
     102,   103,   104,     0,     0,     0,     0,     0,     0,   155,
      63,    64,     0,     0,    67,   182,   105,   106,     0,     0,
       0,     0,     0,   107,     0,     0,   108,     0,     0,    72,
      73,    74,    75,    76,    77,   157,   158,   159,   160,   161,
     162,   163,   164,   165,    87,    88,    89,   166,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
     168,     0,     0,    92,    93,    94,    95,   169,     0,    98,
      99,     0,     0,     0,   102,   103,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     170,   171,    60,     0,     0,     0,     0,   172,     0,     0,
      62,    63,    64,    65,    66,    67,   252,     0,     0,     0,
      69,    70,     0,     0,     0,    71,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,     0,     0,
     137,     0,     0,     0,     0,     0,     0,    90,     0,    91,
       0,     0,     0,     0,    92,    93,    94,    95,     0,    97,
      98,    99,     0,   101,     0,   102,   103,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,   106,     0,     0,    60,     0,     0,   253,     0,
       0,   108,   254,    62,    63,    64,    65,    66,    67,   252,
       0,     0,     0,    69,    70,     0,     0,     0,    71,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      90,     0,    91,     0,     0,     0,     0,    92,    93,    94,
      95,     0,    97,    98,    99,     0,   101,    60,   102,   103,
     104,     0,     0,     0,     0,    62,    63,    64,     0,     0,
      67,   136,     0,     0,   105,   106,     0,     0,     0,     0,
       0,   253,     0,     0,   108,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    92,
      93,    94,    95,     0,     0,    98,    99,   199,     0,     0,
     102,   103,   104,     0,    62,    63,    64,     0,     0,    67,
     136,     0,     0,     0,     0,     0,   105,   106,     0,     0,
       0,     0,     0,   138,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,    98,    99,     0,     0,   492,   102,
     103,   104,     0,    62,    63,    64,     0,     0,    67,   493,
       0,     0,     0,     0,     0,   105,   106,     0,     0,     0,
       0,     0,   138,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,    98,    99,     0,     0,   798,   102,   103,
     104,     0,    62,    63,    64,     0,     0,    67,   136,     0,
       0,     0,     0,     0,   105,   106,     0,     0,     0,     0,
       0,   138,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    98,    99,     0,     0,   862,   102,   103,   104,
       0,    62,    63,    64,     0,     0,    67,   136,     0,     0,
       0,     0,     0,   105,   106,     0,     0,     0,     0,     0,
     138,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    98,    99,     0,     0,   491,   102,   103,   104,     0,
      62,    63,    64,     0,     0,     0,   136,     0,     0,     0,
       0,     0,   105,   106,     0,     0,     0,     0,     0,   138,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      98,    99,     0,     0,   796,   102,   103,   104,     0,    62,
      63,    64,     0,     0,     0,   136,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   138,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,    98,
      99,     0,     0,   860,   102,   103,   104,     0,    62,    63,
      64,     0,     0,     0,   136,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,    98,    99,
       0,     0,     0,   102,   103,   104,     0,    62,    63,    64,
      65,    66,    67,   252,     0,     0,     0,    69,    70,     0,
       0,     0,    71,     0,     0,     0,   138,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    90,     0,    91,     0,     0,     0,
       0,    92,    93,    94,    95,     0,    97,    98,    99,     0,
     101,     0,   102,   103,   104,     0,     0,     0,   360,   155,
      63,    64,     0,     0,    67,   156,     0,     0,   105,   106,
       0,     0,     0,     0,     0,   253,     0,     0,   108,    72,
      73,    74,    75,    76,    77,   157,   158,   159,   160,   161,
     162,   163,   164,   165,    87,    88,    89,   166,   167,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     168,     0,     0,    92,    93,    94,    95,   169,     0,    98,
      99,     0,     0,     0,   102,   103,   104,     0,   155,    63,
      64,     0,     0,    67,   156,     0,     0,     0,     0,     0,
     170,   171,     0,     0,     0,     0,     0,   361,    72,    73,
      74,    75,    76,    77,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    87,    88,    89,   166,   167,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   183,   168,
       0,     0,    92,    93,    94,    95,   169,     0,    98,    99,
       0,     0,     0,   102,   103,   104,     0,   155,    63,    64,
       0,     0,    67,   156,     0,     0,     0,     0,     0,   170,
     171,     0,     0,     0,     0,     0,   172,    72,    73,    74,
      75,    76,    77,   157,   158,   159,   160,   161,   162,   163,
     164,   165,    87,    88,    89,   166,   167,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   168,     0,
       0,    92,    93,    94,    95,   169,     0,    98,    99,     0,
       0,     0,   102,   103,   104,     0,   155,    63,    64,     0,
       0,     0,   156,     0,     0,     0,     0,     0,   170,   171,
       0,     0,     0,     0,     0,   172,    72,    73,    74,    75,
      76,    77,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    87,    88,    89,   166,   167,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   168,     0,     0,
      92,    93,    94,    95,   169,     0,    98,    99,     0,     0,
       0,   102,   103,   104,     0,   155,    63,    64,     0,     0,
       0,   156,     0,     0,     0,     0,     0,   342,   343,     0,
       0,     0,     0,     0,   172,    72,    73,    74,    75,    76,
      77,   157,   158,   159,   160,   161,   162,   163,   164,   165,
      87,    88,    89,   166,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   168,     0,     0,    92,
      93,    94,    95,   169,     0,    98,    99,     0,     0,     0,
     102,   103,   104,     0,    62,    63,    64,     0,     0,    67,
     136,     0,     0,     0,     0,     0,   170,   171,     0,     0,
       0,     0,     0,   172,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,    98,    99,     0,     0,     0,   102,
     103,   104,     0,    62,    63,    64,     0,     0,    67,   319,
       0,     0,     0,     0,     0,   105,   106,     0,     0,     0,
       0,     0,   138,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,     0,     0,    98,    99,     0,     0,     0,   102,   103,
     104,     0,    62,    63,    64,     0,     0,    67,   136,     0,
       0,     0,     0,     0,   105,   320,     0,     0,     0,     0,
       0,   138,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,    98,    99,     0,     0,     0,   102,   103,   104,
       0,    62,    63,    64,     0,     0,    67,   136,     0,     0,
       0,     0,     0,   105,   754,     0,     0,     0,     0,     0,
     138,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,     0,
       0,    98,    99,     0,     0,     0,   102,   103,   104,     0,
      62,    63,    64,     0,     0,     0,   136,     0,     0,     0,
       0,     0,   105,   841,     0,     0,     0,     0,     0,   138,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
      98,    99,     0,     0,     0,   102,   103,   104,     0,    62,
      63,    64,     0,     0,     0,   136,     0,     0,     0,     0,
       0,   105,   106,     0,     0,     0,     0,     0,   138,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,     0,     0,     0,
       0,   524,     0,     0,     0,     0,     0,   525,     0,     0,
       0,     0,     0,    92,    93,    94,    95,     0,     0,    98,
      99,     0,     0,     0,   102,   103,   104,   526,   527,   528,
     529,   530,   531,   532,   533,   534,     0,     0,     0,   166,
     167,    65,    66,     0,   125,     0,     0,   138,    69,    70,
       0,     0,   168,    71,     0,     0,     0,     0,     0,   169,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   342,   343,     0,    90,     0,    91,     0,   535,
       0,     0,     0,     0,     0,     0,     0,    97,     0,     0,
       0,   101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   126,     0,     0,   108
};

static const yytype_int16 yycheck[] =
{
      36,    37,   192,   154,   495,   142,    50,    43,   550,   551,
     273,   274,    21,    31,   105,   106,    26,    53,    16,    17,
      18,    19,    37,    22,    60,    61,    20,   348,   349,    26,
     360,   361,    68,   354,   355,    36,    39,    20,    55,    35,
      22,    22,    95,    35,    97,   126,    61,   108,   127,   170,
     171,   130,   133,    68,   288,   289,   290,   291,   292,    22,
      32,    33,    34,    32,    33,    24,    25,   108,    39,    40,
      41,   107,   108,   267,   268,   269,   270,   271,   272,   170,
     171,    52,    39,    32,    33,    34,    35,    36,    37,   125,
      53,    54,   107,    56,    57,    52,   108,   124,   125,   135,
     136,   137,   138,   130,   130,   132,   132,   187,   144,   126,
     125,   147,   130,   149,   124,   125,   169,   351,   352,   353,
     156,   136,   356,   108,   127,   135,   135,   124,   125,   130,
     128,   130,   395,   323,   128,   131,   172,    27,   135,   131,
     108,   156,    51,    52,    53,   128,   182,   183,   130,   130,
     108,   385,   124,   125,   388,   124,   125,    59,    60,   393,
      51,   124,   125,    60,   655,   124,   125,   182,   204,   108,
     206,   308,   208,   107,   210,   124,   125,    39,    40,    41,
     501,   107,   218,   219,   220,   221,   107,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,    32,
      33,   237,   238,   239,   107,    95,    96,   243,   244,   245,
     246,   108,    39,    40,    41,     0,   252,   253,   574,   575,
     576,   342,   343,    21,   126,   767,   556,   557,   558,   320,
     351,   133,    21,   124,   125,   356,   131,   252,   135,   275,
     276,   277,    21,   279,   280,   281,   282,   283,   284,   285,
     131,    32,    33,    34,    35,    36,    37,    39,    40,    41,
     275,   276,   277,   172,   127,   127,   131,   130,    32,    33,
      34,   307,    39,   309,   310,   311,   312,    32,    33,    34,
      52,   317,    21,   319,    51,    52,   322,    51,    59,    60,
      51,   124,   125,   329,   330,   331,   332,   333,   334,   335,
     336,   337,    51,    52,   319,   132,   496,   204,   131,   206,
     801,    32,    33,    34,    52,    32,    33,    34,    35,    36,
      37,    51,    52,    29,   360,   361,   130,    51,    52,   365,
      51,   135,    51,    52,   370,   371,    53,    54,    51,    56,
      57,    52,   239,   124,   125,    33,   243,   244,   245,   246,
     132,   132,   124,   125,   390,   126,   392,   127,   394,    51,
     124,   125,   133,   124,   125,    36,    37,    51,   130,   124,
     125,   108,    43,   135,   131,   124,   125,    52,   131,   394,
      32,    33,    34,    35,    36,    37,   124,   125,    53,    54,
      61,    56,    57,   429,   124,   125,    52,    68,   434,   435,
     124,   125,    52,   124,   125,   124,   125,   124,   125,    52,
     307,   124,   125,   124,   125,   132,   772,   773,   774,   775,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   124,   125,    35,   131,   107,   108,    16,    17,
     124,   125,   478,   479,   765,    52,   482,   124,   125,   124,
     125,   487,   361,   497,   125,   499,   365,   493,   135,   124,
     125,   370,   371,    84,   131,   136,   502,   132,   124,   125,
      14,    15,   124,   125,   124,   125,    52,   130,   493,   132,
     132,   124,   125,    27,   131,   156,   124,   125,    89,   525,
      91,   130,   848,   132,   850,   131,   852,   135,   854,   131,
      39,    40,    41,   124,   125,   656,   131,   658,   124,   125,
     525,   182,    51,   131,   550,   551,   131,   124,   125,   135,
     556,   557,   558,   559,   560,   561,   562,   563,   564,   565,
      52,    32,    33,    34,    35,    36,    37,    32,    33,    34,
      35,    36,    37,   131,   580,   581,   582,    52,   124,   125,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,    52,   582,   130,   759,
     132,   607,   608,    52,    32,    33,    34,    35,    36,    37,
     130,   252,   132,   124,   125,   482,   127,    23,   624,    51,
      52,    53,    52,   502,   630,    53,    54,   131,    56,    57,
      52,   637,   124,   125,   275,   276,   277,    61,    62,   689,
      39,    40,    41,   649,   694,   695,   652,   124,   125,   124,
     125,    50,   637,   124,   125,    52,   535,    52,   299,   124,
     125,   132,    39,    40,    41,   306,   672,   673,   124,   125,
     131,   550,   551,    50,   131,   124,   125,   130,   319,   132,
     559,   560,   561,   562,   563,   564,   565,   566,   567,   568,
     569,   570,   571,   754,   124,   125,   124,   125,   131,   578,
      52,   128,   124,   125,    32,    33,    34,    35,    36,    37,
     716,   717,   131,   719,   720,   124,   125,    32,    33,    34,
      35,    36,    37,    51,   131,   731,   732,   124,   125,   124,
     125,   124,   125,   739,   130,   741,    51,   130,    53,    54,
     172,    56,    57,   124,   125,   751,   731,   129,   130,   130,
     756,   183,   131,   394,    53,    54,    52,    56,    57,    52,
     130,   767,   132,   769,   770,    32,    33,    34,    35,    36,
      37,   131,   124,   125,   780,   781,    39,    40,    41,    23,
     841,   787,   788,   124,   125,   791,   131,    50,   429,   130,
     431,   797,    16,    17,    18,    19,   124,   125,   677,   678,
     679,   680,   681,   682,   683,   684,   685,    16,    17,   124,
     125,    39,    40,    41,   864,   865,    21,   867,   868,   131,
     870,   871,    50,   873,   874,   124,   125,   131,   124,   125,
     131,   124,   125,   132,   840,   476,   842,   843,    39,    40,
      41,   847,   130,   849,   132,   851,   487,   853,   130,    50,
     132,   857,   493,   131,   286,   287,   131,   124,   125,    39,
      40,    41,   131,    53,    54,   131,    56,    57,   124,   125,
      50,    39,    40,    41,   130,   881,   882,   883,   884,   885,
     886,   887,   888,   762,   525,   124,   125,   766,   767,   131,
     769,   770,    53,    54,    20,    56,    57,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,    63,
      64,    53,    54,   129,    56,    57,   348,   349,   130,    39,
      40,    41,   354,   355,   127,    39,    40,    41,   360,   361,
      50,   124,   125,   365,   124,   125,    50,   130,   370,   371,
      22,   582,   132,    53,    54,   129,    56,    57,   124,   125,
     382,   124,   125,    42,   130,    44,    39,    40,    41,   132,
      49,   602,    32,   124,   125,    30,   129,    50,   847,   127,
     849,   132,   851,    62,   853,    35,    65,    66,    67,   131,
      69,    70,   124,   125,   131,   131,    46,    47,   130,    49,
      39,    40,    41,    53,    54,   130,   637,   132,    58,   131,
     641,    50,   881,   882,   883,   884,   885,   886,   887,   888,
      85,    86,    87,    88,   124,   125,   131,   124,   125,   108,
     130,    61,   131,    53,    54,   132,    56,    57,   131,    89,
      90,    91,    92,   124,   125,   131,   127,   131,    51,   131,
      53,    54,   102,    56,    57,   131,   106,   131,   127,    35,
      52,    53,    54,    20,    56,    57,   124,   125,    21,    21,
      46,    47,    21,    49,   132,    21,   155,    53,    54,   501,
     502,   131,    58,    21,   134,    20,   124,   125,   124,   125,
     102,   127,    36,    37,   132,    53,    54,    41,    56,    57,
     731,    21,   181,    21,   124,   125,    21,   124,   125,   188,
     130,    35,   127,   535,    90,   132,    92,    61,   127,   198,
      21,   124,   125,    21,    68,   204,   102,   206,   550,   551,
     106,    21,   124,   125,   556,   557,   558,   559,   560,   561,
     562,   563,   564,   565,   566,   567,   568,   569,   570,   571,
      35,   124,   125,   129,    21,   131,   578,   108,   134,   132,
      20,    46,    47,   107,    49,   132,   124,   125,    53,    54,
     124,   125,    35,    58,   124,   125,    20,   127,   132,   124,
     125,   125,   126,    46,    47,    20,    49,   132,   124,   125,
      53,    54,   136,   124,   125,    58,   132,    20,   124,   125,
     132,   132,    56,    57,    58,    90,   132,    92,   124,   125,
      53,    54,   156,    56,    57,   132,   132,   102,   132,    53,
      54,   106,    56,    57,    39,   124,   125,    90,   131,    92,
     135,    53,    54,   132,    56,    57,   131,   131,   182,   102,
     131,    46,    47,   106,    49,   131,   131,   131,    53,   134,
     131,   124,   125,    58,   127,   677,   678,   679,   680,   681,
     682,   683,   684,   685,   208,   131,   210,   131,   131,   124,
     125,   134,   216,   217,   131,   124,   125,   132,   222,   131,
     127,   124,   125,   132,   127,    90,   127,    92,   124,   125,
     124,   125,   236,   127,    21,    21,   132,   102,   124,   125,
      39,   106,   124,   125,    39,   127,   132,    21,   252,   253,
     124,   125,   130,   127,   127,    22,   132,   261,   262,   263,
     264,   265,   266,   127,    31,   127,   131,    51,    35,   134,
      51,   275,   276,   277,   278,   127,    43,    44,    45,    52,
     762,    48,    49,   765,   766,   767,    52,   769,   770,   476,
     325,   641,   487,   297,   365,   577,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,   296,   319,    83,   306,   124,   125,
     124,   125,    46,    47,    -1,    49,   132,    -1,   132,   396,
      97,    98,    99,   100,    58,    -1,   103,   104,   108,    -1,
      -1,   108,   109,   110,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,   124,   125,   124,   125,    -1,
      -1,    35,    21,   132,   131,   847,    90,   849,    92,   851,
      -1,   853,    46,    47,    -1,    49,    -1,    -1,   102,    53,
      54,    -1,   106,    -1,    58,   524,   124,   125,   124,   125,
     394,    -1,    -1,    -1,   132,    -1,   132,    -1,    -1,   881,
     882,   883,   884,   885,   886,   887,   888,   131,    -1,    -1,
     134,   124,   125,   124,   125,    -1,    90,    -1,    92,   132,
      -1,   132,   124,   125,    -1,    -1,    -1,    -1,   102,    -1,
     132,    -1,   106,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,    -1,    -1,   124,   125,
      -1,    -1,    22,   592,    -1,   594,   132,   131,   124,   125,
     134,   124,   125,    -1,   468,    35,   132,    -1,    -1,   132,
      -1,   475,   476,    43,    44,    45,   124,   125,    48,    49,
     124,   125,   124,   125,   132,    -1,    -1,    -1,   132,   493,
     132,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,   124,   125,    83,   124,   125,   124,   125,    -1,   132,
      -1,   525,   132,    -1,   132,    -1,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,   124,   125,    -1,   108,   109,
     110,   124,   125,   132,   124,   125,    -1,    -1,    -1,   132,
     124,   125,   132,    -1,   124,   125,   124,   125,   132,    -1,
      -1,   131,   124,   125,   132,   124,   125,    28,   124,   125,
     132,    -1,    -1,   132,    35,    -1,   132,    -1,   582,   583,
      -1,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
     124,   125,    53,    54,    -1,    -1,    -1,    58,   132,    -1,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
     124,   125,   124,   125,   124,   125,   630,    -1,   132,    90,
     132,    92,   132,   637,   638,   639,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    -1,   108,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,   124,   125,    -1,    -1,    35,    -1,    -1,
     131,    -1,    -1,   134,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    -1,    92,    -1,   730,   731,    -1,    97,
      98,    99,   100,    27,   102,   103,   104,   105,   106,    -1,
     108,   109,   110,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    45,    -1,    -1,    48,    49,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,    -1,    -1,   134,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,
      94,    -1,    -1,    97,    98,    99,   100,   101,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    35,    -1,    -1,    -1,    -1,   131,    -1,    -1,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    -1,
      53,    54,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    -1,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    -1,    90,    -1,    92,
      -1,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,   104,    -1,   106,    -1,   108,   109,   110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,    -1,    -1,    35,    -1,    -1,   131,    -1,
      -1,   134,   135,    43,    44,    45,    46,    47,    48,    49,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    -1,    92,    -1,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,   104,    -1,   106,    35,   108,   109,
     110,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      48,    49,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,    -1,    -1,   134,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,    -1,   103,   104,    36,    -1,    -1,
     108,   109,   110,    -1,    43,    44,    45,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,    -1,   103,   104,    -1,    -1,    38,   108,
     109,   110,    -1,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,    -1,    -1,    38,   108,   109,
     110,    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,    -1,    -1,    38,   108,   109,   110,
      -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,    -1,    -1,    38,   108,   109,   110,    -1,
      43,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,    -1,    -1,    38,   108,   109,   110,    -1,    43,
      44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,    -1,   103,
     104,    -1,    -1,    38,   108,   109,   110,    -1,    43,    44,
      45,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,    -1,    43,    44,    45,
      46,    47,    48,    49,    -1,    -1,    -1,    53,    54,    -1,
      -1,    -1,    58,    -1,    -1,    -1,   131,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    92,    -1,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,   104,    -1,
     106,    -1,   108,   109,   110,    -1,    -1,    -1,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,   134,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    -1,    -1,    97,    98,    99,   100,   101,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,    -1,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      -1,    -1,    97,    98,    99,   100,   101,    -1,   103,   104,
      -1,    -1,    -1,   108,   109,   110,    -1,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    -1,    -1,    -1,    -1,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,
      -1,    97,    98,    99,   100,   101,    -1,   103,   104,    -1,
      -1,    -1,   108,   109,   110,    -1,    43,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,
      97,    98,    99,   100,   101,    -1,   103,   104,    -1,    -1,
      -1,   108,   109,   110,    -1,    43,    44,    45,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    -1,    97,
      98,    99,   100,   101,    -1,   103,   104,    -1,    -1,    -1,
     108,   109,   110,    -1,    43,    44,    45,    -1,    -1,    48,
      49,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,
     109,   110,    -1,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    -1,    -1,    -1,   124,   125,    -1,    -1,    -1,
      -1,    -1,   131,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,
     110,    -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,
      -1,    -1,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,    -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,
      -1,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,
     131,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
      -1,   103,   104,    -1,    -1,    -1,   108,   109,   110,    -1,
      43,    44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    98,    99,   100,    -1,    -1,
     103,   104,    -1,    -1,    -1,   108,   109,   110,    -1,    43,
      44,    45,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,    -1,   103,
     104,    -1,    -1,    -1,   108,   109,   110,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      82,    46,    47,    -1,    49,    -1,    -1,   131,    53,    54,
      -1,    -1,    94,    58,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,    90,    -1,    92,    -1,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,
      -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,   134
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    21,   137,   138,   141,   142,   143,   144,   146,
     149,   159,   160,   172,   175,   108,   108,   108,   108,   108,
     108,   108,   107,   107,   107,   107,    14,    15,    27,   176,
       0,    20,   128,    20,   128,    16,    17,    18,    19,   128,
     150,    21,    21,    21,    21,   131,   131,   131,   131,    28,
      35,    42,    43,    44,    45,    46,    47,    48,    49,    53,
      54,    58,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      90,    92,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   108,   109,   110,   124,   125,   131,   134,   179,
     180,   181,   182,   186,   187,   189,   190,   191,   192,   193,
     194,   179,   186,   187,   190,    49,   131,   177,   178,   179,
     187,   127,   179,   129,   177,    22,    49,    83,   131,   147,
     155,   156,   157,   183,   187,   190,   177,    29,   153,    33,
     151,    16,    17,   177,   151,    43,    49,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    81,    82,    94,   101,
     124,   125,   131,   167,   168,   169,   170,   171,   190,   191,
     167,    27,    49,    93,   161,   162,   167,   190,    27,    95,
      96,   173,   174,   108,   145,   145,   145,   145,   131,    36,
     189,   190,   186,   177,   131,   178,   131,   178,   131,   177,
     131,   177,   177,   186,   177,   177,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   128,
     188,   188,   188,   131,   131,   131,   131,   193,   193,   179,
     186,   190,    49,   131,   135,   177,   183,   185,   187,   189,
     190,    32,    33,    34,    35,    36,    37,    53,    54,    56,
      57,   124,   125,    55,   126,    39,    40,    41,    23,   130,
      32,    33,    34,    35,    36,    37,   124,   125,    59,    60,
     126,   133,    61,    62,   186,   179,    21,    23,   127,    20,
     129,   189,   186,   190,   190,   127,   130,   157,   158,    85,
      86,    87,    88,   184,   190,   129,   190,    30,   154,    49,
     125,   190,    32,   152,   127,   129,   152,   177,   186,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   188,   124,   125,   169,   169,   167,   190,   124,   125,
     127,   126,   133,    61,   124,   125,   126,   127,   177,   186,
      42,   131,   163,   167,   190,    39,   127,    32,    33,    34,
     166,   166,   177,   127,   153,   130,   132,   132,   132,   132,
     177,    36,    21,   177,   189,    21,   177,   189,    21,   190,
      21,   190,    21,    21,    50,    21,    21,   179,   179,   190,
     190,   190,   190,   179,   190,   190,   190,   190,   190,   190,
     190,   190,   190,   190,   190,   190,   102,   179,   190,   190,
     189,   189,   189,   189,   189,   132,   132,   132,   186,    21,
     135,   130,   135,   135,    24,    25,   179,   179,   179,   179,
     179,   179,   180,   180,   180,   180,   180,   180,   182,   182,
     186,   186,   186,   179,   190,   190,   190,   190,   190,   190,
     190,   191,   191,   192,   192,   192,   192,   192,    50,   176,
     179,    89,    91,   139,   140,   187,    20,    22,    50,    84,
     156,   189,   157,   190,   190,   190,   190,    20,   190,   127,
     186,    38,    38,    49,   190,   153,    16,    17,    19,   150,
     127,    21,    50,   167,   190,   167,   190,   167,   190,   167,
     190,   167,   190,   167,   190,   167,   190,   167,   190,   167,
     190,   167,   167,   167,    43,    49,    69,    70,    71,    72,
      73,    74,    75,    76,    77,   131,   132,   168,   191,   168,
     191,   169,   192,   192,   192,   168,   191,   168,   191,   169,
      21,    50,   163,   163,   167,   190,    39,    40,    41,    50,
      32,    33,    34,    35,    36,    37,    32,    33,    34,    35,
      36,    37,   162,   167,   190,   167,   190,    21,    21,   108,
      20,    20,    20,    20,   132,   191,   132,   132,   192,   132,
     132,   192,   132,   190,   132,   190,   192,   179,   186,   190,
     182,   181,   130,   132,   132,   132,   132,   130,   130,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   130,   129,   132,   132,   132,   132,
      50,   187,   190,   187,   190,   190,   179,    22,   131,   131,
     127,   130,   179,   139,   179,   190,   190,    22,   189,    31,
     155,   190,    50,   186,   154,   153,   151,    16,   151,   168,
     191,   167,   190,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   130,   130,   132,   177,   186,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   161,   161,   167,   190,
     132,   163,   163,   163,   167,   190,   167,   190,   167,   190,
     167,   190,   167,   190,   167,   190,   167,   190,   167,   167,
     167,   167,   167,   167,   164,   164,    32,    33,   164,    32,
      33,   164,   173,   167,   190,   190,   186,   179,   177,   177,
      51,    51,    51,   187,   190,   190,   190,   135,   135,    26,
     135,    26,   135,   186,   179,   179,   140,   127,   127,    22,
     190,    31,   148,   127,   125,   190,    50,   127,   154,   152,
     127,   152,    51,   190,   190,    21,    50,    51,    52,   166,
     166,   130,   190,   190,   190,   190,   127,   127,   127,   127,
      21,    21,   179,   186,   190,   132,   132,   130,   130,   190,
     190,   130,   132,   127,   190,   127,    38,    51,    38,   190,
     127,   153,   127,   167,   132,   132,   191,   161,   167,   190,
     167,   190,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   165,   164,   164,   164,   164,
     190,   190,    52,    52,    52,   190,   190,   135,   135,   190,
      51,   125,    51,    51,   154,    52,    52,    51,    52,    51,
      52,    51,    52,    51,    52,   132,   132,   130,   132,   190,
      38,   190,    38,   127,   167,   190,   164,   167,   190,   164,
     167,   190,   164,   167,   190,   164,   190,    52,    52,    52,
      52,   166,   166,   166,   166,   166,   166,   166,   166,   132,
     167,   190,   167,   190,   167,   190,   167,   190,   167,   190,
     167,   190,   167,   190,   167,   190,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   136,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   138,   138,   138,   138,   139,   139,   139,
     139,   139,   140,   141,   142,   143,   144,   145,   145,   146,
     146,   146,   146,   146,   147,   147,   148,   148,   149,   149,
     149,   149,   149,   149,   149,   149,   150,   150,   150,   151,
     151,   151,   151,   151,   152,   152,   152,   152,   152,   153,
     153,   154,   154,   155,   155,   155,   155,   156,   157,   158,
     158,   159,   159,   160,   161,   161,   161,   161,   161,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   162,   162,   162,   162,   162,   162,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     164,   164,   165,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   166,   166,   166,   167,   167,
     167,   167,   167,   167,   167,   168,   168,   168,   168,   168,
     169,   169,   169,   170,   170,   170,   171,   171,   171,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     171,   172,   173,   173,   174,   174,   174,   175,   176,   176,
     176,   176,   176,   176,   176,   177,   177,   178,   178,   179,
     179,   179,   179,   179,   179,   179,   180,   180,   181,   181,
     181,   181,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   182,   182,   182,   182,   182,   182,   182,
     182,   182,   182,   183,   183,   184,   184,   184,   184,   185,
     185,   185,   186,   186,   186,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   186,
     186,   186,   186,   187,   187,   188,   188,   189,   189,   190,
     190,   190,   191,   191,   191,   191,   191,   191,   192,   192,
     192,   193,   193,   193,   193,   193,   193,   193,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     5,     8,     8,     7,     1,     3,     6,
       8,     4,     2,     8,     8,     8,     8,     1,     3,     9,
       8,     9,     5,     1,     1,     1,     0,     2,     9,     6,
       8,     5,     9,     6,    11,     8,     0,     1,     2,     0,
       2,     3,     9,     9,     0,     2,     2,     8,     8,     0,
       2,     0,     2,     1,     3,     1,     2,     2,     3,     3,
       4,     5,     5,     5,     1,     3,     4,     5,     7,     4,
       4,     4,     4,     6,     6,     6,     6,    12,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
      12,    12,    12,     8,     8,     8,     8,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     1,     3,     3,     3,     3,
       1,     2,     2,     1,     3,     4,     2,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     6,     6,     7,
       3,     5,     4,     4,     0,     1,     1,     3,     2,     2,
       2,     2,     2,     2,     4,     1,     1,     5,     3,     1,
       3,     3,     3,     3,     3,     3,     1,     4,     1,     3,
       3,     4,     2,     4,     4,     2,     7,     5,     7,     5,
       4,     7,     4,     7,     3,     3,     3,     3,     5,     5,
       6,     4,     7,     4,     2,     2,     2,     2,     2,     1,
       3,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     3,     3,
       4,     4,     7,     2,     3,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     4,     1,     2,
       2,     1,     3,     4,     4,     4,     4,     4,     1,     1,
       1,     2,     2,     4,     4,     2,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     3,     4,     8,     6,     7,     8,     4,     4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* stmt: decl_set  */
#line 158 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2448 "src/zimpl/mmlparse2.c"
    break;

  case 3: /* stmt: decl_par  */
#line 159 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2454 "src/zimpl/mmlparse2.c"
    break;

  case 4: /* stmt: decl_var  */
#line 160 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2460 "src/zimpl/mmlparse2.c"
    break;

  case 5: /* stmt: decl_obj  */
#line 161 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2466 "src/zimpl/mmlparse2.c"
    break;

  case 6: /* stmt: decl_sub  */
#line 162 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2472 "src/zimpl/mmlparse2.c"
    break;

  case 7: /* stmt: decl_sos  */
#line 163 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2478 "src/zimpl/mmlparse2.c"
    break;

  case 8: /* stmt: def_numb  */
#line 164 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2484 "src/zimpl/mmlparse2.c"
    break;

  case 9: /* stmt: def_strg  */
#line 165 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2490 "src/zimpl/mmlparse2.c"
    break;

  case 10: /* stmt: def_bool  */
#line 166 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2496 "src/zimpl/mmlparse2.c"
    break;

  case 11: /* stmt: def_set  */
#line 167 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2502 "src/zimpl/mmlparse2.c"
    break;

  case 12: /* stmt: exec_do  */
#line 168 "src/zimpl/mmlparse2.y"
                { code_set_root((yyvsp[0].code)); }
#line 2508 "src/zimpl/mmlparse2.c"
    break;

  case 13: /* decl_set: DECLSET NAME ASGN sexpr ';'  */
#line 176 "src/zimpl/mmlparse2.y"
                                 {
         (yyval.code) = code_new_inst(i_newsym_set1, 3,
            code_new_name((yyvsp[-3].name)),                                       /* Name */
            code_new_inst(i_idxset_pseudo_new, 1,               /* index set */
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-1].code));                                              /* initial set */
      }
#line 2520 "src/zimpl/mmlparse2.c"
    break;

  case 14: /* decl_set: DECLSET NAME '[' idxset ']' ASGN sexpr ';'  */
#line 183 "src/zimpl/mmlparse2.y"
                                                {
         (yyval.code) = code_new_inst(i_newsym_set1, 3,
            code_new_name((yyvsp[-6].name)),                                       /* Name */
            (yyvsp[-4].code),                                                 /* index set */
            (yyvsp[-1].code));                                                      /* set */
      }
#line 2531 "src/zimpl/mmlparse2.c"
    break;

  case 15: /* decl_set: DECLSET NAME '[' idxset ']' ASGN set_entry_list ';'  */
#line 189 "src/zimpl/mmlparse2.y"
                                                         {
         (yyval.code) = code_new_inst(i_newsym_set2, 3,
            code_new_name((yyvsp[-6].name)),                                       /* Name */
            (yyvsp[-4].code),                                                 /* index set */
            (yyvsp[-1].code));                                   /* initial set_entry_list */
      }
#line 2542 "src/zimpl/mmlparse2.c"
    break;

  case 16: /* decl_set: DECLSET NAME '[' ']' ASGN set_entry_list ';'  */
#line 195 "src/zimpl/mmlparse2.y"
                                                  {
         (yyval.code) = code_new_inst(i_newsym_set2, 3,
            code_new_name((yyvsp[-5].name)),                                       /* Name */
            code_new_inst(i_idxset_pseudo_new, 1,               /* index set */
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-1].code));                                   /* initial set_entry_list */
      }
#line 2554 "src/zimpl/mmlparse2.c"
    break;

  case 17: /* set_entry_list: set_entry  */
#line 205 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_entry_list_new, 1, (yyvsp[0].code)); }
#line 2560 "src/zimpl/mmlparse2.c"
    break;

  case 18: /* set_entry_list: set_entry_list ',' set_entry  */
#line 206 "src/zimpl/mmlparse2.y"
                                   {
         (yyval.code) = code_new_inst(i_entry_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 2568 "src/zimpl/mmlparse2.c"
    break;

  case 19: /* set_entry_list: SUBSETS '(' sexpr ',' cexpr ')'  */
#line 209 "src/zimpl/mmlparse2.y"
                                     {
         (yyval.code) = code_new_inst(i_entry_list_subsets, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(-1)));
      }
#line 2576 "src/zimpl/mmlparse2.c"
    break;

  case 20: /* set_entry_list: SUBSETS '(' sexpr ',' cexpr ',' cexpr ')'  */
#line 212 "src/zimpl/mmlparse2.y"
                                               {
         (yyval.code) = code_new_inst(i_entry_list_subsets, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 2584 "src/zimpl/mmlparse2.c"
    break;

  case 21: /* set_entry_list: POWERSET '(' sexpr ')'  */
#line 215 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_entry_list_powerset, 1, (yyvsp[-1].code));
      }
#line 2592 "src/zimpl/mmlparse2.c"
    break;

  case 22: /* set_entry: tuple sexpr  */
#line 221 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_entry, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 2598 "src/zimpl/mmlparse2.c"
    break;

  case 23: /* def_numb: DEFNUMB DEFNAME '(' name_list ')' ASGN cexpr ';'  */
#line 230 "src/zimpl/mmlparse2.y"
                                                      {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2609 "src/zimpl/mmlparse2.c"
    break;

  case 24: /* def_strg: DEFSTRG DEFNAME '(' name_list ')' ASGN cexpr ';'  */
#line 239 "src/zimpl/mmlparse2.y"
                                                      {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2620 "src/zimpl/mmlparse2.c"
    break;

  case 25: /* def_bool: DEFBOOL DEFNAME '(' name_list ')' ASGN lexpr ';'  */
#line 248 "src/zimpl/mmlparse2.y"
                                                      {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2631 "src/zimpl/mmlparse2.c"
    break;

  case 26: /* def_set: DEFSET DEFNAME '(' name_list ')' ASGN sexpr ';'  */
#line 257 "src/zimpl/mmlparse2.y"
                                                     {
         (yyval.code) = code_new_inst(i_newdef, 3,
            code_new_define((yyvsp[-6].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-4].code)),
            (yyvsp[-1].code));
      }
#line 2642 "src/zimpl/mmlparse2.c"
    break;

  case 27: /* name_list: NAME  */
#line 266 "src/zimpl/mmlparse2.y"
          {
         (yyval.code) = code_new_inst(i_elem_list_new, 1, code_new_name((yyvsp[0].name)));
      }
#line 2650 "src/zimpl/mmlparse2.c"
    break;

  case 28: /* name_list: name_list ',' NAME  */
#line 269 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_elem_list_add, 2, (yyvsp[-2].code), code_new_name((yyvsp[0].name)));
      }
#line 2658 "src/zimpl/mmlparse2.c"
    break;

  case 29: /* decl_par: DECLPAR NAME '[' idxset ']' ASGN cexpr_entry_list par_default ';'  */
#line 279 "src/zimpl/mmlparse2.y"
                                                                       {
         (yyval.code) = code_new_inst(i_newsym_para1, 4, code_new_name((yyvsp[-7].name)), (yyvsp[-5].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2666 "src/zimpl/mmlparse2.c"
    break;

  case 30: /* decl_par: DECLPAR NAME '[' idxset ']' ASGN cexpr ';'  */
#line 282 "src/zimpl/mmlparse2.y"
                                                {
         (yyval.code) = code_new_inst(i_newsym_para2, 4, code_new_name((yyvsp[-6].name)), (yyvsp[-4].code), (yyvsp[-1].code), code_new_inst(i_nop, 0));
      }
#line 2674 "src/zimpl/mmlparse2.c"
    break;

  case 31: /* decl_par: DECLPAR NAME '[' idxset ']' ASGN DEFAULT cexpr ';'  */
#line 285 "src/zimpl/mmlparse2.y"
                                                        {
         (yyval.code) = code_new_inst(i_newsym_para2, 4, code_new_name((yyvsp[-7].name)), (yyvsp[-5].code), (yyvsp[-1].code), code_new_inst(i_nop, 0));
      }
#line 2682 "src/zimpl/mmlparse2.c"
    break;

  case 32: /* decl_par: DECLPAR NAME ASGN par_singleton ';'  */
#line 288 "src/zimpl/mmlparse2.y"
                                         {
         (yyval.code) = code_new_inst(i_newsym_para1, 4,
            code_new_name((yyvsp[-3].name)),
            code_new_inst(i_idxset_pseudo_new, 1, code_new_inst(i_bool_true, 0)),
            (yyvsp[-1].code),
            code_new_inst(i_nop, 0));
      }
#line 2694 "src/zimpl/mmlparse2.c"
    break;

  case 33: /* decl_par: DECLPAR  */
#line 295 "src/zimpl/mmlparse2.y"
             { (yyval.code) = code_new_inst(i_nop, 0); }
#line 2700 "src/zimpl/mmlparse2.c"
    break;

  case 34: /* par_singleton: cexpr_entry_list  */
#line 299 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = (yyvsp[0].code); }
#line 2706 "src/zimpl/mmlparse2.c"
    break;

  case 35: /* par_singleton: cexpr  */
#line 300 "src/zimpl/mmlparse2.y"
                      {
         (yyval.code) = code_new_inst(i_entry_list_new, 1,
            code_new_inst(i_entry, 2, code_new_inst(i_tuple_empty, 0), (yyvsp[0].code)));
      }
#line 2715 "src/zimpl/mmlparse2.c"
    break;

  case 36: /* par_default: %empty  */
#line 307 "src/zimpl/mmlparse2.y"
                    { (yyval.code) = code_new_inst(i_nop, 0); }
#line 2721 "src/zimpl/mmlparse2.c"
    break;

  case 37: /* par_default: DEFAULT cexpr  */
#line 308 "src/zimpl/mmlparse2.y"
                    { (yyval.code) = code_new_inst(i_entry, 2, code_new_inst(i_tuple_empty, 0), (yyvsp[0].code)); }
#line 2727 "src/zimpl/mmlparse2.c"
    break;

  case 38: /* decl_var: DECLVAR NAME '[' idxset ']' var_type lower upper ';'  */
#line 316 "src/zimpl/mmlparse2.y"
                                                          {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-7].name)),
            (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2739 "src/zimpl/mmlparse2.c"
    break;

  case 39: /* decl_var: DECLVAR NAME var_type lower upper ';'  */
#line 323 "src/zimpl/mmlparse2.y"
                                           {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-4].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2753 "src/zimpl/mmlparse2.c"
    break;

  case 40: /* decl_var: DECLVAR NAME '[' idxset ']' IMPLICIT BINARY ';'  */
#line 332 "src/zimpl/mmlparse2.y"
                                                     {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-6].name)),
            (yyvsp[-4].code),
            code_new_varclass(VAR_IMP),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2768 "src/zimpl/mmlparse2.c"
    break;

  case 41: /* decl_var: DECLVAR NAME IMPLICIT BINARY ';'  */
#line 342 "src/zimpl/mmlparse2.y"
                                      {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-3].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_IMP),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            code_new_numb(numb_copy(numb_unknown())),
            code_new_numb(numb_copy(numb_unknown())));
      }
#line 2784 "src/zimpl/mmlparse2.c"
    break;

  case 42: /* decl_var: DECLVAR NAME '[' idxset ']' BINARY priority startval ';'  */
#line 353 "src/zimpl/mmlparse2.y"
                                                              {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-7].name)),
            (yyvsp[-5].code),
            code_new_varclass(VAR_INT),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2798 "src/zimpl/mmlparse2.c"
    break;

  case 43: /* decl_var: DECLVAR NAME BINARY priority startval ';'  */
#line 362 "src/zimpl/mmlparse2.y"
                                               {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-4].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_INT),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0))),
            code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(1))),
            (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2813 "src/zimpl/mmlparse2.c"
    break;

  case 44: /* decl_var: DECLVAR NAME '[' idxset ']' INTEGER lower upper priority startval ';'  */
#line 372 "src/zimpl/mmlparse2.y"
                                                                           {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-9].name)), (yyvsp[-7].code), code_new_varclass(VAR_INT), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2822 "src/zimpl/mmlparse2.c"
    break;

  case 45: /* decl_var: DECLVAR NAME INTEGER lower upper priority startval ';'  */
#line 376 "src/zimpl/mmlparse2.y"
                                                            {
         (yyval.code) = code_new_inst(i_newsym_var, 7,
            code_new_name((yyvsp[-6].name)),
            code_new_inst(i_idxset_pseudo_new, 1,
               code_new_inst(i_bool_true, 0)),              
            code_new_varclass(VAR_INT), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 2834 "src/zimpl/mmlparse2.c"
    break;

  case 46: /* var_type: %empty  */
#line 386 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_varclass(VAR_CON); }
#line 2840 "src/zimpl/mmlparse2.c"
    break;

  case 47: /* var_type: REAL  */
#line 387 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_varclass(VAR_CON); }
#line 2846 "src/zimpl/mmlparse2.c"
    break;

  case 48: /* var_type: IMPLICIT INTEGER  */
#line 388 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_varclass(VAR_IMP); }
#line 2852 "src/zimpl/mmlparse2.c"
    break;

  case 49: /* lower: %empty  */
#line 392 "src/zimpl/mmlparse2.y"
                      {
         (yyval.code) = code_new_inst(i_bound_new, 1, code_new_numb(numb_new_integer(0)));
      }
#line 2860 "src/zimpl/mmlparse2.c"
    break;

  case 50: /* lower: CMP_GE cexpr  */
#line 395 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_inst(i_bound_new, 1, (yyvsp[0].code)); }
#line 2866 "src/zimpl/mmlparse2.c"
    break;

  case 51: /* lower: CMP_GE '-' INFTY  */
#line 396 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_bound(BOUND_MINUS_INFTY); }
#line 2872 "src/zimpl/mmlparse2.c"
    break;

  case 52: /* lower: CMP_GE IF lexpr THEN cexpr ELSE '-' INFTY END  */
#line 397 "src/zimpl/mmlparse2.y"
                                                   {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-6].code),
            code_new_inst(i_bound_new, 1, (yyvsp[-4].code)),
            code_new_bound(BOUND_MINUS_INFTY));
      }
#line 2882 "src/zimpl/mmlparse2.c"
    break;

  case 53: /* lower: CMP_GE IF lexpr THEN '-' INFTY ELSE cexpr END  */
#line 402 "src/zimpl/mmlparse2.y"
                                                   {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-6].code),
            code_new_bound(BOUND_MINUS_INFTY),
            code_new_inst(i_bound_new, 1, (yyvsp[-1].code)));
      }
#line 2892 "src/zimpl/mmlparse2.c"
    break;

  case 54: /* upper: %empty  */
#line 410 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_bound(BOUND_INFTY); }
#line 2898 "src/zimpl/mmlparse2.c"
    break;

  case 55: /* upper: CMP_LE cexpr  */
#line 411 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_inst(i_bound_new, 1, (yyvsp[0].code)); }
#line 2904 "src/zimpl/mmlparse2.c"
    break;

  case 56: /* upper: CMP_LE INFTY  */
#line 412 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_bound(BOUND_INFTY); }
#line 2910 "src/zimpl/mmlparse2.c"
    break;

  case 57: /* upper: CMP_LE IF lexpr THEN cexpr ELSE INFTY END  */
#line 413 "src/zimpl/mmlparse2.y"
                                               {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code),
            code_new_inst(i_bound_new, 1, (yyvsp[-3].code)),
            code_new_bound(BOUND_INFTY));
      }
#line 2920 "src/zimpl/mmlparse2.c"
    break;

  case 58: /* upper: CMP_LE IF lexpr THEN INFTY ELSE cexpr END  */
#line 418 "src/zimpl/mmlparse2.y"
                                               {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code),
            code_new_bound(BOUND_INFTY),
            code_new_inst(i_bound_new, 1, (yyvsp[-1].code)));
      }
#line 2930 "src/zimpl/mmlparse2.c"
    break;

  case 59: /* priority: %empty  */
#line 426 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_numb(numb_new_integer(0)); }
#line 2936 "src/zimpl/mmlparse2.c"
    break;

  case 60: /* priority: PRIORITY cexpr  */
#line 427 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = (yyvsp[0].code); }
#line 2942 "src/zimpl/mmlparse2.c"
    break;

  case 61: /* startval: %empty  */
#line 431 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = code_new_numb(numb_copy(numb_unknown())); }
#line 2948 "src/zimpl/mmlparse2.c"
    break;

  case 62: /* startval: STARTVAL cexpr  */
#line 432 "src/zimpl/mmlparse2.y"
                       { (yyval.code) = (yyvsp[0].code); }
#line 2954 "src/zimpl/mmlparse2.c"
    break;

  case 63: /* cexpr_entry_list: cexpr_entry  */
#line 440 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_entry_list_new, 1, (yyvsp[0].code)); }
#line 2960 "src/zimpl/mmlparse2.c"
    break;

  case 64: /* cexpr_entry_list: cexpr_entry_list ',' cexpr_entry  */
#line 441 "src/zimpl/mmlparse2.y"
                                       {
         (yyval.code) = code_new_inst(i_entry_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 2968 "src/zimpl/mmlparse2.c"
    break;

  case 65: /* cexpr_entry_list: read  */
#line 444 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_read, 1, (yyvsp[0].code)); }
#line 2974 "src/zimpl/mmlparse2.c"
    break;

  case 66: /* cexpr_entry_list: matrix_head matrix_body  */
#line 445 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_list_matrix, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 2980 "src/zimpl/mmlparse2.c"
    break;

  case 67: /* cexpr_entry: tuple cexpr  */
#line 449 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_entry, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 2986 "src/zimpl/mmlparse2.c"
    break;

  case 68: /* matrix_head: WITH cexpr_list WITH  */
#line 453 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = (yyvsp[-1].code); }
#line 2992 "src/zimpl/mmlparse2.c"
    break;

  case 69: /* matrix_body: matrix_head cexpr_list WITH  */
#line 457 "src/zimpl/mmlparse2.y"
                                 {
         (yyval.code) = code_new_inst(i_matrix_list_new, 2, (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 3000 "src/zimpl/mmlparse2.c"
    break;

  case 70: /* matrix_body: matrix_body matrix_head cexpr_list WITH  */
#line 460 "src/zimpl/mmlparse2.y"
                                             {
         (yyval.code) = code_new_inst(i_matrix_list_add, 3, (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code));
      }
#line 3008 "src/zimpl/mmlparse2.c"
    break;

  case 71: /* decl_obj: DECLMIN NAME DO vexpr ';'  */
#line 472 "src/zimpl/mmlparse2.y"
                               {
         (yyval.code) = code_new_inst(i_object_min, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
      }
#line 3016 "src/zimpl/mmlparse2.c"
    break;

  case 72: /* decl_obj: DECLMAX NAME DO vexpr ';'  */
#line 475 "src/zimpl/mmlparse2.y"
                               {
         (yyval.code) = code_new_inst(i_object_max, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
      }
#line 3024 "src/zimpl/mmlparse2.c"
    break;

  case 73: /* decl_sub: DECLSUB NAME DO constraint_list ';'  */
#line 485 "src/zimpl/mmlparse2.y"
                                         {
        (yyval.code) = code_new_inst(i_subto, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
     }
#line 3032 "src/zimpl/mmlparse2.c"
    break;

  case 74: /* constraint_list: constraint  */
#line 491 "src/zimpl/mmlparse2.y"
                {
        (yyval.code) = code_new_inst(i_constraint_list, 2, (yyvsp[0].code), code_new_inst(i_nop, 0));
     }
#line 3040 "src/zimpl/mmlparse2.c"
    break;

  case 75: /* constraint_list: constraint_list AND constraint  */
#line 494 "src/zimpl/mmlparse2.y"
                                    {
        (yyval.code) = code_new_inst(i_constraint_list, 2, (yyvsp[-2].code), (yyvsp[0].code));
     }
#line 3048 "src/zimpl/mmlparse2.c"
    break;

  case 76: /* constraint_list: FORALL idxset DO constraint_list  */
#line 497 "src/zimpl/mmlparse2.y"
                                      {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code)),
           code_new_inst(i_nop, 0));
     }
#line 3058 "src/zimpl/mmlparse2.c"
    break;

  case 77: /* constraint_list: IF lexpr THEN constraint_list END  */
#line 502 "src/zimpl/mmlparse2.y"
                                       {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_expr_if_else, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_inst(i_nop, 0)),
           code_new_inst(i_nop, 0));
      }
#line 3068 "src/zimpl/mmlparse2.c"
    break;

  case 78: /* constraint_list: IF lexpr THEN constraint_list ELSE constraint_list END  */
#line 507 "src/zimpl/mmlparse2.y"
                                                            {
        (yyval.code) = code_new_inst(i_constraint_list, 2, 
           code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code)),
           code_new_inst(i_nop, 0));
      }
#line 3078 "src/zimpl/mmlparse2.c"
    break;

  case 79: /* constraint: vexpr con_type vexpr con_attr_list  */
#line 515 "src/zimpl/mmlparse2.y"
                                        {
        (yyval.code) = code_new_inst(i_constraint, 4, (yyvsp[-3].code), (yyvsp[-2].code), (yyvsp[-1].code), code_new_bits((yyvsp[0].bits)));
     }
#line 3086 "src/zimpl/mmlparse2.c"
    break;

  case 80: /* constraint: vexpr con_type cexpr con_attr_list  */
#line 518 "src/zimpl/mmlparse2.y"
                                        {
        (yyval.code) = code_new_inst(i_constraint, 4, (yyvsp[-3].code), (yyvsp[-2].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-1].code)),
           code_new_bits((yyvsp[0].bits)));
     }
#line 3096 "src/zimpl/mmlparse2.c"
    break;

  case 81: /* constraint: cexpr con_type vexpr con_attr_list  */
#line 523 "src/zimpl/mmlparse2.y"
                                        {
        (yyval.code) = code_new_inst(i_constraint, 4,
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-2].code), (yyvsp[-1].code), code_new_bits((yyvsp[0].bits)));
     }
#line 3106 "src/zimpl/mmlparse2.c"
    break;

  case 82: /* constraint: cexpr con_type cexpr con_attr_list  */
#line 528 "src/zimpl/mmlparse2.y"
                                        { 
        (yyval.code) = code_new_inst(i_constraint, 4,
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-2].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-1].code)),
           code_new_bits((yyvsp[0].bits)));
     }
#line 3118 "src/zimpl/mmlparse2.c"
    break;

  case 83: /* constraint: cexpr con_type vexpr CMP_LE cexpr con_attr_list  */
#line 535 "src/zimpl/mmlparse2.y"
                                                     {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code), (yyvsp[-4].code),
           code_new_contype(CON_RHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3127 "src/zimpl/mmlparse2.c"
    break;

  case 84: /* constraint: cexpr con_type cexpr CMP_LE cexpr con_attr_list  */
#line 539 "src/zimpl/mmlparse2.y"
                                                     {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-5].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)), (yyvsp[-1].code), (yyvsp[-4].code),
           code_new_contype(CON_RHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3137 "src/zimpl/mmlparse2.c"
    break;

  case 85: /* constraint: cexpr con_type vexpr CMP_GE cexpr con_attr_list  */
#line 544 "src/zimpl/mmlparse2.y"
                                                     {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-1].code), (yyvsp[-3].code), (yyvsp[-5].code), (yyvsp[-4].code),
           code_new_contype(CON_LHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3146 "src/zimpl/mmlparse2.c"
    break;

  case 86: /* constraint: cexpr con_type cexpr CMP_GE cexpr con_attr_list  */
#line 548 "src/zimpl/mmlparse2.y"
                                                     {
        (yyval.code) = code_new_inst(i_rangeconst, 6, (yyvsp[-1].code),
           code_new_inst(i_term_expr, 1, (yyvsp[-3].code)),
           (yyvsp[-5].code), (yyvsp[-4].code),
           code_new_contype(CON_LHS), code_new_bits((yyvsp[0].bits))); 
     }
#line 3157 "src/zimpl/mmlparse2.c"
    break;

  case 87: /* constraint: VIF vbool THEN vexpr con_type vexpr ELSE vexpr con_type vexpr END con_attr_list  */
#line 554 "src/zimpl/mmlparse2.y"
                                                                                     {
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3165 "src/zimpl/mmlparse2.c"
    break;

  case 88: /* constraint: VIF vbool THEN cexpr con_type vexpr ELSE vexpr con_type vexpr END con_attr_list  */
#line 557 "src/zimpl/mmlparse2.y"
                                                                                     {
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3175 "src/zimpl/mmlparse2.c"
    break;

  case 89: /* constraint: VIF vbool THEN vexpr con_type cexpr ELSE vexpr con_type vexpr END con_attr_list  */
#line 562 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3185 "src/zimpl/mmlparse2.c"
    break;

  case 90: /* constraint: VIF vbool THEN vexpr con_type vexpr ELSE cexpr con_type vexpr END con_attr_list  */
#line 567 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3195 "src/zimpl/mmlparse2.c"
    break;

  case 91: /* constraint: VIF vbool THEN vexpr con_type vexpr ELSE vexpr con_type cexpr END con_attr_list  */
#line 572 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3204 "src/zimpl/mmlparse2.c"
    break;

  case 92: /* constraint: VIF vbool THEN cexpr con_type cexpr ELSE vexpr con_type vexpr END con_attr_list  */
#line 576 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3216 "src/zimpl/mmlparse2.c"
    break;

  case 93: /* constraint: VIF vbool THEN cexpr con_type vexpr ELSE cexpr con_type vexpr END con_attr_list  */
#line 583 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3228 "src/zimpl/mmlparse2.c"
    break;

  case 94: /* constraint: VIF vbool THEN cexpr con_type vexpr ELSE vexpr con_type cexpr END con_attr_list  */
#line 590 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3239 "src/zimpl/mmlparse2.c"
    break;

  case 95: /* constraint: VIF vbool THEN vexpr con_type cexpr ELSE cexpr con_type vexpr END con_attr_list  */
#line 596 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3250 "src/zimpl/mmlparse2.c"
    break;

  case 96: /* constraint: VIF vbool THEN vexpr con_type cexpr ELSE vexpr con_type cexpr END con_attr_list  */
#line 602 "src/zimpl/mmlparse2.y"
                                                                                     { 
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3261 "src/zimpl/mmlparse2.c"
    break;

  case 97: /* constraint: VIF vbool THEN vexpr con_type vexpr ELSE cexpr con_type cexpr END con_attr_list  */
#line 608 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3271 "src/zimpl/mmlparse2.c"
    break;

  case 98: /* constraint: VIF vbool THEN cexpr con_type cexpr ELSE cexpr con_type vexpr END con_attr_list  */
#line 613 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3284 "src/zimpl/mmlparse2.c"
    break;

  case 99: /* constraint: VIF vbool THEN cexpr con_type cexpr ELSE vexpr con_type cexpr END con_attr_list  */
#line 621 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            (yyvsp[-4].code), (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3298 "src/zimpl/mmlparse2.c"
    break;

  case 100: /* constraint: VIF vbool THEN cexpr con_type vexpr ELSE cexpr con_type cexpr END con_attr_list  */
#line 630 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code), (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3312 "src/zimpl/mmlparse2.c"
    break;

  case 101: /* constraint: VIF vbool THEN vexpr con_type cexpr ELSE cexpr con_type cexpr END con_attr_list  */
#line 639 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code), (yyvsp[-8].code), (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3325 "src/zimpl/mmlparse2.c"
    break;

  case 102: /* constraint: VIF vbool THEN cexpr con_type cexpr ELSE cexpr con_type cexpr END con_attr_list  */
#line 647 "src/zimpl/mmlparse2.y"
                                                                                     { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif_else, 8, (yyvsp[-10].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-8].code)),
            (yyvsp[-7].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-6].code)),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)),
            (yyvsp[-3].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3340 "src/zimpl/mmlparse2.c"
    break;

  case 103: /* constraint: VIF vbool THEN vexpr con_type vexpr END con_attr_list  */
#line 658 "src/zimpl/mmlparse2.y"
                                                           {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), (yyvsp[-4].code), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3348 "src/zimpl/mmlparse2.c"
    break;

  case 104: /* constraint: VIF vbool THEN cexpr con_type vexpr END con_attr_list  */
#line 661 "src/zimpl/mmlparse2.y"
                                                           {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), 
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code), (yyvsp[-2].code), code_new_bits((yyvsp[0].bits)));
      }
#line 3357 "src/zimpl/mmlparse2.c"
    break;

  case 105: /* constraint: VIF vbool THEN vexpr con_type cexpr END con_attr_list  */
#line 665 "src/zimpl/mmlparse2.y"
                                                           {
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code), 
            (yyvsp[-4].code), (yyvsp[-3].code), code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), 
            code_new_bits((yyvsp[0].bits)));
      }
#line 3367 "src/zimpl/mmlparse2.c"
    break;

  case 106: /* constraint: VIF vbool THEN cexpr con_type cexpr END con_attr_list  */
#line 670 "src/zimpl/mmlparse2.y"
                                                           { /* ??? This is an error */
         (yyval.code) = code_new_inst(i_vif, 5, (yyvsp[-6].code),
            code_new_inst(i_term_expr, 1, (yyvsp[-4].code)), (yyvsp[-3].code), 
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), code_new_bits((yyvsp[0].bits)));
      }
#line 3377 "src/zimpl/mmlparse2.c"
    break;

  case 107: /* vbool: vexpr CMP_NE vexpr  */
#line 678 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_ne, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3383 "src/zimpl/mmlparse2.c"
    break;

  case 108: /* vbool: cexpr CMP_NE vexpr  */
#line 679 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_ne, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3391 "src/zimpl/mmlparse2.c"
    break;

  case 109: /* vbool: vexpr CMP_NE cexpr  */
#line 682 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_ne, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3399 "src/zimpl/mmlparse2.c"
    break;

  case 110: /* vbool: vexpr CMP_EQ vexpr  */
#line 685 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_eq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3405 "src/zimpl/mmlparse2.c"
    break;

  case 111: /* vbool: cexpr CMP_EQ vexpr  */
#line 686 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_eq, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3413 "src/zimpl/mmlparse2.c"
    break;

  case 112: /* vbool: vexpr CMP_EQ cexpr  */
#line 689 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_eq, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3421 "src/zimpl/mmlparse2.c"
    break;

  case 113: /* vbool: vexpr CMP_LE vexpr  */
#line 692 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_le, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3427 "src/zimpl/mmlparse2.c"
    break;

  case 114: /* vbool: cexpr CMP_LE vexpr  */
#line 693 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_le, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3435 "src/zimpl/mmlparse2.c"
    break;

  case 115: /* vbool: vexpr CMP_LE cexpr  */
#line 696 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_le, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3443 "src/zimpl/mmlparse2.c"
    break;

  case 116: /* vbool: vexpr CMP_GE vexpr  */
#line 699 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_ge, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3449 "src/zimpl/mmlparse2.c"
    break;

  case 117: /* vbool: cexpr CMP_GE vexpr  */
#line 700 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_ge, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3457 "src/zimpl/mmlparse2.c"
    break;

  case 118: /* vbool: vexpr CMP_GE cexpr  */
#line 703 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_ge, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3465 "src/zimpl/mmlparse2.c"
    break;

  case 119: /* vbool: vexpr CMP_LT vexpr  */
#line 706 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_lt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3471 "src/zimpl/mmlparse2.c"
    break;

  case 120: /* vbool: cexpr CMP_LT vexpr  */
#line 707 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_lt, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3479 "src/zimpl/mmlparse2.c"
    break;

  case 121: /* vbool: vexpr CMP_LT cexpr  */
#line 710 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_lt, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3487 "src/zimpl/mmlparse2.c"
    break;

  case 122: /* vbool: vexpr CMP_GT vexpr  */
#line 713 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vbool_gt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3493 "src/zimpl/mmlparse2.c"
    break;

  case 123: /* vbool: cexpr CMP_GT vexpr  */
#line 714 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_gt, 2, code_new_inst(i_term_expr, 1, (yyvsp[-2].code)), (yyvsp[0].code));
      }
#line 3501 "src/zimpl/mmlparse2.c"
    break;

  case 124: /* vbool: vexpr CMP_GT cexpr  */
#line 717 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_vbool_gt, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3509 "src/zimpl/mmlparse2.c"
    break;

  case 125: /* vbool: vbool AND vbool  */
#line 720 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_vbool_and, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3515 "src/zimpl/mmlparse2.c"
    break;

  case 126: /* vbool: vbool OR vbool  */
#line 721 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_vbool_or,  2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3521 "src/zimpl/mmlparse2.c"
    break;

  case 127: /* vbool: vbool XOR vbool  */
#line 722 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_vbool_xor, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3527 "src/zimpl/mmlparse2.c"
    break;

  case 128: /* vbool: NOT vbool  */
#line 723 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_vbool_not, 1, (yyvsp[0].code)); }
#line 3533 "src/zimpl/mmlparse2.c"
    break;

  case 129: /* vbool: '(' vbool ')'  */
#line 724 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = (yyvsp[-1].code); }
#line 3539 "src/zimpl/mmlparse2.c"
    break;

  case 130: /* con_attr_list: %empty  */
#line 728 "src/zimpl/mmlparse2.y"
                                { (yyval.bits) = 0; }
#line 3545 "src/zimpl/mmlparse2.c"
    break;

  case 131: /* con_attr_list: con_attr_list ',' con_attr  */
#line 729 "src/zimpl/mmlparse2.y"
                                { (yyval.bits) = (yyvsp[-2].bits) | (yyvsp[0].bits); }
#line 3551 "src/zimpl/mmlparse2.c"
    break;

  case 132: /* con_attr: SCALE  */
#line 733 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_SCALE; }
#line 3557 "src/zimpl/mmlparse2.c"
    break;

  case 133: /* con_attr: SEPARATE  */
#line 734 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_SEPAR; }
#line 3563 "src/zimpl/mmlparse2.c"
    break;

  case 134: /* con_attr: CHECKONLY  */
#line 735 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_CHECK; }
#line 3569 "src/zimpl/mmlparse2.c"
    break;

  case 135: /* con_attr: INDICATOR  */
#line 736 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_INDIC; }
#line 3575 "src/zimpl/mmlparse2.c"
    break;

  case 136: /* con_attr: QUBO  */
#line 737 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_QUBO;  }
#line 3581 "src/zimpl/mmlparse2.c"
    break;

  case 137: /* con_attr: PENALTY1  */
#line 738 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY1; }
#line 3587 "src/zimpl/mmlparse2.c"
    break;

  case 138: /* con_attr: PENALTY2  */
#line 739 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY2; }
#line 3593 "src/zimpl/mmlparse2.c"
    break;

  case 139: /* con_attr: PENALTY3  */
#line 740 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY3; }
#line 3599 "src/zimpl/mmlparse2.c"
    break;

  case 140: /* con_attr: PENALTY4  */
#line 741 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY4; }
#line 3605 "src/zimpl/mmlparse2.c"
    break;

  case 141: /* con_attr: PENALTY5  */
#line 742 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY5; }
#line 3611 "src/zimpl/mmlparse2.c"
    break;

  case 142: /* con_attr: PENALTY6  */
#line 743 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY6; }
#line 3617 "src/zimpl/mmlparse2.c"
    break;

  case 143: /* con_attr: PENALTY7  */
#line 744 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY7; }
#line 3623 "src/zimpl/mmlparse2.c"
    break;

  case 144: /* con_attr: PENALTY8  */
#line 745 "src/zimpl/mmlparse2.y"
               { (yyval.bits) = LP_FLAG_CON_PENALTY8; }
#line 3629 "src/zimpl/mmlparse2.c"
    break;

  case 145: /* con_type: CMP_LE  */
#line 749 "src/zimpl/mmlparse2.y"
             { (yyval.code) = code_new_contype(CON_RHS); }
#line 3635 "src/zimpl/mmlparse2.c"
    break;

  case 146: /* con_type: CMP_GE  */
#line 750 "src/zimpl/mmlparse2.y"
             { (yyval.code) = code_new_contype(CON_LHS); }
#line 3641 "src/zimpl/mmlparse2.c"
    break;

  case 147: /* con_type: CMP_EQ  */
#line 751 "src/zimpl/mmlparse2.y"
             { (yyval.code) = code_new_contype(CON_EQUAL); }
#line 3647 "src/zimpl/mmlparse2.c"
    break;

  case 148: /* vexpr: vproduct  */
#line 755 "src/zimpl/mmlparse2.y"
              { (yyval.code) = (yyvsp[0].code); }
#line 3653 "src/zimpl/mmlparse2.c"
    break;

  case 149: /* vexpr: vexpr '+' vproduct  */
#line 756 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_term_add, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3659 "src/zimpl/mmlparse2.c"
    break;

  case 150: /* vexpr: vexpr '-' vproduct  */
#line 757 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_term_sub, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3665 "src/zimpl/mmlparse2.c"
    break;

  case 151: /* vexpr: vexpr '+' cproduct  */
#line 758 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_term_const, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3671 "src/zimpl/mmlparse2.c"
    break;

  case 152: /* vexpr: vexpr '-' cproduct  */
#line 759 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_term_sub, 2, (yyvsp[-2].code), code_new_inst(i_term_expr, 1, (yyvsp[0].code)));
      }
#line 3679 "src/zimpl/mmlparse2.c"
    break;

  case 153: /* vexpr: cexpr '+' vproduct  */
#line 762 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_term_const, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 3685 "src/zimpl/mmlparse2.c"
    break;

  case 154: /* vexpr: cexpr '-' vproduct  */
#line 763 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_term_sub, 2,
            code_new_inst(i_term_expr, 1, (yyvsp[-2].code)),
            (yyvsp[0].code));
      }
#line 3695 "src/zimpl/mmlparse2.c"
    break;

  case 155: /* vproduct: vfactor  */
#line 771 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = (yyvsp[0].code); }
#line 3701 "src/zimpl/mmlparse2.c"
    break;

  case 156: /* vproduct: vproduct '*' cfactor  */
#line 772 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[-2].code), (yyvsp[0].code));  }
#line 3707 "src/zimpl/mmlparse2.c"
    break;

  case 157: /* vproduct: vproduct '/' cfactor  */
#line 773 "src/zimpl/mmlparse2.y"
                             {
         (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[-2].code),
            code_new_inst(i_expr_div, 2, code_new_numb(numb_new_integer(1)), (yyvsp[0].code)));
      }
#line 3716 "src/zimpl/mmlparse2.c"
    break;

  case 158: /* vproduct: cproduct '*' vfactor  */
#line 777 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 3722 "src/zimpl/mmlparse2.c"
    break;

  case 159: /* vproduct: vproduct '*' vfactor  */
#line 778 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_term_mul, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3728 "src/zimpl/mmlparse2.c"
    break;

  case 161: /* vfactor: '+' vfactor  */
#line 783 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = (yyvsp[0].code); }
#line 3734 "src/zimpl/mmlparse2.c"
    break;

  case 162: /* vfactor: '-' vfactor  */
#line 784 "src/zimpl/mmlparse2.y"
                              { 
         (yyval.code) = code_new_inst(i_term_coeff, 2, (yyvsp[0].code), code_new_numb(numb_new_integer(-1)));
      }
#line 3742 "src/zimpl/mmlparse2.c"
    break;

  case 163: /* vexpo: vval  */
#line 790 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = (yyvsp[0].code); }
#line 3748 "src/zimpl/mmlparse2.c"
    break;

  case 164: /* vexpo: vval POW cfactor  */
#line 791 "src/zimpl/mmlparse2.y"
                           { 
         (yyval.code) = code_new_inst(i_term_power, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3756 "src/zimpl/mmlparse2.c"
    break;

  case 165: /* vexpo: SUM idxset DO vproduct  */
#line 794 "src/zimpl/mmlparse2.y"
                           {
         (yyval.code) = code_new_inst(i_term_sum, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3764 "src/zimpl/mmlparse2.c"
    break;

  case 166: /* vval: VARSYM symidx  */
#line 800 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 3772 "src/zimpl/mmlparse2.c"
    break;

  case 167: /* vval: VABS '(' vexpr ')'  */
#line 803 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vabs, 1, (yyvsp[-1].code)); }
#line 3778 "src/zimpl/mmlparse2.c"
    break;

  case 168: /* vval: SQRT '(' vexpr ')'  */
#line 804 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(-2)), (yyvsp[-1].code)); }
#line 3784 "src/zimpl/mmlparse2.c"
    break;

  case 169: /* vval: LOG '(' vexpr ')'  */
#line 805 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(3)), (yyvsp[-1].code)); }
#line 3790 "src/zimpl/mmlparse2.c"
    break;

  case 170: /* vval: EXP '(' vexpr ')'  */
#line 806 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(4)), (yyvsp[-1].code)); }
#line 3796 "src/zimpl/mmlparse2.c"
    break;

  case 171: /* vval: LN '(' vexpr ')'  */
#line 807 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(5)), (yyvsp[-1].code)); }
#line 3802 "src/zimpl/mmlparse2.c"
    break;

  case 172: /* vval: SIN '(' vexpr ')'  */
#line 808 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(6)), (yyvsp[-1].code)); }
#line 3808 "src/zimpl/mmlparse2.c"
    break;

  case 173: /* vval: COS '(' vexpr ')'  */
#line 809 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(7)), (yyvsp[-1].code)); }
#line 3814 "src/zimpl/mmlparse2.c"
    break;

  case 174: /* vval: TAN '(' vexpr ')'  */
#line 810 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(8)), (yyvsp[-1].code)); }
#line 3820 "src/zimpl/mmlparse2.c"
    break;

  case 175: /* vval: ABS '(' vexpr ')'  */
#line 811 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(9)), (yyvsp[-1].code)); }
#line 3826 "src/zimpl/mmlparse2.c"
    break;

  case 176: /* vval: SGN '(' vexpr ')'  */
#line 812 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_vexpr_fun, 2, code_new_numb(numb_new_integer(10)), (yyvsp[-1].code)); }
#line 3832 "src/zimpl/mmlparse2.c"
    break;

  case 177: /* vval: POWER '(' vexpr ',' cexpr ')'  */
#line 813 "src/zimpl/mmlparse2.y"
                                    {
         (yyval.code) = code_new_inst(i_vexpr_fun, 3, code_new_numb(numb_new_integer(11)), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3840 "src/zimpl/mmlparse2.c"
    break;

  case 178: /* vval: SGNPOW '(' vexpr ',' cexpr ')'  */
#line 816 "src/zimpl/mmlparse2.y"
                                   {
         (yyval.code) = code_new_inst(i_vexpr_fun, 3, code_new_numb(numb_new_integer(12)), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3848 "src/zimpl/mmlparse2.c"
    break;

  case 179: /* vval: IF lexpr THEN vexpr ELSE vexpr END  */
#line 819 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 3856 "src/zimpl/mmlparse2.c"
    break;

  case 180: /* vval: '(' vexpr ')'  */
#line 822 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = (yyvsp[-1].code); }
#line 3862 "src/zimpl/mmlparse2.c"
    break;

  case 181: /* decl_sos: DECLSOS NAME DO soset ';'  */
#line 830 "src/zimpl/mmlparse2.y"
                               {
        (yyval.code) = code_new_inst(i_sos, 2, code_new_name((yyvsp[-3].name)), (yyvsp[-1].code));
     }
#line 3870 "src/zimpl/mmlparse2.c"
    break;

  case 182: /* soset: sos_type priority DO vexpr  */
#line 836 "src/zimpl/mmlparse2.y"
                                {
        (yyval.code) = code_new_inst(i_soset, 3, (yyvsp[0].code), (yyvsp[-3].code), (yyvsp[-2].code));
     }
#line 3878 "src/zimpl/mmlparse2.c"
    break;

  case 183: /* soset: FORALL idxset DO soset  */
#line 839 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3886 "src/zimpl/mmlparse2.c"
    break;

  case 184: /* sos_type: %empty  */
#line 845 "src/zimpl/mmlparse2.y"
                 { (yyval.code) = code_new_numb(numb_new_integer(1)); }
#line 3892 "src/zimpl/mmlparse2.c"
    break;

  case 185: /* sos_type: TYPE1  */
#line 846 "src/zimpl/mmlparse2.y"
                 { (yyval.code) = code_new_numb(numb_new_integer(1)); }
#line 3898 "src/zimpl/mmlparse2.c"
    break;

  case 186: /* sos_type: TYPE2  */
#line 847 "src/zimpl/mmlparse2.y"
                 { (yyval.code) = code_new_numb(numb_new_integer(2)); }
#line 3904 "src/zimpl/mmlparse2.c"
    break;

  case 187: /* exec_do: DO command ';'  */
#line 855 "src/zimpl/mmlparse2.y"
                    { (yyval.code) = (yyvsp[-1].code); }
#line 3910 "src/zimpl/mmlparse2.c"
    break;

  case 188: /* command: PRINT cexpr_list  */
#line 859 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3916 "src/zimpl/mmlparse2.c"
    break;

  case 189: /* command: PRINT tuple  */
#line 860 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3922 "src/zimpl/mmlparse2.c"
    break;

  case 190: /* command: PRINT sexpr  */
#line 861 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3928 "src/zimpl/mmlparse2.c"
    break;

  case 191: /* command: PRINT lexpr  */
#line 862 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_print, 1, (yyvsp[0].code)); }
#line 3934 "src/zimpl/mmlparse2.c"
    break;

  case 192: /* command: PRINT VARSYM  */
#line 863 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_print, 1, code_new_symbol((yyvsp[0].sym))); }
#line 3940 "src/zimpl/mmlparse2.c"
    break;

  case 193: /* command: CHECK lexpr  */
#line 864 "src/zimpl/mmlparse2.y"
                      { (yyval.code) = code_new_inst(i_check, 1, (yyvsp[0].code)); }
#line 3946 "src/zimpl/mmlparse2.c"
    break;

  case 194: /* command: FORALL idxset DO command  */
#line 865 "src/zimpl/mmlparse2.y"
                              {
        (yyval.code) = code_new_inst(i_forall, 2, (yyvsp[-2].code), (yyvsp[0].code));
     }
#line 3954 "src/zimpl/mmlparse2.c"
    break;

  case 195: /* idxset: pure_idxset  */
#line 875 "src/zimpl/mmlparse2.y"
                 { (yyval.code) = (yyvsp[0].code); }
#line 3960 "src/zimpl/mmlparse2.c"
    break;

  case 196: /* idxset: sexpr  */
#line 876 "src/zimpl/mmlparse2.y"
           {
         (yyval.code) = code_new_inst(i_idxset_new, 3,
            code_new_inst(i_tuple_empty, 0), (yyvsp[0].code), code_new_inst(i_bool_true, 0));
      }
#line 3969 "src/zimpl/mmlparse2.c"
    break;

  case 197: /* pure_idxset: tuple IN sexpr WITH lexpr  */
#line 883 "src/zimpl/mmlparse2.y"
                               {
         (yyval.code) = code_new_inst(i_idxset_new, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3977 "src/zimpl/mmlparse2.c"
    break;

  case 198: /* pure_idxset: tuple IN sexpr  */
#line 886 "src/zimpl/mmlparse2.y"
                    {
         (yyval.code) = code_new_inst(i_idxset_new, 3, (yyvsp[-2].code), (yyvsp[0].code), code_new_inst(i_bool_true, 0));
      }
#line 3985 "src/zimpl/mmlparse2.c"
    break;

  case 200: /* sexpr: sexpr UNION sunion  */
#line 893 "src/zimpl/mmlparse2.y"
                         { (yyval.code) = code_new_inst(i_set_union, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 3991 "src/zimpl/mmlparse2.c"
    break;

  case 201: /* sexpr: sexpr '+' sunion  */
#line 894 "src/zimpl/mmlparse2.y"
                      {
         (yyval.code) = code_new_inst(i_set_union, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 3999 "src/zimpl/mmlparse2.c"
    break;

  case 202: /* sexpr: sexpr SYMDIFF sunion  */
#line 897 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_set_sdiff, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4005 "src/zimpl/mmlparse2.c"
    break;

  case 203: /* sexpr: sexpr '-' sunion  */
#line 898 "src/zimpl/mmlparse2.y"
                       {
         (yyval.code) = code_new_inst(i_set_minus, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4013 "src/zimpl/mmlparse2.c"
    break;

  case 204: /* sexpr: sexpr WITHOUT sunion  */
#line 901 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_set_minus, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4019 "src/zimpl/mmlparse2.c"
    break;

  case 205: /* sexpr: sexpr INTER sunion  */
#line 902 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_set_inter, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4025 "src/zimpl/mmlparse2.c"
    break;

  case 207: /* sunion: UNION idxset DO sproduct  */
#line 906 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_set_union2, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4031 "src/zimpl/mmlparse2.c"
    break;

  case 209: /* sproduct: sproduct CROSS sval  */
#line 911 "src/zimpl/mmlparse2.y"
                                   { (yyval.code) = code_new_inst(i_set_cross, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4037 "src/zimpl/mmlparse2.c"
    break;

  case 210: /* sproduct: sproduct '*' sval  */
#line 912 "src/zimpl/mmlparse2.y"
                       {
         (yyval.code) = code_new_inst(i_set_cross, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4045 "src/zimpl/mmlparse2.c"
    break;

  case 211: /* sproduct: INTER idxset DO sval  */
#line 915 "src/zimpl/mmlparse2.y"
                               { (yyval.code) = code_new_inst(i_set_inter2, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4051 "src/zimpl/mmlparse2.c"
    break;

  case 212: /* sval: SETSYM symidx  */
#line 919 "src/zimpl/mmlparse2.y"
                    {
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4059 "src/zimpl/mmlparse2.c"
    break;

  case 213: /* sval: SETDEF '(' cexpr_list ')'  */
#line 922 "src/zimpl/mmlparse2.y"
                               {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4069 "src/zimpl/mmlparse2.c"
    break;

  case 214: /* sval: PERMUTE '(' sexpr ')'  */
#line 927 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_set_permute, 1, (yyvsp[-1].code)); }
#line 4075 "src/zimpl/mmlparse2.c"
    break;

  case 215: /* sval: '{' '}'  */
#line 928 "src/zimpl/mmlparse2.y"
             { (yyval.code) = code_new_inst(i_set_empty, 1, code_new_size(0)); }
#line 4081 "src/zimpl/mmlparse2.c"
    break;

  case 216: /* sval: '{' cexpr TO cexpr BY cexpr '}'  */
#line 929 "src/zimpl/mmlparse2.y"
                                     {
         (yyval.code) = code_new_inst(i_set_range2, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4089 "src/zimpl/mmlparse2.c"
    break;

  case 217: /* sval: '{' cexpr TO cexpr '}'  */
#line 932 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_set_range2, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(1)));
      }
#line 4097 "src/zimpl/mmlparse2.c"
    break;

  case 218: /* sval: '{' cexpr UNTIL cexpr BY cexpr '}'  */
#line 935 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_set_range, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4105 "src/zimpl/mmlparse2.c"
    break;

  case 219: /* sval: '{' cexpr UNTIL cexpr '}'  */
#line 938 "src/zimpl/mmlparse2.y"
                               {
         (yyval.code) = code_new_inst(i_set_range, 3, (yyvsp[-3].code), (yyvsp[-1].code), code_new_numb(numb_new_integer(1)));
      }
#line 4113 "src/zimpl/mmlparse2.c"
    break;

  case 220: /* sval: ARGMIN idxset DO cexpr  */
#line 941 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_set_argmin, 3, code_new_numb(numb_new_integer(1)), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4121 "src/zimpl/mmlparse2.c"
    break;

  case 221: /* sval: ARGMIN '(' cexpr ')' idxset DO cexpr  */
#line 944 "src/zimpl/mmlparse2.y"
                                                      {
         (yyval.code) = code_new_inst(i_set_argmin, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4129 "src/zimpl/mmlparse2.c"
    break;

  case 222: /* sval: ARGMAX idxset DO cexpr  */
#line 947 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_set_argmax, 3, code_new_numb(numb_new_integer(1)), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4137 "src/zimpl/mmlparse2.c"
    break;

  case 223: /* sval: ARGMAX '(' cexpr ')' idxset DO cexpr  */
#line 950 "src/zimpl/mmlparse2.y"
                                                      {
         (yyval.code) = code_new_inst(i_set_argmax, 3, (yyvsp[-4].code), (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4145 "src/zimpl/mmlparse2.c"
    break;

  case 224: /* sval: '(' sexpr ')'  */
#line 953 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = (yyvsp[-1].code); }
#line 4151 "src/zimpl/mmlparse2.c"
    break;

  case 225: /* sval: '{' tuple_list '}'  */
#line 954 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_set_new_tuple, 1, (yyvsp[-1].code)); }
#line 4157 "src/zimpl/mmlparse2.c"
    break;

  case 226: /* sval: '{' cexpr_list '}'  */
#line 955 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_set_new_elem, 1, (yyvsp[-1].code)); }
#line 4163 "src/zimpl/mmlparse2.c"
    break;

  case 227: /* sval: '{' idxset '}'  */
#line 956 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_set_idxset, 1, (yyvsp[-1].code)); }
#line 4169 "src/zimpl/mmlparse2.c"
    break;

  case 228: /* sval: '{' idxset DO cexpr '}'  */
#line 957 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_set_expr, 2, (yyvsp[-3].code), (yyvsp[-1].code)); }
#line 4175 "src/zimpl/mmlparse2.c"
    break;

  case 229: /* sval: '{' idxset DO tuple '}'  */
#line 958 "src/zimpl/mmlparse2.y"
                             { (yyval.code) = code_new_inst(i_set_expr, 2, (yyvsp[-3].code), (yyvsp[-1].code)); }
#line 4181 "src/zimpl/mmlparse2.c"
    break;

  case 230: /* sval: PROJ '(' sexpr ',' tuple ')'  */
#line 959 "src/zimpl/mmlparse2.y"
                                  {
         (yyval.code) = code_new_inst(i_set_proj, 2, (yyvsp[-3].code), (yyvsp[-1].code));
       }
#line 4189 "src/zimpl/mmlparse2.c"
    break;

  case 231: /* sval: INDEXSET '(' SETSYM ')'  */
#line 962 "src/zimpl/mmlparse2.y"
                             {
          (yyval.code) = code_new_inst(i_set_indexset, 1, code_new_symbol((yyvsp[-1].sym)));
       }
#line 4197 "src/zimpl/mmlparse2.c"
    break;

  case 232: /* sval: IF lexpr THEN sexpr ELSE sexpr END  */
#line 965 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4205 "src/zimpl/mmlparse2.c"
    break;

  case 233: /* read: READ cexpr AS cexpr  */
#line 971 "src/zimpl/mmlparse2.y"
                         { (yyval.code) = code_new_inst(i_read_new, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4211 "src/zimpl/mmlparse2.c"
    break;

  case 234: /* read: read read_par  */
#line 972 "src/zimpl/mmlparse2.y"
                         { (yyval.code) = code_new_inst(i_read_param, 2, (yyvsp[-1].code), (yyvsp[0].code)); }
#line 4217 "src/zimpl/mmlparse2.c"
    break;

  case 235: /* read_par: SKIP cexpr  */
#line 976 "src/zimpl/mmlparse2.y"
                   { (yyval.code) = code_new_inst(i_read_skip, 1, (yyvsp[0].code)); }
#line 4223 "src/zimpl/mmlparse2.c"
    break;

  case 236: /* read_par: USE cexpr  */
#line 977 "src/zimpl/mmlparse2.y"
                   { (yyval.code) = code_new_inst(i_read_use, 1, (yyvsp[0].code)); }
#line 4229 "src/zimpl/mmlparse2.c"
    break;

  case 237: /* read_par: COMMENT cexpr  */
#line 978 "src/zimpl/mmlparse2.y"
                   { (yyval.code) = code_new_inst(i_read_comment, 1, (yyvsp[0].code)); }
#line 4235 "src/zimpl/mmlparse2.c"
    break;

  case 238: /* read_par: MATCH cexpr  */
#line 979 "src/zimpl/mmlparse2.y"
                   { (yyval.code) = code_new_inst(i_read_match, 1, (yyvsp[0].code)); }
#line 4241 "src/zimpl/mmlparse2.c"
    break;

  case 239: /* tuple_list: tuple  */
#line 983 "src/zimpl/mmlparse2.y"
           {
         (yyval.code) = code_new_inst(i_tuple_list_new, 1, (yyvsp[0].code));
      }
#line 4249 "src/zimpl/mmlparse2.c"
    break;

  case 240: /* tuple_list: tuple_list ',' tuple  */
#line 986 "src/zimpl/mmlparse2.y"
                           {
         (yyval.code) = code_new_inst(i_tuple_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4257 "src/zimpl/mmlparse2.c"
    break;

  case 241: /* tuple_list: read  */
#line 989 "src/zimpl/mmlparse2.y"
          { (yyval.code) = code_new_inst(i_read, 1, (yyvsp[0].code)); }
#line 4263 "src/zimpl/mmlparse2.c"
    break;

  case 242: /* lexpr: cexpr CMP_EQ cexpr  */
#line 993 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_eq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4269 "src/zimpl/mmlparse2.c"
    break;

  case 243: /* lexpr: cexpr CMP_NE cexpr  */
#line 994 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_ne, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4275 "src/zimpl/mmlparse2.c"
    break;

  case 244: /* lexpr: cexpr CMP_GT cexpr  */
#line 995 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_gt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4281 "src/zimpl/mmlparse2.c"
    break;

  case 245: /* lexpr: cexpr CMP_GE cexpr  */
#line 996 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_ge, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4287 "src/zimpl/mmlparse2.c"
    break;

  case 246: /* lexpr: cexpr CMP_LT cexpr  */
#line 997 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_lt, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4293 "src/zimpl/mmlparse2.c"
    break;

  case 247: /* lexpr: cexpr CMP_LE cexpr  */
#line 998 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_bool_le, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4299 "src/zimpl/mmlparse2.c"
    break;

  case 248: /* lexpr: sexpr CMP_EQ sexpr  */
#line 999 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_seq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4305 "src/zimpl/mmlparse2.c"
    break;

  case 249: /* lexpr: sexpr CMP_NE sexpr  */
#line 1000 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_sneq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4311 "src/zimpl/mmlparse2.c"
    break;

  case 250: /* lexpr: sexpr CMP_GT sexpr  */
#line 1001 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_subs, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 4317 "src/zimpl/mmlparse2.c"
    break;

  case 251: /* lexpr: sexpr CMP_GE sexpr  */
#line 1002 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_sseq, 2, (yyvsp[0].code), (yyvsp[-2].code)); }
#line 4323 "src/zimpl/mmlparse2.c"
    break;

  case 252: /* lexpr: sexpr CMP_LT sexpr  */
#line 1003 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_subs, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4329 "src/zimpl/mmlparse2.c"
    break;

  case 253: /* lexpr: sexpr CMP_LE sexpr  */
#line 1004 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_sseq, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4335 "src/zimpl/mmlparse2.c"
    break;

  case 254: /* lexpr: lexpr AND lexpr  */
#line 1005 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_and, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4341 "src/zimpl/mmlparse2.c"
    break;

  case 255: /* lexpr: lexpr OR lexpr  */
#line 1006 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_or,  2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4347 "src/zimpl/mmlparse2.c"
    break;

  case 256: /* lexpr: lexpr XOR lexpr  */
#line 1007 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_xor, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4353 "src/zimpl/mmlparse2.c"
    break;

  case 257: /* lexpr: NOT lexpr  */
#line 1008 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_not, 1, (yyvsp[0].code)); }
#line 4359 "src/zimpl/mmlparse2.c"
    break;

  case 258: /* lexpr: '(' lexpr ')'  */
#line 1009 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = (yyvsp[-1].code); }
#line 4365 "src/zimpl/mmlparse2.c"
    break;

  case 259: /* lexpr: tuple IN sexpr  */
#line 1010 "src/zimpl/mmlparse2.y"
                        { (yyval.code) = code_new_inst(i_bool_is_elem, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4371 "src/zimpl/mmlparse2.c"
    break;

  case 260: /* lexpr: EXISTS '(' idxset ')'  */
#line 1011 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_bool_exists, 1, (yyvsp[-1].code)); }
#line 4377 "src/zimpl/mmlparse2.c"
    break;

  case 261: /* lexpr: BOOLDEF '(' cexpr_list ')'  */
#line 1012 "src/zimpl/mmlparse2.y"
                                {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4387 "src/zimpl/mmlparse2.c"
    break;

  case 262: /* lexpr: IF lexpr THEN lexpr ELSE lexpr END  */
#line 1017 "src/zimpl/mmlparse2.y"
                                        {
        (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
     }
#line 4395 "src/zimpl/mmlparse2.c"
    break;

  case 263: /* tuple: CMP_LT CMP_GT  */
#line 1023 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_tuple_empty, 0); }
#line 4401 "src/zimpl/mmlparse2.c"
    break;

  case 264: /* tuple: CMP_LT cexpr_list CMP_GT  */
#line 1024 "src/zimpl/mmlparse2.y"
                              { (yyval.code) = code_new_inst(i_tuple_new, 1, (yyvsp[-1].code));  }
#line 4407 "src/zimpl/mmlparse2.c"
    break;

  case 265: /* symidx: %empty  */
#line 1028 "src/zimpl/mmlparse2.y"
                  {
         (yyval.code) = code_new_inst(i_tuple_empty, 0);
      }
#line 4415 "src/zimpl/mmlparse2.c"
    break;

  case 266: /* symidx: '[' cexpr_list ']'  */
#line 1031 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_tuple_new, 1, (yyvsp[-1].code));
      }
#line 4423 "src/zimpl/mmlparse2.c"
    break;

  case 267: /* cexpr_list: cexpr  */
#line 1037 "src/zimpl/mmlparse2.y"
           {
         (yyval.code) = code_new_inst(i_elem_list_new, 1, (yyvsp[0].code));
      }
#line 4431 "src/zimpl/mmlparse2.c"
    break;

  case 268: /* cexpr_list: cexpr_list ',' cexpr  */
#line 1040 "src/zimpl/mmlparse2.y"
                          {
         (yyval.code) = code_new_inst(i_elem_list_add, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4439 "src/zimpl/mmlparse2.c"
    break;

  case 269: /* cexpr: cproduct  */
#line 1046 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = (yyvsp[0].code); }
#line 4445 "src/zimpl/mmlparse2.c"
    break;

  case 270: /* cexpr: cexpr '+' cproduct  */
#line 1047 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_add, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4451 "src/zimpl/mmlparse2.c"
    break;

  case 271: /* cexpr: cexpr '-' cproduct  */
#line 1048 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_sub, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4457 "src/zimpl/mmlparse2.c"
    break;

  case 272: /* cproduct: cfactor  */
#line 1052 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = (yyvsp[0].code); }
#line 4463 "src/zimpl/mmlparse2.c"
    break;

  case 273: /* cproduct: cproduct '*' cfactor  */
#line 1053 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_mul, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4469 "src/zimpl/mmlparse2.c"
    break;

  case 274: /* cproduct: cproduct '/' cfactor  */
#line 1054 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_div, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4475 "src/zimpl/mmlparse2.c"
    break;

  case 275: /* cproduct: cproduct MOD cfactor  */
#line 1055 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_mod, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4481 "src/zimpl/mmlparse2.c"
    break;

  case 276: /* cproduct: cproduct DIV cfactor  */
#line 1056 "src/zimpl/mmlparse2.y"
                           { (yyval.code) = code_new_inst(i_expr_intdiv, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4487 "src/zimpl/mmlparse2.c"
    break;

  case 277: /* cproduct: PROD idxset DO cfactor  */
#line 1057 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_expr_prod, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4495 "src/zimpl/mmlparse2.c"
    break;

  case 279: /* cfactor: '+' cexpo  */
#line 1064 "src/zimpl/mmlparse2.y"
                { (yyval.code) = (yyvsp[0].code); }
#line 4501 "src/zimpl/mmlparse2.c"
    break;

  case 280: /* cfactor: '-' cexpo  */
#line 1065 "src/zimpl/mmlparse2.y"
                { (yyval.code) = code_new_inst(i_expr_neg, 1, (yyvsp[0].code)); }
#line 4507 "src/zimpl/mmlparse2.c"
    break;

  case 282: /* cexpo: cval POW cfactor  */
#line 1070 "src/zimpl/mmlparse2.y"
                          { (yyval.code) = code_new_inst(i_expr_pow, 2, (yyvsp[-2].code), (yyvsp[0].code)); }
#line 4513 "src/zimpl/mmlparse2.c"
    break;

  case 283: /* cexpo: SUM idxset DO cproduct  */
#line 1071 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_expr_sum, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4521 "src/zimpl/mmlparse2.c"
    break;

  case 284: /* cexpo: MIN pure_idxset DO cfactor  */
#line 1074 "src/zimpl/mmlparse2.y"
                                {
         (yyval.code) = code_new_inst(i_expr_min, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4529 "src/zimpl/mmlparse2.c"
    break;

  case 285: /* cexpo: MAX pure_idxset DO cfactor  */
#line 1077 "src/zimpl/mmlparse2.y"
                                {
         (yyval.code) = code_new_inst(i_expr_max, 2, (yyvsp[-2].code), (yyvsp[0].code));
      }
#line 4537 "src/zimpl/mmlparse2.c"
    break;

  case 286: /* cexpo: MIN '(' idxset ')'  */
#line 1080 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_expr_sglmin, 1, (yyvsp[-1].code));
         }
#line 4545 "src/zimpl/mmlparse2.c"
    break;

  case 287: /* cexpo: MAX '(' idxset ')'  */
#line 1083 "src/zimpl/mmlparse2.y"
                        {
         (yyval.code) = code_new_inst(i_expr_sglmax, 1, (yyvsp[-1].code));
      }
#line 4553 "src/zimpl/mmlparse2.c"
    break;

  case 288: /* cval: NUMB  */
#line 1089 "src/zimpl/mmlparse2.y"
                { (yyval.code) = code_new_numb((yyvsp[0].numb)); }
#line 4559 "src/zimpl/mmlparse2.c"
    break;

  case 289: /* cval: STRG  */
#line 1090 "src/zimpl/mmlparse2.y"
                { (yyval.code) = code_new_strg((yyvsp[0].strg));  }
#line 4565 "src/zimpl/mmlparse2.c"
    break;

  case 290: /* cval: NAME  */
#line 1091 "src/zimpl/mmlparse2.y"
                {
         (yyval.code) = code_new_inst(i_local_deref, 1, code_new_name((yyvsp[0].name)));
      }
#line 4573 "src/zimpl/mmlparse2.c"
    break;

  case 291: /* cval: NUMBSYM symidx  */
#line 1094 "src/zimpl/mmlparse2.y"
                    { 
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4581 "src/zimpl/mmlparse2.c"
    break;

  case 292: /* cval: STRGSYM symidx  */
#line 1097 "src/zimpl/mmlparse2.y"
                    { 
         (yyval.code) = code_new_inst(i_symbol_deref, 2, code_new_symbol((yyvsp[-1].sym)), (yyvsp[0].code));
      }
#line 4589 "src/zimpl/mmlparse2.c"
    break;

  case 293: /* cval: NUMBDEF '(' cexpr_list ')'  */
#line 1100 "src/zimpl/mmlparse2.y"
                                {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4599 "src/zimpl/mmlparse2.c"
    break;

  case 294: /* cval: STRGDEF '(' cexpr_list ')'  */
#line 1105 "src/zimpl/mmlparse2.y"
                                {
         (yyval.code) = code_new_inst(i_define_deref, 2,
            code_new_define((yyvsp[-3].def)),
            code_new_inst(i_tuple_new, 1, (yyvsp[-1].code)));
      }
#line 4609 "src/zimpl/mmlparse2.c"
    break;

  case 295: /* cval: cval FAC  */
#line 1110 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_fac, 1, (yyvsp[-1].code)); }
#line 4615 "src/zimpl/mmlparse2.c"
    break;

  case 296: /* cval: CARD '(' sexpr ')'  */
#line 1111 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_card, 1, (yyvsp[-1].code)); }
#line 4621 "src/zimpl/mmlparse2.c"
    break;

  case 297: /* cval: ABS '(' cexpr ')'  */
#line 1112 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_abs, 1, (yyvsp[-1].code)); }
#line 4627 "src/zimpl/mmlparse2.c"
    break;

  case 298: /* cval: SGN '(' cexpr ')'  */
#line 1113 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_sgn, 1, (yyvsp[-1].code)); }
#line 4633 "src/zimpl/mmlparse2.c"
    break;

  case 299: /* cval: ROUND '(' cexpr ')'  */
#line 1114 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_round, 1, (yyvsp[-1].code)); }
#line 4639 "src/zimpl/mmlparse2.c"
    break;

  case 300: /* cval: FLOOR '(' cexpr ')'  */
#line 1115 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_floor, 1, (yyvsp[-1].code)); }
#line 4645 "src/zimpl/mmlparse2.c"
    break;

  case 301: /* cval: CEIL '(' cexpr ')'  */
#line 1116 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_ceil, 1, (yyvsp[-1].code)); }
#line 4651 "src/zimpl/mmlparse2.c"
    break;

  case 302: /* cval: LOG '(' cexpr ')'  */
#line 1117 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_log, 1, (yyvsp[-1].code)); }
#line 4657 "src/zimpl/mmlparse2.c"
    break;

  case 303: /* cval: LN '(' cexpr ')'  */
#line 1118 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_ln, 1, (yyvsp[-1].code)); }
#line 4663 "src/zimpl/mmlparse2.c"
    break;

  case 304: /* cval: EXP '(' cexpr ')'  */
#line 1119 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_exp, 1, (yyvsp[-1].code)); }
#line 4669 "src/zimpl/mmlparse2.c"
    break;

  case 305: /* cval: SQRT '(' cexpr ')'  */
#line 1120 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_sqrt, 1, (yyvsp[-1].code)); }
#line 4675 "src/zimpl/mmlparse2.c"
    break;

  case 306: /* cval: SIN '(' cexpr ')'  */
#line 1121 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_sin, 1, (yyvsp[-1].code)); }
#line 4681 "src/zimpl/mmlparse2.c"
    break;

  case 307: /* cval: COS '(' cexpr ')'  */
#line 1122 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_cos, 1, (yyvsp[-1].code)); }
#line 4687 "src/zimpl/mmlparse2.c"
    break;

  case 308: /* cval: TAN '(' cexpr ')'  */
#line 1123 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_tan, 1, (yyvsp[-1].code)); }
#line 4693 "src/zimpl/mmlparse2.c"
    break;

  case 309: /* cval: ASIN '(' cexpr ')'  */
#line 1124 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_asin, 1, (yyvsp[-1].code)); }
#line 4699 "src/zimpl/mmlparse2.c"
    break;

  case 310: /* cval: ACOS '(' cexpr ')'  */
#line 1125 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_acos, 1, (yyvsp[-1].code)); }
#line 4705 "src/zimpl/mmlparse2.c"
    break;

  case 311: /* cval: ATAN '(' cexpr ')'  */
#line 1126 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_atan, 1, (yyvsp[-1].code)); }
#line 4711 "src/zimpl/mmlparse2.c"
    break;

  case 312: /* cval: '(' cexpr ')'  */
#line 1128 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = (yyvsp[-1].code); }
#line 4717 "src/zimpl/mmlparse2.c"
    break;

  case 313: /* cval: LENGTH '(' cexpr ')'  */
#line 1129 "src/zimpl/mmlparse2.y"
                            { (yyval.code) = code_new_inst(i_expr_length, 1, (yyvsp[-1].code)); }
#line 4723 "src/zimpl/mmlparse2.c"
    break;

  case 314: /* cval: SUBSTR '(' cexpr ',' cexpr ',' cexpr ')'  */
#line 1130 "src/zimpl/mmlparse2.y"
                                              {
         (yyval.code) = code_new_inst(i_expr_substr, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4731 "src/zimpl/mmlparse2.c"
    break;

  case 315: /* cval: RANDOM '(' cexpr ',' cexpr ')'  */
#line 1133 "src/zimpl/mmlparse2.y"
                                    {
         (yyval.code) = code_new_inst(i_expr_rand, 2, (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4739 "src/zimpl/mmlparse2.c"
    break;

  case 316: /* cval: IF lexpr THEN cexpr ELSE cexpr END  */
#line 1136 "src/zimpl/mmlparse2.y"
                                        {
         (yyval.code) = code_new_inst(i_expr_if_else, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4747 "src/zimpl/mmlparse2.c"
    break;

  case 317: /* cval: ORD '(' sexpr ',' cexpr ',' cexpr ')'  */
#line 1139 "src/zimpl/mmlparse2.y"
                                           {
         (yyval.code) = code_new_inst(i_expr_ord, 3, (yyvsp[-5].code), (yyvsp[-3].code), (yyvsp[-1].code));
      }
#line 4755 "src/zimpl/mmlparse2.c"
    break;

  case 318: /* cval: MIN '(' cexpr_list ')'  */
#line 1142 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_expr_min2, 1, (yyvsp[-1].code));
      }
#line 4763 "src/zimpl/mmlparse2.c"
    break;

  case 319: /* cval: MAX '(' cexpr_list ')'  */
#line 1145 "src/zimpl/mmlparse2.y"
                            {
         (yyval.code) = code_new_inst(i_expr_max2, 1, (yyvsp[-1].code));
      }
#line 4771 "src/zimpl/mmlparse2.c"
    break;


#line 4775 "src/zimpl/mmlparse2.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

