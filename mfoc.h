#define uchar unsigned char
#define uint unsigned int



int trailer_block(uint block);
bool checkKey(uint block, uchar *key, uchar type);
void checkDefaultKey();
uchar test(uchar *p, uint size, uchar *pback);
