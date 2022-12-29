

void Cache_FreeLow (const int new_low_hunk);
void Cache_FreeHigh (const int new_high_hunk);


void *Z_TagMalloc			(const int size, const int tag);
void  Z_CheckHeap			(void);

void *Hunk_HighAllocName	(const int size, const char *name);
int   Hunk_HighMark			(void);
void  Hunk_FreeToHighMark	(const int mark);