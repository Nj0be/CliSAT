# 0 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp"
# 1 "/home/benia/uni/Tesi_triennale/CliSAT/cmake-build-debug//"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp"
# 1 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.h" 1
# 24 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.h"
# 1 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/bbtypes.h" 1
# 25 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/bbtypes.h"
using U8 = unsigned char;
using U16 = unsigned short;
using U32 = unsigned long;
using BITBOARD = unsigned long long;
using BYTE = unsigned char;
using BOOL = int;

constexpr BITBOARD ONE = 0xFFFFFFFFFFFFFFFF;
constexpr BITBOARD EVEN = 0x5555555555555555;
constexpr U32 EVEN_32 = 0x55555555;
constexpr U8 EVEN_8 = 0x55;
constexpr BITBOARD ZERO = 0x0000000000000000;

constexpr int EMPTY_ELEM = -1;


constexpr int WORD_SIZE = 64;
constexpr int WORD_SIZE_MINUS_ONE = WORD_SIZE - 1;
constexpr int MASK_LIM = WORD_SIZE + 1;
# 25 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.h" 2


class Tables{

private:
 Tables(){};
 virtual ~Tables(){};

public:
 static int InitAllTables();
private:
 static void init_masks();
 static void init_popc8();
 static void init_popc();
 static void init_mlsb();
 static void init_lsb_l();


 static void init_cached_index();




public:

 static BITBOARD mask[64];
 static U8 mask8[8];
 static BITBOARD mask_low[65];
 static BITBOARD mask_high[66];


 static BITBOARD mask_mid[64 ][64 ];


 static BITBOARD mask0_1W;
 static BITBOARD mask0_2W;
 static BITBOARD mask0_3W;
 static BITBOARD mask0_4W;

 static int pc[65536];
 static int lsb[65536];
 static int lsba[4][65536];
 static int msba[4][65536];


 static int pc8[256];
 static int pc_sa[65536];
 static int msb[65536];
# 81 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.h"
 static const int T_32[37];
 static const int T_64[67];
 static const int indexDeBruijn64_ISOL[64];
 static const int indexDeBruijn64_SEP[64];
# 93 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.h"
};
# 2 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp" 2


int Tables::pc[65536];
int Tables::lsb[65536];
int Tables::msb[65536];
int Tables::pc8[256];
int Tables::pc_sa[65536];
int Tables::msba[4][65536];
int Tables::lsba[4][65536];

BITBOARD Tables::mask[64];
BITBOARD Tables::mask_low[65];
BITBOARD Tables::mask_high[66];

BITBOARD Tables::mask0_1W;
BITBOARD Tables::mask0_2W;
BITBOARD Tables::mask0_3W;
BITBOARD Tables::mask0_4W;
U8 Tables::mask8[8];
BITBOARD Tables::mask_mid[64 ][64 ];
# 35 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp"
struct Init{
 Init(){Tables::InitAllTables();}
} initTables;





const int Tables::T_64[67]={
 -1,0,1,39,2,15,40,23,
 3,12,16,59,41,19,24,54,
 4,-1,13,10,17,62,60,28,
 42,30,20,51,25,44,55,47,
 5,32,-1,38,14,22,11,58,
 18,53,63,9,61,27,29,50,
 43,46,31,37,21,57,52,8,
 26,49,45,36,56,7,48,35,
 6,34,33 };


const int Tables::T_32[37]={
 -1,0,1,26,2,23,27,-1,
 3,16,24,30,28,11,-1,13,
 4,7,17,-1,25,22,31,15,
 29,10,12,6,-1,21,14,9,
 5,20,8,19,18 };


