/** @file
 *  @brief Internal APIs for Bluetooth Hearing Access Profile.
 */

/*
 * Copyright (c) 2021 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <bluetooth/audio/has.h>
#include <bluetooth/bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_HAS_PRESET_NAME_MIN			1
#define BT_HAS_PRESET_NAME_MAX			40

#define BT_HAS_PRESET_INDEX_INVALID		0x00

#define BT_HAS_OP_READ_ALL_PRESETS		0x00
#define BT_HAS_OP_READ_PRESET_BY_INDEX		0x01
#define BT_HAS_OP_READ_PRESET_RSP		0x02
#define BT_HAS_OP_PRESET_CHANGED		0x03
#define BT_HAS_OP_WRITE_PRESET_NAME		0x04
#define BT_HAS_OP_SET_ACTIVE_PRESET		0x05
#define BT_HAS_OP_SET_NEXT_PRESET		0x06
#define BT_HAS_OP_SET_PREV_PRESET		0x07
#define BT_HAS_OP_SET_ACTIVE_PRESET_SYNC	0x08
#define BT_HAS_OP_SET_NEXT_PRESET_SYNC		0x09
#define BT_HAS_OP_SET_PREV_PRESET_SYNC		0x0a

#define BT_HAS_ERR_INVALID_OPCODE		0x80
#define BT_HAS_ERR_WRITE_NAME_NOT_ALLOWED	0x81
#define BT_HAS_ERR_PRESET_SYNC_NOT_SUPP		0x82
#define BT_HAS_ERR_OPERATION_NOT_POSSIBLE	0x83
#define BT_HAS_ERR_INVALID_PARAM_LEN		0x84

struct bt_has_cp_hdr {
	uint8_t opcode;
	uint8_t data[0];
} __packed;

struct bt_has_cp_read_preset_by_index_req {
	uint8_t index;
	uint8_t name[0];
} __packed;

struct bt_has_cp_write_preset_name_req {
	uint8_t index;
	uint8_t name[0];
} __packed;

struct bt_has_cp_set_active_preset_req {
	uint8_t index;
} __packed;

struct bt_has_cp_read_preset_rsp {
	uint8_t is_last;
	uint8_t index;
	uint8_t properties;
	uint8_t name[0];
} __packed;

union _bt_has_preset {
	struct bt_has_preset preset;
	struct {
		uint8_t index;
		uint8_t properties;
		char name[BT_HAS_PRESET_NAME_MAX + 1];
	};
};

struct bt_has {
	/** Registered application callbacks */
	struct bt_has_cb *cb;

	/** Preset record list */
	union _bt_has_preset preset_records[BT_HAS_PRESET_COUNT];

	/** Hearing Aid Fetures value */
	uint8_t features;

	/** Active Preset Index */
	uint8_t active_preset_index;
};

/** @brief Hearing Access Profile server internal representation */
struct bt_has_server {
	/** Common profile reference object */
	struct bt_has has;

	/** GATT Service object */
	struct bt_gatt_service service;
};

/** @def BT_HAS_SERVER(_has)
 *  @brief Helper macro getting container object of type bt_has_server
 *  address having the same container has member address as object in question.
 *
 *  @param _has Address of object of bt_has type
 *
 *  @return Address of in memory bt_has_server object type containing
 *          the address of in question object.
 */
#define BT_HAS_SERVER(_has) CONTAINER_OF(_has, struct bt_has_server, has)

/** @brief Hearing Access Profile client internal representation */
struct bt_has_client {
	/** Common profile reference object */
	struct bt_has has;

	/** Profile connection reference */
	struct bt_conn *conn;

	uint16_t hearing_aid_features_handle;
	uint16_t control_point_handle;
	uint16_t active_preset_handle;

	bool busy;
	union {
		struct bt_gatt_read_params read_params;
		struct bt_gatt_write_params write_params;
	};
};

/** @def BT_HAS_CLIENT(_has)
 *  @brief Helper macro getting container object of type bt_has_client
 *  address having the same container has member address as object in question.
 *
 *  @param _has Address of object of bt_has type
 *
 *  @return Address of in memory bt_has_client object type containing
 *          the address of in question object.
 */
#define BT_HAS_CLIENT(_has) CONTAINER_OF(_has, struct bt_has_client, has)

#ifdef __cplusplus
}
#endif
