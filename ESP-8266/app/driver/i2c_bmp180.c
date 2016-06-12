/* reaper7 */

#include <math.h>
#include "i2c.h"
#include "i2c_bmp180.h"

int16_t ac1, ac2, ac3;
uint16_t ac4, ac5, ac6;
int16_t b1, b2;
int16_t mb, mc, md; 

int16_t ICACHE_FLASH_ATTR
BMP180_readRegister16(uint8_t reg) {   
  i2c_start(); 
  i2c_writeByte(BMP180_ADDRESS); 
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  i2c_writeByte(reg);
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  i2c_start(); 
  i2c_writeByte(BMP180_ADDRESS+1);  
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  uint8_t msb = i2c_readByte();                 
  i2c_send_ack(1);                       		
  uint8_t lsb = i2c_readByte();     
  i2c_send_ack(0);                       		
  i2c_stop();                           	
  int16_t res = msb << 8;
  res += lsb;	
  return res;							
}

int16_t ICACHE_FLASH_ATTR
BMP180_readRawValue(uint8_t cmd) {
  i2c_start();                  			
  i2c_writeByte(BMP180_ADDRESS);   			
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  i2c_writeByte(BMP180_CTRL_REG);	          	
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  i2c_writeByte(cmd);       					
  if(!i2c_check_ack()){
    //os_printf("slave not ack..\n return \n");
    i2c_stop();
    return(0);
  }
  i2c_stop();                   			
  os_delay_us(CONVERSION_TIME*1000);
  int16_t res = BMP180_readRegister16(BMP180_DATA_REG);
  return res;
}

uint32_t ICACHE_FLASH_ATTR
BMP180_GetVal(uint8_t mode)
{
  int32_t UT;
  uint16_t UP;
  int32_t B3, B5, B6;
  uint32_t B4, B7;
  int32_t X1, X2, X3;
  int32_t T, P;
  
  UT = BMP180_readRawValue(BMP_CMD_MEASURE_TEMP);
  UP = BMP180_readRawValue(BMP_CMD_MEASURE_PRESSURE_0);

  X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md); 
  B5 = X1 + X2;
  T  = (B5+8) >> 4;

  if(mode==GET_BMP_TEMPERATURE)
    return (T);
   
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = (((int32_t)ac1 * 4 + X3) + 2) / 4;
  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL);
  
  if (B7 < 0x80000000) {
    P = (B7 * 2) / B4;
  } else {
    P = (B7 / B4) * 2;
  }

  X1 = (P >> 8) * (P >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * P) >> 16;
  P = P + ((X1 + X2 + (int32_t)3791) >> 4);
  
  if(mode==GET_BMP_REAL_PRESSURE)
    return P;
    
  if(mode>=GET_BMP_RELATIVE_PRESSURE)
    return (int32_t)(pow(((float)MYALTITUDE/44330)+1,5.255F)*P);
}

bool ICACHE_FLASH_ATTR
BMP180_Init()
{
  if (!BMP180_readRegister16(BMP180_VERSION_REG))
    return 0;

  ac1 = BMP180_readRegister16(0xAA);				 
  ac2 = BMP180_readRegister16(0xAC);
  ac3 = BMP180_readRegister16(0xAE);
  ac4 = BMP180_readRegister16(0xB0);
  ac5 = BMP180_readRegister16(0xB2);
  ac6 = BMP180_readRegister16(0xB4);
  b1 =  BMP180_readRegister16(0xB6);
  b2 =  BMP180_readRegister16(0xB8);
  mb =  BMP180_readRegister16(0xBA);
  mc =  BMP180_readRegister16(0xBC);
  md =  BMP180_readRegister16(0xBE);
  
  return 1;
}

