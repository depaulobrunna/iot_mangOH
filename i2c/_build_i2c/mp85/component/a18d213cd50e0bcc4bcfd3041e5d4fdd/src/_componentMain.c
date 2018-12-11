/*
 * AUTO-GENERATED _componentMain.c for the i2cComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

// Component log session variables.
le_log_SessionRef_t i2cComponent_LogSession;
le_log_Level_t* i2cComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _i2cComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _i2cComponent_Init(void)
{
    LE_DEBUG("Initializing i2cComponent component library.");

    // Register the component with the Log Daemon.
    i2cComponent_LogSession = log_RegComponent("i2cComponent", &i2cComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_i2cComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
