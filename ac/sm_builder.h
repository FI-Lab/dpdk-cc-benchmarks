#ifndef SM_BUILDER
#define SM_BUILDER

#include "acsmx.h"
#include "acsmx2.h"
#include "bnfa_search.h"
#include "snort_debug.h"
#include "util.h"

#define ACSM_MODE
//#define ACSM2_MODE

#if defined(ACSM_MODE)
typedef ACSM_STRUCT sm_hdl_t;

#elif defined(ACSM2_MODE)
typedef ACSM_STRUCT2 sm_hdl_t;

#else
typedef bnfa_struct_t sm_hdl_t;

#endif


//build state machine from pattern file
sm_hdl_t* sm_build(char *pat_file);

//destroy sm
void sm_destroy(sm_hdl_t *hdl);

//len = strlen(T)
//return 0 == failure
int sm_search(sm_hdl_t *hdl, unsigned char *T, int len);


#endif
