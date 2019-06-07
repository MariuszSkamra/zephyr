/** @file
 *  @brief Bluetooth Mesh Model Sensor APIs.
 */

/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <bluetooth/mesh.h>

/** @brief Sensor Setting state */
struct bt_mesh_sensor_setting {
	/** Sensor Setting Property ID */
	const u16_t id;

	/** Sensor Setting read callback.
	 *
	 *  @param sensor	The sensor this setting belongs to.
	 *  @param buf		Buffer to place the read result in.
	 *  @param len		Buffer length.
	 *  @param user_data	Sensor Setting user data.
	 *
	 *  @return Number of bytes read.
	 */
	int (*read)(struct bt_mesh_sensor *sensor, void *buf, size_t len,
		    void *user_data);

	/** Sensor Setting write callback (optional).
	 *
	 *  @param sensor	The sensor this setting belongs to.
	 *  @param buf		Buffer with the data to write.
	 *  @param len		Length of data to write.
	 *  @param user_data	Sensor Setting user data.
	 *
	 *  @return Number of bytes written.
	 */
	int (*write)(struct bt_mesh_sensor *sensor, void *buf, size_t len,
		     void *user_data);

	/** Sensor Setting user data */
	void *user_data;
};

/** @brief Sensor State */
struct bt_mesh_sensor {
	/** Sensor Property ID */
	const u16_t id;

	struct {
		const u16_t tolerance_pos:12;	/**< Positive Tolerance */
		const u16_t tolerance_neg:12;	/**< Negative Tolerance */
		const u8_t  sampling_func;	/**< Sampling Function */
		const u8_t  period;		/**< Measurement Period */
		const u8_t  interval;		/**< Update Interval */
	} desc;

	/** Sensor Data read callback.
	 *
	 *  @param sensor	The sensor that's value being read.
	 *  @param buf		Buffer to place the read result in.
	 *  @param len		Buffer length.
	 *
	 *  @return Number of bytes read.
	 */
	int (*read)(struct bt_mesh_sensor *sensor, void *buf, size_t len);

	/** Optional Sensor Settings */
	struct bt_mesh_sensor_setting *settings;
	size_t setting_cnt;

	/* TODO: Optional Sensor Cadence */

	/** Sensor user data */
	void *user_data;
};

/** @brief Callback for updating sensor publication buffer */
int bt_mesh_sensor_pub(struct bt_mesh_model *model);

/** @def BT_MESH_SENSOR_PUB_DEFINE
 *
 *  A helper to define a sensor publication context.
 *
 *  @param _name 		Name given to the publication context variable.
 *  @param _max_size		Maximum size of sensor value to be published.
 */
#define BT_MESH_SENSOR_PUB_DEFINE(_name, _max_size) 		\
	BT_MESH_MODEL_PUB_DEFINE(_name, bt_mesh_sensor_pub,	\
				 (1 + 2 + (_max_size)))

/** @brief Mesh Sensor Server Model Context */
struct _bt_mesh_sensor_srv {
	const struct bt_mesh_sensor *sensors;
	size_t sensor_cnt;
};

extern const struct bt_mesh_model_op _sensor_srv_op[];
extern const struct bt_mesh_model_op _sensor_setup_srv_op[];

/** @def BT_MESH_MODEL_SENSOR_SRV
 *
 *  Define a new sensor server and sensor server setup models. Note that this
 *  API needs to be repeated for each element that the application wants to have
 *  a server server model on. Each instance also needs a unique
 *  _bt_mesh_sensor_srv and bt_mesh_model_pub context.
 *
 *  @param _sensors	Pointer to a unique array of struct bt_mesh_sensor.
 *  @param pub		Pointer to a unique struct bt_mesh_model_pub.
 *
 *  @return New mesh model instances.
 */
#define BT_MESH_MODEL_SENSOR_SRV(_sensors, _pub)			\
		BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_SRV,		\
			      _sensor_srv_op, _pub,			\
			      (&(struct _bt_mesh_sensor_srv) {		\
				.states = _sensors,			\
				.state_cnt = ARRAY_SIZE(_sensors) })),	\
		BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_SETUP_SRV,	\
			      _sensor_setup_srv_op, NULL,		\
			      (&(struct _bt_mesh_sensor_srv) {		\
				.states = _sensors,			\
				.state_cnt = ARRAY_SIZE(_sensors) })),	\
