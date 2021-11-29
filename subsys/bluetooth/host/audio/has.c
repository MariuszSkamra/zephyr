/*
 * Copyright (c) 2022 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/byteorder.h>
#include <sys/check.h>

#include <device.h>
#include <init.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/audio/has.h>

#include "has_internal.h"

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_HAS)
#define LOG_MODULE_NAME bt_has
#include "common/log.h"

#define BT_HAS_CHRC_PROP_FEATURES	(BT_GATT_CHRC_READ)
#define BT_HAS_CHRC_PROP_CONTROL_POINT	(BT_GATT_CHRC_WRITE |		\
					 BT_GATT_CHRC_INDICATE |	\
					 BT_GATT_CHRC_NOTIFY)
#define BT_HAS_CHRC_PROP_ACTIVE_PRESET	(BT_GATT_CHRC_READ |		\
					 BT_GATT_CHRC_NOTIFY)

#if defined(CONFIG_BT_HAS)
static struct bt_has_server has_server;
#define IS_HAS_SERVER(has)	((has) == &has_server.has)
#else
#define IS_HAS_SERVER(has)	0
#endif /* BT_HAS */
#if defined(CONFIG_BT_HAS_CLIENT)
static struct bt_has_client has_client[CONFIG_BT_MAX_CONN];
#define IS_HAS_CLIENT(has)	!IS_HAS_SERVER(has)
#else
#define IS_HAS_CLIENT(has)	0
#endif /* BT_HAS_CLIENT */
#define IS_HANDLE_VALID(handle) (handle != 0x0000)
#define IS_PRESET_VALID(preset) ((preset)->index != BT_HAS_PRESET_INDEX_INVALID)

static struct bt_has_preset *server_preset_lookup(struct bt_has_server *server,
						  uint8_t index)
{
	return NULL;
}

static struct bt_has_preset *client_preset_lookup(struct bt_has_client *client,
						  uint8_t index)
{
	return NULL;
}

static int read_preset_rsp(struct bt_conn *conn, struct bt_has_server *server,
			   struct bt_has_preset *preset, uint8_t is_last)
{
	struct bt_has_cp_hdr *hdr;
	struct bt_has_cp_read_preset_rsp *rsp;
	const uint8_t slen = strlen(preset->name);
	uint8_t buf[sizeof(*hdr) + sizeof(*rsp) + slen];

	hdr = (void *)buf;
	hdr->opcode = BT_HAS_OP_READ_PRESET_RSP;
	rsp = (void *)hdr->data;
	rsp->is_last = is_last;
	rsp->index = preset->index;
	rsp->properties = preset->properties;
	strncpy(rsp->name, preset->name, slen);

	return bt_gatt_notify_uuid(conn, BT_UUID_HAS_CONTROL_POINT,
				   server->service.attrs, buf, sizeof(buf));
}

static ssize_t read_all_presets(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf, uint16_t len)
{
	struct bt_has_server *server = attr->user_data;
	struct bt_has_preset *preset;
	int err;

	preset = &server->has.preset_records[0].preset;
	if (!IS_PRESET_VALID(preset)) {
		/* preset list is empty */
		return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
	}

	for (int i = 1; i < ARRAY_SIZE(server->has.preset_records); i++) {
		struct bt_has_preset *next;
		
		next = &server->has.preset_records[i].preset;
		if (!IS_PRESET_VALID(next)) {
			break;
		}

		err = read_preset_rsp(conn, server, preset, 0x00);
		if (err != 0) {
			return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
		}

		preset = next;
	}

	err = read_preset_rsp(conn, server, preset, 0x01);
	if (err != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
	}

	return 0;
}

static ssize_t read_preset_by_index(struct bt_conn *conn,
				    const struct bt_gatt_attr *attr,
				    const void *buf, uint16_t len)
{
	const struct bt_has_cp_read_preset_by_index_req *req = buf;
	struct bt_has_server *server = attr->user_data;
	struct bt_has_preset *preset;
	int err;

	preset = server_preset_lookup(server, req->index);
	if (!preset) {
		return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
	}

	err = read_preset_rsp(conn, server, preset, 0x01);
	if (err != 0) {
		return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
	}

	return 0;
}

