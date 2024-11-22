/*!
*	@File: crc32.h
*	@Brief:
*	this file is used to declare the crc32 function used when communicating with the cpx3
*
*/

#ifndef _CRC32_H_
#define _CRC32_H_

#define CRC_POLYNOMIAL             0x04C11DB7
#define CRC_INITIAL_REMAINDER      0x00000000
#define CRC_FINAL_XOR_VALUE        0x00000000
#define CRC_WIDTH                  32
#include <stdint.h>

namespace CRC32 {

/*! \fn DWORD crc32_init(void)
*	\brief initialises crc32 lookuptable (crc32_table[256])
*	\return success
*/
uint32_t crc32_init(void);

/*! \fn unsigned __int32 crc32_calc(unsigned __int8 *frame, unsigned __int16 frame_length)
*	\brief calculates crc32
*	@param[in] unsigned __int8 *frame - pointer to bytes sequence to calculate crc32
*	@param[in] unsigned __int16 frame_length - size of bytes sequence
*	\returns 32bit crc
*/
uint32_t crc32_calc(const uint8_t *frame, uint16_t frame_length);

/*! \fn bool crc32_check(unsigned __int8 *frame, unsigned __int16 frame_length, unsigned __int32 crc32_to_check);
*	\brief checks a crc32
*	@param[in] unsigned __int8 *frame - pointer to bytes sequence to calculate crc32
*	@param[in] unsigned __int16 frame_length - size of bytes sequence
*	@param[in] unsigned __int32 crc32_to_check - crc for comparison
*	\returns crc check correct (true)or not (false)
*/
bool crc32_check(const uint8_t *frame, uint16_t frame_length, uint32_t crc32_to_check);

}  // namespace CRC32


#endif
