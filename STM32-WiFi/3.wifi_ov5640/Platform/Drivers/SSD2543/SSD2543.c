/* Includes ------------------------------------------------------------------*/
#include "SSD2543.h"

#define ssd2543_log(M, ...) 	custom_log("ssd2543", M, ##__VA_ARGS__)
#define ssd2543_log_trace() 	custom_log_trace("ssd2543")

typedef struct ChipSetting 
{
	uint8_t No;
	uint8_t Reg;
	uint8_t Data1;
	uint8_t Data2;
}SSD2543_CHIPSETTING;

typedef struct touch_x_y_p
{
	bool 	 detected;	
	uint32_t FingerX;
	uint32_t FingerY;
	uint32_t FingerP;
}TOUCH_X_Y_P;


struct ChipSetting ssd2543_read_Table[] = 
{
	{2,0x06,0x00,0x00},
	{2,0x07,0x00,0x00},
	{2,0x08,0x00,0x00},
	{2,0x09,0x00,0x00},
	{2,0x0A,0x00,0x00},
	{2,0x0B,0x00,0x00},
	{2,0x0C,0x00,0x00},
	{2,0x0D,0x00,0x00},
	{2,0x0E,0x00,0x00},
	{2,0x0F,0x00,0x00},
	{2,0x10,0x00,0x00},
	{2,0x11,0x00,0x00},
	{2,0x12,0x00,0x00},
	{2,0x13,0x00,0x00},
	{2,0x14,0x00,0x00},
	{2,0x15,0x00,0x00},
	{2,0x16,0x00,0x00},
	{2,0x17,0x00,0x00},
	{2,0x18,0x00,0x00},
	{2,0x19,0x00,0x00},
	{2,0x1A,0x00,0x00},
	{2,0x1B,0x00,0x00},
	{2,0x28,0x00,0x00},
	
	{2,0x30,0x00,0x00},
	{2,0xD7,0x00,0x00},
	{2,0xD8,0x00,0x00},
	{2,0xDB,0x00,0x00},
	
	{2,0x33,0x00,0x00},
	{2,0x34,0x00,0x00},
	{2,0x36,0x00,0x00},
	{2,0x37,0x00,0x00},
	
	{2,0x40,0x00,0x00},
	{2,0x41,0x00,0x00},
	{2,0x42,0x00,0x00},
	{2,0x43,0x00,0x00},
	{2,0x44,0x00,0x00},
	{2,0x45,0x00,0x00},
	{2,0x46,0x00,0x00},
	
	{2,0x56,0x00,0x00},
	{2,0x59,0x00,0x00},
	
	{2,0x65,0x00,0x00},
	{2,0x66,0x00,0x00},
	{2,0x67,0x00,0x00},
	
	{2,0x7A,0x00,0x00},
	{2,0x7B,0x00,0x00},
	
	{2,0x25,0x00,0x00}, 
};

struct ChipSetting ssd2543_cfgTable[] = 
{
	{2,0x06,0x19,0x0F},
	{2,0x07,0x00,0xE0},
	{2,0x08,0x00,0xE1},
	{2,0x09,0x00,0xE2},
	{2,0x0A,0x00,0xE3},
	{2,0x0B,0x00,0xE4},
	{2,0x0C,0x00,0xE5},
	{2,0x0D,0x00,0xE6},
	{2,0x0E,0x00,0xE7},
	{2,0x0F,0x00,0xE8},
	{2,0x10,0x00,0xE9},
	{2,0x11,0x00,0xEA},
	{2,0x12,0x00,0xEB},
	{2,0x13,0x00,0xEC},
	{2,0x14,0x00,0xED},
	{2,0x15,0x00,0xEE},
	{2,0x16,0x00,0xEF},
	{2,0x17,0x00,0xF0},
	{2,0x18,0x00,0xF1},
	{2,0x19,0x00,0xF2},
	{2,0x1A,0x00,0xF3},
	{2,0x1B,0x00,0xF4},
	{2,0x28,0x00,0x14},
	
