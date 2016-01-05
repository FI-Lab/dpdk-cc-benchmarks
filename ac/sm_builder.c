#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "sm_builder.h"

int parse_binary(char *bin, unsigned char *pat, int *elt_num)
{
	int cnt, ch_num, i;
	char *s, *sp, **tok;
	s = bin;
	cnt = 0;
	while(*s != '|')
	{
		cnt++;
		s++;
	}
	*s = '\0';
	ch_num = (cnt + 1) / 3;
	tok = (char **)malloc(sizeof(char*) * ch_num);
	for(i = 0, s = sp = bin; i < ch_num; s = NULL, i++)
	{
		tok[i] = strtok_r(s, " ", &sp);
		*pat++ = (unsigned char)strtoul(tok[i], NULL, 16);
	}
	free(tok);
	*elt_num = ch_num;
	return cnt + 1;
}


int parse_one_line(char *raw, unsigned char *pat)
{
	int len = 0;
	int tmp;
	while(*raw != '\0' && *raw != '\n')
	{
		if(*raw != '|')
		{
			pat[len++] = *raw++;
		}
		else
		{
			raw += parse_binary(++raw, &pat[len], &tmp);
			len += tmp;
		}
	}
	return len;
}


sm_hdl_t* sm_build(char *pat_file)
{
	sm_hdl_t *hdl;
#if defined(ACSM_MODE)
	assert((hdl = acsmNew(NULL, NULL, NULL)) != NULL);
#elif defined(ACSM2_MODE)
	assert((hdl = acsmNew2(NULL, NULL, NULL)) != NULL);
#else
	assert((hdl = bnfaNew(NULL, NULL, NULL)) != NULL);
	hdl->bnfaMethod = 1;
#endif
	FILE *fp = fopen(pat_file, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "acsm build failed! file not exist! %s\n", pat_file);
        exit(-1);
    }
	unsigned char pat[512];
	char buf[512];
	int pat_len;
	while(fgets(buf, 512, fp) != NULL)
	{
		//printf("%s", buf);
		pat_len = parse_one_line(buf, pat);
		//printf("%d %s\n", pat_len, pat);
#if defined(ACSM_MODE)
		acsmAddPattern(hdl, pat, pat_len, 0, 0, 0, 0, NULL, 0);
#elif defined(ACSM2_MODE)
		acsmAddPattern2(hdl, pat, pat_len, 0, 0, 0, 0, NULL, 0);
#else
		bnfaAddPattern(hdl, pat, pat_len, 0, 0, NULL);
#endif
	}
#if defined(ACSM_MODE)
	acsmCompile(hdl, NULL, NULL);
	//printf("%d\n", acsmPatternCount(hdl));
#elif defined(ACSM2_MODE)
	acsmCompile2(hdl, NULL, NULL);
	acsmPrintInfo2(hdl);
	//acsmPrintDetailInfo2(hdl);
	//acsmPrintSummaryInfo2();
#else
	bnfaCompile(hdl, NULL, NULL);
	bnfaPrintInfo(hdl);
#endif
    fclose(fp);
	return hdl;
}


void sm_destroy(sm_hdl_t *hdl)
{

#if defined(ACSM_MODE)
	acsmFree(hdl);
#elif defined(ACSM2_MODE)
	acsmFree2(hdl);
#else
	bnfaFree(hdl);
#endif

}


static int match_found(void *id, void *tree, int index, void *data, void *neg_list)
{
	//printf("id: %s\n", (char*)id);
	return 0;
}


//1 success
//0 failure
int sm_search(sm_hdl_t *hdl, unsigned char *T, int len)
{
	int start_state = 0;
	int ret;
#if defined(ACSM_MODE)
	ret = acsmSearch(hdl, T, len, match_found, NULL, &start_state);
#elif defined(ACSM2_MODE)
	ret = acsmSearch2(hdl, T, len, match_found, NULL, &start_state);
#else
	ret = bnfaSearch(hdl, T, len, match_found, NULL, 0, &start_state);
#endif
	//printf("%d\n", start_state);
	return ret;
}


#if 0
int main(int argc, char **argv)
{
	unsigned char tst[256], *p;
	char a[256] = "|1A 3A 2A|abc|2B 3B 4B|as798ashfkjgsafy891273678412t6487|00 00 00 00 11 FF 2F|";
	int len;
	p = tst;
	/*printf("%d\n", len = parse_one_line(a, p));
	while(--len >= 0)
	{
		printf("%2x\n", (unsigned int)p[len]);
	}*/
	sm_hdl_t *hdl;
	hdl = sm_build(argv[1]);
	while(1)
	{
		scanf("%s", a);
		printf("%d\n", sm_search(hdl, a, strlen(a)));
	}
	sm_destroy(hdl);
	return 0;
}
#endif
