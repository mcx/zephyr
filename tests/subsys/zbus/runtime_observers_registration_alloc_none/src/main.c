/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

struct sensor_data_msg {
	int a;
	int b;
};

ZBUS_CHAN_DEFINE(chan1,                  /* Name */
		 struct sensor_data_msg, /* Message type */

		 NULL,                 /* Validator */
		 NULL,                 /* User data */
		 ZBUS_OBSERVERS_EMPTY, /* observers */
		 ZBUS_MSG_INIT(0)      /* Initial value major 0, minor 1, build 1023 */
);

ZBUS_CHAN_DEFINE(chan2,                  /* Name */
		 struct sensor_data_msg, /* Message type */

		 NULL,                 /* Validator */
		 NULL,                 /* User data */
		 ZBUS_OBSERVERS(lis2), /* observers */
		 ZBUS_MSG_INIT(0)      /* Initial value major 0, minor 1, build 1023 */
);

ZBUS_CHAN_DEFINE(chan3,                  /* Name */
		 struct sensor_data_msg, /* Message type */

		 NULL,                 /* Validator */
		 NULL,                 /* User data */
		 ZBUS_OBSERVERS_EMPTY, /* observers */
		 ZBUS_MSG_INIT(0)      /* Initial value major 0, minor 1, build 1023 */
);

ZBUS_SUBSCRIBER_DEFINE(sub1, 1);
ZBUS_SUBSCRIBER_DEFINE(sub2, 1);

static int count_callback1;
static void callback1(const struct zbus_channel *chan)
{
	++count_callback1;
}

ZBUS_LISTENER_DEFINE(lis1, callback1);

static int count_callback2;
static void callback2(const struct zbus_channel *chan)
{
	++count_callback2;
}

ZBUS_LISTENER_DEFINE(lis2, callback2);
ZBUS_LISTENER_DEFINE(lis3, callback2);
ZBUS_LISTENER_DEFINE(lis4, callback2);
ZBUS_LISTENER_DEFINE(lis5, callback2);
ZBUS_LISTENER_DEFINE(lis6, callback2);
ZBUS_LISTENER_DEFINE(lis7, callback2);

