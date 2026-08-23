#ifndef __AML_FHY_H__
#define __AML_FHY_H__


#if defined (HAL_SIM_VER)
unsigned int MAC_RD_REG(unsigned int addr);
void MAC_WR_REG(unsigned int addr,unsigned int data);

unsigned int AON_RD_REG(unsigned int addr);
void AON_WR_REG(unsigned int addr,unsigned int data);
#endif

void phy_register_set(void);
void coexit_bt_thread_enable(void);
#endif
