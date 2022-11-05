#include "std.h"
#include "rawir.h"


//////////////////////////////////////////////////////////////////////
// SIRC protocol
//   from - http://www.xs4all.nl/~sbp/knowledge/ir/sirc.htm

// timing_val = 20
//  carrier freq = 40kHz
//  1 tick = 25us
//  24 ticks for basic time unit (600 us)

#define TIME_1 24
#define TIME_2 (TIME_1*2)
#define TIME_4 (TIME_1*4)

// type must be 12, 15 or 20

void send_code(int type, int dev, int cmd) 
{
	u32 raw_buffer[100];
	int iL = 0;
	int i;
	// start burst
	raw_buffer[iL++] = IR_SIGNAL_ON(TIME_4);
    raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
	// 7 bit command
	for(i = 0; i < 7; i++)
	{
		if(cmd & (1 << i))
			raw_buffer[iL++] = IR_SIGNAL_ON(TIME_2);
		else 
			raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);
		
		raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
	}
	// rest is address
	for(i = 0; i < type - 7; i++)
	{
		if(dev & (1 << i))
			raw_buffer[iL++] = IR_SIGNAL_ON(TIME_2);
		else 
			raw_buffer[iL++] = IR_SIGNAL_ON(TIME_1);
		
		raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_1);
	}

	raw_buffer[iL++] = IR_SIGNAL_OFF(TIME_4);

	RAW_IR_DATA irdata;
    irdata.timing_val = 40/2; // 40kHz carrier
    irdata.raw_buffer = raw_buffer;
    irdata.raw_count = iL;

    int err = SendRawIR(&irdata, 1); 
    if (err != 0)
        printf("send $%x failed\n", cmd);
}