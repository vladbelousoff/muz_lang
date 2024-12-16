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
muz_lexer_init(muzLexerT* self)
{
  muz_list_init(&self->tokens);
  self->lexer_stamp.line = 1;
  self->lexer_stamp.source_position = 0;
  self->lexer_stamp.line_position = 0;
}

void
muz_lexer_term(muzLexerT* self)
{
  muz_list_term(&self->tokens);
}

static int
muz_is_end_of_line(char symbol)
{
  return symbol == '\n';
}

static int
muz_is_space(char symbol)
{
  return symbol == ' ';
}

static int
muz_is_whitespace(char symbol)
{
  return muz_is_space(symbol) || muz_is_end_of_line(symbol) || symbol == '\t' || symbol == '\r';
}

static int
muz_is_alpha(char symbol)
{
  return symbol >= 'A' && symbol <= 'z';
}

static int
muz_is_digit(char symbol)
{
  return symbol >= '0' && symbol <= '9';
}

static int
muz_is_quote(char symbol)
{
  return symbol == '\'';
}

static int
muz_is_service_symbol(char symbol)
{
  return strchr("~!@#$%^&*/%(){}-+=.;,>", symbol) != NULL;
}

static int
muz_lexer_eat_symbol(muzLexerT* self, const char* source)
{
  const char symbol = source[self->lexer_stamp.source_position++];
  if (muz_is_end_of_line(symbol)) {
    self->lexer_stamp.line_position = 0;
    self->lexer_stamp.line++;
  } else {
    self->lexer_stamp.line_position++;
  }

  return symbol;
}

static char
muz_lexer_get_symbol(const muzLexerT* self, const char* source)
{
  return source[self->lexer_stamp.source_position];
}

static int
muz_lexer_process_digit(muzLexerT* self, const char* source)
{
  unsigned long buffer_position = 0;
  unsigned long dot_count = 0;

  char buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  for (;; muz_lexer_eat_symbol(self, source)) {
    const char symbol = muz_lexer_get_symbol(self, source);

    if (muz_is_whitespace(symbol)) {
      buffer_position++;
      break;
    }

    if (muz_is_end_of_line(symbol)) {
      buffer_position++;
      break;
    }

    if (symbol == '_') {
      buffer[buffer_position++] = symbol;
      continue;
    }

    if (symbol == '-') {
      if (buffer_position != 0) {
        return -1;
      }
      buffer[buffer_position++] = symbol;
      continue;
    }

    if (symbol == '.') {
      if (dot_count == 1) {
        return -1;
      }
      buffer[buffer_position++] = symbol;
      dot_count++;
      continue;
    }

    if (muz_is_digit(symbol)) {
      buffer[buffer_position++] = symbol;
      continue;
    }

    if (symbol == 0) {
      buffer_position++;
      break;
    }

    break;
  }

  struct muz_token* token = (struct muz_token*)malloc(sizeof(struct muz_token) + buffer_position);
  if (token) {
    memcpy(token->buffer, buffer, buffer_position);
    token->lexer_stamp = self->lexer_stamp;
    if (dot_count == 0) {
      token->id = MUZ_TOKEN_ID_INTEGER;
    } else {
      token->id = MUZ_TOKEN_ID_REAL;
    }

    muz_list_push_back(&self->tokens, &token->list_entry);
    muz_log_debug("Adding token (type: '%s', value: '%s')", muz_tokens[token->id], buffer);

    return 0;
  }

  return -1;
}

static int
muz_lexer_process_alpha(muzLexerT* self, const char* source)
{
  unsigned long buffer_position = 0;

  char buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  for (;; muz_lexer_eat_symbol(self, source)) {
    const char symbol = muz_lexer_get_symbol(self, source);

    if (symbol == '_' || muz_is_alpha(symbol)) {
      buffer[buffer_position++] = symbol;
      continue;
    }

    if (muz_is_whitespace(symbol)) {
      buffer_position++;
      break;
    }

    if (symbol == 0) {
      buffer_position++;
      break;
    }

    buffer_position++;
    break;
  }

  struct muz_token* token = (struct muz_token*)malloc(sizeof(struct muz_token) + buffer_position);
  if (token) {
    memcpy(token->buffer, buffer, buffer_position);
    token->lexer_stamp = self->lexer_stamp;
    token->id = MUZ_TOKEN_ID_IDENTIFIER;

    muz_list_push_back(&self->tokens, &token->list_entry);
    muz_log_debug("Adding token (type: '%s', value: '%s')", muz_tokens[token->id], buffer);

    return 0;
  }

  return -1;
}

static int
muz_lexer_process_string(muzLexerT* self, const char* source)
{
  unsigned long buffer_position = 0;

  char buffer[MUZ_TOKEN_ID_MAX_LENGTH] = { 0 };

  muz_lexer_eat_symbol(self, source);
  for (;;) {
    const char symbol = (char)muz_lexer_eat_symbol(self, source);

    if (muz_is_quote(symbol)) {
      buffer_position++;
      break;
    }

    if (symbol == 0) {
      return -1;
    }

    buffer[buffer_position++] = symbol;
  }

  struct muz_token* token = (struct muz_token*)malloc(sizeof(struct muz_token) + buffer_position);
  if (token) {
    memcpy(token->buffer, buffer, buffer_position);
    token->lexer_stamp = self->lexer_stamp;
    token->id = MUZ_TOKEN_ID_STRING;

    muz_list_push_back(&self->tokens, &token->list_entry);
    muz_log_debug("Adding token (type: '%s', value: '%s')", muz_tokens[token->id], buffer);

    return 0;
  }

  return -1;
}