const int Tables::indexDeBruijn64_ISOL[64] = {
 63, 0, 58, 1, 59, 47, 53, 2,
 60, 39, 48, 27, 54, 33, 42, 3,
 61, 51, 37, 40, 49, 18, 28, 20,
 55, 30, 34, 11, 43, 14, 22, 4,
 62, 57, 46, 52, 38, 26, 32, 41,
 50, 36, 17, 19, 29, 10, 13, 21,
 56, 45, 25, 31, 35, 16, 9, 12,
 44, 24, 15, 8, 23, 7, 6, 5 };


 const int Tables::indexDeBruijn64_SEP[64] = {
    0, 47, 1, 56, 48, 27, 2, 60,
   57, 49, 41, 37, 28, 16, 3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11, 4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30, 9, 24,
   13, 18, 8, 12, 7, 6, 5, 63 };




void Tables::init_masks(){

 BITBOARD uno = 1;

 for (int c=0;c<64;c++)
    mask[c]= uno<<c;




 U8 uno8 = 1;

 for (int c=0;c<8;c++)
    mask8[c]= uno8<<c;



 BITBOARD aux=0;
 for (int c=0;c<64;c++)
 {
  for ( int j=0; j<c ; j++)
  {
   mask_low[c] |= mask[j];

  }
 }

 mask_low[64]=ONE;



 for (int c=0;c<64;c++)
 {
  mask_high[c]= ~mask_low[c] ^ mask[c] ;
 }

 mask_high[64]=ZERO;
 mask_high[MASK_LIM]=ONE;





 for (int c=0;c<64;c++) {
  for ( int j=0; j<64 ; j++ ) {
   if(j<c) continue;

   if(j==c) mask_mid[c][j]=mask[c];
   else mask_mid[c][j]= mask_low[j] & mask_high[c] | mask[c] | mask[j];
  }
 }




 mask0_1W= ONE <<16;
 mask0_2W= (mask0_1W<<16) | (~mask0_1W);
 mask0_3W= (mask0_2W<<16) | (~mask0_1W);
 mask0_4W= (mask0_3W<<16) | (~mask0_1W);
}




void Tables::init_popc8(){
 int n;
 int c;
 int k;

 pc8[0]=0;

 for (c=1;c<256;c++) {
  n=0;
  for (k=0;k<=7;k++) {
    if (c & (1 << k))
       n++;
  }
  pc8[c]=n;
 }
}




void Tables::init_popc(){


 int n,c,k;
 pc[0]=0;

 for (c=1;c<65536;c++) {
  n=0;
  for (k=0;k<=15;k++) {
    if (c & (1 << k))
    n++;
  }
  pc[c]=n;
 }



 pc_sa[0]=0;

 for (c=1;c<65536;c++) {
  n=0;
  for(k=0;k<13;k+=4)
    n+=0xF & (c>>k);

  pc_sa[c]=n;
 }
}




void Tables::init_mlsb(){


 int c,k;

 msb[0] = EMPTY_ELEM;
 for ( c=1;c<65536;c++){
  for (k=15;k>=0;k--){
    if (c & (1 << k)) {
     msb[c] = k;
    break;
   }
  }
 }

 lsb[0]= EMPTY_ELEM;
 for ( c=1;c<65536;c++){
  for (k=0;k<16;k++) {
    if (c & (1 << k)) {
     lsb[c] = k;
     break;
   }
  }
 }


 for(k=0; k<4; k++)
  for ( c=1;c<65536;c++)
   lsba[k][c]=lsb[c]+k*16;

 for(k=0; k<4; k++)
  lsba[k][0]=EMPTY_ELEM;


 for(k=0; k<4; k++)
  for (c=1;c<65536;c++)
   msba[k][c]=msb[c]+k*16;

 for(k=0; k<4; k++)
   msba[k][0]=EMPTY_ELEM;
}

void Tables::init_lsb_l(){
# 286 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp"
}



void Tables::init_cached_index()
{
# 306 "/home/benia/uni/Tesi_triennale/CliSAT/BitGraph/src/bitscan/tables.cpp"
}




int Tables::InitAllTables(){
 init_mlsb();
    init_popc();
    init_popc8();
    init_masks();
 init_lsb_l();
 init_cached_index();

return 1;
}
