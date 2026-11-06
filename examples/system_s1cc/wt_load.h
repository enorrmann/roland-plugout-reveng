/* wt_load.h: load a wavetable frame over USB into ARM RAM, then flush it to a destination. Two stages on purpose, so
 * the same loader serves both the slave's bank-5 frame (SL_WT=b5, engine.c) and, later, the master's EDM delay line
 * (EDM_WT, model.c): the transport fills an ARM-RAM buffer (wtl_buf, Q15), and a CTRL "commit" calls WTL_COMMIT(buf,
 * n), which the including region defines (write bank 5 here; write the delay memory there). Nothing is reinstalled.
 *
 * Transport -- the SYSTEM-1 port of re/dsplive.py's live-poke protocol, with no extra region-0 work. The PC
 * (tools/wt_load.py) sends, per word, a pitch bend (14 data bits) and then a CC1 (the 7-bit command). Our region-0
 * assigner already forwards pitch bend -> voice parameter 0x130 and CC1 -> voice parameter 0x131 (model.c a_bend /
 * a_cc), and both reach this region's parameter handler. The region calls wtl_param(id, value) from that handler; it
 * acts when the command id changes value (its sequence bit, bit 6, toggles on every command, so a repeat is still a
 * change):
 *     cmd = seq<<6 | op<<4 | ext                 (7 bits, one CC)
 *       op 0 ADDR   index = data (0 .. WTL_MAX-1); the next sample lands at wtl_buf[index]
 *       op 1 DATA   wtl_buf[index++] = sext18(data | ext<<14) >> 2   (an 18-bit value * 2^17 -> Q15)
 *       op 2 CTRL   ext 0 commit (WTL_COMMIT(wtl_buf, wtl_n)); ext 1 nop; ext 2 clear (wtl_n = 0, WTL_REVERT())
 * data is read with WTL_GET(WTL_DATA_ID) when the command arrives -- robust to a dropped data event, exactly as
 * dsplive reads the bend from the slave store (os_get(2, 0x465)).
 *
 * UNPROVEN on hardware as of 2026-11-06: that voice parameters 0x130 / 0x131 reach region 1's parameter handler
 * (notes 0x110+v and velocities 0x120+v, also group 2, do). If the device does not load, retarget the two ids with
 * -DWTL_DATA_ID / -DWTL_CMD_ID to a pair known to reach the region and resend (tools/wt_load.py --data-cc / --cmd-cc).
 *
 * The including region must, before #include "wt_load.h":
 *   - define WTL_COMMIT(buf, n)      flush n Q15 samples of buf to the destination
 *   - optionally define WTL_REVERT() undo a load (e.g. restore the COLOR-picked frame); default: nothing
 *   - optionally define WTL_GET(id)  read a stored parameter; default: os_get(2, id)
 *   - optionally define WTL_MAX      buffer length in samples; default: 128
 *   - optionally define WTL_DATA_ID / WTL_CMD_ID   the two transport ids; defaults 0x130 / 0x131
 */
#ifndef WT_LOAD_H
#define WT_LOAD_H

#include <stdint.h>

#ifndef WTL_COMMIT
#error "wt_load.h: define WTL_COMMIT(buf, n) before including it"
#endif
#ifndef WTL_REVERT
#define WTL_REVERT()
#endif
#ifndef WTL_GET
#define WTL_GET(id) os_get(2, (id))
#endif
#ifndef WTL_MAX
#define WTL_MAX 128
#endif
#ifndef WTL_DATA_ID
#define WTL_DATA_ID 0x130                             /* pitch bend (model.c a_bend) */
#endif
#ifndef WTL_CMD_ID
#define WTL_CMD_ID 0x131                              /* CC1 / modulation (model.c a_cc case 1) */
#endif

static int32_t wtl_buf[WTL_MAX];                      /* the frame in ARM RAM, Q15 (x * 2^-15) */
static int wtl_n, wtl_idx, wtl_cmd_last = -1;         /* samples received, write cursor, last command (never -1) */

/* 14-bit stored bend -> the 14-bit data P the PC sent (a_bend stored P - 0x2000; os_get returns it signed) */
static int wtl_data(void) { return (WTL_GET(WTL_DATA_ID) + 0x2000) & 0x3FFF; }

static int wtl_param(int id, int value)               /* 1 if it consumed the parameter */
{
    if (id != WTL_CMD_ID)
        return 0;
    if ((value & 0x7F) == wtl_cmd_last)               /* the OS re-handed the same command: already done */
        return 1;
    wtl_cmd_last = value & 0x7F;
    int op = (value >> 4) & 3, ext = value & 15;
    if (op == 0) {                                    /* ADDR */
        int i = wtl_data();
        wtl_idx = i < WTL_MAX ? i : WTL_MAX - 1;
    } else if (op == 1) {                             /* DATA */
        uint32_t x = (uint32_t)wtl_data() | (uint32_t)ext << 14;   /* 18 bits */
        int32_t s = (int32_t)(x << 14) >> 14;         /* sign-extend from 18 bits */
        if (wtl_idx < WTL_MAX) {
            wtl_buf[wtl_idx] = s >> 2;                 /* value * 2^17 -> Q15 (value * 2^15) */
            if (wtl_idx + 1 > wtl_n)
                wtl_n = wtl_idx + 1;
            wtl_idx++;
        }
    } else if (op == 2) {                             /* CTRL */
        if (ext == 0)
            WTL_COMMIT(wtl_buf, wtl_n);
        else if (ext == 2) {
            wtl_n = 0;
            wtl_idx = 0;
            WTL_REVERT();
        }
    }
    return 1;
}

#endif
