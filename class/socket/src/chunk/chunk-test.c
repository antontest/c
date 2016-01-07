#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <chunk.h>

int main(int argc, char **argv)
{
    unsigned char t[] = {1, 2, 3};
    unsigned char t1[] = {4, 5, 6, 7};
    chunk_t chunk = chunk_create(t, sizeof(t));
    chunk_t chunk1 = chunk_create(t1, sizeof(t1));
    printf("chunk len: %d, chunk: %d, %d\n", chunk.len, chunk.ptr[0], chunk.ptr[1]);
    printf("chunk len: %d\n", chunk_length("cc", chunk, chunk1));

    unsigned char *m = (unsigned char *)malloc(sizeof(t) + sizeof(t1) + 1);
    if (!m) return -1;
    chunk_t ch = chunk_create_cat(m, "cc", chunk, chunk1);
    printf("ch len: %d\n", ch.len);
    chunk_t ch_split, ch_split1;
    chunk_split(ch, "mm", 5, &ch_split, 3, &ch_split1);
    printf("ch_split len: %d, 5: %d, ch_split1 len: %d, 1: %d\n", ch_split.len, ch_split.ptr[4], ch_split1.len, ch_split1.ptr[0]);
    free(m);
    return 0;
}
