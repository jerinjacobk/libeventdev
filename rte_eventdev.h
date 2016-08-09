
/**
 * Get the total number of event devices that have been successfully
 * initialised.
 *
 * @return
 *   The total number of usable event devices.
 */
extern uint8_t
rte_eventdev_count(void);

/**
 * Get the device identifier for the named event device.
 *
 * @param name
 *   Event device name to select the event device identifier.
 *
 * @return
 *   Returns event device identifier on success.
 *   - <0: Failure to find named event device.
 */
extern uint8_t
rte_eventdev_get_dev_id(const char *name);

/*
 * Return the NUMA socket to which a device is connected.
 *
 * @param dev_id
 *   The identifier of the device.
 * @return
 *   The NUMA socket id to which the device is connected or
 *   a default of zero if the socket could not be determined.
 *   - -1: dev_id value is out of range.
 */
extern int
rte_eventdev_socket_id(uint8_t dev_id);

/**  Event device information */
struct rte_eventdev_info {
	const char *driver_name;	/**< Event driver name */
	struct rte_pci_device *pci_dev;	/**< PCI information */
	uint32_t min_sched_wait_ns;
	/**< Minimum supported scheduler wait delay in ns by this device */
	uint32_t max_sched_wait_ns;
	/**< Maximum supported scheduler wait delay in ns by this device */
	uint32_t sched_wait_ns;
	/**< Configured scheduler wait delay in ns of this device */
	uint32_t max_flow_queues_log2;
	/**< LOG2 of maximum flow queues supported by this device */
	uint8_t  max_sched_groups;
	/**< Maximum schedule groups supported by this device */
	uint8_t  max_sched_group_priority_levels;
	/**< Maximum schedule group priority levels supported by this device */
}

/**
 * Retrieve the contextual information of an event device.
 *
 * @param dev_id
 *   The identifier of the device.
 * @param[out] dev_info
 *   A pointer to a structure of type *rte_eventdev_info* to be filled with the
 *   contextual information of the device.
 */
extern void
rte_eventdev_info_get(uint8_t dev_id, struct rte_eventdev_info *dev_info);

/** Event device configuration structure */
struct rte_eventdev_config {
	uint32_t sched_wait_ns;
	/**< rte_event_schedule() wait for *sched_wait_ns* ns on this device */
	uint32_t nb_flow_queues_log2;
	/**< LOG2 of the number of flow queues to configure on this device */
	uint8_t  nb_sched_groups;
	/**< The number of schedule groups to configure on this device */
};

/**
 * Configure an event device.
 *
 * This function must be invoked first before any other function in the
 * API. This function can also be re-invoked when a device is in the
 * stopped state.
 *
 * The caller may use rte_eventdev_info_get() to get the capability of each
 * resources available in this event device.
 *
 * @param dev_id
 *   The identifier of the device to configure.
 * @param config
 *   The event device configuration structure.
 *
 * @return
 *   - 0: Success, device configured.
 *   - <0: Error code returned by the driver configuration function.
 */
extern int
rte_eventdev_configure(uint8_t dev_id, struct rte_eventdev_config *config);


#define RTE_EVENT_SCHED_GRP_PRI_HIGHEST	0
/**< Highest schedule group priority */
#define RTE_EVENT_SCHED_GRP_PRI_NORMAL	128
/**< Normal schedule group priority */
#define RTE_EVENT_SCHED_GRP_PRI_LOWEST	255
/**< Lowest schedule group priority */

struct rte_eventdev_sched_group_conf {
	rte_cpuset_t lcore_list;
	/**< List of l-cores has membership in this schedule group */
	uint8_t priority;
	/**< Priority for this schedule group relative to other schedule groups.
	     If the event device's *max_sched_group_priority_levels* are not in
	     the range of requested *priority* then event driver can normalize
	     to required priority value in the range of
	     [RTE_EVENT_SCHED_GRP_PRI_HIGHEST, RTE_EVENT_SCHED_GRP_PRI_LOWEST]*/
	uint8_t enable_all_lcores;
	/**< Ignore *core_list* and enable all the l-cores */
};

