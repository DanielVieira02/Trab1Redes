#include "front.h"

#define PROGRESS_STR "####################################################################################################"
#define DIM(x) (sizeof(x)/sizeof(*(x)))

static const char     *sizes[]   = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t  exbibytes = 1024ULL * 1024ULL * 1024ULL *
                                   1024ULL * 1024ULL * 1024ULL;

char * calculate_size(uint64_t size) {   
    char     *result = (char *) malloc(sizeof(char) * 20);
    uint64_t  multiplier = exbibytes;
    int i;

    for (i = 0; i < DIM(sizes); i++, multiplier /= 1024)
    {   
        if (size < multiplier)
            continue;
        if (size % multiplier == 0)
            sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes[i]);
        else
            sprintf(result, "%.1f %s", (float) size / multiplier, sizes[i]);
        return result;
    }
    strcpy(result, "0");
    return result;
}

void imprime_barra_progresso(double progresso) {
    int valor = (int)(progresso * 100);
    int l_pad = (int)(progresso * 100);
    int r_pad = 100 - l_pad;
    printf("\r%3d%% [%.*s%*s]", valor, l_pad, PROGRESS_STR, r_pad, " ");
    fflush(stdout);
}

void imprime_recebendo(uint64_t quantidade_bytes) {
    printf("\rRecebendo... (%s)", calculate_size(quantidade_bytes));
    fflush(stdout);
}