/* 
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: CC-BY-ND-4.0
 */

/** @file bf_rt_server.h
 *
 *  @brief Contains BF-RT gRPC server APIs
 */
#ifndef _BF_RT_SERVER_H
#define _BF_RT_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start server and bind to default address (0.0.0.0:50052)
 *
 * @param[in] program_name      P4 program name to use
 * @param[in] local_only        Specifies if server should bind to local
 *                              loopback interface only
 */
void bf_rt_grpc_server_run(const char *program_name, bool local_only);

/**
 * @brief Start server and bind specified address
 *
 * @param[in] server_address    Server address to bind to represented as string
 *                              in format ip:port
 */
void bf_rt_grpc_server_run_with_addr(const char *server_address);

#ifdef __cplusplus
}
#endif

#endif  // _BF_RT_SERVER_H