/**
 * Allocate and set up a schedule group for a event device.
 *
 * @param dev_id
 *   The identifier of the device.
 * @param group_id
 *   The index of the schedule group to setup. The value must be in the range
 *   [0, nb_sched_groups - 1] previously supplied to rte_eventdev_configure().
 * @param group_conf
 *   The pointer to the configuration data to be used for the schedule group.
 *   NULL value is allowed, in which case default configuration	used.
 * @param socket_id
 *   The *socket_id* argument is the socket identifier in case of NUMA.
 *   The value can be *SOCKET_ID_ANY* if there is no NUMA constraint for the
 *   DMA memory allocated for the receive schedule group.
 *
 * @return
 *   - 0: Success, schedule group correctly set up.
 *   - <0: Schedule group configuration failed
 */
extern int
rte_eventdev_sched_group_setup(uint8_t dev_id, uint8_t group_id,
		const struct rte_eventdev_sched_group_conf *group_conf,
		int socket_id);

/**
 * Get the number of schedule groups on a specific event device
 *
 * @param dev_id
 *   Event device identifier.
 * @return
 *   - The number of configured schedule groups
 */
extern uint16_t
rte_eventdev_sched_group_count(uint8_t dev_id);

/**
 * Get the priority of the schedule group on a specific event device
 *
 * @param dev_id
 *   Event device identifier.
 * @param group_id
 *   Schedule group identifier.
 * @return
 *   - The configured priority of the schedule group in
 *     [RTE_EVENT_SCHED_GRP_PRI_HIGHEST, RTE_EVENT_SCHED_GRP_PRI_LOWEST] range
 */
extern uint8_t
rte_eventdev_sched_group_priority(uint8_t dev_id, uint8_t group_id);

/**
 * Get the configured flow queue id mask of a specific event device
 *
 * *flow_queue_id_mask* can be used to generate *flow_queue_id* value in the
 * range [0 - (2^max_flow_queues_log2 -1)] of a specific event device.
 * *flow_queue_id* value will be used in the event enqueue operation
 * and comparing scheduled event *flow_queue_id* value against enqueued value.
 *
 * @param dev_id
 *   Event device identifier.
 * @return
 *   - The configured flow queue id mask
 */
extern uint32_t
rte_eventdev_flow_queue_id_mask(uint8_t dev_id);

/**
 * Start an event device.
 *
 * The device start step is the last one and consists of setting the schedule
 * groups and flow queues to start accepting the events and schedules to l-cores.
 *
 * On success, all basic functions exported by the API (event enqueue,
 * event schedule and so on) can be invoked.
 *
 * @param dev_id
 *   Event device identifier
 * @return
 *   - 0: Success, device started.
 *   - <0: Error code of the driver device start function.
 */
extern int
rte_eventdev_start(uint8_t dev_id);

/**
 * Stop an event device. The device can be restarted with a call to
 * rte_eventdev_start()
 *
 * @param dev_id
 *   Event device identifier.
 */
extern void
rte_eventdev_stop(uint8_t dev_id);

/**
 * Close an event device. The device cannot be restarted!
 *
 * @param dev_id
 *   Event device identifier
 *
 * @return
 *  - 0 on successfully closing device
 *  - <0 on failure to close device
 */
extern int
rte_eventdev_close(uint8_t dev_id);


/* Scheduler synchronization method */

#define RTE_SCHED_SYNC_ORDERED		0
/**< Ordered flow queue synchronization
 *
 * Events from an ordered flow queue can be scheduled to multiple l-cores for
 * concurrent processing while maintaining the original event order. This
 * scheme enables the user to achieve high single flow throughput by avoiding
 * SW synchronization for ordering between l-cores.
 *
 * The source flow queue ordering is maintained when events are enqueued to
 * their destination queue(s) within the same ordered queue synchronization
 * context. A l-core holds the context until it requests another event from the
 * scheduler, which implicitly releases the context. User may allow the
 * scheduler to release the context earlier than that by calling
 * rte_event_schedule_release()
 *
 * Events from the source flow queue appear in their original order when
 * dequeued from a destination flow queue irrespective of its
 * synchronization method. Event ordering is based on the received event(s),
 * but also other (newly allocated or stored) events are ordered when enqueued
 * within the same ordered context.Events not enqueued (e.g. freed or stored)
 * within the context are considered missing from reordering and are skipped at
 * this time (but can be ordered again within another context).
 *
 */

