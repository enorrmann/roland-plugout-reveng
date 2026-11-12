/* sect_only.c: build_multi.sh SECT_SWITCH (README.md §12): kernel k<i> (i > 0) of a wave-pair PLUG-OUT as its voice
 * sections only. Engine k0 does everything else (mains, ARM side, tables, notes): the kernels' mains are the same
 * program (build_multi.sh checks it), so k<i> adds only its section programs and the function that places them
 * (FC40 only, flag 0: as the stock synth, engine.c k<i>_engine_sections). Built with -DS1_SUB=k<i>_engine against
 * that kernel's s1_programs.h; s1_programs.h's other tables are not referenced and the compiler drops them. */
#include <stdint.h>
#include "syninfo.h"
#include "s1_programs.h"

typedef void (*place_fn)(int index, const struct s1_prog *prog, const struct s1_prog *d1, const struct s1_prog *d2,
                         const struct s1_prog *d3);
static struct s1_prog set_none[] = {
    { 0, 0, 0 },
};

#define SECT_FN_(x) x##_sections
#define SECT_FN(x) SECT_FN_(x)
void *S1_SUB(void) { return 0; }                 /* no engine object: multi.c uses k0's */
void SECT_FN(S1_SUB)(void)
{
#if NSECT
    for (int i = 0; i < NSECT; i++)
        ((place_fn)SVC(0x40))(sect_index[i], sect_set[i], set_none, set_none, set_none);
#endif
}
