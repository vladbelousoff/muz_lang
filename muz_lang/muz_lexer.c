#include "muz_lexer.h"

#include "muz_list.h"
#include "muz_log.h"
#include "muz_token.h"

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdlib.h>

void
muzLexer_Init(muzLexerT* self)
{
  muz_list_init(&self->Tokens);
  self->Stamp.Line = 1;
  self->Stamp.SourcePosition = 0;
  self->Stamp.LinePosition = 0;
}

void
muzLexer_Term(muzLexerT* self)
{
  muz_list_term(&self->Tokens);
}

static int
muzIsEndOfLine(char Symbol)
{
  return Symbol == '\n';
}

static int
muzIsSpace(char Symbol)
{
  return Symbol == ' ';
}

static int
muzIsWhitespace(char Symbol)
{
  return muzIsSpace(Symbol) || muzIsEndOfLine(Symbol) || Symbol == '\t' || Symbol == '\r';
}

static int
muzIsAlpha(char Symbol)
{
  return Symbol >= 'A' && Symbol <= 'z';
}

static int
muzIsDigit(char Symbol)
{
  return Symbol >= '0' && Symbol <= '9';
}

static int
muzIsQuote(char Symbol)
{
  return Symbol == '\'';
}

static int
muzIsServiceSymbol(char Symbol)
{
  return strchr("~!@#$%^&*/%(){}-+=.;,>", Symbol) != NULL;
}

static int
muzLexer_EatSymbol(muzLexerT* self, const char* Source)
{
  const char Symbol = Source[self->Stamp.SourcePosition++];
  if (muzIsEndOfLine(Symbol)) {
    self->Stamp.LinePosition = 0;
    self->Stamp.Line++;
  } else {
    self->Stamp.LinePosition++;
  }

  return Symbol;
}

static char
muzLexer_GetSymbol(const muzLexerT* self, const char* Source)
{
  return Source[self->Stamp.SourcePosition];
}

static int
muzLexer_ProcessDigit(muzLexerT* self, const char* Source)
{
  unsigned long BufferPosition = 0;
  unsigned long DotCount = 0;

  char Buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  for (;; muzLexer_EatSymbol(self, Source)) {
    const char Symbol = muzLexer_GetSymbol(self, Source);

    if (muzIsWhitespace(Symbol)) {
      BufferPosition++;
      break;
    }

    if (muzIsEndOfLine(Symbol)) {
      BufferPosition++;
      break;
    }

    if (Symbol == '_') {
      Buffer[BufferPosition++] = Symbol;
      continue;
    }

    if (Symbol == '-') {
      if (BufferPosition != 0) {
        return -1;
      }
      Buffer[BufferPosition++] = Symbol;
      continue;
    }

    if (Symbol == '.') {
      if (DotCount == 1) {
        return -1;
      }
      Buffer[BufferPosition++] = Symbol;
      DotCount++;
      continue;
    }

    if (muzIsDigit(Symbol)) {
      Buffer[BufferPosition++] = Symbol;
      continue;
    }

    if (Symbol == 0) {
      BufferPosition++;
      break;
    }

    break;
  }

  struct muz_token* Token = (struct muz_token*)malloc(sizeof(struct muz_token) + BufferPosition);
  if (Token) {
    memcpy(Token->buffer, Buffer, BufferPosition);
    Token->lexer_stamp = self->Stamp;
    if (DotCount == 0) {
      Token->id = MUZ_TOKEN_ID_INTEGER;
    } else {
      Token->id = MUZ_TOKEN_ID_REAL;
    }

    muz_list_push_back(&self->Tokens, &Token->list_entry);
    MuzLogD("Adding token (type: '%s', value: '%s')", muz_tokens[Token->id], Buffer);

    return 0;
  }

  return -1;
}

static int
muzLexer_ProcessAlpha(muzLexerT* self, const char* Source)
{
  unsigned long BufferPosition = 0;

  char Buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  for (;; muzLexer_EatSymbol(self, Source)) {
    const char Symbol = muzLexer_GetSymbol(self, Source);

    if (Symbol == '_' || muzIsAlpha(Symbol)) {
      Buffer[BufferPosition++] = Symbol;
      continue;
    }

    if (muzIsWhitespace(Symbol)) {
      BufferPosition++;
      break;
    }

    if (Symbol == 0) {
      BufferPosition++;
      break;
    }

    BufferPosition++;
    break;
  }

  struct muz_token* Token = (struct muz_token*)malloc(sizeof(struct muz_token) + BufferPosition);
  if (Token) {
    memcpy(Token->buffer, Buffer, BufferPosition);
    Token->lexer_stamp = self->Stamp;
    Token->id = MUZ_TOKEN_ID_IDENTIFIER;

    muz_list_push_back(&self->Tokens, &Token->list_entry);
    MuzLogD("Adding token (type: '%s', value: '%s')", muz_tokens[Token->id], Buffer);

    return 0;
  }

  return -1;
}

