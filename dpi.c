#include "dpi.h"
#include <rte_common.h>
#include <rte_eal.h>
#include <errno.h>

sm_hdl_t *dpi_hdl;

void dpi_engine_init(char *pf)
{
    dpi_hdl = sm_build(pf);
    if(dpi_hdl == NULL)
    {
        rte_exit(EINVAL, "dpi engine init failed!\n");
    }
}

int dpi_engine_exec(unsigned char *T, int len)
{
    return sm_search(dpi_hdl, T, len);
}

void dpi_engine_close()
{
    sm_destroy(dpi_hdl);
}