static ssize_t write_preset_name(struct bt_conn *conn,
				 const struct bt_gatt_attr *attr,
				 const void *buf, uint16_t len)
{
	const struct bt_has_cp_write_preset_name_req *req = buf;
	struct bt_has_server *server = attr->user_data;
	struct bt_has_preset *preset;

	if (len < (sizeof(*req) + BT_HAS_PRESET_NAME_MIN) || 
	    len > (sizeof(*req) + BT_HAS_PRESET_NAME_MAX)) {
		return BT_GATT_ERR(BT_HAS_ERR_INVALID_PARAM_LEN);
	}

	preset = server_preset_lookup(server, req->index);
	if (!preset) {
		return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
	}

	// TODO

	return 0;
}

static ssize_t set_active_preset(struct bt_conn *conn,
				 const struct bt_gatt_attr *attr,
				 const void *buf, uint16_t len, bool sync)
{
	return 0;
}

static ssize_t set_next_preset(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, const void *buf,
			       uint16_t len, bool sync)
{
	return 0;
}

static ssize_t set_prev_preset(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, const void *buf,
			       uint16_t len, bool sync)
{
	return 0;
}

static ssize_t read_features(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     uint16_t len, uint16_t offset)
{
	struct bt_has_server *has = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &has->has.features, 1);
}

static ssize_t write_control_point(struct bt_conn *conn,
				   const struct bt_gatt_attr *attr,
				   const void *buf, uint16_t len,
				   uint16_t offset, uint8_t flags)
{
	const struct bt_has_cp_hdr *hdr = buf;
	ssize_t result = len;
	int err;

	if (offset > 0) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len == 0 || buf == NULL) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	/* Decrement the length by the header size */
	len -= sizeof(*hdr);

	switch (hdr->opcode) {
		case BT_HAS_OP_READ_ALL_PRESETS:
			err = read_all_presets(conn, attr, hdr->data, len);
			break;

		case BT_HAS_OP_READ_PRESET_BY_INDEX:
			err = read_preset_by_index(conn, attr, hdr->data, len);
			break;

		case BT_HAS_OP_WRITE_PRESET_NAME:
			err = write_preset_name(conn, attr, hdr->data, len);
			break;

		case BT_HAS_OP_SET_ACTIVE_PRESET:
			err = set_active_preset(conn, attr, hdr->data, len,
						false);
			break;

		case BT_HAS_OP_SET_NEXT_PRESET:
			err = set_next_preset(conn, attr, hdr->data, len,
					      false);
			break;

		case BT_HAS_OP_SET_PREV_PRESET:
			err = set_prev_preset(conn, attr, hdr->data, len,
					      false);
			break;

#if defined(CONFIG_BT_HAS_HA_PRESET_SYNC_SUPPORT)
		case BT_HAS_OP_SET_ACTIVE_PRESET_SYNC:
			err = set_active_preset(conn, attr, hdr->data, len,
						true);
			break;

		case BT_HAS_OP_SET_NEXT_PRESET_SYNC:
			err = set_next_preset(conn, attr, hdr->data, len,
					      true);
			break;

		case BT_HAS_OP_SET_PREV_PRESET_SYNC:
			err = set_prev_preset(conn, attr, hdr->data, len,
					      true);
			break;
#else
		case BT_HAS_OP_SET_ACTIVE_PRESET_SYNC:
		case BT_HAS_OP_SET_NEXT_PRESET_SYNC:
		case BT_HAS_OP_SET_PREV_PRESET_SYNC:
			return BT_GATT_ERR(BT_HAS_ERR_PRESET_SYNC_NOT_SUPP); 
#endif /* BT_HAS_HA_PRESET_SYNC_SUPPORT */

	default:
		return BT_GATT_ERR(BT_HAS_ERR_INVALID_OPCODE);
	}

	if (err) {
		result = err;
	}

	return result;
}

static ssize_t read_active_preset(struct bt_conn *conn,
				  const struct bt_gatt_attr *attr, void *buf,
				  uint16_t len, uint16_t offset)
{
	struct bt_has_server *server = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &server->has.active_preset_index, 1);
}

/* Hearing Access Service GATT Attributes */
static struct bt_gatt_attr gatt_attr[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_HAS),
	BT_GATT_CHARACTERISTIC(BT_UUID_HAS_FEATURES,
			       BT_HAS_CHRC_PROP_FEATURES,
			       BT_GATT_PERM_READ_ENCRYPT,
			       read_features, NULL, &has_server),
	BT_GATT_CHARACTERISTIC(BT_UUID_HAS_CONTROL_POINT,
			       BT_HAS_CHRC_PROP_CONTROL_POINT,
			       BT_GATT_PERM_WRITE_ENCRYPT,
			       NULL, write_control_point, &has_server),
	BT_GATT_CHARACTERISTIC(BT_UUID_HAS_ACTIVE_PRESET,
			       BT_HAS_CHRC_PROP_ACTIVE_PRESET,
			       BT_GATT_PERM_READ_ENCRYPT,
			       read_active_preset, NULL, &has_server)};

