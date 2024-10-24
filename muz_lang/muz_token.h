#pragma once

#define MUZ_TOKEN_ID_MAX_LENGTH 128

typedef enum muzTokenIdT_
{
   MUZ_TOKEN_ID_UNKNOWN,    // 0
   MUZ_TOKEN_ID_FUNCTION,   // 1
   MUZ_TOKEN_ID_LBRACE,     // 2
   MUZ_TOKEN_ID_RBRACE,     // 3
   MUZ_TOKEN_ID_LPAREN,     // 4
   MUZ_TOKEN_ID_RPAREN,     // 5
   MUZ_TOKEN_ID_LBRACKET,   // 6
   MUZ_TOKEN_ID_RBRACKET,   // 7
   MUZ_TOKEN_ID_IDENTIFIER, // 8
   MUZ_TOKEN_ID_INTEGER,    // 9
   MUZ_TOKEN_ID_REAL,       // 10
   MUZ_TOKEN_ID_STRING,     // 11
   MUZ_TOKEN_ID_MUL,        // 12
   MUZ_TOKEN_ID_DIV,        // 13
   MUZ_TOKEN_ID_MOD,        // 14
   MUZ_TOKEN_ID_ADD,        // 15
   MUZ_TOKEN_ID_SUB,        // 16
   MUZ_TOKEN_ID_DOT,        // 17
   MUZ_TOKEN_ID_SEMICOLON,  // 18
   MUZ_TOKEN_ID_ASSIGN,     // 19
} muzTokenIdT;

static char* MuzTokens[] = {
   "<unknown>", // MUZ_TOKEN_ID_UNKNOWN
   "function",  // MUZ_TOKEN_ID_FUNCTION
   "{",         // MUZ_TOKEN_ID_LBRACE
   "}",         // MUZ_TOKEN_ID_RBRACE
   "(",         // MUZ_TOKEN_ID_LPAREN
   ")",         // MUZ_TOKEN_ID_RPAREN
   "[",         // MUZ_TOKEN_ID_LBRACKET
   "]",         // MUZ_TOKEN_ID_RBRACKET
   "<ident>",   // MUZ_TOKEN_ID_IDENTIFIER
   "<integer>", // MUZ_TOKEN_ID_INTEGER
   "<real>",    // MUZ_TOKEN_ID_REAL
   "<string>",  // MUZ_TOKEN_ID_STRING
   "*",         // MUZ_TOKEN_ID_MUL
   "/",         // MUZ_TOKEN_ID_DIV
   "%",         // MUZ_TOKEN_ID_MOD
   "+",         // MUZ_TOKEN_ID_ADD
   "-",         // MUZ_TOKEN_ID_SUB
   ".",         // MUZ_TOKEN_ID_DOT
   ";",         // MUZ_TOKEN_ID_SEMICOLON
   "=",         // MUZ_TOKEN_ID_ASSIGN
   0,
};

typedef struct muzTokenT
{
   muzListEntryT ListEntry;
   muzTokenIdT Id;
   muzLexerStampT Stamp;
   char Buffer[0];
} muzTokenT;