static int
muz_lexer_process_operator(muzLexerT* self, const char* source)
{
  enum muz_token_id token_id;

  const char symbol = (char)muz_lexer_eat_symbol(self, source);
  switch (symbol) {
    case '*':
      token_id = MUZ_TOKEN_ID_MUL;
      break;
    case '/':
      token_id = MUZ_TOKEN_ID_DIV;
      break;
    case '%':
      token_id = MUZ_TOKEN_ID_MOD;
      break;
    case '+':
      token_id = MUZ_TOKEN_ID_ADD;
      break;
    case '-':
      token_id = MUZ_TOKEN_ID_SUB;
      break;
    case '(':
      token_id = MUZ_TOKEN_ID_LPAREN;
      break;
    case ')':
      token_id = MUZ_TOKEN_ID_RPAREN;
      break;
    case '{':
      token_id = MUZ_TOKEN_ID_LBRACE;
      break;
    case '}':
      token_id = MUZ_TOKEN_ID_RBRACE;
      break;
    case '.':
      token_id = MUZ_TOKEN_ID_DOT;
      break;
    case ';':
      token_id = MUZ_TOKEN_ID_SEMICOLON;
      break;
    case '=':
      token_id = MUZ_TOKEN_ID_ASSIGN;
      break;
    case ',':
      token_id = MUZ_TOKEN_ID_COMMA;
      break;
    default:
      return -1;
  }

  struct muz_token* token = malloc(sizeof(struct muz_token));
  if (token) {
    token->id = token_id;
    token->lexer_stamp = self->lexer_stamp;

    muz_list_push_back(&self->tokens, &token->list_entry);
    muz_log_debug("Adding token (type: '%s')", muz_tokens[token->id]);

    return 0;
  }

  return -1;
}

enum muz_symbol
{
  MUZ_SYMBOL_UNKNOWN,
  MUZ_SYMBOL_SPACE,
  MUZ_SYMBOL_NEWLINE,
  MUZ_SYMBOL_DIGIT,
  MUZ_SYMBOL_ALPHA,
  MUZ_SYMBOL_QUOTE,
  MUZ_SYMBOL_SERVICE,
};

static enum muz_symbol
muz_get_symbol_type(char symbol)
{
  if (muz_is_end_of_line(symbol)) {
    return MUZ_SYMBOL_NEWLINE;
  }

  if (muz_is_whitespace(symbol)) {
    return MUZ_SYMBOL_SPACE;
  }

  if (muz_is_alpha(symbol)) {
    return MUZ_SYMBOL_ALPHA;
  }

  if (muz_is_digit(symbol)) {
    return MUZ_SYMBOL_DIGIT;
  }

  if (muz_is_service_symbol(symbol)) {
    return MUZ_SYMBOL_SERVICE;
  }

  if (muz_is_quote(symbol)) {
    return MUZ_SYMBOL_QUOTE;
  }

  return MUZ_SYMBOL_UNKNOWN;
}

typedef int (*muz_symbol_processor_func)(muzLexerT*, const char*);

static int
muz_lexer_process_unknown_symbol(const muzLexerT* self, const char* source)
{
  muz_log_error("Unknown symbol '%c', line: %lu, column: %lu", muz_lexer_get_symbol(self, source), self->lexer_stamp.line, self->lexer_stamp.line_position);
  return -1;
}

static muz_symbol_processor_func symbol_processors[] = {
  muz_lexer_process_unknown_symbol, // MUZ_SYMBOL_UNKNOWN
  muz_lexer_eat_symbol,             // MUZ_SYMBOL_SPACE
  muz_lexer_eat_symbol,             // MUZ_SYMBOL_NEWLINE
  muz_lexer_process_digit,          // MUZ_SYMBOL_DIGIT
  muz_lexer_process_alpha,          // MUZ_SYMBOL_ALPHA
  muz_lexer_process_string,         // MUZ_SYMBOL_QUOTE
  muz_lexer_process_operator,       // MUZ_SYMBOL_SERVICE
};

static int
muz_lexer_process_symbol(muzLexerT* self, const char* source)
{
  const char symbol = muz_lexer_get_symbol(self, source);
  const enum muz_symbol symbol_type = muz_get_symbol_type(symbol);
  const muz_symbol_processor_func symbol_processor = symbol_processors[symbol_type];
  return symbol_processor(self, source);
}

void
muz_lexer_tokenize(muzLexerT* self, const char* source)
{
  while (muz_lexer_get_symbol(self, source) != 0) {
    if (muz_lexer_process_symbol(self, source) == -1) {
      exit(-1);
    }
  }
}