int bt_has_register(struct bt_has_register_param *param, struct bt_has **has)
{
	static bool registered;
	int err;

	CHECKIF(param == NULL) {
		return -EINVAL;
	}

	if (registered) {
		*has = &has_server.has;
		return -EALREADY;
	}

	has_server.service = (struct bt_gatt_service)BT_GATT_SERVICE(gatt_attr);

	err = bt_gatt_service_register(&has_server.service);
	if (err != 0) {
		BT_DBG("HAS service register failed: %d", err);
		return err;
	}

	has_server.has.cb = param->cb;

	*has = &has_server.has;

	registered = true;

	return 0;
}

static inline struct bt_has_client *bt_has_client_by_conn(struct bt_conn *conn)
{
#if defined(CONFIG_BT_HAS_CLIENT)
	return &has_client[bt_conn_index(conn)];
#else
	return NULL;
#endif /* CONFIG_BT_HAS_CLIENT */
}

static uint8_t client_preset_active_get_cb(struct bt_conn *conn, uint8_t err,
					   struct bt_gatt_read_params *params,
					   const void *data, uint16_t length)
{
	struct bt_has_client *client = bt_has_client_by_conn(conn);

	client->busy = false;

	return BT_GATT_ITER_STOP;
}

static int client_preset_active_get(struct bt_has_client *client)
{
	int err;

	CHECKIF(client->conn == NULL) {
		return -ENOTCONN;
	}

	if (!IS_HANDLE_VALID(client->active_preset_handle)) {
		return -ENOTSUP;
	}

	if (client->busy) {
		return -EBUSY;
	}

	client->read_params.func = client_preset_active_get_cb;
	client->read_params.handle_count = 1;
	client->read_params.single.handle = client->active_preset_handle;
	client->read_params.single.offset = 0U;

	err = bt_gatt_read(client->conn, &client->read_params);
	if (err) {
		return err;
	}

	client->busy = true;

	return 0;
}

static void client_preset_active_set_cb(struct bt_conn *conn, uint8_t err,
				        struct bt_gatt_write_params *params)
{
	struct bt_has_client *client = bt_has_client_by_conn(conn);

	client->busy = false;
}

static int client_control_point_write(struct bt_has_client *client,
				      bt_gatt_write_func_t func,
				      const void *data, uint16_t length)
{
	int err;

	CHECKIF(client->conn == NULL) {
		return -ENOTCONN;
	}

	if (!IS_HANDLE_VALID(client->control_point_handle)) {
		return -EPERM;
	}

	if (client->busy) {
		return -EBUSY;
	}

	client->write_params.func = func;
	client->write_params.offset = 0U;
	client->write_params.data = data;
	client->write_params.length = length;

	err = bt_gatt_write(client->conn, &client->write_params);
	if (err == 0) {
		return err;
	}

	client->busy = true;

	return 0;
}

static int client_preset_active_set(struct bt_has_client *client,
				    uint8_t index)
{
	struct bt_has_cp_hdr *hdr;
	struct bt_has_cp_set_active_preset_req *req;
	uint8_t buf[sizeof(*hdr) + sizeof(*req)];

	hdr = (void *)buf;
	hdr->opcode = BT_HAS_OP_SET_ACTIVE_PRESET;
	req = (void *)hdr->data;
	req->index = index;

	return client_control_point_write(client, client_preset_active_set_cb,
					  buf, sizeof(buf));
}

static int client_preset_active_set_next(struct bt_has_client *client)
{
	struct bt_has_cp_hdr hdr = {
		.opcode = BT_HAS_OP_SET_NEXT_PRESET,
	};

	return client_control_point_write(client, client_preset_active_set_cb,
					  &hdr, sizeof(hdr));
}

static int client_preset_active_set_prev(struct bt_has_client *client)
{
	struct bt_has_cp_hdr hdr = {
		.opcode = BT_HAS_OP_SET_PREV_PRESET,
	};

	return client_control_point_write(client, client_preset_active_set_cb,
					  &hdr, sizeof(hdr));
}

