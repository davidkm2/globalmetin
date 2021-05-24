#ifndef _attr_transfer_h_
#define _attr_transfer_h_

#define MAX_ATTR_TRANSFER_SLOT	3
#define ATTR_TRANSFER_MAX_DISTANCE	1000

void AttrTransfer_open(LPCHARACTER ch);
void AttrTransfer_close(LPCHARACTER ch);
void AttrTransfer_clean_item(LPCHARACTER ch);
bool AttrTransfer_make(LPCHARACTER ch);
void AttrTransfer_add_item(LPCHARACTER ch, int w_index, int i_index);
void AttrTransfer_delete_item(LPCHARACTER ch, int w_index);
#endif
