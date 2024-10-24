#pragma once

#define muzListRecord(Address, Type, Field)      ((Type*)((char*)(Address) - (char*)(&((Type*)0)->Field)))
#define muzListForEach(Position, Head)           for (Position = (Head)->Next; Position != Head; Position = Position->Next)
#define muzListForEachSafe(Position, Safe, Head) for (Position = (Head)->Next, Safe = Position->Next; Position != (Head); Position = Safe, Safe = Position->Next)

typedef struct muzListEntryT_
{
   struct muzListEntryT_* Prev;
   struct muzListEntryT_* Next;
} muzListEntryT;

typedef muzListEntryT muzListHeadT;

static void
muzList_Init(muzListHeadT* Self)
{
   Self->Prev = (muzListEntryT*)Self;
   Self->Next = (muzListEntryT*)Self;
}

static void
muzList_Term(muzListHeadT* Self)
{
   Self->Prev = (muzListEntryT*)0;
   Self->Next = (muzListEntryT*)0;
}

static int
muzList_IsEmpty(muzListHeadT* Head)
{
   return Head->Next == Head;
}

static void
muzList_Add_(muzListEntryT* New, muzListEntryT* Prev, muzListEntryT* Next)
{
   Next->Prev = New;
   New->Next = Next;
   New->Prev = Prev;
   Prev->Next = New;
}

static void
muzList_PushFront(muzListHeadT* Head, muzListEntryT* Entry)
{
   muzList_Add_(Entry, (muzListEntryT*)Head, Head->Next);
}

static void
muzList_PushBack(muzListHeadT* Head, muzListEntryT* Entry)
{
   muzList_Add_(Entry, Head->Prev, (muzListEntryT*)Head);
}

static muzListEntryT*
muzList_GetFront(muzListHeadT* Head)
{
   muzListEntryT* Entry = Head->Next;
   if (Entry != Head) {
      return Entry;
   }

   return 0;
}

static void
muzList_Remove_(muzListEntryT* Prev, muzListEntryT* Next)
{
   Next->Prev = Prev;
   Prev->Next = Next;
}

static void
muzList_Remove(muzListEntryT* entry)
{
   muzList_Remove_(entry->Prev, entry->Next);
}
