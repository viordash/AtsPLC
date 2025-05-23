/**
 * @file
 * SNTP client API
 */

/*
 * Copyright (c) 2007-2009 Frédéric Bernon, Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Frédéric Bernon, Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_APPS_SNTP_H
#define LWIP_HDR_APPS_SNTP_H

#include "lwip/apps/sntp_opts.h"
#include "lwip/ip_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SNTP operating modes: default is to poll using unicast.
   The mode has to be set before calling sntp_init(). */
#define SNTP_OPMODE_POLL            0
#define SNTP_OPMODE_LISTENONLY      1
void sntp_setoperatingmode(u8_t operating_mode);
u8_t sntp_getoperatingmode(void);

void sntp_init(void);
void sntp_stop(void);
u8_t sntp_enabled(void);

void sntp_setserver(u8_t idx, const ip_addr_t *addr);
const ip_addr_t* sntp_getserver(u8_t idx);

#if SNTP_MONITOR_SERVER_REACHABILITY
u8_t sntp_getreachability(u8_t idx);
#endif /* SNTP_MONITOR_SERVER_REACHABILITY */

#if SNTP_SERVER_DNS
void sntp_setservername(u8_t idx, const char *server);
const char *sntp_getservername(u8_t idx);
#endif /* SNTP_SERVER_DNS */

#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
void sntp_servermode_dhcp(int set_servers_from_dhcp);
#else /* SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6 */
#define sntp_servermode_dhcp(x)
#endif /* SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6 */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_APPS_SNTP_H */


// Copyright 2015-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __SNTP_H__
#define __SNTP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The time update takes place in the sntp_sync_time() function.
 * The user has the ability to redefine this function in order
 * to re-define its functionality. This function has two time update modes,
 * which can be set via the sntp_set_sync_mode() function.
 * Two modes are available:
 * - the first is an immediate update when receiving time from the sntp server,
 * - the second is a smooth time update (if the time error is no more than 35 minutes,
 *   and an immediate update if the error is more than 35 minutes).
 *
 * To receive notification of time synchronization,
 * you can use the callback function or get the synchronization status
 * via the sntp_get_sync_status() function.
 *
 * To determine the time synchronization time on the device, you can use:
 * 1) sntp_set_time_sync_notification_cb() function to set the callback function,
 *    which is convenient to use to receive notification of the update time.
 * 2) sntp_get_sync_status() function for getting time synchronization status.
 *    After the time synchronization is completed, the status will be
 *    SNTP_SYNC_STATUS_COMPLETED, after, it will be reseted to SNTP_SYNC_STATUS_RESET
 *    to wait for the next sync cycle.
 */

/// SNTP time update mode
typedef enum {
    SNTP_SYNC_MODE_IMMED,   /*!< Update system time immediately when receiving a response from the SNTP server. */
    SNTP_SYNC_MODE_SMOOTH,  /*!< Smooth time updating. Time error is gradually reduced using adjtime function. If the difference between SNTP response time and system time is large (more than 35 minutes) then update immediately. */
} sntp_sync_mode_t;

/// SNTP sync status
typedef enum {
    SNTP_SYNC_STATUS_RESET,         // Reset status.
    SNTP_SYNC_STATUS_COMPLETED,     // Time is synchronized.
    SNTP_SYNC_STATUS_IN_PROGRESS,   // Smooth time sync in progress.
} sntp_sync_status_t;

/**
 * @brief SNTP callback function for notifying about time sync event
 *
 * @param tv Time received from SNTP server.
 */
typedef void (*sntp_sync_time_cb_t) (struct timeval *tv);

/**
 * @brief This function updates the system time.
 *
 * This is a weak-linked function. It is possible to replace all SNTP update functionality
 * by placing a sntp_sync_time() function in the app firmware source.
 * If the default implementation is used, calling sntp_set_sync_mode() allows
 * the time synchronization mode to be changed to instant or smooth.
 * If a callback function is registered via sntp_set_time_sync_notification_cb(),
 * it will be called following time synchronization.
 *
 * @param tv Time received from SNTP server.
 */
void sntp_sync_time(struct timeval *tv);

/**
 * @brief Set the sync mode
 *
 * Allowable two mode: SNTP_SYNC_MODE_IMMED and SNTP_SYNC_MODE_SMOOTH.
 * @param sync_mode Sync mode.
 */
void sntp_set_sync_mode(sntp_sync_mode_t sync_mode);

/**
 * @brief Get set sync mode
 *
 * @return  SNTP_SYNC_MODE_IMMED: Update time immediately.
 *          SNTP_SYNC_MODE_SMOOTH: Smooth time updating.
 */
sntp_sync_mode_t sntp_get_sync_mode(void);

/**
 * @brief Get status of time sync
 *
 * After the update is completed, the status will be returned as SNTP_SYNC_STATUS_COMPLETED.
 * After that, the status will be reset to SNTP_SYNC_STATUS_RESET.
 * If the update operation is not completed yet, the status will be SNTP_SYNC_STATUS_RESET.
 * If a smooth mode was chosen and the synchronization is still continuing (adjtime works), then it will be SNTP_SYNC_STATUS_IN_PROGRESS.
 *
 * @return  SNTP_SYNC_STATUS_RESET: Reset status.
 *          SNTP_SYNC_STATUS_COMPLETED: Time is synchronized.
 *          SNTP_SYNC_STATUS_IN_PROGRESS: Smooth time sync in progress.
 */
sntp_sync_status_t sntp_get_sync_status(void);

/**
 * @brief Set status of time sync
 *
 * @param sync_status status of time sync (see sntp_sync_status_t)
 */
void sntp_set_sync_status(sntp_sync_status_t sync_status);

/**
 * @brief Set a callback function for time synchronization notification
 *
 * @param callback a callback function
 */
void sntp_set_time_sync_notification_cb(sntp_sync_time_cb_t callback);

/**
 * @brief Set the sync interval of SNTP operation
 *
 * Note: SNTPv4 RFC 4330 enforces a minimum sync interval of 15 seconds.
 * This sync interval will be used in the next attempt update time throught SNTP.
 * To apply the new sync interval call the sntp_restart() function,
 * otherwise, it will be applied after the last interval expired.
 *
 * @param interval_ms   The sync interval in ms. It cannot be lower than 15 seconds, otherwise 15 seconds will be set.
 */
void sntp_set_sync_interval(uint32_t interval_ms);

/**
 * @brief Get the sync interval of SNTP operation
 *
 * @return  the sync interval
 */
uint32_t sntp_get_sync_interval(void);

/**
 * @brief Restart SNTP
 *
 * @return True  - Restart
 *         False - SNTP was not initialized yet
 */
bool sntp_restart(void);

#ifdef __cplusplus
}
#endif

#endif // __SNTP_H__
