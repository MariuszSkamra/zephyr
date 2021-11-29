/*
 * Copyright (c) 2022 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_BLUETOOTH_AUDIO_HAS_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_AUDIO_HAS_H_

/**
 * @brief Hearing Access Service (HAS)
 *
 * @defgroup bt_has Hearing Access Service (HAS)
 *
 * @ingroup bluetooth
 * @{
 *
 * The Hearing Access Service is used to identify a hearing aid and optionally
 * to control hearing aid presets. This API implements the server functionality.
 *
 * [Experimental] Users should note that the APIs can change as a part of
 * ongoing development.
 */

#include <zephyr/types.h>
#include <bluetooth/bluetooth.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_BT_HAS)
#define BT_HAS_PRESET_COUNT CONFIG_BT_HAS_PRESET_COUNT
#else
#define BT_HAS_PRESET_COUNT 0
#endif /* CONFIG_BT_HAS */

/** @brief Opaque Hearing Access Service object. */
struct bt_has;

/** Hearing Aid device type */
enum {
	/** Binaural Hearing Aid. */
	BT_HAS_BINAURAL_HEARING_AID,

	/** Monaural Hearing Aid */
	BT_HAS_MONAURAL_HEARING_AID,

	/** Banded Hearing Aid */
	BT_HAS_BANDED_HEARING_AID,
};

/** Preset Properties values */
enum {
	/** No properties set */
	BT_HAS_PROP_NONE = 0,

	/** Preset name can be written by the client */
	BT_HAS_PROP_WRITABLE = BIT(0),

	/** Preset availability */
	BT_HAS_PROP_AVAILABLE = BIT(1),
};

/** @brief Preset representation */
struct bt_has_preset {
	/** Preset index */
	uint8_t index;
	/** Preset properties */
	uint8_t properties;
	/** Preset name */
	char *name;
};

/** @brief Hearing Access Service initialization parameters */
struct bt_has_register_param {
	/** Hearing Access Service Preset initial parameters. */
	struct bt_has_preset preset_list[BT_HAS_PRESET_COUNT];

	/** Hearing Access Service callback structure. */
	struct bt_has_cb *cb;
};

/**
 * @brief Initialize Hearing Access Service.
 *
 * @param      param	Hearing Access Service register parameters.
 * @param[out] has	Pointer to the Hearing Access Service object.
 * 			This will still be valid if the return value is
 * 			-EALREADY.
 *
 * @return 0 if success, errno on failure.
 */
int bt_has_register(struct bt_has_register_param *param, struct bt_has **has);

/**
 * @brief Get Active Preset.
 *
 * Get the index of currently active preset.
 *
 * @param has	Pointer to the Hearing Access Service object.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_active_get(struct bt_has *has);

/**
 * @brief Set Active Preset.
 *
 * Set the preset record identified by the Index field as the active preset.
 *
 * @param has	Pointer to the Hearing Access Service object.
 * @param index	Preset record index.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_active_set(struct bt_has *has, uint8_t index);

/**
 * @brief Set Next Preset.
 *
 * Set the next preset record on the server list as the active preset.
 *
 * @param has	Pointer to the Hearing Access Service object.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_active_set_next(struct bt_has *has);

/**
 * @brief Set Previous Preset.
 *
 * Set the previous preset record on the server list as the active preset.
 *
 * @param has	Pointer to the Hearing Access Service object.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_active_set_prev(struct bt_has *has);

/**
 * @brief Set Preset Name.
 *
 * Set the preset record name identified by the Index field.
 *
 * @param has	Pointer to the Hearing Access Service object.
 * @param index	Preset record index.
 * @param name	Preset name to be written.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_name_set(struct bt_has *has, uint8_t index, const char *name);

/**
 * @brief Get Preset Record.
 *
 * Get the preset record identified by the Index field.
 *
 * @param has	Pointer to the Hearing Access Service object.
 * @param index	Preset record index.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_get(struct bt_has *has, uint8_t index);

/**
 * @brief Get Preset List.
 *
 * Get the preset record list.
 *
 * @param has	Pointer to the Hearing Access Service object.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_has_preset_list_get(struct bt_has *has);

/** @brief Hearing Access Service callback structure. */
struct bt_has_cb {
#if defined(CONFIG_BT_HAS)
	/**
	 * @brief Preset set active callback
	 *
	 * Once the preset becomes active, the bt_has_preset_active_set shall
	 * be called to notify all the clients.
	 *
	 * @param has	Pointer to the Hearing Access Service object.
	 * @param index	Preset record index requested to activate
	 * @param sync	Whether the server must relay this change to the
	 *		other member of the Binaural Hearing Aid Set.
	 *
	 * @return 0 in case of success or -EBUSY if operation cannot be
	 *	   executed at the time.
	 */
	int (*activate)(struct bt_has *has, const struct bt_has_preset *preset,
			bool sync);
#endif /* CONFIG_BT_HAS */
#if defined(CONFIG_BT_HAS_CLIENT)
	/**
	 * @brief Callback function for Hearing Access Service.
	 *
	 * Called when the service is locally found as the server.
	 * Called when the service is remotely found as the client.
	 * Called if the bt_has_find_service has been called or autonomously
	 * on reconnection to the known, bonded device.
	 *
	 * @param has		Pointer to the Hearing Access Service object.
	 * @param err		0 in case of success or negative value in case
	 *			of error.
	 * @param type		Hearing Aid Device type.
	 */
	void (*service_found)(struct bt_has *has, int err, uint8_t type);
#endif /* CONFIG_BT_HAS_CLIENT */
	/**
	 * @brief Callback function for Hearing Access Service active preset.
	 *
	 * Called when the value is locally read as the server.
	 * Called when the value is remotely read as the client.
	 * Called if the value is changed by either the server or client.
	 *
	 * @param has		Pointer to the Hearing Access Service object.
	 * @param err		0 in case of success or negative value in case
	 *			of error.
	 * @param index		Active preset index.
	 */
	void (*active_preset)(struct bt_has *has, int err, uint8_t index);

	/**
	 * @brief Callback function for Hearing Access Service preset name.
	 *
	 * Called when the value is locally read as the server.
	 * Called when the value is remotely read as the client.
	 * Called if the value is changed by either the server or client.
	 *
	 * @param has		Pointer to the Hearing Access Service object.
	 * @param err		0 in case of success or negative value in case
	 *			of error.
	 * @param preset	Preset record including name.
	 */
	void (*preset)(struct bt_has *has, int err,
		       const struct bt_has_preset *preset, bool is_active);
};

/**
 * @brief Registers the callbacks used by the Hearing Access Service.
 *
 * @param cb	The callback structure.
 *
 * @return 0 if success, errno on failure.
 */
int bt_has_cb_register(struct bt_has_cb *cb);

/**
 * @brief Get the Bluetooth connection object of the service object.
 *
 * @param has	Service object.
 *
 * @return Bluetooth connection object or NULL if local service object.
 */
struct bt_conn *bt_has_conn_get(const struct bt_has *has);

/**
 * @brief Get a Hearing Access Service.
 *
 * Find a Hearing Access Service on a server identified by conn.
 *
 * @param conn	Bluetooth connection object.
 *
 * @return 0 if success, errno on failure.
 */
int bt_has_get(struct bt_conn *conn);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_AUDIO_HAS_H_ */
