/** @file
 *  @brief Bluetooth Mesh Generic Models.
 */

/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensor_srv.h"

#define OP_SENSOR_DESC_GET		BT_MESH_MODEL_OP_2(0x82, 0x30)
#define OP_SENSOR_DESC_STATUS		BT_MESH_MODEL_OP_1(0x51)
#define OP_SENSOR_GET			BT_MESH_MODEL_OP_2(0x82, 0x31)
#define OP_SENSOR_STATUS		BT_MESH_MODEL_OP_1(0x52)
#define OP_SENSOR_COL_GET		BT_MESH_MODEL_OP_2(0x82, 0x32)
#define OP_SENSOR_COL_STATUS		BT_MESH_MODEL_OP_1(0x53)
#define OP_SENSOR_SERIES_GET		BT_MESH_MODEL_OP_2(0x82, 0x33)
#define OP_SENSOR_SERIES_STATUS		BT_MESH_MODEL_OP_1(0x54)
#define OP_SENSOR_CAD_GET		BT_MESH_MODEL_OP_2(0x82, 0x34)
#define OP_SENSOR_CAD_SET		BT_MESH_MODEL_OP_1(0x55)
#define OP_SENSOR_CAD_SET_UNACK		BT_MESH_MODEL_OP_1(0x56)
#define OP_SENSOR_CAD_STATUS		BT_MESH_MODEL_OP_1(0x57)
#define OP_SENSOR_SETTINGS_GET		BT_MESH_MODEL_OP_2(0x82, 0x35)
#define OP_SENSOR_SETTINGS_STATUS	BT_MESH_MODEL_OP_1(0x58)
#define OP_SENSOR_SETTING_GET		BT_MESH_MODEL_OP_2(0x82, 0x36)
#define OP_SENSOR_SETTING_SET		BT_MESH_MODEL_OP_1(0x59)
#define OP_SENSOR_SETTING_SET_UNACK	BT_MESH_MODEL_OP_1(0x5a)
#define OP_SENSOR_SETTING_STATUS	BT_MESH_MODEL_OP_1(0x5b)

static const struct bt_mesh_sensor *
sensor_lookup_id(struct _bt_mesh_sensor_srv *srv, u16_t id)
{
	struct bt_mesh_sensor *sensor = srv->sensors;
	int i;

	for (i = 0; i < srv->sensor_cnt; i++) {
		if (sensor->id == id) {
			return sensor;
		}

		sensor++;
	}

	return NULL;
}

static void sensor_desc_status_prepare(const struct bt_mesh_sensor *sensor,
				       struct net_buf_simple *msg)
{
	u32_t u32_val;

	net_buf_simple_add_le16(msg, sensor->id);

	u32_val = sensor->desc.tolerance_pos |
		  (u32_t)sensor->desc.tolerance_neg << 12 |
		  (u32_t)sensor->desc.sampling_func << 24;
	net_buf_simple_add_le32(&msg, u32_val);
	net_buf_simple_add_u8(&msg, sensor->desc.period);
	net_buf_simple_add_u8(&msg, sensor->desc.interval);
}

static void sensor_desc_get(struct bt_mesh_model *model,
			    struct bt_mesh_msg_ctx *ctx,
			    struct net_buf_simple *buf)
{
	struct _bt_mesh_sensor_srv *srv = model->user_data;
	const struct bt_mesh_sensor *sensor;
	struct net_buf_simple *msg;

	if (buf->len) {
		u16_t id = net_buf_simple_pull_le16(buf);

		msg = NET_BUF_SIMPLE(1 + 8 + 4);

		bt_mesh_model_msg_init(&msg, OP_SENSOR_DESC_STATUS);

		sensor = sensor_lookup_id(srv, id);
		if (sensor) {
			sensor_desc_status_prepare(sensor, msg);
		} else {
			/*
			 * When requested Property ID does not exist on the
			 * element, the Descriptor field shall contain the
			 * requested Property ID value and the other fields of
			 * the Sensor Descriptor state shall be omitted.
			 */
			net_buf_simple_add_le16(msg, id);
		}
	} else {
		int i;

		msg = NET_BUF_SIMPLE(1 + (8 * srv->sensor_cnt) + 4);

		bt_mesh_model_msg_init(&msg, OP_SENSOR_DESC_STATUS);

		sensor = srv->sensors;
		for (i = 0; i < srv->sensor_cnt; i++) {
			sensor_desc_status_prepare(sensor, msg);
			sensor++;
		}
	}

	if (bt_mesh_model_send(model, ctx, &msg, NULL, NULL)) {
		BT_ERR("Unable to send Sensor Descriptor Status");
	}
}

static void sensor_get(struct bt_mesh_model *model,
		       struct bt_mesh_msg_ctx *ctx,
		       struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_col_get(struct bt_mesh_model *model,
			   struct bt_mesh_msg_ctx *ctx,
			   struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_series_get(struct bt_mesh_model *model,
			      struct bt_mesh_msg_ctx *ctx,
			      struct net_buf_simple *buf)
{
	/* TODO */
}

/* Mapping of message handlers for Sensor Server (0x1100) */
const struct bt_mesh_model_op _sensor_srv_op[] = {
	{ OP_SENSOR_DESC_GET, 0, sensor_desc_get },
	{ OP_SENSOR_GET, 0, sensor_get },
	{ OP_SENSOR_COL_GET, 2, sensor_col_get },
	{ OP_SENSOR_SERIES_GET, 2, sensor_series_get },
};

static void sensor_cad_get(struct bt_mesh_model *model,
			   struct bt_mesh_msg_ctx *ctx,
			   struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_cad_set(struct bt_mesh_model *model,
			   struct bt_mesh_msg_ctx *ctx,
			   struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_cad_set_unack(struct bt_mesh_model *model,
				 struct bt_mesh_msg_ctx *ctx,
				 struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_settings_get(struct bt_mesh_model *model,
			        struct bt_mesh_msg_ctx *ctx,
			        struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_setting_get(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_setting_set(struct bt_mesh_model *model,
			       struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{
	/* TODO */
}

static void sensor_setting_set_unack(struct bt_mesh_model *model,
				     struct bt_mesh_msg_ctx *ctx,
				     struct net_buf_simple *buf)
{
	/* TODO */
}

/* Mapping of message handlers for Sensor Setup Server (0x1101) */
const struct bt_mesh_model_op _sensor_setup_srv_op[] = {
	{ OP_SENSOR_CAD_GET, 2, sensor_cad_get },
	{ OP_SENSOR_CAD_SET, 32, sensor_cad_set },
	{ OP_SENSOR_CAD_SET_UNACK, 32, sensor_cad_set_unack },
	{ OP_SENSOR_SETTINGS_GET, 2, sensor_settings_get },
	{ OP_SENSOR_SETTING_GET, 4, sensor_setting_get },
	{ OP_SENSOR_SETTING_SET, 4, sensor_setting_set },
	{ OP_SENSOR_SETTING_SET_UNACK, 4, sensor_setting_set_unack },

};

int bt_mesh_sensor_pub(struct bt_mesh_model *mod)
{
	/* TODO */
	return 0;
}
