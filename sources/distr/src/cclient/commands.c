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
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "commands.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "system.h"
#include <utils.h>
#include <multiway-join.h>
#include <ogr_api.h>
#include <geos_c.h>

extern void RequestCopy(int to, int from, char *dataset, int x, int y);
extern void Clean();
extern void EndDgeo();
extern void StoreState(char *folder);
extern void LoadState(char *folder);

extern int yylineno;
int global_servers;
bool simulate_load;
bool geos_prepare;
extern OGRwkbGeometryType ds_type;
extern char *yytext;
int yydebug = 1;
int yyparse(void);
int yylex();
extern FILE *yyin;
extern char *error_linebuf;
extern enum HistogramType htype;
char *verbose;
bool printed_nonverbose_header;

void yyerror(const char *s)
{
	fprintf(stderr, "%d: %s %s\n", 
		yylineno, s, yytext);
	exit(1);
}

int yywrap() {
	return 1;
}

void increase_stack_size() {
	struct rlimit limit;
	if (getrlimit(RLIMIT_STACK, &limit) != 0) {
		printf("Error getting stack size limits. errno=%d\n", errno);
		return;
	}

	// set stack to max size
	// this is used mostly in LP/LR optimization functions
	unsigned long long old_stack_size = limit.rlim_cur;
	limit.rlim_cur = limit.rlim_max;
	if (setrlimit(RLIMIT_STACK, &limit) != 0) {
		printf("Error increasing stack size. errno=%d\n", errno);
		return;
	}
	unsigned long long limit_max = limit.rlim_max;
	printf("Stack size increased from %llu to %llu\n", old_stack_size,
		limit_max);
}

int startparser(char *argv) {
	signal(SIGSEGV, handle_sigsegv);
	signal(SIGABRT, handle_sigsegv);

	//srand(1376371803); // fix for proper testing times
	srand((unsigned int)time(NULL));

	increase_stack_size();
	
	// fix to print thousand separator
	setlocale(LC_NUMERIC, "");

	// print verbose messages
	printed_nonverbose_header = 0;
	verbose = getenv("MW_VERBOSE");

	char *env_servers = getenv("MW_SERVERS");
	if (env_servers)
		global_servers = atoi(env_servers);

	//optimize_smr2();

	// GEOS and OGR init
	OGRRegisterAll();
	initGEOS(geos_messages, geos_messages);

	if (argv) {
		setenv("MW_FILENAME", argv, 1);
		yyin = fopen(argv, "r");
	}
	yyparse();
	if (yyin)
		fclose(yyin);

	finishGEOS();

	//getchar();
	return 0;
}

void print_plan() {
	for(GList *l = joinplan; l != NULL; l = g_list_next(l)) {
		multiway_input_chain *i = l->data;
		printf("Dataset %s, check %s\n", i->name, i->pcheck == CHECKR ? "R" : "S");
	}
}


#line 191 "commands.c"

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

