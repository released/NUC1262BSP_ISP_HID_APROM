# NUC1262BSP_ISP_HID_APROM
 NUC1262BSP_ISP_HID_APROM


update @ 2022/11/21

1. 2 project for ISP upgrade application flow 

2. Scenario notice:

	- Boot loader project : ISP_HID 
	
		- under sct file (hid_iap.sct) , will allocate flash size 
		
				LDROM_Bootloader.bin : 0x100000 ~ 0xFFF (default LDROM size : 4K)
				
				use SRAM specific address to store flag : 0x20004FF0
			
				APROM_Bootloader.bin : 0x1D00 0x2000 (reserve 8K size , to store extra boot loader code 

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_KEIL_sct.jpg)		
	
		- when power on , will check power on source (ex : power on reset , nReset , from application code)
	
		- use CRC to calculate Application code checksum (length : 0x1D000 - 4 : 0x1CFFC )
		
		- load Application code checksum , from specific address (at 0x1C000 last 4 bytes : 0x1CFFC)
		
		- power on from LDROM , and CRC calculate and compare correct		

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/boot_from_LDROM_to_APROM.jpg)
				
		- if two checksum result are different , will stuck in Boot loader , and wait for ISP code update
		
![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_checksum_err.jpg)		
		
		- if boot from application code , to LDROM code , by press digit 1

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_press_1_to_LDROM.jpg)	

		- when use ISP tool , select APROM , Reset and run 
		
![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/ISP_connect.jpg)

		- when ISP upgrade finish 
		
![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_upgrade_finish.jpg)

		- if reset from application code , will entry timeout counting , jump to application code if TIMEOUT		

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_press_1_timeout.jpg)

	
	- Application code project : AP
	
		- use SRecord , to calculate application code checksum , add binary to hex , by SRecord tool

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_KEIL_checksum_calculate.jpg)
	
		- SRecord file : srec_cat.exe 

		- under generateChecksum.bat will execute generateChecksum.cmd , generateCRCbinary.cmd , generateCRChex.cmd
	
			- generateChecksum.cmd : calculate checksum by load the original binary file , and display on KEIL project
		
			- generateCRCbinary.cmd : calculate checksum by load the original binary file , and fill 0xFF , range up to 0x1D000
		
			- generateCRChex.cmd : conver binary file into hex file
		
		- check sum calculate will start from 0 to 0x1D000-4 : 0x1CFFC , and store in 0x1CFFC , the last 4 bytes 
		
		- at KEIL output file , file name is APROM_application , under \obj folder , 
	
			which mapping to generateChecksum.cmd , generateCRCbinary.cmd , generateCRChex.cmd
	
			modify the file name in KEIL project , also need to modify the file name in these 3 generate***.cmd

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_KEIL_output_file.jpg)

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_SRecord_cmd_file.jpg)
		
		- after project compile finish , binary size will be 116K (total application code size : 0x1D000)
		
		- under terminal , use keyboard , '1' , will write specific value in SRAM flag , and return to boot loader
	
![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/KEIL_SRAM_alloction_for_LDROM_APOM.jpg)
			
	- reserve data flash address : 0x1F000 ( 4K )
	
4. Flash allocation

	- LDROM_Bootloader.bin : 0x100000 ~ 0xFFF
	
	- APROM_Bootloader.bin : 0x1D000 0x2000
	
	- Application code START address : 0x00000
	
	- Data flash : 0x1F000
	
	- Chcecksum storage : 0x1CFFC

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/FLASH_calculate.jpg)
	
![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/FLASH_allocation.jpg)
	
5. Function assignment

	- debug port : UART0 (PB12 , PB13) , in Boot loader an Application code project
		
	- enable SRAM flag storage and CRC , Timer1 , USB module
	
6. Need to use ICP tool , to programm boot loader project file (LDROM_Bootloader.bin , APROM_Bootloader.bin)

below is boot loader project , Config setting 

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_ICP_config.jpg)

below is boot loader project , ICP programming setting 

- LDROM_Bootloader.bin : under LDROM

- APROM_Bootloader.bin : 0x1D000

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_ICP_update.jpg)

7. under Application code KEIL project setting 

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_KEIL_checksum_calculate.jpg)

in Application project , press '1' will reset to Boot loader 

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/APROM_press_1_to_LDROM.jpg)

8. under boot loader project , below is sct file content

![image](https://github.com/released/NUC1262BSP_ISP_HID_APROM/blob/main/LDROM_KEIL_sct.jpg)


