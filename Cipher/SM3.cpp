#include "SM3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl\evp.h>
#include <openssl\engine.h>
#include <assert.h>






typedef unsigned char BYTE;
typedef BYTE *PBYTE;
typedef unsigned int UINT;
//typedef unsigned int DWORD;

static const DWORD T_j[64] = 
{
	/*0-15*/
	0x79cc4519,0x79cc4519,0x79cc4519,0x79cc4519,
	0x79cc4519,0x79cc4519,0x79cc4519,0x79cc4519,
	0x79cc4519,0x79cc4519,0x79cc4519,0x79cc4519,
	0x79cc4519,0x79cc4519,0x79cc4519,0x79cc4519,
	/*16-63*/
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a,
	0x7a879d8a,0x7a879d8a,0x7a879d8a,0x7a879d8a
};
static const DWORD IV[8] = 
{
	0x7380166f, 
	0x4914b2b9, 
	0x172442d7, 
	0xda8a0600, 
	0xa96f30bc, 
	0x163138aa, 
	0xe38dee4d, 
	0xb0fb0e4e
};

DWORD SM3_Rotate_Left(DWORD a, DWORD k)
{
	k = k % 32;
	return ((a << k) & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) >> (32 - k));
}

/*布尔函数*/
DWORD SM3_FF_j(DWORD X, DWORD Y, DWORD Z, DWORD j)
{
	DWORD ret=0;
	//if (0 <= j && j < 16)
	if( j < 16 )
	{
		ret = X ^ Y ^ Z;
	}
	else if (16 <= j && j < 64)
	{
		ret = (X & Y) | (X & Z) | (Y & Z);
	}
	return ret;
}

DWORD SM3_GG_j(DWORD X, DWORD Y, DWORD Z, DWORD j)
{
	DWORD ret=0;
	//if (0 <= j && j < 16)
	if( j < 16)
	{
		ret = X ^ Y ^ Z;
	}
	else if(16 <= j && j < 64)
	{
		ret = (X & Y) | ((~ X) & Z);
	}
	return ret;
}