	{2,0x30,0x08,0x0F},
	{2,0xD7,0x00,0x03},
	{2,0xD8,0x00,0x06},
	{2,0xDB,0x00,0x03},
	
	{2,0x33,0x00,0x03},
	{2,0x34,0xC6,0x60},
	{2,0x36,0x00,0x20},
	{2,0x37,0x07,0xC4},
	
	{2,0x40,0x10,0xC8},
	{2,0x41,0x00,0x30},
	{2,0x42,0x00,0x50},
	{2,0x43,0x00,0x30},
	{2,0x44,0x00,0x50},
	{2,0x45,0x00,0x00},
	{2,0x46,0x10,0x1F},
	
	{2,0x56,0x80,0x10},
	{2,0x59,0x80,0x10},
	
	{2,0x65,0x00,0x01},
	{2,0x66,0x1E,0x00},
	{2,0x67,0x1E,0xC4},
	
	{2,0x7A,0xFF,0xFF},
	{2,0x7B,0x00,0x03},
	
	{2,0x25,0x00,0x0C}, 
};



struct ChipSetting ssd2543_Reset[] = 
{
	{2,0x01,0x00,0x00},
};

struct ChipSetting ssd2543_Resume[] = 
{
	{2,0x04,0x00,0x00},	
};


TOUCH_X_Y_P  touch_info_all[FINGERNO];
uint8_t touch_detected_count = 0;

OSStatus ssd2543_Init(void);

/* I2C device */
mico_i2c_device_t ssd2543_i2c_device = {
	SSD2543_I2C_PORT, 0x48, I2C_ADDRESS_WIDTH_7BIT, I2C_HIGH_SPEED_MODE
};

OSStatus ssd2543_IO_Init(void)
{
	// I2C init
	MicoI2cFinalize(&ssd2543_i2c_device);   // in case error
	MicoI2cInitialize(&ssd2543_i2c_device);
	
	if(false == MicoI2cProbeDevice(&ssd2543_i2c_device, 5))
	{
		ssd2543_log("ssd2543_ERROR: no i2c device found!");
		return kNotInitializedErr;
	}
	return kNoErr;
}

OSStatus ssd2543_IO_Write(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToWrite)
{
	mico_i2c_message_t ssd2543_i2c_msg = {NULL, NULL, 0, 0, 0, false};
	OSStatus iError = kNoErr;
	uint8_t array[8];
	uint8_t stringpos;
	
	array[0] = RegisterAddr;
	
	for(stringpos = 0; stringpos < NumByteToWrite; stringpos++) 
	{
		array[stringpos + 1] = *(pBuffer + stringpos); 
	}
	
	iError = MicoI2cBuildTxMessage(&ssd2543_i2c_msg, array, NumByteToWrite + 1, 3);
	
	iError = MicoI2cTransfer(&ssd2543_i2c_device, &ssd2543_i2c_msg, 1);
	if(kNoErr != iError){
		iError = kWriteErr;
	}
	
	return kNoErr;
}

OSStatus ssd2543_IO_Read(uint8_t* pBuffer, uint8_t RegisterAddr, uint16_t NumByteToRead)
{
	mico_i2c_message_t ssd2543_i2c_msg = {NULL, NULL, 0, 0, 0, false};
	OSStatus iError = kNoErr;
	uint8_t array[8] = {0};
	
	array[0] = RegisterAddr;
	
	iError = MicoI2cBuildCombinedMessage(&ssd2543_i2c_msg, array, pBuffer, 1, NumByteToRead, 3);
	if(kNoErr != iError)
	{
		return kReadErr; 
	}
	
	iError = MicoI2cTransfer(&ssd2543_i2c_device, &ssd2543_i2c_msg, 1);
	if(kNoErr != iError)
	{
		return kReadErr;
	}
	return kNoErr;
}


