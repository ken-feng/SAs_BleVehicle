
#include "gap_interface.h"
#include "ble_constants.h"
#include "otap_client_att.h"

uint32_t Ble_SendData(deviceId_t device_id, uint8_t *pdata, uint16_t length)
{
	BleApp_NotifyDKData(device_id, pdata,length);
	return 0;
}
