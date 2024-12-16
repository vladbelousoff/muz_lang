#pragma once

#include "muz_list.h"

struct muz_lexer_stamp
{
  unsigned long source_position;
  unsigned long line_position;
  unsigned long line;
};

typedef struct muzLexerT_
{
  struct muz_list_entry tokens;
  struct muz_lexer_stamp lexer_stamp;
} muzLexerT;

void muz_lexer_init(muzLexerT* self);
void muz_lexer_term(muzLexerT* self);

void muz_lexer_tokenize(muzLexerT* self, const char* source);
