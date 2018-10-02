/*
 * Copyright (c) 2014 Red Hat, Inc.
 *
 * This file is licensed to you under your choice of the GNU Lesser
 * General Public License, version 2.1 or any later version (LGPLv2.1 or
 * later), or the Apache License 2.0.
 */

/*
 * APIs for both libtcmu users and tcmu-runner plugins to use.
 */

#ifndef __LIBTCMU_COMMON_H
#define __LIBTCMU_COMMON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tcmu_device;
struct tgt_port;
struct tcmulib_cmd;

/*
 * TCMU return status codes
 */
enum {
	TCMU_STS_ASYNC_HANDLED = -2,
	TCMU_STS_NOT_HANDLED = -1,
	TCMU_STS_OK = 0,
	TCMU_STS_NO_RESOURCE,
	/* handler has setup sense. */
	TCMU_STS_PASSTHROUGH_ERR,
	TCMU_STS_BUSY,
	TCMU_STS_WR_ERR,
	TCMU_STS_RD_ERR,
	TCMU_STS_MISCOMPARE,
	TCMU_STS_INVALID_CMD,
	TCMU_STS_INVALID_CDB,
	TCMU_STS_INVALID_PARAM_LIST,
	TCMU_STS_INVALID_PARAM_LIST_LEN,
	TCMU_STS_TIMEOUT,
	TCMU_STS_FENCED,
	TCMU_STS_HW_ERR,
	TCMU_STS_RANGE,
	TCMU_STS_FRMT_IN_PROGRESS,
	TCMU_STS_CAPACITY_CHANGED,
	TCMU_STS_NOTSUPP_SAVE_PARAMS,
	TCMU_STS_WR_ERR_INCOMPAT_FRMT,
	TCMU_STS_TRANSITION,
	TCMU_STS_IMPL_TRANSITION_ERR,
	TCMU_STS_EXPL_TRANSITION_ERR,
	TCMU_STS_NO_LOCK_HOLDERS,
	/* xcopy specific errors */
	TCMU_STS_NOTSUPP_SEG_DESC_TYPE,
	TCMU_STS_NOTSUPP_TGT_DESC_TYPE,
	TCMU_STS_CP_TGT_DEV_NOTCONN,
	TCMU_STS_INVALID_CP_TGT_DEV_TYPE,
};

#define SENSE_BUFFERSIZE 96

#define CFGFS_ROOT "/sys/kernel/config/target"
#define CFGFS_CORE CFGFS_ROOT"/core"

#define CFGFS_TARGET_MOD "/sys/module/target_core_user"
#define CFGFS_MOD_PARAM CFGFS_TARGET_MOD"/parameters"

/* Temporarily limit this to 32M */
#define VPD_MAX_UNMAP_LBA_COUNT            (32 * 1024 * 1024)
#define VPD_MAX_UNMAP_BLOCK_DESC_COUNT     0x04
/* Temporarily limit this is 0x1 */
#define MAX_CAW_LENGTH                     0x01

#define max(a, b) ({			\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	(void) (&_a == &_b);		\
	_a < _b ? _b : _a; })

#define min(a, b) ({			\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	(void) (&_a == &_b);		\
	_a < _b ? _a : _b; })

#define round_up(a, b) ({		\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	((_a + (_b - 1)) / _b) * _b; })

#define round_down(a, b) ({		\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	(_a - (_a % _b)); })

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define VPD_MAX_WRITE_SAME_LENGTH 0xFFFFFFFF

typedef void (*cmd_done_t)(struct tcmu_device *, struct tcmulib_cmd *, int);

struct tcmulib_cmd {
	uint16_t cmd_id;
	uint8_t *cdb;
	struct iovec *iovec;
	size_t iov_cnt;
	uint8_t sense_buf[SENSE_BUFFERSIZE];

	/*
	 * this is mostly used by compound operations as such operations
	 * need to carry some state around for multiple commands.
	 */
	void *cmdstate;

	/* callback to finish/continue command processing */
	cmd_done_t done;
};

