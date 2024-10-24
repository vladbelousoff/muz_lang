#pragma once

#include "muz_list.h"

typedef struct muzLexerT_
{
   muzListHeadT Tokens;
   unsigned long SourcePosition;
   unsigned long LinePosition;
   unsigned long Line;
   unsigned long TokenCurrentPosition;
} muzLexerT;

void muzLexer_Init(muzLexerT* Self);
void muzLexer_Term(muzLexerT* Self);

void muzLexer_Tokenize(muzLexerT* Self, const char* Source);