#include "commands.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_WORD = 3,                       /* WORD  */
  YYSYMBOL_FILENAME = 4,                   /* FILENAME  */
  YYSYMBOL_NUMBER = 5,                     /* NUMBER  */
  YYSYMBOL_TOKSTRING = 6,                  /* TOKSTRING  */
  YYSYMBOL_TOKCOMMENT = 7,                 /* TOKCOMMENT  */
  YYSYMBOL_TOKAS = 8,                      /* TOKAS  */
  YYSYMBOL_TOKLOAD = 9,                    /* TOKLOAD  */
  YYSYMBOL_TOKGPREPARE = 10,               /* TOKGPREPARE  */
  YYSYMBOL_TOKINDEX = 11,                  /* TOKINDEX  */
  YYSYMBOL_TOKSELECT = 12,                 /* TOKSELECT  */
  YYSYMBOL_TOKPLAN = 13,                   /* TOKPLAN  */
  YYSYMBOL_TOKFROM = 14,                   /* TOKFROM  */
  YYSYMBOL_TOKJOIN = 15,                   /* TOKJOIN  */
  YYSYMBOL_SIMPLEQUOTE = 16,               /* SIMPLEQUOTE  */
  YYSYMBOL_TOKWITH = 17,                   /* TOKWITH  */
  YYSYMBOL_TOKM = 18,                      /* TOKM  */
  YYSYMBOL_TOKSERVERS = 19,                /* TOKSERVERS  */
  YYSYMBOL_TOKSIMULATE = 20,               /* TOKSIMULATE  */
  YYSYMBOL_TOKSTAR = 21,                   /* TOKSTAR  */
  YYSYMBOL_TOKLAZY = 22,                   /* TOKLAZY  */
  YYSYMBOL_TOKR0 = 23,                     /* TOKR0  */
  YYSYMBOL_TOKCLEAN = 24,                  /* TOKCLEAN  */
  YYSYMBOL_TOKCLEANI = 25,                 /* TOKCLEANI  */
  YYSYMBOL_TOKENDDGEO = 26,                /* TOKENDDGEO  */
  YYSYMBOL_TOKHJJOIN = 27,                 /* TOKHJJOIN  */
  YYSYMBOL_TOKRJJOIN = 28,                 /* TOKRJJOIN  */
  YYSYMBOL_TOKINLJOIN = 29,                /* TOKINLJOIN  */
  YYSYMBOL_TOK_P_HJJOIN = 30,              /* TOK_P_HJJOIN  */
  YYSYMBOL_TOK_P_RJJOIN = 31,              /* TOK_P_RJJOIN  */
  YYSYMBOL_TOK_P_INLJOIN = 32,             /* TOK_P_INLJOIN  */
  YYSYMBOL_TOKPOINT = 33,                  /* TOKPOINT  */
  YYSYMBOL_TOKPOLYGON = 34,                /* TOKPOLYGON  */
  YYSYMBOL_TOKLINE = 35,                   /* TOKLINE  */
  YYSYMBOL_TOKSYSTEM = 36,                 /* TOKSYSTEM  */
  YYSYMBOL_TOKSETHIST = 37,                /* TOKSETHIST  */
  YYSYMBOL_TOKGRID = 38,                   /* TOKGRID  */
  YYSYMBOL_TOKSETOPT = 39,                 /* TOKSETOPT  */
  YYSYMBOL_TOKSETQNAME = 40,               /* TOKSETQNAME  */
  YYSYMBOL_TOKSTORE = 41,                  /* TOKSTORE  */
  YYSYMBOL_42_ = 42,                       /* ';'  */
  YYSYMBOL_43_ = 43,                       /* '('  */
  YYSYMBOL_44_ = 44,                       /* ')'  */
  YYSYMBOL_45_ = 45,                       /* ','  */
  YYSYMBOL_YYACCEPT = 46,                  /* $accept  */
  YYSYMBOL_commands = 47,                  /* commands  */
  YYSYMBOL_command = 48,                   /* command  */
  YYSYMBOL_load_switch = 49,               /* load_switch  */
  YYSYMBOL_file_identifier = 50,           /* file_identifier  */
  YYSYMBOL_load_parameters = 51,           /* load_parameters  */
  YYSYMBOL_load_param = 52,                /* load_param  */
  YYSYMBOL_index_switch = 53,              /* index_switch  */
  YYSYMBOL_index_parameters = 54,          /* index_parameters  */
  YYSYMBOL_index_param = 55,               /* index_param  */
  YYSYMBOL_m_switch = 56,                  /* m_switch  */
  YYSYMBOL_index_type = 57,                /* index_type  */
  YYSYMBOL_plan_switch = 58,               /* plan_switch  */
  YYSYMBOL_select_switch = 59,             /* select_switch  */
  YYSYMBOL_joins = 60,                     /* joins  */
  YYSYMBOL_join_switch = 61,               /* join_switch  */
  YYSYMBOL_join_type = 62,                 /* join_type  */
  YYSYMBOL_join_conditions = 63            /* join_conditions  */
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
typedef yytype_int8 yy_state_t;

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   81

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  50
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  81

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   296


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      43,    44,     2,     2,    45,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
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
      35,    36,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   145,   145,   146,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   166,   182,   183,
     187,   188,   192,   193,   194,   195,   196,   197,   202,   214,
     215,   219,   220,   224,   228,   229,   230,   234,   266,   298,
     299,   303,   351,   352,   353,   354,   355,   356,   357,   361,
     362
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
  "\"end of file\"", "error", "\"invalid token\"", "WORD", "FILENAME",
  "NUMBER", "TOKSTRING", "TOKCOMMENT", "TOKAS", "TOKLOAD", "TOKGPREPARE",
  "TOKINDEX", "TOKSELECT", "TOKPLAN", "TOKFROM", "TOKJOIN", "SIMPLEQUOTE",
  "TOKWITH", "TOKM", "TOKSERVERS", "TOKSIMULATE", "TOKSTAR", "TOKLAZY",
  "TOKR0", "TOKCLEAN", "TOKCLEANI", "TOKENDDGEO", "TOKHJJOIN", "TOKRJJOIN",
  "TOKINLJOIN", "TOK_P_HJJOIN", "TOK_P_RJJOIN", "TOK_P_INLJOIN",
  "TOKPOINT", "TOKPOLYGON", "TOKLINE", "TOKSYSTEM", "TOKSETHIST",
  "TOKGRID", "TOKSETOPT", "TOKSETQNAME", "TOKSTORE", "';'", "'('", "')'",
  "','", "$accept", "commands", "command", "load_switch",
  "file_identifier", "load_parameters", "load_param", "index_switch",
  "index_parameters", "index_param", "m_switch", "index_type",
  "plan_switch", "select_switch", "joins", "join_switch", "join_type",
  "join_conditions", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-46)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-39)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -46,     0,   -46,     3,     2,   -11,    -4,   -46,   -46,   -46,
      44,   -15,    35,    57,    11,    19,   -46,   -46,   -46,   -46,
     -46,   -46,    11,   -46,    54,    60,    61,   -46,   -46,   -46,
     -46,   -46,   -46,    52,    66,    26,    26,    62,    55,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,     1,   -46,    56,    17,
      70,    -1,   -46,    71,   -46,    72,   -46,   -46,   -46,    -1,
     -46,   -46,   -46,    33,    32,   -46,   -46,    75,   -46,    74,
     -46,   -46,   -46,   -46,   -46,   -46,   -10,   -46,   -46,    77,
     -46
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     8,     9,    10,
       0,     0,     0,     0,     0,     0,     4,     5,     7,     6,
      18,    19,     0,    16,     0,     0,     0,    11,    12,    13,
      14,    15,     3,     0,     0,     0,     0,     0,     0,    48,
      43,    44,    42,    45,    46,    47,     0,    39,     0,     0,
       0,     0,    40,     0,    20,     0,    36,    34,    35,    28,
      30,    31,    32,     0,    17,    33,    29,     0,    22,     0,
      24,    25,    26,    27,    21,    49,     0,    23,    41,     0,
      50
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -46,   -46,   -46,   -46,    -6,   -46,   -46,   -46,   -46,    16,
     -46,   -46,   -46,   -46,    45,   -45,   -46,   -46
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,    15,    16,    23,    64,    74,    17,    59,    60,
      61,    62,    18,    19,    46,    47,    48,    76
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       2,    52,    39,    25,    52,    24,    20,    21,    31,     3,
      26,     4,     5,     6,    20,    21,    33,    55,    39,    22,
      56,    57,    58,    28,     7,     8,     9,    39,    40,    41,
      42,    43,    44,    45,    78,    79,    10,    11,    29,    12,
      13,    14,    68,   -38,    40,    41,    42,    43,    44,    45,
      27,    69,    70,    40,    41,    42,    43,    44,    45,   -37,
      30,    32,    34,    35,    36,    71,    72,    73,    37,    38,
      50,    53,    51,    54,    63,    66,    67,    65,    75,    77,
      80,    49
};

