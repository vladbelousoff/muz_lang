#pragma once

#include "muz_list.h"

typedef struct muzLexerStampT_ {
   unsigned long SourcePosition;
   unsigned long LinePosition;
   unsigned long Line;
} muzLexerStampT;

typedef struct muzLexerT_ {
   muzListHeadT Tokens;
   muzLexerStampT Stamp;
} muzLexerT;

void muzLexer_Init(muzLexerT *Self);
void muzLexer_Term(muzLexerT *Self);

void muzLexer_Tokenize(muzLexerT *Self, const char *Source);
