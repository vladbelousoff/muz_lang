#pragma once

#include "muz_list.h"

typedef struct muzLexerStampT_
{
  unsigned long SourcePosition;
  unsigned long LinePosition;
  unsigned long Line;
} muzLexerStampT;

typedef struct muzLexerT_
{
  struct muz_list_entry Tokens;
  muzLexerStampT Stamp;
} muzLexerT;

void muzLexer_Init(muzLexerT* self);
void muzLexer_Term(muzLexerT* self);

void muzLexer_Tokenize(muzLexerT* self, const char* Source);
