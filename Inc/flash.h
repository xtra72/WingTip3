#ifndef FLASH_H__
#define FLASH_H__

RET_VALUE   FLASH_erase(uint32_t startAddress, uint32_t size);
RET_VALUE   FLASH_write(uint32_t address, void* data, uint32_t size);

#endif