static void client_preset_get_cb(struct bt_conn *conn, uint8_t err,
				 struct bt_gatt_write_params *params)
{
	struct bt_has_client *client = bt_has_client_by_conn(conn);

	client->busy = false;
}

static int client_preset_get(struct bt_has_client *client, uint8_t index)
{
	struct bt_has_cp_hdr *hdr;
	struct bt_has_cp_read_preset_by_index_req *req;
	uint8_t buf[sizeof(*hdr) + sizeof(*req)];

	hdr = (void *)buf;
	hdr->opcode = BT_HAS_OP_READ_PRESET_BY_INDEX;
	req = (void *)hdr->data;
	req->index = index;

	return client_control_point_write(client, client_preset_get_cb, buf,
					  sizeof(buf));
}

static int client_preset_name_set(struct bt_has_client *client, uint8_t index,
				  const char *name, ssize_t len)
{
	struct bt_has_cp_hdr *hdr;
	struct bt_has_cp_write_preset_name_req *req;
	uint8_t buf[sizeof(*hdr) + sizeof(*req) + len];

	hdr = (void *)buf;
	hdr->opcode = BT_HAS_OP_WRITE_PRESET_NAME;
	req = (void *)hdr->data;
	req->index = index;
	strncpy(req->name, name, len);

	return client_control_point_write(client, client_preset_active_set_cb,
					  buf, sizeof(buf));
}

static int client_preset_list_get(struct bt_has_client *client)
{
	struct bt_has_cp_hdr hdr = {
		.opcode = BT_HAS_OP_SET_PREV_PRESET,
	};

	return client_control_point_write(client, client_preset_active_set_cb,
					  &hdr, sizeof(hdr));
}

static int server_preset_active_get(struct bt_has_server *server)
{
	int err;
	
	if (!server->has.cb || !server->has.cb->active_preset) {
		return -EINVAL;
	}

	err = server->has.active_preset_index ? 0 : -ENODATA;

	// /* TODO: call it from thread */
	server->has.cb->active_preset(&server->has, err,
				      server->has.active_preset_index);

	return 0;
}

static int server_preset_active_set(struct bt_has_server *server,
				    uint8_t index)
{
	

	return 0;
}

static int server_preset_active_set_next(struct bt_has_server *server)
{
	// TODO

	return 0;
}

static int server_preset_active_set_prev(struct bt_has_server *server)
{
	// TODO

	return 0;
}

static int server_preset_get(struct bt_has_server *server, uint8_t index)
{
	// TODO

	return 0;
}

static int server_preset_name_set(struct bt_has_server *server, uint8_t index,
				  const char *name)
{
	// TODO

	return 0;
}

static int server_preset_list_get(struct bt_has_server *server)
{
	// TODO

	return 0;
}

int bt_has_preset_active_get(struct bt_has *has)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_active_get(BT_HAS_CLIENT(has));
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_active_get(BT_HAS_SERVER(has));
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_active_set(struct bt_has *has, uint8_t index)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_active_set(BT_HAS_CLIENT(has), index);
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_active_set(BT_HAS_SERVER(has), index);
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_active_set_next(struct bt_has *has)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_active_set_next(BT_HAS_CLIENT(has));
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_active_set_next(BT_HAS_SERVER(has));
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_active_set_prev(struct bt_has *has)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_active_set_prev(BT_HAS_CLIENT(has));
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_active_set_prev(BT_HAS_SERVER(has));
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_get(struct bt_has *has, uint8_t index)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_get(BT_HAS_CLIENT(has), index);
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_get(BT_HAS_SERVER(has), index);
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_name_set(struct bt_has *has, uint8_t index, const char *name)
{
	size_t len;

	CHECKIF(has == NULL || name == NULL) {
		return -EINVAL;
	}

	len = strlen(name);
	if (len < BT_HAS_PRESET_NAME_MIN || len > BT_HAS_PRESET_NAME_MAX) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_name_set(BT_HAS_CLIENT(has), index, name,
					      len);
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_name_set(BT_HAS_SERVER(has), index, name);
	}

	return -EOPNOTSUPP;
}

int bt_has_preset_list_get(struct bt_has *has)
{
	CHECKIF(has == NULL) {
		return -EINVAL;
	}

	if (IS_HAS_CLIENT(has)) {
		return client_preset_list_get(BT_HAS_CLIENT(has));
	}

	if (IS_HAS_SERVER(has)) {
		return server_preset_list_get(BT_HAS_SERVER(has));
	}

	return -EOPNOTSUPP;
}
