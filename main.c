#include <stdio.h>
#include <stdlib.h>

#include "muz_lang/muz_lexer.h"
#include "muz_lang/muz_log.h"

int
main(int ArgC, char* ArgV[])
{
   if (ArgC < 2) {
      MuzLogE("Provide a file name in the command line!");
      return -1;
   }

   FILE* File = fopen(ArgV[1], "r");
   if (File == NULL) {
      MuzLogE("Failed to open file '%s'", ArgV[1]);
      return -1;
   }

   fseek(File, 0, SEEK_END);
   long FileSize = ftell(File);
   rewind(File);

   char* Buffer = (char*)malloc(sizeof(char) * (FileSize + 1));
   if (Buffer == NULL) {
      MuzLogE("Memory allocation failed!");
      fclose(File);
      return -1;
   }

   size_t BuffersRead = fread(Buffer, sizeof(char), FileSize, File);
   Buffer[BuffersRead] = 0;

   muzLexerT Lexer;
   muzLexer_Init(&Lexer);
   muzLexer_Tokenize(&Lexer, Buffer);
   muzLexer_Term(&Lexer);

   fclose(File);
   free(Buffer);

   return 0;
}
