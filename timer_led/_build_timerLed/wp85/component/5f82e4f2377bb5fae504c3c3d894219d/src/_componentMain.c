/*
 * AUTO-GENERATED _componentMain.c for the timerLedComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _timerLedComponent_mangoh_ledGpio_ServiceInstanceName;
const char** mangoh_ledGpio_ServiceInstanceNamePtr = &_timerLedComponent_mangoh_ledGpio_ServiceInstanceName;
void mangoh_ledGpio_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t timerLedComponent_LogSession;
le_log_Level_t* timerLedComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _timerLedComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _timerLedComponent_Init(void)
{
    LE_DEBUG("Initializing timerLedComponent component library.");

    // Connect client-side IPC interfaces.
    mangoh_ledGpio_ConnectService();

    // Register the component with the Log Daemon.
    timerLedComponent_LogSession = log_RegComponent("timerLedComponent", &timerLedComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_timerLedComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
