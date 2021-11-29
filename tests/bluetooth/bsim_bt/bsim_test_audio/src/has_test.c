/*
 * Copyright (c) 2022 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_BT_HAS
#include "bluetooth/audio/has.h"
#include "common.h"

extern enum bst_result_t bst_result;

static struct bt_has *has;

static struct bt_conn *g_conn;
static bool g_is_connected;

static struct bt_has_cb has_cb = {

};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		FAIL("Failed to connect to %s (%u)\n", addr, err);
		return;
	}
	printk("Connected to %s\n", addr);
	g_conn = conn;
	g_is_connected = true;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void test_standalone(void)
{
	int err;
	struct bt_has_register_param has_param;
	char preset_name[CONFIG_BT_HAS_PRESET_COUNT][41];

	err = bt_enable(NULL);
	if (err) {
		FAIL("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	memset(&has_param, 0, sizeof(has_param));

	for (int i = 0; i < ARRAY_SIZE(has_param.preset_list); i++) {
		has_param.preset_list[i].index = (uint8_t)i;
		has_param.preset_list[i].properties = BT_HAS_PROP_WRITABLE |
						      BT_HAS_PROP_AVAILABLE;
		snprintf(preset_name[i], sizeof(preset_name[i]),
			 "Preset %d", i + 1);
		has_param.preset_list[i].name = preset_name[i];
	}

	has_param.cb = &has_cb;

	err = bt_has_register(&has_param, &has);
	if (err) {
		FAIL("HAS register failed (err %d)\n", err);
		return;
	}

	PASS("HAS passed\n");
}

static const struct bst_test_instance test_has[] = {
	{
		.test_id = "has_standalone",
		.test_post_init_f = test_init,
		.test_tick_f = test_tick,
		.test_main_f = test_standalone
	},
	BSTEST_END_MARKER
};

struct bst_test_list *test_has_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_has);
}
#else
struct bst_test_list *test_has_install(struct bst_test_list *tests)
{
	return tests;
}

#endif /* CONFIG_BT_HAS */
