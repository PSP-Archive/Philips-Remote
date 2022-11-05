#include "std.h"

// Kernel mode access required for hardware hacks
PSP_MODULE_INFO("Philips Remote", 0x1000, 1, 1); // KMEM access for ir hacks
PSP_MAIN_THREAD_ATTR(0); // Kernel thread

#include "stdexit.c_" // standard exit code

static u32 PollButtons(u32* buttonsShiftPtr)
{
	static u32 s_buttonsLastPoll = 0;

	SceCtrlData pad;
	sceCtrlReadBufferPositive(&pad, 1);
    u32 buttonsNew = pad.Buttons & ~s_buttonsLastPoll;
	s_buttonsLastPoll = pad.Buttons;
    if (buttonsShiftPtr != NULL)
	    *buttonsShiftPtr = s_buttonsLastPoll & ~buttonsNew;
        // first time shift button is pressed it comes through normally
    return buttonsNew;
}

// test modes
static void DoTVMode();
static void DoTestMode();
static void DoDVDMode();

int main(void)
{
	SetupCallbacks();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    while (1)
    {
        // main mode
		pspDebugScreenInit();
		printf("Philips IR\n\n");
        printf("Press TRIANGLE to exit\n");
        printf("Press CIRCLE for TV\n");
        printf("Press SQUARE for DVD\n");
        printf("Press CROSS for TEST MODE\n");

		u32 buttons; 
        while ((buttons = PollButtons(NULL)) == 0)
			sceDisplayWaitVblankStart(); // slow things down a little

		if (buttons & PSP_CTRL_TRIANGLE)
            break;
		if (buttons & PSP_CTRL_CIRCLE)
			DoTVMode();
		else if (buttons & PSP_CTRL_SQUARE)
            DoDVDMode(); 
		else if (buttons & PSP_CTRL_CROSS)
            DoTestMode();
        // when modes exit - redraw the instructions
    }
    printf("\nGoodbye\n");
    sceKernelExitGame();
	return 0;
}

//////////////////////////////////////////////////////////////////////
// TV Mode

#include "rc6.h"

static void DoTVMode()
{
	u8 tvDevId = 0;
	pspDebugScreenInit();
    printf("TV Mode\n");
    
	printf("Press SELECT for BACK\n");
    printf("Press START to turn TV on/off\n");
    printf("Press LEFT/RIGHT for volume up/down\n");
    printf("Press UP/DOWN for channel up/down\n");
	printf("Press CROSS for MUTE\n");
	printf("Press TRIANGLE for Text\n");
    printf("Hold RTrigger and press LEFT/RIGHT/UP/DOWN for menu navigate\n");
    printf("Hold RTrigger and press CROSS or CIRCLE for OK\n");
    printf("Hold L and press SQUARE/TRIANGLE/CIRCLE/CROSS for RED/GREEN/YELLOW/BLUE\n");
    

	while (1)
	{
		u32 buttonsShift;
		u32 buttons = PollButtons(&buttonsShift);

		if (buttons & PSP_CTRL_SELECT)
            break;
        if (buttonsShift & PSP_CTRL_RTRIGGER)
        {
            // R Trigger for menu navigate
	        if (buttons & PSP_CTRL_UP)
                send_rc6_philips(tvDevId,IR_BUTTON_UP);
	        else if (buttons & PSP_CTRL_DOWN)
                send_rc6_philips(tvDevId,IR_BUTTON_DOWN);
	        else if (buttons & PSP_CTRL_LEFT)
                send_rc6_philips(tvDevId,IR_BUTTON_LEFT);
	        else if (buttons & PSP_CTRL_RIGHT)
                send_rc6_philips(tvDevId,IR_BUTTON_RIGHT);
	        else if (buttons & PSP_CTRL_CROSS)
                send_rc6_philips(tvDevId,IR_BUTTON_OK);
	        else if (buttons & PSP_CTRL_CIRCLE)
                send_rc6_philips(tvDevId,IR_BUTTON_OK); 
	        else if (buttons & PSP_CTRL_SQUARE)
                send_rc6_philips(tvDevId,IR_BUTTON_ENTER);
        }
        else if (buttonsShift & PSP_CTRL_LTRIGGER)
        {
            // L Trigger for modes
	        if (buttons & PSP_CTRL_CROSS)
				send_rc6_philips(tvDevId,IR_BUTTON_BLUE);
	        else if (buttons & PSP_CTRL_CIRCLE)
				send_rc6_philips(tvDevId,IR_BUTTON_YELLOW);
	        else if (buttons & PSP_CTRL_SQUARE)
				send_rc6_philips(tvDevId,IR_BUTTON_RED);
	        else if (buttons & PSP_CTRL_TRIANGLE)
				send_rc6_philips(tvDevId,IR_BUTTON_GREEN);
        }
        else
        {
            // no shift
	        if (buttons & PSP_CTRL_UP)
				send_rc6_philips(tvDevId,IR_BUTTON_CH_UP);
	        else if (buttons & PSP_CTRL_DOWN)
				send_rc6_philips(tvDevId,IR_BUTTON_CH_DOWN);
	        else if (buttons & PSP_CTRL_LEFT)
				send_rc6_philips(tvDevId,IR_BUTTON_VOL_DOWN);
	        else if (buttons & PSP_CTRL_RIGHT)
				send_rc6_philips(tvDevId,IR_BUTTON_VOL_UP);
			else if (buttons & PSP_CTRL_CROSS)
				send_rc6_philips(tvDevId,IR_BUTTON_MUTE);
			else if (buttons & PSP_CTRL_TRIANGLE)
				send_rc6_philips(tvDevId,IR_BUTTON_TEXT);
	        else if (buttons & PSP_CTRL_START)
				send_rc6_philips(tvDevId,IR_BUTTON_POWER);
	        
        }
    }
    printf("Exiting TV Mode\n");
}

