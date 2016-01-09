#ifndef __USER_FX_LAN__
#define	__USER_FX_LAN__
#include "user_fx.h"

u8 *fx_lan_get(u8 *uri, u8 *params, int timeout);
u8 *fx_lan_post(u8 *uri, u8 *params, int timeout); 

bool fx_lan_enquiry(u8 *ip);
bool fx_lan_force_on(u8 *ip, u8 addr_type, u16 addr);
bool fx_lan_force_off(u8 *ip, u8 addr_type, u16 addr);
bool fx_lan_read(u8 *ip, u8 addr_type, u16 addr, u8 *out, u16 len);
bool fx_lan_write(u8 *ip, u8 addr_type, u16 addr, u8 *data, u16 len);

#endif // __USER_FX_LAN__