static int
muzLexer_ProcessString(muzLexerT* self, const char* Source)
{
  unsigned long BufferPosition = 0;

  char Buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  muzLexer_EatSymbol(self, Source);
  for (;;) {
    const char Symbol = (char)muzLexer_EatSymbol(self, Source);

    if (muzIsQuote(Symbol)) {
      BufferPosition++;
      break;
    }

    if (Symbol == 0) {
      return -1;
    }

    Buffer[BufferPosition++] = Symbol;
  }

  struct muz_token* Token = (struct muz_token*)malloc(sizeof(struct muz_token) + BufferPosition);
  if (Token) {
    memcpy(Token->buffer, Buffer, BufferPosition);
    Token->lexer_stamp = self->Stamp;
    Token->id = MUZ_TOKEN_ID_STRING;

    muz_list_push_back(&self->Tokens, &Token->list_entry);
    MuzLogD("Adding token (type: '%s', value: '%s')", muz_tokens[Token->id], Buffer);

    return 0;
  }

  return -1;
}

static int
muzLexer_ProcessOperator(muzLexerT* self, const char* Source)
{
  enum muz_token_id TokenId;

  const char Symbol = (char)muzLexer_EatSymbol(self, Source);
  switch (Symbol) {
    case '*':
      TokenId = MUZ_TOKEN_ID_MUL;
      break;
    case '/':
      TokenId = MUZ_TOKEN_ID_DIV;
      break;
    case '%':
      TokenId = MUZ_TOKEN_ID_MOD;
      break;
    case '+':
      TokenId = MUZ_TOKEN_ID_ADD;
      break;
    case '-':
      TokenId = MUZ_TOKEN_ID_SUB;
      break;
    case '(':
      TokenId = MUZ_TOKEN_ID_LPAREN;
      break;
    case ')':
      TokenId = MUZ_TOKEN_ID_RPAREN;
      break;
    case '{':
      TokenId = MUZ_TOKEN_ID_LBRACE;
      break;
    case '}':
      TokenId = MUZ_TOKEN_ID_RBRACE;
      break;
    case '.':
      TokenId = MUZ_TOKEN_ID_DOT;
      break;
    case ';':
      TokenId = MUZ_TOKEN_ID_SEMICOLON;
      break;
    case '=':
      TokenId = MUZ_TOKEN_ID_ASSIGN;
      break;
    case ',':
      TokenId = MUZ_TOKEN_ID_COMMA;
      break;
    default:
      return -1;
  }

  struct muz_token* Token = malloc(sizeof(struct muz_token));
  if (Token) {
    Token->id = TokenId;
    Token->lexer_stamp = self->Stamp;

    muz_list_push_back(&self->Tokens, &Token->list_entry);
    MuzLogD("Adding token (type: '%s')", muz_tokens[Token->id]);

    return 0;
  }

  return -1;
}

typedef enum muzSymbolT_
{
  MUZ_SYMBOL_UNKNOWN,
  MUZ_SYMBOL_SPACE,
  MUZ_SYMBOL_NEWLINE,
  MUZ_SYMBOL_DIGIT,
  MUZ_SYMBOL_ALPHA,
  MUZ_SYMBOL_QUOTE,
  MUZ_SYMBOL_SERVICE,
} muzSymbolT;

static muzSymbolT
muzGetSymbolType(char Symbol)
{
  if (muzIsEndOfLine(Symbol)) {
    return MUZ_SYMBOL_NEWLINE;
  }

  if (muzIsWhitespace(Symbol)) {
    return MUZ_SYMBOL_SPACE;
  }

  if (muzIsAlpha(Symbol)) {
    return MUZ_SYMBOL_ALPHA;
  }

  if (muzIsDigit(Symbol)) {
    return MUZ_SYMBOL_DIGIT;
  }

  if (muzIsServiceSymbol(Symbol)) {
    return MUZ_SYMBOL_SERVICE;
  }

  if (muzIsQuote(Symbol)) {
    return MUZ_SYMBOL_QUOTE;
  }

  return MUZ_SYMBOL_UNKNOWN;
}

typedef int (*muzSymbolProcessorT)(muzLexerT*, const char*);

static int
muzLexer_ProcessUnknownSymbol(const muzLexerT* self, const char* Source)
{
  MuzLogE("Unknown symbol '%c', line: %lu, column: %lu", muzLexer_GetSymbol(self, Source), self->Stamp.Line, self->Stamp.LinePosition);
  return -1;
}

static muzSymbolProcessorT SymbolProcessors[] = {
  muzLexer_ProcessUnknownSymbol, // MUZ_SYMBOL_UNKNOWN
  muzLexer_EatSymbol,            // MUZ_SYMBOL_SPACE
  muzLexer_EatSymbol,            // MUZ_SYMBOL_NEWLINE
  muzLexer_ProcessDigit,         // MUZ_SYMBOL_DIGIT
  muzLexer_ProcessAlpha,         // MUZ_SYMBOL_ALPHA
  muzLexer_ProcessString,        // MUZ_SYMBOL_QUOTE
  muzLexer_ProcessOperator,      // MUZ_SYMBOL_SERVICE
};

static int
muzLexer_ProcessSymbol(muzLexerT* self, const char* Source)
{
  char Symbol = muzLexer_GetSymbol(self, Source);
  muzSymbolT SymbolType = muzGetSymbolType(Symbol);
  muzSymbolProcessorT SymbolProcessor = SymbolProcessors[SymbolType];
  return SymbolProcessor(self, Source);
}

void
muzLexer_Tokenize(muzLexerT* self, const char* Source)
{
  while (muzLexer_GetSymbol(self, Source) != 0) {
    if (muzLexer_ProcessSymbol(self, Source) == -1) {
      exit(-1);
    }
  }
}
