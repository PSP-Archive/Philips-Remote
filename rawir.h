// Raw IR playback
//////////////////////////////////////////////////////////////////////////

typedef struct _RAW_IR_DATA
{
    int timing_val;
        // used for carrier frequency (freq = timing_val * 2 kHz)
        //  and for PWM sample ticks (1 tick = timing_val * 1.25 us)
    const u32* raw_buffer;
    int raw_count;
} RAW_IR_DATA;

int SendRawIR(const RAW_IR_DATA* dataP, int count);
    // return 0 if ok, -1 if error


// Common settings:
// timing_val = 20: freq = 40kHz, tick = 25us
// timing_val = 18: freq = 36kHz, tick = 22.5us
// NOTE: frequency and timing are *both* directly proportional to 'timing_val'


//////////////////////////////////////////////////////////////////////////
// utilities for building the "raw_buffer"
// each 32 bit value represents a PWM sample
//      first byte = duration (in ticks)
//               range 1->255. For longer delays, use two raw_buffer entries
//      second byte = 0
//      third byte = 1 for IR on (eg: start bit),
//                0 for IR off (eg: stop bit/idle)
//      fourth byte = 0

static inline u32 IR_SIGNAL_ON(int ticks) // .025 ms ticks
{
    assert(ticks >= 0 && ticks < 256);
    return ticks | 0x10000;
}
static inline u32 IR_SIGNAL_OFF(int ticks) // .025 ms ticks
{
    assert(ticks >= 0 && ticks < 256);
    return ticks;
}

#define IR_SIGNAL_IS_ON(lVal) (((lVal) & 0x10000) != 0)

//////////////////////////////////////////////////////////////////////////
