#include "legato.h"
#include "interfaces.h"

/*static void led_config(void)
{
	ledGpio_SetPushPullOutput(LEDGPIO_ACTIVE_HIGH, true);
}*/

COMPONENT_INIT
{
	LE_INFO("HELL!!!O WORLD");
	///led_config();
	while(1);
}