static const yytype_int8 yycheck[] =
{
       0,    46,     1,    14,    49,     3,     3,     4,    14,     9,
      14,    11,    12,    13,     3,     4,    22,    18,     1,    16,
      21,    22,    23,    38,    24,    25,    26,     1,    27,    28,
      29,    30,    31,    32,    44,    45,    36,    37,     3,    39,
      40,    41,    10,    42,    27,    28,    29,    30,    31,    32,
       6,    19,    20,    27,    28,    29,    30,    31,    32,    42,
       3,    42,     8,     3,     3,    33,    34,    35,    16,     3,
       8,    15,    17,     3,     3,    59,    43,     5,     3,     5,
       3,    36
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    47,     0,     9,    11,    12,    13,    24,    25,    26,
      36,    37,    39,    40,    41,    48,    49,    53,    58,    59,
       3,     4,    16,    50,     3,    14,    14,     6,    38,     3,
       3,    50,    42,    50,     8,     3,     3,    16,     3,     1,
      27,    28,    29,    30,    31,    32,    60,    61,    62,    60,
       8,    17,    61,    15,     3,    18,    21,    22,    23,    54,
      55,    56,    57,     3,    51,     5,    55,    43,    10,    19,
      20,    33,    34,    35,    52,     3,    63,     5,    44,    45,
       3
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    46,    47,    47,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    49,    50,    50,
      51,    51,    52,    52,    52,    52,    52,    52,    53,    54,
      54,    55,    55,    56,    57,    57,    57,    58,    59,    60,
      60,    61,    62,    62,    62,    62,    62,    62,    62,    63,
      63
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     7,     1,     1,
       0,     2,     1,     2,     1,     1,     1,     1,     6,     2,
       1,     1,     1,     2,     1,     1,     1,     4,     4,     1,
       2,     6,     1,     1,     1,     1,     1,     1,     1,     1,
       3
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


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
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
      yychar = yylex ();
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
  case 3: /* commands: commands command ';'  */
#line 146 "commands.y"
                               { }
#line 1299 "commands.c"
    break;

  case 4: /* command: load_switch  */
#line 150 "commands.y"
                      { geos_prepare = false; simulate_load = false; }
#line 1305 "commands.c"
    break;

  case 6: /* command: select_switch  */
#line 152 "commands.y"
                        { }
#line 1311 "commands.c"
    break;

  case 8: /* command: TOKCLEAN  */
#line 154 "commands.y"
                   { Clean(); }
#line 1317 "commands.c"
    break;

  case 9: /* command: TOKCLEANI  */
#line 155 "commands.y"
                    { clean_intermediates(); }
#line 1323 "commands.c"
    break;

  case 10: /* command: TOKENDDGEO  */
#line 156 "commands.y"
                     { EndDgeo(); }
#line 1329 "commands.c"
    break;

  case 11: /* command: TOKSYSTEM TOKSTRING  */
#line 157 "commands.y"
                              { run_system_cmd((yyvsp[0].sval)); }
#line 1335 "commands.c"
    break;

  case 12: /* command: TOKSETHIST TOKGRID  */
#line 158 "commands.y"
                             { htype = HIST_GRID; }
#line 1341 "commands.c"
    break;

  case 13: /* command: TOKSETOPT WORD  */
#line 159 "commands.y"
                         { setenv("MW_OPTMETHOD", (yyvsp[0].sval), 1); }
#line 1347 "commands.c"
    break;

  case 14: /* command: TOKSETQNAME WORD  */
#line 160 "commands.y"
                           { setenv("MW_QNAME", (yyvsp[0].sval), 1); }
#line 1353 "commands.c"
    break;

  case 15: /* command: TOKSTORE file_identifier  */
#line 161 "commands.y"
                                   { StoreState((yyvsp[0].sval)); }
#line 1359 "commands.c"
    break;

  case 16: /* command: TOKLOAD file_identifier  */
#line 162 "commands.y"
                                  { LoadState((yyvsp[0].sval)); }
#line 1365 "commands.c"
    break;

  case 17: /* load_switch: TOKLOAD SIMPLEQUOTE file_identifier SIMPLEQUOTE TOKAS WORD load_parameters  */
#line 167 "commands.y"
        {
		printf("\n> Loading dataset %s from file %s", (yyvsp[-1].sval), (yyvsp[-4].sval));
		geos_prepare = false ? printf(", prepared.\n") : printf(", unprepared.\n");
		
		bool result = run_load_action((yyvsp[-4].sval), (yyvsp[-1].sval), geos_prepare, global_servers, simulate_load);
		
		free((yyvsp[-4].sval));
		free((yyvsp[-1].sval));

		if (!result)
			exit(1);
	}
#line 1382 "commands.c"
    break;

  case 22: /* load_param: TOKGPREPARE  */
#line 192 "commands.y"
                      { geos_prepare = true; }
#line 1388 "commands.c"
    break;

  case 23: /* load_param: TOKSERVERS NUMBER  */
#line 193 "commands.y"
                            { global_servers = (yyvsp[0].ival); }
#line 1394 "commands.c"
    break;

  case 24: /* load_param: TOKSIMULATE  */
#line 194 "commands.y"
                      { simulate_load = true; }
#line 1400 "commands.c"
    break;

  case 25: /* load_param: TOKPOINT  */
#line 195 "commands.y"
                      { ds_type = wkbPoint; }
#line 1406 "commands.c"
    break;

  case 26: /* load_param: TOKPOLYGON  */
#line 196 "commands.y"
                      { ds_type = wkbPolygon; }
#line 1412 "commands.c"
    break;

  case 27: /* load_param: TOKLINE  */
#line 197 "commands.y"
                      { ds_type = wkbLineString; }
#line 1418 "commands.c"
    break;

  case 28: /* index_switch: TOKINDEX WORD TOKAS WORD TOKWITH index_parameters  */
#line 203 "commands.y"
        {
		/*printf("> Creating index %s for dataset %s. Options: M=%d\n", $4, $2, M);
		bool result = run_index_action($2, $4, M, rtreetype);
		free($2);
		free($4);

		if (!result)
			exit(1);*/
	}
#line 1432 "commands.c"
    break;

  case 30: /* index_parameters: index_param  */
#line 215 "commands.y"
                      {}
#line 1438 "commands.c"
    break;

  case 33: /* m_switch: TOKM NUMBER  */
#line 224 "commands.y"
                    { 
	}
#line 1445 "commands.c"
    break;

  case 34: /* index_type: TOKLAZY  */
#line 228 "commands.y"
                  {  }
#line 1451 "commands.c"
    break;

  case 35: /* index_type: TOKR0  */
#line 229 "commands.y"
                  {  }
#line 1457 "commands.c"
    break;

  case 36: /* index_type: TOKSTAR  */
#line 230 "commands.y"
                  {  }
#line 1463 "commands.c"
    break;

  case 37: /* plan_switch: TOKPLAN TOKFROM WORD joins  */
#line 234 "commands.y"
                                   {
		// include the from table on the start
		multiway_input_chain *mfrom = g_new(multiway_input_chain, 1);
		mfrom->algorithm = join_none;
		mfrom->name = strdup((yyvsp[-1].sval));
		mfrom->rtree = NULL;
		mfrom->pcheck = CHECKR;
		joinplan = g_list_prepend(joinplan, mfrom);

		if (verbose)
			print_plan();

		/*bool result = */
		run_distr_join_action(joinplan, true, djoin_execute);
		
		// Cleaning
		for(GList *l = joinplan; l != NULL; l = g_list_next(l)) {
			multiway_input_chain *i = l->data;
			free(i->name);
			g_free(i);
		}
		g_list_free(joinplan);
		joinplan = NULL;
		lastjoinpredicate = NULL;
		free((yyvsp[-1].sval));

		//if (!result)
		//	exit(1);
	}
#line 1497 "commands.c"
    break;

  case 38: /* select_switch: TOKSELECT TOKFROM WORD joins  */
#line 266 "commands.y"
                                     {
		// include the from table on the start
		multiway_input_chain *mfrom = g_new(multiway_input_chain, 1);
		mfrom->algorithm = join_none;
		mfrom->name = strdup((yyvsp[-1].sval));
		mfrom->rtree = NULL;
		mfrom->pcheck = CHECKR;
		joinplan = g_list_prepend(joinplan, mfrom);

		if (verbose)
			print_plan();

		/*bool result = */
		run_distr_join_action(joinplan, false, djoin_execute);
		
		// Cleaning
		for(GList *l = joinplan; l != NULL; l = g_list_next(l)) {
			multiway_input_chain *i = l->data;
			free(i->name);
			g_free(i);
		}
		g_list_free(joinplan);
		joinplan = NULL;
		lastjoinpredicate = NULL;
		free((yyvsp[-1].sval));

		//if (!result)
		//	exit(1);
	}
#line 1531 "commands.c"
    break;

  case 41: /* join_switch: join_type TOKJOIN WORD '(' join_conditions ')'  */
#line 303 "commands.y"
                                                       { 
		//printf("\t%s %s x %s (", $1, $3, $5);

		multiway_input_chain *mfrom = g_new(multiway_input_chain, 1);

		if (strcmp((yyvsp[-5].sval), "inl") == 0)
			mfrom->algorithm = join_inl;
		else if (strcmp((yyvsp[-5].sval), "hj") == 0)
			mfrom->algorithm = join_hj;
		else if (strcmp((yyvsp[-5].sval), "rj") == 0)
			mfrom->algorithm = join_rj;
		else if (strcmp((yyvsp[-5].sval), "phj") == 0)
			mfrom->algorithm = join_p_hj;
		else if (strcmp((yyvsp[-5].sval), "prj") == 0)
			mfrom->algorithm = join_p_rj;
		else if (strcmp((yyvsp[-5].sval), "pinl") == 0)
			mfrom->algorithm = join_p_inl;

		mfrom->name = strdup((yyvsp[-3].sval));
		mfrom->rtree = NULL;

		if (joinplan == NULL) {
			mfrom->pcheck = CHECKR;
		} else {
			//printf("\nCondition: %s\n", jconditions[0]);
			multiway_input_chain *l = lastjoinpredicate;
			//printf("\n Comparing %s with %s\n", l->name, jconditions[0]);
			if (strcmp(l->name, jconditions[0]) == 0)
				mfrom->pcheck = CHECKS;
			else
				mfrom->pcheck = CHECKR;
		}

		lastjoinpredicate = mfrom;
		joinplan = g_list_append(joinplan, mfrom);

		for(int i=0; i < jcondcount; i++) {
			//printf("%s", jconditions[i]);
			//if (i+1 != jcondcount)
			//	printf(", ");
			free(jconditions[i]);
		}
		//printf(")\n");
		free((yyvsp[-5].sval));
		free((yyvsp[-3].sval));
	}
#line 1582 "commands.c"
    break;

  case 42: /* join_type: TOKINLJOIN  */
#line 351 "commands.y"
                     { jcondcount = 0; }
#line 1588 "commands.c"
    break;

  case 43: /* join_type: TOKHJJOIN  */
#line 352 "commands.y"
                    { jcondcount = 0; }
#line 1594 "commands.c"
    break;

  case 44: /* join_type: TOKRJJOIN  */
#line 353 "commands.y"
                    { jcondcount = 0; }
#line 1600 "commands.c"
    break;

  case 45: /* join_type: TOK_P_HJJOIN  */
#line 354 "commands.y"
                       { jcondcount = 0; }
#line 1606 "commands.c"
    break;

  case 46: /* join_type: TOK_P_RJJOIN  */
#line 355 "commands.y"
                       { jcondcount = 0; }
#line 1612 "commands.c"
    break;

  case 47: /* join_type: TOK_P_INLJOIN  */
#line 356 "commands.y"
                        { jcondcount = 0; }
#line 1618 "commands.c"
    break;

  case 48: /* join_type: error  */
#line 357 "commands.y"
                { fprintf(stderr, "Join algorithm not implemented\n"); yyclearin; yyerrok; }
#line 1624 "commands.c"
    break;

  case 49: /* join_conditions: WORD  */
#line 361 "commands.y"
               { jconditions[jcondcount++] = (yyvsp[0].sval); }
#line 1630 "commands.c"
    break;

  case 50: /* join_conditions: join_conditions ',' WORD  */
#line 362 "commands.y"
                                   { jconditions[jcondcount++] = (yyvsp[0].sval); }
#line 1636 "commands.c"
    break;


#line 1640 "commands.c"

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

#line 365 "commands.y"