#define RTE_SCHED_SYNC_ATOMIC		1
/**< Atomic flow queue synchronization
 *
 * Events from an atomic flow queue can be scheduled only to a single l-core at
 * a time. The l-core is guaranteed to have exclusive (atomic) access to the
 * associated flow queue context, which enables the user to avoid SW
 * synchronization. Atomic flow queue also helps to maintain event ordering
 * since only one l-core at a time is able to process events from a flow queue.
 *
 * The atomic queue synchronization context is dedicated to the l-core until it
 * requests another event from the scheduler, which implicitly releases the
 * context. User may allow the scheduler to release the context earlier than
 * that by calling rte_event_schedule_release()
 *
 */

#define RTE_SCHED_SYNC_PARALLEL		2
/**< Parallel flow queue
 *
 * The scheduler performs priority scheduling, load balancing etc functions
 * but does not provide additional event synchronization or ordering.
 * It's free to schedule events from single parallel queue to multiple l-core
 * for concurrent processing. Application is responsible for flow queue context
 * synchronization and event ordering (SW synchronization).
 *
 */

/* Event types to classify the event source */

#define RTE_EVENT_TYPE_ETHDEV		0x0
/**< The event generated from ethdev subsystem */
#define RTE_EVENT_TYPE_CRYPTODEV	0x1
/**< The event generated from crypodev subsystem */
#define RTE_EVENT_TYPE_TIMERDEV		0x2
/**< The event generated from timerdev subsystem */
#define RTE_EVENT_TYPE_LCORE		0x3
/**< The event generated from l-core. Application may use *sub_event_type*
 * to further classify the event */
#define RTE_EVENT_TYPE_INVALID		0xf
/**< Invalid event type */
#define RTE_EVENT_TYPE_MAX		0x16

/**< The generic rte_event structure to hold the event attributes */
struct rte_event {
        union {
		uint64_t u64;
		struct {
			uint32_t flow_queue_id;
			/**< Flow queue identifier to choose the flow queue in
			 * enqueue and schedule operation.
			 * The value must be the range of
			 * rte_eventdev_flow_queue_id_mask() */
			uint8_t  sched_group_id;
			/**< Schedule group identifier to choose the schedule
			 * group in enqueue and schedule operation.
			 * The value must be in the range
			 * [0, nb_sched_groups - 1] previously supplied to
			 * rte_eventdev_configure(). */
			uint8_t  sched_sync;
			/**< Scheduler synchronization method associated
			 * with flow queue for enqueue and schedule operation */
			uint8_t  event_type;
			/**< Event type to classify the event source  */
			uint8_t  sub_event_type;
			/**< Sub-event types based on the event source */
		};
	};
	union {
		uintptr_t event;
		/**< Opaque event pointer */
		struct rte_mbuf *mbuf;
		/**< mbuf pointer if the scheduled event is associated with mbuf */
	};
}

/**
 *
 * Enqueue the event object supplied in *rte_event* structure on flow queue
 * identified as *flow_queue_id* associated with the schedule group
 * *sched_group_id*, scheduler synchronization method and its event types
 * on an event device designated by its *dev_id*.
 *
 * @param dev_id
 *   Event device identifier.
 * @param ev
 *   Pointer to struct rte_event
 * @return
 *  - 0 on success
 *  - <0 on failure
 */
extern int
rte_eventdev_enqueue(uint8_t dev_id, struct rte_event *ev);

