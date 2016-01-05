#include <stdio.h>
#include "acsmx.h"
#include "acsmx2.h"
#include "bnfa_search.h"


/*
 *  Text Data Buffer
 */
char text[512];

/*
 *    A Match is found
 */
int MatchFound (void* id, void* tree, int index, void *data, void* neg_list)
{
    fprintf(stdout, "id    %s\n", (char *)id);
    fprintf(stdout, "data  %s\n", (char *)data); 
    fprintf(stdout, "index %d\n", index);
    return 0;
}


/*
 *
 */
int main (int argc, char **argv) 
{
    int i, nocase = 0;
    int start_state = 0;
    //ACSM_STRUCT * acsm;
    bnfa_struct_t *bnfa;
    if (argc < 3) {
        fprintf (stderr,
                "Usage: acsmx pattern word-1 word-2 ... word-n  -nocase\n");
        exit (0);
    }
    //acsm = acsmNew(NULL, NULL, NULL);
    bnfa = bnfaNew(NULL, NULL, NULL);
    strcpy(text, argv[1]);

    for (i = 1; i < argc; i++)
        if (strcmp(argv[i], "-nocase") == 0)
            nocase = 1;

    for (i = 2; i < argc; i++) {
        if (argv[i][0] == '-')
            continue;
        //acsmAddPattern(acsm, (unsigned char*)argv[i], strlen(argv[i]), nocase, 0, 0, 0, argv[i], i - 2);
	bnfaAddPattern(bnfa, (unsigned char*)argv[i], strlen(argv[i]), nocase, 0, argv[i]);
    }
    bnfaCompile(bnfa, NULL, NULL);
    bnfaSearch(bnfa, (unsigned char*)text, strlen(text), MatchFound, (void *)0, 0, &start_state);
    bnfaFree(bnfa);
    return (0);
}

