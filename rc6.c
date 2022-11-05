#include "std.h"
#include "rawir.h"


//////////////////////////////////////////////////////////////////////
// RC6 send_rc6_philips - mode 0
//   from - http://www.xs4all.nl/~sbp/knowledge/ir/rc6.htm

// timing_val = 18
//  carrier freq = 36kHz
//  1 tick = 22.5us
// ~20 ticks for basic time unit (444.44 us)

#define TIME_1 20
#define TIME_2 (TIME_1*2)
#define TIME_6 (TIME_1*6)


#define OPTIMIZE

// Manchester data encoding
static int AddDataBit0(u32* raw_buffer, int iL)
{
#ifdef OPTIMIZE
    if (!IR_SIGNAL_IS_ON(raw_buffer[iL-1]))
	    raw_buffer[iL-1] += TIME_1; // merge it
    else
	    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
#else
	    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
#endif
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);
    return iL;
}

static int AddDataBit1(u32* raw_buffer, int iL)
{
#ifdef OPTIMIZE
    if (IR_SIGNAL_IS_ON(raw_buffer[iL-1]))
	    raw_buffer[iL-1] += TIME_1; // merge it
    else
	    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);
#else
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);
#endif
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
    return iL;
}

static int AddDataByte(u32* raw_buffer, int iL, u8 bData)
    // return new 'iL'
{
    int i;
    for (i = 0; i < 8; i++)
    {
        if (bData & (0x80 >> i))
			iL = AddDataBit1(raw_buffer, iL);
        else
			iL = AddDataBit0(raw_buffer, iL);
    }
    return iL;
}

void send_rc6_philips(u8 devId, u8 code)
{
    static bool s_btrFlip = false;
    u32 raw_buffer[100]; // big enough for worst case
    int iL = 0;

    // leader
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_6);
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_2);
    // DATA 1000 (lead bit, mode 0)
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);  // start LB
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_2); // finish LB, start "0"
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);  // finish "0"
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1); // start "0"
    raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);  // finish "0"
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1); // start "0"
	if(s_btrFlip)
	{
		raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1+TIME_2); // finish "0" start trail bit "1"
		raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_2); // finish trail bit "1"
	}
	else
	{
		raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1); // finish "0" 
		raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_2); // start trail bit "0"
		raw_buffer[iL++] = IR_SIGNAL_ON(TIME_2); // finish trail bit "0"
	}
	s_btrFlip = !s_btrFlip;

    // send device ID
    iL = AddDataByte(raw_buffer, iL, devId);
    // send command
    iL = AddDataByte(raw_buffer, iL, code);

    // stop bit
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_6);
    raw_buffer[iL++] = IR_SIGNAL_OFF(50); // overkill
    
    assert(iL <= sizeof(raw_buffer)/sizeof(u32));

	RAW_IR_DATA irdata;
    irdata.timing_val = 36/2; // 36kHz carrier
    irdata.raw_buffer = raw_buffer;
    irdata.raw_count = iL;

    int err = SendRawIR(&irdata, 1);
    if (err != 0)
        printf("send_media_center $%x failed\n", code);
}

