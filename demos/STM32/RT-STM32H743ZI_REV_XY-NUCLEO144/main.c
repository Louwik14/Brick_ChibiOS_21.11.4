/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "rt_test_root.h"
#include "oslib_test_root.h"

#define SDRAM_BASE_ADDRESS  0xC0000000U
#define SDRAM_SIZE_BYTES    0x02000000U

static bool sdram_test(uint32_t *fail_address, uint16_t *expected,
                       uint16_t *actual) {
  static const uint16_t patterns[] = {
    0x0000U, 0xFFFFU, 0xAAAAU, 0x5555U
  };
  const uint32_t addresses[] = {
    SDRAM_BASE_ADDRESS,
    SDRAM_BASE_ADDRESS + (SDRAM_SIZE_BYTES / 2U),
    SDRAM_BASE_ADDRESS + SDRAM_SIZE_BYTES - sizeof(uint16_t)
  };

  for (size_t i = 0; i < (sizeof(addresses) / sizeof(addresses[0])); ++i) {
    volatile uint16_t *ptr = (volatile uint16_t *)addresses[i];

    for (size_t p = 0; p < (sizeof(patterns) / sizeof(patterns[0])); ++p) {
      *ptr = patterns[p];
      *actual = *ptr;
      if (*actual != patterns[p]) {
        *fail_address = addresses[i];
        *expected = patterns[p];
        return false;
      }
    }

    *ptr = (uint16_t)(addresses[i] >> 1);
    *actual = *ptr;
    if (*actual != (uint16_t)(addresses[i] >> 1)) {
      *fail_address = addresses[i];
      *expected = (uint16_t)(addresses[i] >> 1);
      return false;
    }
  }

  return true;
}

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */

/*
 * Application entry point.
 */
int main(void) {

  uint32_t fail_address = 0U;
  uint16_t expected = 0U;
  uint16_t actual = 0U;

  halInit();
  chSysInit();

  sdStart(&SD1, NULL);

  if (sdram_test(&fail_address, &expected, &actual)) {
    chprintf((BaseSequentialStream *)&SD1, "SDRAM OK\r\n");
  } else {
    chprintf((BaseSequentialStream *)&SD1,
             "SDRAM FAIL @ address 0x%08lx (read/write mismatch)\r\n",
             (uint32_t)fail_address);
  }

  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