//////////////////////////////////////////////////////////////////////
// DVD Mode

static void DoDVDMode()
{
	u8 dvdId = 4;
	pspDebugScreenInit();
    printf("DVD Mode\n");
    
	printf("Press SELECT for BACK\n");
    printf("Press START to turn DVD on/off\n");
	printf("Press TRIANGLE for PLAY\n");
	printf("Press SQUARE for STOP\n");
	printf("Press CIRCLE for PAUSE\n");
	printf("Press LEFT/RIGHT/UP/DOWN for menu navigate\n");
	printf("Press CROSS for OK\n");
    printf("Hold RTrigger and press LEFT/RIGHT for FORWARD/REWIND\n");    
    printf("Hold LTrigger and press CROSS for Subtitles\n");
    printf("Hold LTrigger and press CIRCLE for Disk Menu\n");
    printf("Hold LTrigger and press SQUARE for System Menu\n");
    

	while (1)
	{
		u32 buttonsShift;
		u32 buttons = PollButtons(&buttonsShift);

		if (buttons & PSP_CTRL_SELECT)
            break;
        if (buttonsShift & PSP_CTRL_RTRIGGER)
        {
            // R Trigger 
	        if (buttons & PSP_CTRL_LEFT)
                send_rc6_philips(dvdId,IR_BUTTON_REW);
	        else if (buttons & PSP_CTRL_RIGHT)
                send_rc6_philips(dvdId,IR_BUTTON_FWD);
	        else if (buttons & PSP_CTRL_CROSS)
                send_rc6_philips(dvdId,IR_BUTTON_OK);
	        else if (buttons & PSP_CTRL_CIRCLE)
                send_rc6_philips(dvdId,IR_BUTTON_OK);
	        else if (buttons & PSP_CTRL_SQUARE)
                send_rc6_philips(dvdId,IR_BUTTON_ENTER);
        }
        else if (buttonsShift & PSP_CTRL_LTRIGGER)
        {
            // L Trigger 
	        if (buttons & PSP_CTRL_CROSS)
				send_rc6_philips(dvdId,IR_BUTTON_SUBTITLE);
	        else if (buttons & PSP_CTRL_CIRCLE)
				send_rc6_philips(dvdId,IR_BUTTON_DVDMENU);
	        else if (buttons & PSP_CTRL_SQUARE)
				send_rc6_philips(dvdId,IR_BUTTON_SYSMENU);
	        
        }
        else
        {
            // no shift
	        if (buttons & PSP_CTRL_UP)
				send_rc6_philips(dvdId,IR_BUTTON_UP);
	        else if (buttons & PSP_CTRL_DOWN)
				send_rc6_philips(dvdId,IR_BUTTON_DOWN);
	        else if (buttons & PSP_CTRL_LEFT)
				send_rc6_philips(dvdId,IR_BUTTON_LEFT);
	        else if (buttons & PSP_CTRL_RIGHT)
				send_rc6_philips(dvdId,IR_BUTTON_RIGHT);
	        else if (buttons & PSP_CTRL_START)
				send_rc6_philips(dvdId,IR_BUTTON_POWER);
	        else if (buttons & PSP_CTRL_TRIANGLE)
				send_rc6_philips(dvdId,IR_BUTTON_PLAY);
			else if (buttons & PSP_CTRL_CIRCLE)
				send_rc6_philips(dvdId,IR_BUTTON_PAUSE);
			else if (buttons & PSP_CTRL_SQUARE)
				send_rc6_philips(dvdId,IR_BUTTON_STOP);
			else if (buttons & PSP_CTRL_CROSS)
				send_rc6_philips(dvdId,IR_BUTTON_OK);
        }
    }
    printf("Exiting DVD Mode\n");
}

//////////////////////////////////////////////////////////////////////
// Test Mode

static void PrintTestModeInfo()
{
	pspDebugScreenInit();
    printf("Philips Test Mode\n");

    printf("Press CROSS for Send\n");
    printf("Press TRIANGLE to exit\n");
    printf("Press UP for DeviceID+\n");
    printf("Press DOWN for DeviceID-\n");
    printf("Press LEFT for Code+ \n");
    printf("Press RIGHT for Code-\n");
}

static void DoTestMode()
{
	int count = 0;
    u8 devId, code, oldId, oldCode;
    devId = 0;
    code = 0;
    oldId = 1;
    oldCode = 1;
	PrintTestModeInfo();
	while (1)
	{
		u32 buttonsShift;
		u32 buttons = PollButtons(&buttonsShift);
		if (buttons & PSP_CTRL_TRIANGLE)
            break;
        
		if (buttons & PSP_CTRL_UP){
			if(devId < 255)
				devId++;
			else 
				devId = 0;
		}       
		else if (buttons & PSP_CTRL_DOWN){
	        if(devId > 0)
				devId--;
			else
				devId = 255;
	    }
		else if (buttons & PSP_CTRL_LEFT){
	        if(code > 0)
				code--;
	        else 
				code = 255; 
	    }
		else if (buttons & PSP_CTRL_RIGHT){
	        if(code < 255)
	            code++;
	        else
	            code = 0;
	    }
	    else if (buttons & PSP_CTRL_CROSS)
			send_rc6_philips(devId, code);
		if(devId != oldId || code != oldCode){
			if(count < 25)
				count++;
			else{
				PrintTestModeInfo();
				count = 0;
			}
			printf("Dev: %x, Code: %x\n", devId, code);
			oldId = devId;
			oldCode = code;
		}
        
    }
    printf("Exiting Test Mode\n");
}

//////////////////////////////////////////////////////////////////////