/* Set/Get methods for the opaque tcmu_device */
void *tcmu_get_dev_private(struct tcmu_device *dev);
void tcmu_set_dev_private(struct tcmu_device *dev, void *priv);
void *tcmu_get_daemon_dev_private(struct tcmu_device *dev);
void tcmu_set_daemon_dev_private(struct tcmu_device *dev, void *priv);
int tcmu_get_dev_fd(struct tcmu_device *dev);
char *tcmu_get_dev_cfgstring(struct tcmu_device *dev);
void tcmu_set_dev_num_lbas(struct tcmu_device *dev, uint64_t num_lbas);
uint64_t tcmu_get_dev_num_lbas(struct tcmu_device *dev);
int tcmu_update_num_lbas(struct tcmu_device *dev, uint64_t new_size);
void tcmu_set_dev_block_size(struct tcmu_device *dev, uint32_t block_size);
uint32_t tcmu_get_dev_block_size(struct tcmu_device *dev);
void tcmu_set_dev_max_xfer_len(struct tcmu_device *dev, uint32_t len);
uint32_t tcmu_get_dev_max_xfer_len(struct tcmu_device *dev);
void tcmu_set_dev_opt_unmap_gran(struct tcmu_device *dev, uint32_t len);
uint32_t tcmu_get_dev_opt_unmap_gran(struct tcmu_device *dev);
void tcmu_set_dev_unmap_gran_align(struct tcmu_device *dev, uint32_t len);
uint32_t tcmu_get_dev_unmap_gran_align(struct tcmu_device *dev);
void tcmu_set_dev_write_cache_enabled(struct tcmu_device *dev, bool enabled);
bool tcmu_get_dev_write_cache_enabled(struct tcmu_device *dev);
void tcmu_set_dev_solid_state_media(struct tcmu_device *dev, bool solid_state);
bool tcmu_get_dev_solid_state_media(struct tcmu_device *dev);
struct tcmulib_handler *tcmu_get_dev_handler(struct tcmu_device *dev);
struct tcmur_handler *tcmu_get_runner_handler(struct tcmu_device *dev);
void tcmu_block_device(struct tcmu_device *dev);
void tcmu_unblock_device(struct tcmu_device *dev);
void tcmu_flush_device(struct tcmu_device *dev);

/* Helper routines for processing commands */
char *tcmu_get_cfgfs_str(const char *path);
int tcmu_set_cfgfs_str(const char *path, const char *val, int val_len);
int tcmu_get_cfgfs_int(const char *path);
int tcmu_set_cfgfs_ul(const char *path, unsigned long val);
int tcmu_get_attribute(struct tcmu_device *dev, const char *name);
bool tcmu_cfgfs_file_is_supported(struct tcmu_device *dev, const char *name);
int tcmu_exec_cfgfs_dev_action(struct tcmu_device *dev, const char *name,
			       unsigned long val);
int tcmu_set_dev_size(struct tcmu_device *dev);
long long tcmu_get_dev_size(struct tcmu_device *dev);
char *tcmu_get_wwn(struct tcmu_device *dev);
int tcmu_set_control(struct tcmu_device *dev, const char *key, unsigned long val);
void tcmu_reset_netlink(void);
void tcmu_block_netlink(void);
void tcmu_unblock_netlink(void);
int tcmu_get_cdb_length(uint8_t *cdb);
uint64_t tcmu_get_lba(uint8_t *cdb);
uint32_t tcmu_get_xfer_length(uint8_t *cdb);
off_t tcmu_compare_with_iovec(void *mem, struct iovec *iovec, size_t size);
size_t tcmu_seek_in_iovec(struct iovec *iovec, size_t count);
void tcmu_seek_in_cmd_iovec(struct tcmulib_cmd *cmd, size_t count);
void tcmu_zero_iovec(struct iovec *iovec, size_t iov_cnt);
size_t tcmu_memcpy_into_iovec(struct iovec *iovec, size_t iov_cnt, void *src, size_t len);
size_t tcmu_memcpy_from_iovec(void *dest, size_t len, struct iovec *iovec, size_t iov_cnt);
size_t tcmu_iovec_length(struct iovec *iovec, size_t iov_cnt);
bool char_to_hex(unsigned char *val, char c);

/* Basic implementations of mandatory SCSI commands */
int tcmu_set_sense_data(uint8_t *sense_buf, uint8_t key, uint16_t asc_ascq);
void tcmu_set_sense_info(uint8_t *sense_buf, uint32_t info);
void tcmu_set_sense_key_specific_info(uint8_t *sense_buf, uint16_t info);
void __tcmu_set_sense_data(uint8_t *sense_buf, uint8_t key, uint16_t asc_ascq);
int tcmu_emulate_inquiry(struct tcmu_device *dev, struct tgt_port *port, uint8_t *cdb, struct iovec *iovec, size_t iov_cnt);
int tcmu_emulate_start_stop(struct tcmu_device *dev, uint8_t *cdb);
int tcmu_emulate_test_unit_ready(uint8_t *cdb, struct iovec *iovec, size_t iov_cnt);
int tcmu_emulate_read_capacity_10(uint64_t num_lbas, uint32_t block_size, uint8_t *cdb,
				  struct iovec *iovec, size_t iov_cnt);
int tcmu_emulate_read_capacity_16(uint64_t num_lbas, uint32_t block_size, uint8_t *cdb,
				  struct iovec *iovec, size_t iov_cnt);
int tcmu_emulate_mode_sense(struct tcmu_device *dev, uint8_t *cdb,
			    struct iovec *iovec, size_t iov_cnt);
int tcmu_emulate_mode_select(struct tcmu_device *dev, uint8_t *cdb,
			     struct iovec *iovec, size_t iov_cnt);
/* SCSI helpers */
void tcmu_print_cdb_info(struct tcmu_device *dev, const struct tcmulib_cmd *cmd, const char *info);

#ifdef __cplusplus
}
#endif

#endif
