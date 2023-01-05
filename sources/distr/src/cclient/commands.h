/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_COMMANDS_H_INCLUDED
# define YY_YY_COMMANDS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    WORD = 258,                    /* WORD  */
    FILENAME = 259,                /* FILENAME  */
    NUMBER = 260,                  /* NUMBER  */
    TOKSTRING = 261,               /* TOKSTRING  */
    TOKCOMMENT = 262,              /* TOKCOMMENT  */
    TOKAS = 263,                   /* TOKAS  */
    TOKLOAD = 264,                 /* TOKLOAD  */
    TOKGPREPARE = 265,             /* TOKGPREPARE  */
    TOKINDEX = 266,                /* TOKINDEX  */
    TOKSELECT = 267,               /* TOKSELECT  */
    TOKPLAN = 268,                 /* TOKPLAN  */
    TOKFROM = 269,                 /* TOKFROM  */
    TOKJOIN = 270,                 /* TOKJOIN  */
    SIMPLEQUOTE = 271,             /* SIMPLEQUOTE  */
    TOKWITH = 272,                 /* TOKWITH  */
    TOKM = 273,                    /* TOKM  */
    TOKSERVERS = 274,              /* TOKSERVERS  */
    TOKSIMULATE = 275,             /* TOKSIMULATE  */
    TOKSTAR = 276,                 /* TOKSTAR  */
    TOKLAZY = 277,                 /* TOKLAZY  */
    TOKR0 = 278,                   /* TOKR0  */
    TOKCLEAN = 279,                /* TOKCLEAN  */
    TOKCLEANI = 280,               /* TOKCLEANI  */
    TOKENDDGEO = 281,              /* TOKENDDGEO  */
    TOKHJJOIN = 282,               /* TOKHJJOIN  */
    TOKRJJOIN = 283,               /* TOKRJJOIN  */
    TOKINLJOIN = 284,              /* TOKINLJOIN  */
    TOK_P_HJJOIN = 285,            /* TOK_P_HJJOIN  */
    TOK_P_RJJOIN = 286,            /* TOK_P_RJJOIN  */
    TOK_P_INLJOIN = 287,           /* TOK_P_INLJOIN  */
    TOKPOINT = 288,                /* TOKPOINT  */
    TOKPOLYGON = 289,              /* TOKPOLYGON  */
    TOKLINE = 290,                 /* TOKLINE  */
    TOKSYSTEM = 291,               /* TOKSYSTEM  */
    TOKSETHIST = 292,              /* TOKSETHIST  */
    TOKGRID = 293,                 /* TOKGRID  */
    TOKSETOPT = 294,               /* TOKSETOPT  */
    TOKSETQNAME = 295,             /* TOKSETQNAME  */
    TOKSTORE = 296                 /* TOKSTORE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 121 "commands.y"
 char* sval; int ival; 

#line 108 "commands.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_COMMANDS_H_INCLUDED  */