ZTEST(basic, test_specification_based__zbus_obs_add_rm_obs)
{
	count_callback1 = 0;
	struct sensor_data_msg sd = {.a = 10, .b = 100};
	static struct zbus_observer_node n1, n2, n3, n4, n5, n6;

	/* Tyring to add same static observer as one dynamic */
	zassert_equal(-EEXIST, zbus_chan_add_obs_with_node(&chan2, &lis2, &n2, K_MSEC(200)));

	zassert_equal(0, zbus_chan_pub(&chan1, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 0, "The counter could not be more than zero, no obs");

	zassert_equal(0, zbus_chan_add_obs_with_node(&chan1, &lis1, &n1, K_MSEC(200)));

	/* The node cannot be reused */
	zassert_equal(-EBUSY, zbus_chan_add_obs_with_node(&chan2, &lis1, &n1, K_MSEC(200)));

	zassert_equal(-EALREADY, zbus_chan_add_obs_with_node(&chan1, &lis1, &n1, K_MSEC(200)),
		      "It cannot be added twice");

	zassert_equal(0, zbus_chan_pub(&chan1, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 1, "The counter could not be more than zero, no obs, %d",
		      count_callback1);

	zassert_equal(0, zbus_chan_rm_obs(&chan1, &lis1, K_MSEC(200)), "It must remove the obs");
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan1, &lis1, &n1, K_MSEC(200)));
	/* The node cannot be reused */
	zassert_equal(-EBUSY, zbus_chan_add_obs_with_node(&chan3, &lis1, &n1, K_MSEC(200)));
	zassert_equal(0, zbus_chan_rm_obs(&chan1, &lis1, K_MSEC(200)), "It must remove the obs");
	zassert_equal(-ENODATA, zbus_chan_rm_obs(&chan1, &lis1, K_MSEC(200)),
		      "It cannot be removed twice");

	zassert_equal(0, zbus_chan_pub(&chan1, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 1, "The counter could not be more than zero, no obs, %d",
		      count_callback1);

	count_callback2 = 0;

	zassert_equal(0, zbus_chan_pub(&chan2, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 1, "The counter could not be more than zero, no obs");

	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &lis3, &n3, K_MSEC(200)));

	zassert_equal(-EALREADY, zbus_chan_add_obs_with_node(&chan2, &lis3, &n3, K_MSEC(200)),
		      "It cannot be added twice");

	zassert_equal(0, zbus_chan_pub(&chan2, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 3, "The counter could not be more than zero, no obs, %d",
		      count_callback2);
	count_callback2 = 0;
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &sub1, &n1, K_MSEC(200)));
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &sub2, &n2, K_MSEC(200)));
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &lis4, &n4, K_MSEC(200)),
		      "It must add the obs");
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &lis5, &n5, K_MSEC(200)),
		      "It must add the obs");
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan2, &lis6, &n6, K_MSEC(200)),
		      "It must add the obs");

	zassert_equal(0, zbus_chan_pub(&chan2, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 5);

	/* To cause an error to sub1 and sub2. They have the queue full in this point */
	/* ENOMSG must be the result */
	zassert_equal(-ENOMSG, zbus_chan_pub(&chan2, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 10);

	zassert_equal(0, zbus_chan_rm_obs(&chan2, &sub1, K_MSEC(200)));
	zassert_equal(0, zbus_chan_rm_obs(&chan2, &sub2, K_MSEC(200)));
}

struct aux2_wq_data {
	struct k_work work;
};

static struct aux2_wq_data wq_handler;

static void wq_dh_cb(struct k_work *item)
{
	static struct zbus_observer_node node;

	zassert_equal(-EAGAIN, zbus_chan_add_obs_with_node(&chan2, &sub1, &node, K_MSEC(200)));
	zassert_equal(-EAGAIN, zbus_chan_rm_obs(&chan2, &sub2, K_MSEC(200)));
}

ZTEST(basic, test_specification_based__zbus_obs_add_rm_obs_busy)
{
	zassert_equal(0, zbus_chan_claim(&chan2, K_NO_WAIT));

	k_work_init(&wq_handler.work, wq_dh_cb);
	k_work_submit(&wq_handler.work);
	k_msleep(1000);

	zassert_equal(0, zbus_chan_finish(&chan2));
}

ZBUS_CHAN_DEFINE(chan4,                  /* Name */
		 struct sensor_data_msg, /* Message type */

		 NULL,                                 /* Validator */
		 NULL,                                 /* User data */
		 ZBUS_OBSERVERS(prio_lis6, prio_lis5), /* observers */
		 ZBUS_MSG_INIT(0) /* Initial value major 0, minor 1, build 1023 */
);

static int execution_sequence_idx;
static uint8_t execution_sequence[6] = {0};

#define CALLBACK_DEF(_lis, _idx)                                                                   \
	static void _CONCAT(prio_cb, _idx)(const struct zbus_channel *chan)                        \
	{                                                                                          \
		execution_sequence[execution_sequence_idx] = _idx;                                 \
		++execution_sequence_idx;                                                          \
	}                                                                                          \
	ZBUS_LISTENER_DEFINE(_lis, _CONCAT(prio_cb, _idx))

CALLBACK_DEF(prio_lis1, 1);
CALLBACK_DEF(prio_lis2, 2);
CALLBACK_DEF(prio_lis3, 3);
CALLBACK_DEF(prio_lis4, 4);
CALLBACK_DEF(prio_lis5, 5);
CALLBACK_DEF(prio_lis6, 6);

ZBUS_CHAN_ADD_OBS(chan4, prio_lis3, 3);
ZBUS_CHAN_ADD_OBS(chan4, prio_lis4, 2);

/* Checking the ZBUS_CHAN_ADD_OBS. The execution sequence must be: 6, 5, 4, 3, 2, 1. */

ZTEST(basic, test_specification_based__zbus_obs_priority)
{
	struct sensor_data_msg sd = {.a = 70, .b = 116};
	static struct zbus_observer_node n1, n2;

	execution_sequence_idx = 0;

	zassert_equal(0, zbus_chan_add_obs_with_node(&chan4, &prio_lis2, &n1, K_MSEC(200)));
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan4, &prio_lis1, &n2, K_MSEC(200)));

	zassert_equal(0, zbus_chan_pub(&chan4, &sd, K_MSEC(500)));

	zassert_equal(execution_sequence[0], 6);
	zassert_equal(execution_sequence[1], 5);
	zassert_equal(execution_sequence[2], 4);
	zassert_equal(execution_sequence[3], 3);
	zassert_equal(execution_sequence[4], 2);
	zassert_equal(execution_sequence[5], 1);
}

