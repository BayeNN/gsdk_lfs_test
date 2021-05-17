#ifndef _SL_BTCTRL_LINKLAYER_H_
#define _SL_BTCTRL_LINKLAYER_H_
#include "sl_status.h"
#include <stdint.h>

void sl_bt_controller_init(void);

void sl_btctrl_init(void);

/**
 * Allocate memory buffers for controller
 *
 * @param memsize size of memory to allocate
 * @returns number of memory buffers allocated
 */
uint32_t sl_btctrl_init_mem(uint32_t memsize);

void sli_btctrl_set_interrupt_priorities();

sl_status_t sl_btctrl_init_ll(void);

void sli_btctrl_set_address(uint8_t *address);

//Initialize memory objects used by LinkLayer
//In future these should be configured individually
sl_status_t sl_btctrl_init_basic(uint8_t connections, uint8_t adv_sets, uint8_t whitelist);

void sli_btctrl_events_init(void);

/**
 * Initialize and enable adaptive frequency hopping
 */
void sl_btctrl_init_afh(uint32_t flags);

/**
 * @brief Initilize periodic advertiser
 */
void sl_btctrl_init_periodic_adv();

/**
 * @brief Initilize periodic advertiser
 */
void sl_btctrl_init_periodic_scan();

/**
 * @brief Allocate memory for synchronized scanners
 *
 * @param num_scan Number of Periodic Scanners Allowed
 * @return SL_STATUS_OK if allocation was succesfull, failure reason otherwise
 */
sl_status_t sl_btctrl_alloc_periodic_scan(uint8_t num_scan);

/**
 * @brief Allocate memory for periodic advertisers
 *
 * @param num_adv Number of advertisers to allocate
 */
void sl_btctrl_alloc_periodic_adv(uint8_t num_adv);

/**
 * Call to enable the even connection scheduling algorithm.
 * This function should be called before link layer initialization.
 */
void sl_btctrl_enable_even_connsch();

#endif