/**
 * Enqueue a burst of events objects supplied in *rte_event* structure
 * on an event device designated by its *dev_id*.
 *
 * The rte_eventdev_enqueue_burst() function is invoked to enqueue
 * multiple event objects. Its the burst variant of rte_eventdev_enqueue()
 * function
 *
 * The *num* parameter is the number of event objects to enqueue which are
 * supplied in the *ev* array of *rte_event* structure.
 *
 * The rte_eventdev_enqueue_burst() function returns the number of
 * events objects it actually enqueued . A return value equal to
 * *num* means that all event objects have been enqueued.
 *
 * @param dev_id
 *   The identifier of the device.
 * @param ev
 *   The address of an array of *num* pointers to *rte_event* structure
 *   which contain the event object enqueue operations to be processed.
 * @param num
 *   The number of event objects to enqueue
 *
 * @return
 * The number of event objects actually enqueued on the event device. The return
 * value can be less than the value of the *num* parameter when the
 * event devices flow queue is full or if invalid parameters are specified in
 * a *rte_event*. If return value is less than *num*, the remaining events at
 * the end of ev[] are not consumed, and the caller has to take care of them.
 */
extern int
rte_eventdev_enqueue_burst(uint8_t dev_id, struct rte_event *ev[], int num);

/**
 * Schedule an event to the caller l-core from the event device designated by
 * its *dev_id*.
 *
 * rte_event_schedule() does not dictate the specifics of scheduling algorithm as
 * each eventdev driver may have different criteria to schedule an event.
 * However, in general, from an application perspective scheduler may use
 * following scheme to dispatch an event to l-core
 *
 * 1) Selection of schedule group
 *   a) The Number of schedule group available in the event device
 *   b) The caller l-core membership in the schedule group.
 *   c) Schedule group priority relative to other schedule groups.
 * 2) Selection of flow queue and event
 *   a) The Number of flow queues  available in event device
 *   b) Scheduler synchronization method associated with the flow queue
 *
 * On successful scheduler event dispatch, The caller l-core holds scheduler
 * synchronization context associated with the dispatched event, an explicit
 * rte_event_schedule_release() or rte_event_schedule_ctxt_*() or next
 * rte_event_schedule() call shall release the context
 *
 * @param dev_id
 *   The identifier of the device.
 * @param[out] ev
 *   Pointer to struct rte_event. On successful event dispatch, Implementation
 *   updates the event attributes
 * @param wait
 *   When true, wait for event till available or *sched_wait_ns* ns which
 *   previously supplied to rte_eventdev_configure()
 *
 * @return
 * When true, a valid event has been dispatched by the scheduler.
 *
 */
extern bool
rte_event_schedule(uint8_t dev_id, struct rte_event *ev, bool wait);

/**
 * Schedule an event to the caller l-core from a specific schedule group
 * *group_id* of event device designated by its *dev_id*.
 *
 * Like rte_event_schedule(), but schedule group provided as argument *group_id*
 *
 * @param dev_id
 *   The identifier of the device.
 * @param group_id
 *   Schedule group identifier to select the schedule group for event dispatch
 * @param[out] ev
 *   Pointer to struct rte_event. On successful event dispatch, Implementation
 *   updates the event attributes
 * @param wait
 *   When true, wait for event till available or *sched_wait_ns* ns which
 *   previously supplied to rte_eventdev_configure()
 *
 * @return
 * When true, a valid event has been dispatched by the scheduler.
 *
 */
extern bool
rte_event_schedule_from_group(uint8_t dev_id, uint8_t group_id,
				struct rte_event *ev, bool wait);

/**
 * Release the current scheduler synchronization context associated with the
 * scheduler dispatched event
 *
 * If current scheduler synchronization context method is *RTE_SCHED_SYNC_ATOMIC*
 * then this function hints the scheduler that the user has completed critical
 * section processing in the current atomic context.
 * The scheduler is now allowed to schedule events from the same flow queue to
 * another l-core.
 * Early atomic context release may increase parallelism and thus system
 * performance, but user needs to design carefully the split into critical vs.
 * non-critical sections.
 *
 * If current scheduler synchronization context method is *RTE_SCHED_SYNC_ORDERED*
 * then this function hints the scheduler that the user has done all enqueues
 * that need to maintain event order in the current ordered context.
 * The scheduler is allowed to release the ordered context of this l-core and
 * avoid reordering any following enqueues.
 * Early ordered context release may increase parallelism and thus system
 * performance, since scheduler may start reordering events sooner than the next
 * schedule call.
 *
 * If current scheduler synchronization context method is *RTE_SCHED_SYNC_PARALLEL*
 * then this function is a nop
 *
 * @param dev_id
 *   The identifier of the device.
 *
 */