/**
 * @brief Test removing the first observer in a runtime observer list.
 */
ZTEST(basic, test_remove_first_runtime_observer_alloc_none)
{
	struct sensor_data_msg sd = {.a = 42, .b = 84};
	static struct zbus_observer_node n1, n2, n3;

	/* Reset callback counters */
	count_callback1 = 0;
	count_callback2 = 0;

	/* Start with an empty channel (chan3) */
	zassert_equal(0, zbus_chan_pub(&chan3, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 0, "No observers should be called initially");
	zassert_equal(count_callback2, 0, "No observers should be called initially");

	/* Add multiple observers to create a list using alloc_none variant */
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan3, &lis1, &n1, K_MSEC(200)),
		      "First observer should be added successfully");
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan3, &lis3, &n2, K_MSEC(200)),
		      "Second observer should be added successfully");
	zassert_equal(0, zbus_chan_add_obs_with_node(&chan3, &lis4, &n3, K_MSEC(200)),
		      "Third observer should be added successfully");

	/* Verify all observers are called */
	zassert_equal(0, zbus_chan_pub(&chan3, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 1, "lis1 callback should be called once");
	zassert_equal(count_callback2, 2, "lis3 and lis4 callbacks should be called");

	/* Reset counters for the main test */
	count_callback1 = 0;
	count_callback2 = 0;

	/* Remove the first observer in the list (lis1) */
	zassert_equal(0, zbus_chan_rm_obs(&chan3, &lis1, K_MSEC(200)),
		      "First observer should be removed successfully");

	/* Verify only the remaining observers are called */
	zassert_equal(0, zbus_chan_pub(&chan3, &sd, K_MSEC(500)));
	zassert_equal(count_callback1, 0, "lis1 callback should not be called after removal");
	zassert_equal(count_callback2, 2, "lis3 and lis4 callbacks should still be called");

	/* Remove the new first observer (lis3) to test the same scenario again */
	count_callback2 = 0;
	zassert_equal(0, zbus_chan_rm_obs(&chan3, &lis3, K_MSEC(200)),
		      "New first observer should be removed successfully");

	/* Verify only lis4 is called */
	zassert_equal(0, zbus_chan_pub(&chan3, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 1, "Only lis4 callback should be called");

	/* Remove the last observer */
	count_callback2 = 0;
	zassert_equal(0, zbus_chan_rm_obs(&chan3, &lis4, K_MSEC(200)),
		      "Last observer should be removed successfully");

	/* Verify no observers are called */
	zassert_equal(0, zbus_chan_pub(&chan3, &sd, K_MSEC(500)));
	zassert_equal(count_callback2, 0, "No callbacks should be called after all removed");

	/* Test error case: try to remove non-existent observer */
	zassert_equal(-ENODATA, zbus_chan_rm_obs(&chan3, &lis1, K_MSEC(200)),
		      "Removing non-existent observer should return -ENODATA");
}

ZTEST_SUITE(basic, NULL, NULL, NULL, NULL, NULL);