void ssd2543_reset(void)
{
	uint32_t i = 0;
	
	for(i = 0; i < sizeof(ssd2543_Reset) / sizeof(ssd2543_Reset[0]); i ++)
	{
		ssd2543_IO_Write(&(ssd2543_Reset[i].Data1), ssd2543_Reset[i].Reg, ssd2543_Reset[i].No);
	}
	
	mico_thread_msleep(100);
	
	return;
}

void ssd2543_reg_read(void)
{
	uint32_t i = 0;
	
	for(i = 0; i < sizeof(ssd2543_read_Table) / sizeof(ssd2543_read_Table[0]); i ++)
	{
		ssd2543_IO_Read(&(ssd2543_read_Table[i].Data1), ssd2543_read_Table[i].Reg, ssd2543_read_Table[i].No);
	}
	
	return;
}

void ssd2543_reg_init(void)
{
	uint32_t i = 0;
	OSStatus err = kNoErr;
	
	ssd2543_reset();
	
	for(i = 0; i < sizeof(ssd2543_cfgTable) / sizeof(ssd2543_cfgTable[0]); i ++)
	{
		err = ssd2543_IO_Write(&(ssd2543_cfgTable[i].Data1), ssd2543_cfgTable[i].Reg, ssd2543_cfgTable[i].No);
		require_noerr_action(err, exit, ssd2543_log("ssd2543_IO_Write() error~,reg = 0x%02X", ssd2543_cfgTable[i].Reg));
	}
	
	mico_thread_msleep(300);
	
	ssd2543_reg_read();
 
 exit:	
	return;
}


OSStatus ssd2543_Init(void)
{
	OSStatus err = kNoErr;
	
	err = ssd2543_IO_Init();
	require_noerr_action(err, exit, ssd2543_log("ssd2543_IO_Init() error~"));
	
	ssd2543_reg_init();

 exit:	
	return err;  
}

void ssd2543_ts_work(void)
{
	uint32_t i = 0;
	uint8_t finger_info[4] = {0};
	uint8_t touch_status[2] = {0};
	uint16_t EventStatus = 0;
	
	ssd2543_IO_Read(touch_status, TOUCH_STATUS, 2);
	EventStatus = (touch_status[0] << 8 | touch_status[1]) >> 4;
	
	memset(touch_info_all, 0, sizeof(touch_info_all));
	touch_detected_count = 0;
	
	if(EventStatus != 0)
	{
		for(i = 0; i < FINGERNO; i ++)
		{
			if((EventStatus >> i ) & 0x1)
			{
				ssd2543_IO_Read(finger_info, FINGER01_REG + i, 4);
				touch_info_all[i].detected = true;
				touch_info_all[i].FingerY = (((finger_info[2] >> 4) & 0x0F) << 8) | finger_info[0];
				touch_info_all[i].FingerX = (((finger_info[2] >> 0) & 0x0F) << 8) | finger_info[1];
				touch_info_all[i].FingerP = finger_info[3];
				
				if(touch_info_all[i].FingerX != 0xFFF)
				{
					touch_detected_count ++;
				}else
				{
					EventStatus = EventStatus & (~(1 << i));
					
					touch_info_all[i].FingerX = 0x0FFF;
					touch_info_all[i].FingerY = 0x0FFF;
					touch_info_all[i].FingerP = 0;
					touch_info_all[i].detected = false;
				}
			}else
			{
				touch_info_all[i].FingerX = 0x0FFF;
				touch_info_all[i].FingerY = 0x0FFF;
				touch_info_all[i].FingerP = 0;
				touch_info_all[i].detected = false;
			}
		}
		
		ssd2543_log("count = %d", touch_detected_count);
		for(i = 0; i < FINGERNO; i ++)
		{
			if(touch_info_all[i].detected == true)
				ssd2543_log("[%d] X:%ld, Y:%ld, P:%ld", i, touch_info_all[i].FingerX, touch_info_all[i].FingerY, touch_info_all[i].FingerP);
		}
	}

	return;
}




