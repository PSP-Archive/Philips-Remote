#include "std.h"

#include "syshelp.c_"

#include "rawir.h"

//////////////////////////////////////////////////////////////////////
// Direct system access to replace much of "sceSircsSend"-like functionality
//  still using built-in interrupt handler

// user entries
extern u32 sceKernelWaitEventFlag(u32, u32, u32, u32*, u32*);
extern u32 sceKernelClearEventFlag(u32 r4id, u32 r5);

// system/kernel entries - lookup dynamically
static u32 (*sceKernelSuspendIntr)(u32 r4);
static u32 (*sceKernelResumeIntr)(u32 r4);
static void (*sceSysregSircsIoEnable)();
static void (*sceSysregSircsIoDisable)();
static void (*sceGpioPortClear)(int r4);
static void (*sceGpioPortSet)(int r4);
static void (*sceSysreg_driver_0x6417cdd6)();
static void (*sceSysreg_driver_0x20388c9e)();

static bool InitSysEnt()
{
	sceKernelSuspendIntr = FindProc("sceInterruptManager", "InterruptManagerForKernel", 0x750e2507);
	sceKernelResumeIntr = FindProc("sceInterruptManager", "InterruptManagerForKernel", 0x494d6d2b);
        // also in ThreadManForUser

	sceSysregSircsIoEnable = FindProc("sceSYSREG_Driver", "sceSysreg_driver", 0x4c49a8bc);
	sceSysregSircsIoDisable = FindProc("sceSYSREG_Driver", "sceSysreg_driver", 0x26fa0928);
	sceGpioPortSet = FindProc("sceGPIO_Driver", "sceGpio_driver", 0x310f0ccf);
	sceGpioPortClear = FindProc("sceGPIO_Driver", "sceGpio_driver", 0x103c3eb2);
	sceSysreg_driver_0x6417cdd6 = FindProc("sceSYSREG_Driver", "sceSysreg_driver", 0x6417cdd6);
	sceSysreg_driver_0x20388c9e = FindProc("sceSYSREG_Driver", "sceSysreg_driver", 0x20388c9e);

    return (!g_bFindProcError);
}

//////////////////////////////////////////////////////////////////////
// SIRCS Interrupt Handler glue

// The built-in SIRCS interrupt handler is very close to what we need,
//  so we use it as-is
//  We directly access the system globals used by the handler

struct SIRCS_HANDLER_DATA // DO NOT CHANGE
{
    // global data used by the system (at address 0x880d6328 in firmware 1.00)
    u32 data0; // not used
    u32 clToSend;
    u32 clDataSent;
    const u32* plDataNext;
    u32 semaId;
    u32 eventFlag;
    u32 active;     // 0 normally, set to 1 when handler needed
}; // DO NOT CHANGE

static struct SIRCS_HANDLER_DATA volatile* g_sircsHandlerData;

//////////////////////////////////////////////////////////////////////

int SendRawIR(const RAW_IR_DATA* dataP, int count)
{
    int err;

    static bool bInited = false;
    if (!bInited)
    {
        // one-time init (put absolute system addresses in globals)
	    if (!InitSysEnt())
	    {
	        printf("Error: dynamic binding failed (system)!\n");
	        return -1;
	    }
	
	    // find the globals used by the SIRCS driver (sceSircsEnd entry point)
		u8* pDriver = FindProc("sceSIRCS_IrDA_Driver", "sceSircs_driver", 0x19155a2f);
	    if (pDriver == NULL)
	    {
	        printf("Error: dynamic binding failed (SIRCS driver)!\n");
	        return -1;
	    }
		g_sircsHandlerData = (struct SIRCS_HANDLER_DATA*)(pDriver+ 0x19B8);
	        // 0x19B8 offset same for 1.0 and 1.50 firmware
	
        // sanity check
	    if (g_sircsHandlerData->eventFlag != FindEventFlag("SceSIRCS") ||
		    g_sircsHandlerData->semaId != FindSemaphore("SceSIRCS"))
	    {
	        printf("Error: dynamic binding failed (SIRCS data)!\n");
	        return -1;
	    }
        bInited = true; // do once, addresses won't move
    }

    // take control of hardware & driver
	g_sircsHandlerData->active = 1;
    err = sceKernelWaitSema(g_sircsHandlerData->semaId, 1, NULL);
    if (err != 0)
    {
		g_sircsHandlerData->active = 0;
        printf("Error: SIRCS device busy\n");
        return -1;
    }

	sceSysreg_driver_0x6417cdd6();
	sceSysregSircsIoEnable();
	sceGpioPortClear(4);
	
    // party on the hardware registers!
    volatile u32* plHardware = (u32*)0xBE340000;

    // REVIEW: explore more variations here
    plHardware[0] = 0xA0;
    plHardware[1] = dataP->timing_val;
    plHardware[3] = 0x7;

	sceKernelSuspendIntr(0xE);

    // send loop
    while (count-- > 0)
    {
		sceKernelClearEventFlag(g_sircsHandlerData->eventFlag, -1);
		g_sircsHandlerData->plDataNext = dataP->raw_buffer;
		g_sircsHandlerData->clToSend = dataP->raw_count;
        g_sircsHandlerData->clDataSent = 0;

		// pre-fill hardware fifo - usually fills at 16, interrupts after 8
		while ((plHardware[4] & 0x20) == 0)
        {
		    plHardware[0x14/4] = *(g_sircsHandlerData->plDataNext)++;
	        g_sircsHandlerData->clDataSent++;
        }
    
        plHardware[0] |= 0x1200;
            // start the hardware to do its thing,
            //  and the interrupt handler will do the rest

        // wait for completion
	    u32 result = 0;
		sceKernelWaitEventFlag(g_sircsHandlerData->eventFlag, 1, 0x11, &result, 0);
    }

	// clean up
	sceKernelResumeIntr(0xE);
	sceGpioPortSet(4);
	sceSysreg_driver_0x20388c9e();
	sceSysregSircsIoDisable();

    // give up control of hardware & driver
	g_sircsHandlerData->active = 0;
	sceKernelSignalSema(g_sircsHandlerData->semaId, 1);

    return 0;
}

///////////////////////////////////////////////////////////

