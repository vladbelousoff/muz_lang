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
muzLexer_Init(muzLexerT* Self)
{
   muzList_Init(&Self->Tokens);
   Self->Stamp.Line = 1;
   Self->Stamp.SourcePosition = 0;
   Self->Stamp.LinePosition = 0;
}

void
muzLexer_Term(muzLexerT* Self)
{
   muzList_Term(&Self->Tokens);
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
muzLexer_EatSymbol(muzLexerT* Self, const char* Source)
{
   char Symbol = Source[Self->Stamp.SourcePosition++];
   if (muzIsEndOfLine(Symbol)) {
      Self->Stamp.LinePosition = 0;
      Self->Stamp.Line++;
   } else {
      Self->Stamp.LinePosition++;
   }

   return Symbol;
}

static char
muzLexer_GetSymbol(muzLexerT* Self, const char* Source)
{
   return Source[Self->Stamp.SourcePosition];
}

static int
muzLexer_ProcessDigit(muzLexerT* Self, const char* Source)
{
   unsigned long BufferPosition = 0;
   unsigned long DotCount = 0;

   char Buffer[MUZ_TOKEN_ID_MAX_LENGTH];
   memset(Buffer, 0, MUZ_TOKEN_ID_MAX_LENGTH);

   for (;; muzLexer_EatSymbol(Self, Source)) {
      const char Symbol = muzLexer_GetSymbol(Self, Source);

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

   muzTokenT* Token = (muzTokenT*)malloc(sizeof(muzTokenT) + BufferPosition);
   if (Token) {
      memcpy(Token->Buffer, Buffer, BufferPosition);
      Token->Stamp = Self->Stamp;
      if (DotCount == 0) {
         Token->Id = MUZ_TOKEN_ID_INTEGER;
      } else {
         Token->Id = MUZ_TOKEN_ID_REAL;
      }

      muzList_PushBack(&Self->Tokens, &Token->ListEntry);
      MuzLogD("Adding token (type: '%s', value: '%s')", MuzTokens[Token->Id], Buffer);

      return 0;
   }

   return -1;
}

static int
muzLexer_ProcessAlpha(muzLexerT* Self, const char* Source)
{
   unsigned long BufferPosition = 0;

   char Buffer[MUZ_TOKEN_ID_MAX_LENGTH];
   memset(Buffer, 0, MUZ_TOKEN_ID_MAX_LENGTH);

   for (;; muzLexer_EatSymbol(Self, Source)) {
      char Symbol = muzLexer_GetSymbol(Self, Source);

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

   muzTokenT* Token = (muzTokenT*)malloc(sizeof(muzTokenT) + BufferPosition);
   if (Token) {
      memcpy(Token->Buffer, Buffer, BufferPosition);
      Token->Stamp = Self->Stamp;
      Token->Id = MUZ_TOKEN_ID_IDENTIFIER;

      muzList_PushBack(&Self->Tokens, &Token->ListEntry);
      MuzLogD("Adding token (type: '%s', value: '%s')", MuzTokens[Token->Id], Buffer);

      return 0;
   }

   return -1;
}

static int
muzLexer_ProcessString(muzLexerT* Self, const char* Source)
{
   unsigned long BufferPosition = 0;

   char Buffer[MUZ_TOKEN_ID_MAX_LENGTH];
   memset(Buffer, 0, MUZ_TOKEN_ID_MAX_LENGTH);

   muzLexer_EatSymbol(Self, Source);
   for (;;) {
      char Symbol = (char)muzLexer_EatSymbol(Self, Source);

      if (muzIsQuote(Symbol)) {
         BufferPosition++;
         break;
      }

      if (Symbol == 0) {
         return -1;
      }

      Buffer[BufferPosition++] = Symbol;
   }

   muzTokenT* Token = (muzTokenT*)malloc(sizeof(muzTokenT) + BufferPosition);
   if (Token) {
      memcpy(Token->Buffer, Buffer, BufferPosition);
      Token->Stamp = Self->Stamp;
      Token->Id = MUZ_TOKEN_ID_STRING;

      muzList_PushBack(&Self->Tokens, &Token->ListEntry);
      MuzLogD("Adding token (type: '%s', value: '%s')", MuzTokens[Token->Id], Buffer);

      return 0;
   }

   return -1;
}

static int
muzLexer_ProcessOperator(muzLexerT* Self, const char* Source)
{
   muzTokenIdT TokenId;

   char Symbol = (char)muzLexer_EatSymbol(Self, Source);
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

   muzTokenT* Token = (muzTokenT*)malloc(sizeof(muzTokenT));
   if (Token) {
      Token->Id = TokenId;
      Token->Stamp = Self->Stamp;

      muzList_PushBack(&Self->Tokens, &Token->ListEntry);
      MuzLogD("Adding token (type: '%s')", MuzTokens[Token->Id]);

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
muzLexer_ProcessUnknownSymbol(muzLexerT* Self, const char* Source)
{
   MuzLogE("Unknown symbol '%c', line: %lu, column: %lu", muzLexer_GetSymbol(Self, Source), Self->Stamp.Line, Self->Stamp.LinePosition);
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
muzLexer_ProcessSymbol(muzLexerT* Self, const char* Source)
{
   char Symbol = muzLexer_GetSymbol(Self, Source);
   muzSymbolT SymbolType = muzGetSymbolType(Symbol);
   muzSymbolProcessorT SymbolProcessor = SymbolProcessors[SymbolType];
   return SymbolProcessor(Self, Source);
}

void
muzLexer_Tokenize(muzLexerT* Self, const char* Source)
{
   while (muzLexer_GetSymbol(Self, Source) != 0) {
      if (muzLexer_ProcessSymbol(Self, Source) == -1) {
         exit(-1);
      }
   }
}