extern void
rte_event_schedule_release(uint8_t dev_id);

/**
 * Update the current schedule context associated with caller l-core
 *
 * rte_event_schedule_ctxt_update() can be used to support run-to-completion
 * model where the application requires the current *event* to stay on the same
 * l-core as it moves through the series of processing stages, provided the
 * event type is *RTE_EVENT_TYPE_LCORE*.
 *
 * In the context of run-to-completion model, rte_eventdev_enqueue()
 * and its associated rte_event_schedule() can be replaced by
 * rte_event_schedule_ctxt_update() if caller requires to current event to
 * stay on caller l-core for new *flow_queue_id* and/or new *sched_sync*
 * and/or new *sub_event_type* values
 *
 * All of the arguments should be equal to their current schedule context values
 * unless the application needs the dispatcher to modify the  event attribute
 * of a dispatched event.
 *
 * rte_event_schedule_ctxt_update() is a costly operation, by splitting it as
 * functions(rte_event_schedule_ctxt_update() and rte_event_schedule_ctxt_wait())
 * allows caller to overlap the context update latency with other profitable
 * work
 *
 * @param dev_id
 *   The identifier of the device.
 * @param flow_queue_id
 *   The new flow queue identifier
 * @param sched_sync
 *   The new schedule synchronization method
 * @param sub_event_type
 *   The new sub_event_type where event_type == RTE_EVENT_TYPE_LCORE
 * @param wait
 *   When true, wait until context update completes
 *   When false, request to update the attribute may optionally start an
 *   operation that may not finish when this function returns.
 *   In that case, this function return '1' to indicate the application to
 *   call rte_event_schedule_ctxt_wait() before processing with an
 *   operation that requires the completion of the requested event attribute
 *   change
 * @return
 *  - <0 on failure
 *  - 0 on if event attribute update operation has been completed.
 *  - 1 on if event attribute update operation has begun asynchronously.
 *
 */
extern int
rte_event_schedule_ctxt_update(uint8_t dev_id, uint32_t flow_queue_id,
		uint8_t  sched_sync, uint8_t sub_event_type, bool wait);

/**
 * Wait for l-core associated event update operation to complete on the
 * event device designated by its *dev_id*.
 *
 * The caller l-core wait until a previously started event attribute update
 * operation from the same l-core till it completes
 *
 * This function is invoked when rte_event_schedule_ctxt_update() returns '1'
 *
 * @param dev_id
 *   The identifier of the device.
 */
extern void
rte_event_schedule_ctxt_wait(uint8_t dev_id);

/**
 * Join the caller l-core to a schedule group *group_id* of the event device
 * designated by its *dev_id*.
 *
 * l-core membership in the schedule group can be configured with
 * rte_eventdev_sched_group_setup() prior to rte_eventdev_start()
 *
 * @param dev_id
 *   The identifier of the device.
 * @param group_id
 *   Schedule group identifier to select the schedule group to join
 *
 * @return
 *  - 0 on success
 *  - <0 on failure
 */
extern int
rte_event_schedule_group_join(uint8_t dev_id, uint8_t group_id);

/**
 * Leave the caller l-core from a schedule group *group_id* of the event device
 * designated by its *dev_id*.
 *
 * This function will unsubscribe the calling l-core from receiving  events from
 * the specified  schedule group *group_id*
 *
 * l-core membership in the schedule group can be configured with
 * rte_eventdev_sched_group_setup() prior to rte_eventdev_start()
 *
 * @param dev_id
 *   The identifier of the device.
 * @param group_id
 *   Schedule group identifier to select the schedule group to join
 *
 * @return
 *  - 0 on success
 *  - <0 on failure
 */
extern int
rte_event_schedule_group_leave(uint8_t dev_id, uint8_t group_id);
