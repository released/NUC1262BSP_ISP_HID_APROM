@echo off
setlocal

call checksum_config.cmd

set SREC=srec_cat

set APP_BIN=obj\APROM_application.bin
set TMP_IMG=obj\_aprom_crc_tmp.bin
set APP_HEX=obj\APROM_application.hex
set APP_HEX_CRC=obj\APROM_application_crc.hex

echo ========================================================
echo Generate CRC32 (ABSOLUTE address semantics)
echo --------------------------------------------------------
echo APP_START  = %APP_START%
echo APP_SIZE   = %APP_SIZE%
echo CRC_ADDR   = %CRC_ADDR%
echo APP_BIN    = %APP_BIN%
echo ========================================================

:: =========================================================
:: Derived values (DO NOT EDIT)
:: =========================================================

:: Relative offset inside app-only binary
set CRC_SIZE=4
set /a CRC_OFFSET=CRC_ADDR - APP_START
set /a CRC_END=CRC_ADDR + CRC_SIZE
set /a CRC_OFFSET_END=CRC_OFFSET + CRC_SIZE

:: --------------------------------------------------------
:: Step 1: Build temporary APROM image and calculate CRC
:: (Used only for CRC calculation / dump)
:: --------------------------------------------------------
%SREC% ^
  %APP_BIN% -binary ^
  -offset %APP_START% ^
  -fill 0xFF %APROM_BASE% %APROM_SIZE% ^
  -crop %APROM_BASE% %CRC_ADDR% ^
  -crc32-l-e %CRC_ADDR% ^
  -o %TMP_IMG% -binary

if errorlevel 1 goto err

:: --------------------------------------------------------
:: Step 2: Write CRC back to app-only binary (relative offset)
:: --------------------------------------------------------
%SREC% ^
  %APP_BIN% -binary ^
  -fill 0xFF 0x0000 %APP_SIZE% ^
  -crop 0x0000 %CRC_OFFSET% ^
  -crc32-l-e %CRC_OFFSET% ^
  -o %APP_BIN% -binary

:: --------------------------------------------------------
:: Step 3: Dump checksum (last 4 bytes) to terminal
:: --------------------------------------------------------
echo.
echo ---- CRC32 @ %CRC_ADDR% (HEX dump) ----
%SREC% ^
  %APP_BIN% -binary ^
  -crop %CRC_OFFSET% %CRC_OFFSET_END% ^
  -o - -HEX_Dump


if errorlevel 1 goto err

:: --------------------------------------------------------
:: Step 4: Generate CRC-patched Intel HEX for flashing
:: --------------------------------------------------------
%SREC% ^
  %APP_BIN% -binary ^
  -offset %APP_START% ^
  -o %APP_HEX% -Intel

if errorlevel 1 goto err

copy /y %APP_HEX% %APP_HEX_CRC% >nul
if errorlevel 1 goto err

echo.
echo CRC written back to app-only binary successfully.
echo CRC-patched HEX generated: %APP_HEX%
echo CRC-patched HEX copy generated: %APP_HEX_CRC%
exit /b 0

:err
echo CRC generation FAILED
exit /b 1
