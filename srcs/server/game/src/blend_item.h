/*********************************************************************
 * date        : 2007.02.24
 * file        : blend_item.h
 * author      : mhh
 * description :
 */

#ifndef _blend_item_h_
#define _blend_item_h_

#define	MAX_BLEND_ITEM_VALUE		5

bool	Blend_Item_init();
bool	Blend_Item_load(char *file);
bool	Blend_Item_set_value(LPITEM item);
bool	Blend_Item_find(DWORD item_vnum);

#endif	/* _blend_item_h_ */
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