/*置换函数*/
DWORD SM3_P_0(DWORD X)
{
	return X ^ (SM3_Rotate_Left(X, 9)) ^ (SM3_Rotate_Left(X, 17));
}
DWORD SM3_P_1(DWORD X)
{
	return X ^ (SM3_Rotate_Left(X, 15)) ^ (SM3_Rotate_Left(X, 23));
}
extern void PrintHexStr(const void *hex,int size);
int SM3_CF(DWORD *dV_i, BYTE *B_i, DWORD *dV_i_1)
{
	DWORD W[68];
	DWORD W_1[64];
	DWORD j;
	/*寄存器*/
	DWORD A;
	DWORD B;
	DWORD C;
	DWORD D;
	DWORD E;
	DWORD F;
	DWORD G;
	DWORD H;

	/*中间变量*/
	DWORD SS1;
	DWORD SS2;
	DWORD TT1;
	DWORD TT2;

	if(dV_i == NULL || dV_i_1 == NULL || B_i == NULL)
		return -1;

	memset(W, 0, sizeof(W));
	memset(W_1, 0, sizeof(W_1));

	/*先将B_i分成16个双字DWORD，
	/每个B_i有512bit, 64Byte, 分成4B一组，16个双字。*/
	for (j = 0;j < 16; j++)
	{
		W[j] = B_i[j*4 + 0] << 24
			| B_i[j*4 + 1] << 16
			| B_i[j*4 + 2] << 8
			| B_i[j*4 + 3];
	}
	for (j = 16;j < 68; j++)
	{
		W[j] = SM3_P_1(W[j-16] ^ W[j-9] 
		^ (SM3_Rotate_Left(W[j-3], 15))) 
			^ (SM3_Rotate_Left(W[j-13], 7)) 
			^ W[j-6];
	}
	for (j = 0;j < 64; j++)
	{
		W_1[j] = W[j] ^ W[j+4];
	}

	A = dV_i[0];
	B = dV_i[1];
	C = dV_i[2];
	D = dV_i[3];
	E =  dV_i[4];
	F =  dV_i[5];
	G = dV_i[6];
	H = dV_i[7];

	for (j = 0;j < 64;j++)
	{
		SS1 =SM3_Rotate_Left(((SM3_Rotate_Left(A, 12)) + E + (SM3_Rotate_Left(T_j[j], j))) & 0xFFFFFFFF, 7);
		SS2 = SS1 ^ (SM3_Rotate_Left(A, 12));
		TT1 = (SM3_FF_j(A, B, C, j) + D + SS2 + W_1[j]) & 0xFFFFFFFF;
		TT2 = (SM3_GG_j(E, F, G, j) + H + SS1 + W[j]) & 0xFFFFFFFF;
		D = C;
		C = SM3_Rotate_Left(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = SM3_Rotate_Left(F, 19);
		F = E;
		E = SM3_P_0(TT2);
	}
	dV_i_1[0] = (A ^ dV_i[0]);
	dV_i_1[1] = (B ^ dV_i[1]);
	dV_i_1[2] = (C ^ dV_i[2]);
	dV_i_1[3] = (D ^ dV_i[3]);
	dV_i_1[4] = (E ^ dV_i[4]);
	dV_i_1[5] = (F ^ dV_i[5]);
	dV_i_1[6] = (G ^ dV_i[6]);
	dV_i_1[7] = (H ^ dV_i[7]);
	return 1;
}


const int SM3_BLOCK_SIZE = 64;

const int NID_SM3 = 1001;

extern const int SM3_RESULT_LENGTH =32;


typedef struct _SM3DataBuf{
	unsigned char buff[SM3_BLOCK_SIZE];
	unsigned int  len;
	unsigned int  total_len;
	DWORD dV_i[8];
	int flag;
}SM3DataBuf;

static int SM3_init(EVP_MD_CTX *ctx)
	{
		SM3DataBuf *data_buf = (SM3DataBuf *)ctx->md_data;
		data_buf->flag=0;
		data_buf->len=0;
		data_buf->total_len=0;
		memset(data_buf->buff,0,SM3_BLOCK_SIZE);
		for(int i=0;i<8;++i)
		{
			data_buf->dV_i[i]=IV[i];
		}
		return 1;
	}
static int SM3_update(EVP_MD_CTX *ctx,const void *data,size_t count)
	{
		if( NULL == ctx ) return 0;
		if( NULL == data) return count==0;

		SM3DataBuf *ctx_data_buf = (SM3DataBuf *)ctx->md_data;
		ctx_data_buf->total_len+=count;
		int datalen = count+ctx_data_buf->len;
		if(datalen<SM3_BLOCK_SIZE)
		{
			memcpy(ctx_data_buf->buff+ctx_data_buf->len,data,count);
			ctx_data_buf->len+=count;
			return 1;
		}

		int r_len = datalen%SM3_BLOCK_SIZE;
		int buflen=datalen-r_len;
		BYTE * buff=new BYTE[buflen];
		memcpy(buff,ctx_data_buf->buff,ctx_data_buf->len);
		memcpy(buff+ctx_data_buf->len,data,count-r_len);

		ctx_data_buf->len=r_len;
		memcpy(ctx_data_buf->buff,(unsigned char *)data+count-r_len,r_len);

		DWORD dV_i_1[8];
		BYTE bMsg_buf[64];
		int nPosition=0;
		while(nPosition < buflen)
		{
			memcpy(bMsg_buf, buff+nPosition, 64);
			SM3_CF(ctx_data_buf->dV_i, bMsg_buf, dV_i_1);
			for (int i = 0;i < 8;i++)
			{
				ctx_data_buf->dV_i[i] = dV_i_1[i];
			}
			nPosition = nPosition + 64;
		}
		delete []buff;
		ctx_data_buf->flag=1;

		return 1;
	}
static int SM3_final(EVP_MD_CTX *ctx,unsigned char *md)
{
	if(NULL==ctx || NULL==md)
	{
		return 0;
	}

	SM3DataBuf *ctx_data_buf = (SM3DataBuf *)ctx->md_data;
		
//	if( (0 == ctx_data_buf->flag) && ( 0 == ctx_data_buf->len) )
//	{
//		return 0;
//	}

	unsigned int total_len = ctx_data_buf->total_len * 8;

	if(0!=ctx_data_buf->len)
	{
		const int min_pad_len=9;
		if((min_pad_len+ctx_data_buf->len)<=SM3_BLOCK_SIZE)
		{
			ctx_data_buf->buff[ctx_data_buf->len]=0x80;
			memset(ctx_data_buf->buff+ctx_data_buf->len+1,0,SM3_BLOCK_SIZE-ctx_data_buf->len-min_pad_len);
			
			ctx_data_buf->buff[SM3_BLOCK_SIZE-1]= (total_len  >> ((8 -1 - 7) * 8) )&0xFF;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-2]= (total_len >> ((8 -1 - 6) * 8) )&0xFF;;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-3]= (total_len >> ((8 -1 - 5) * 8) )&0xFF;;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-4]= (total_len >> ((8 -1 - 4) * 8) )&0xFF;;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-5]=0;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-6]=0;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-7]=0;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-8]=0;
		}
		else
		{
			ctx_data_buf->buff[ctx_data_buf->len]=0x80;
			memset(ctx_data_buf->buff+ctx_data_buf->len+1,0,SM3_BLOCK_SIZE-ctx_data_buf->len+1);
			DWORD dV_i_1[8];
			SM3_CF(ctx_data_buf->dV_i, ctx_data_buf->buff, dV_i_1);
			for (int i = 0;i < 8;i++)
			{
				ctx_data_buf->dV_i[i] = dV_i_1[i];
			}
			memset(ctx_data_buf->buff,0,SM3_BLOCK_SIZE);
			ctx_data_buf->buff[SM3_BLOCK_SIZE-1]= (total_len >> ((8 -1 - 7) * 8) )&0xFF;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-2]= (total_len >> ((8 -1 - 6) * 8) )&0xFF;;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-3]= (total_len >> ((8 -1 - 5) * 8) )&0xFF;;
			ctx_data_buf->buff[SM3_BLOCK_SIZE-4]= (total_len >> ((8 -1 - 4) * 8) )&0xFF;;
		}
	}
	else
	{
		memset(ctx_data_buf->buff,0,SM3_BLOCK_SIZE);
		ctx_data_buf->buff[0]=0x80;
		ctx_data_buf->buff[SM3_BLOCK_SIZE-1]= (total_len >> ((8 -1 - 7) * 8) )&0xFF;
		ctx_data_buf->buff[SM3_BLOCK_SIZE-2]= (total_len >> ((8 -1 - 6) * 8) )&0xFF;;
		ctx_data_buf->buff[SM3_BLOCK_SIZE-3]= (total_len >> ((8 -1 - 5) * 8) )&0xFF;;
		ctx_data_buf->buff[SM3_BLOCK_SIZE-4]= (total_len >> ((8 -1 - 4) * 8) )&0xFF;;
	}

	DWORD dV_i_1[8];
		
	SM3_CF(ctx_data_buf->dV_i, ctx_data_buf->buff, dV_i_1);
		
	for (int i = 0; i < 8; i++)
	{
		md[i*4] = (dV_i_1[i] >> 24) & 0xFF;
		md[i*4+1] = (dV_i_1[i] >> 16) & 0xFF;
		md[i*4+2] = (dV_i_1[i] >> 8) & 0xFF;
		md[i*4+3] = (dV_i_1[i]) & 0xFF;
	}

	ctx_data_buf->flag=0;
	ctx_data_buf->len=0;

	return 1;
}


static const EVP_MD EVP_MD_SM3=
	{
	NID_SM3,
	0,
	SM3_RESULT_LENGTH,
	0,
	SM3_init,
	SM3_update,
	SM3_final,
	nullptr,
	nullptr,
	EVP_PKEY_RSA_method,
	SM3_BLOCK_SIZE,//SHA_CBLOCK,
	sizeof(SM3DataBuf),//sizeof(EVP_MD *)+sizeof(SHA_CTX),
	nullptr
	};

const EVP_MD * EVP_SM3()
{
	return &EVP_MD_SM3;
}
