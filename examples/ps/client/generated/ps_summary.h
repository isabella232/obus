/**
 * @file ps_summary.h
 *
 * @brief obus ps_summary object client api
 *
 * @author obusgen 1.0.3 generated file, do not modify it.
 */
#ifndef _PS_SUMMARY_H_
#define _PS_SUMMARY_H_

#include "libobus.h"

OBUS_BEGIN_DECLS

/**
 * @brief ps_summary object uid
 **/
#define PS_SUMMARY_UID 2

/**
 * @brief ps_summary mode enumeration.
 *
 * PCPU mode
 **/
enum ps_summary_mode {
	/** PCPU of a process can never go above 100 event if SMP */
	PS_SUMMARY_MODE_SOLARIS = 0,
	/** PCPU can go up to ncpus *100 */
	PS_SUMMARY_MODE_IRIX = 1,
};

/**
 * @brief check if value is one of enum ps_summary_mode values.
 *
 * @param[in]  value  value to be checked.
 *
 * @retval 1 value is one of ps_summary_mode values.
 * @retval 0 value is not one of ps_summary_mode values.
 **/
int ps_summary_mode_is_valid(int32_t value);

/**
 * @brief get ps_summary_mode string value.
 *
 * @param[in]  value  mode value to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *ps_summary_mode_str(enum ps_summary_mode value);

/**
 * @brief ps_summary event type enumeration.
 *
 * This enumeration describes all kind of ps_summary events.
 **/
enum ps_summary_event_type {
	/** Process summary info updated */
	PS_SUMMARY_EVENT_UPDATED = 0,
	/** Refresh rate has been changed */
	PS_SUMMARY_EVENT_REFRESH_RATE_CHANGED,
	/** Mode has been changed */
	PS_SUMMARY_EVENT_MODE_CHANGED,
	/** for internal use only*/
	PS_SUMMARY_EVENT_COUNT,
};

/**
 * @brief get ps_summary_event_type string value.
 *
 * @param[in]  type  event type to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *ps_summary_event_type_str(enum ps_summary_event_type type);

/**
 * @brief ps_summary object structure
 *
 * This opaque structure represent an ps_summary object.
 **/
struct ps_summary;

/**
 * @brief ps_summary object event structure
 *
 * This opaque structure represent an ps_summary object event.
 **/
struct ps_summary_event;

/**
 * @brief ps bus event structure
 *
 * This opaque structure represent an ps bus event.
 **/
struct ps_bus_event;

/**
 * @brief ps_summary object info fields structure.
 *
 * This structure contains a presence bit for each fields
 * (property or method state) in ps_summary object.
 * When a bit is set, the corresponding field in
 * @ref ps_summary_info structure must be taken into account.
 **/
struct ps_summary_info_fields {
	/** pcpus field presence bit */
	unsigned int pcpus:1;
	/** task_total field presence bit */
	unsigned int task_total:1;
	/** task_running field presence bit */
	unsigned int task_running:1;
	/** task_sleeping field presence bit */
	unsigned int task_sleeping:1;
	/** task_stopped field presence bit */
	unsigned int task_stopped:1;
	/** task_zombie field presence bit */
	unsigned int task_zombie:1;
	/** refresh_rate field presence bit */
	unsigned int refresh_rate:1;
	/** mode field presence bit */
	unsigned int mode:1;
	/** set_refresh_rate method presence bit */
	unsigned int method_set_refresh_rate:1;
	/** set_mode method presence bit */
	unsigned int method_set_mode:1;
};

/**
 * @brief ps_summary object info structure.
 *
 * This structure represent ps_summary object contents.
 **/
struct ps_summary_info {
	/** fields presence bit structure */
	struct ps_summary_info_fields fields;
	/** Cpu usage in percent for each cpu */
	const float *pcpus;
	/** size of pcpus array */
	uint32_t n_pcpus;
	/** Total number of tasks (processes) */
	uint32_t task_total;
	/** Total number of running tasks */
	uint32_t task_running;
	/** Total number of sleeping tasks */
	uint32_t task_sleeping;
	/** Total number of stopped tasks */
	uint32_t task_stopped;
	/** Total number of zombie tasks */
	uint32_t task_zombie;
	/** Refresh rate in seconds */
	uint32_t refresh_rate;
	/** Mode for PCPU */
	enum ps_summary_mode mode;
	/** method set_refresh_rate state */
	enum obus_method_state method_set_refresh_rate;
	/** method set_mode state */
	enum obus_method_state method_set_mode;
};

/**
 * @brief read current ps_summary object fields values.
 *
 * This function is used to read current object fields values.
 *
 * @param[in]  object  ps_summary object.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  object is NULL or not an ps_summary object.
 *
 * @note: object info pointer returned never changed during object life cycle
 * so that user may keep a reference on this pointer until object destruction.
 * this is not the case for info pointers members.
 **/

const struct ps_summary_info *
ps_summary_get_info(const struct ps_summary *object);

/**
 * @brief log ps_summary object.
 *
 * This function log object and its current fields values.
 *
 * @param[in]  object  ps_summary object.
 * @param[in]  level   obus log level.
 **/
void ps_summary_log(const struct ps_summary *object, enum obus_log_level level);

/**
 * @brief set ps_summary object user data pointer.
 *
 * This function store a user data pointer in a ps_summary object.
 * This pointer is never used by libobus and can be retrieved using
 * @ref ps_summary_get_user_data function.
 *
 * @param[in]  object      ps_summary object.
 * @param[in]  user_data   user data pointer.
 *
 * @retval  0        success.
 * @retval  -EINVAL  object is NULL.
 **/
int ps_summary_set_user_data(struct ps_summary *object, void *user_data);

/**
 * @brief get ps_summary object user data pointer.
 *
 * This function retrieve user data pointer stored in a ps_summary object
 * by a previous call to @ref ps_summary_set_user_data function
 *
 * @param[in]  object  ps_summary object.
 *
 * @retval  user_data  user data pointer.
 **/
void *ps_summary_get_user_data(const struct ps_summary *object);

/**
 * @brief get registered ps_summary object obus handle.
 *
 * This function retrieve ps_summary object obus handle.
 * obus handle is an unsigned 16 bits integer.
 * object handle is generated during object creation.
 * object handle can be used to reference an object into another one.
 *
 * @param[in]  object  ps_summary object.
 *
 * @retval  handle               registered object obus handle.
 * @retval  OBUS_INVALID_HANDLE  if object is not registered.
 **/
obus_handle_t ps_summary_get_handle(const struct ps_summary *object);

/**
 * @brief get ps_summary object from obus handle.
 *
 * This function retrieve ps_summary object given its obus handle.
 *
 * @param[in]  client  ps bus client
 * @param[in]  handle  ps_summary object handle.
 *
 * @retval  object  ps_summary object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a ps_summary.
 **/
struct ps_summary *
ps_summary_from_handle(struct obus_client *client, obus_handle_t handle);

/**
 * @brief get next registered ps_summary object in bus.
 *
 * This function retrieve the next registered ps_summary object in bus.
 *
 * @param[in]  client    ps bus client
 * @param[in]  previous  previous ps_summary object in list (may be NULL).
 *
 * @retval  object  next ps_summary object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    no more ps_summary objects in bus.
 *
 * @note: if @p previous is NULL, then the first
 * registered ps_summary object is returned.
 **/
struct ps_summary *
ps_summary_next(struct obus_client *client, struct ps_summary *previous);

/**
 * @brief get ps_summary event type.
 *
 * This function is used to retrieved ps_summary event type.
 *
 * @param[in]  event  ps_summary event.
 *
 * @retval     type   ps_summary event type.
 **/
enum ps_summary_event_type
ps_summary_event_get_type(const struct ps_summary_event *event);

/**
 * @brief log ps_summary event.
 *
 * This function log ps_summary event and its associated fields values.
 *
 * @param[in]  event   ps_summary event.
 * @param[in]  level   obus log level.
 **/
void ps_summary_event_log(const struct ps_summary_event *event,
			  enum obus_log_level level);

/**
 * @brief check ps_summary event contents is empty.
 *
 * This function check if each event field has its presence bit cleared.
 *
 * @param[in]  event   ps_summary event.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int ps_summary_event_is_empty(const struct ps_summary_event *event);

/**
 * @brief commit ps_summary event contents in object.
 *
 * This function copy ps_summary event contents in object.
 *
 * @param[in]  event   ps_summary event.
 *
 * @retval     0     Commit succeed.
 * @retval     <0    Commit failed.
 *
 * @note: if not call by client, event commit is done internally
 * on client provider event callback return.
 **/
int ps_summary_event_commit(struct ps_summary_event *event);

/**
 * @brief read ps_summary event associated fields values.
 *
 * This function is used to read event fields values.
 *
 * @param[in]  event   ps_summary event.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  event is NULL or not an ps_summary object event.
 **/
const struct ps_summary_info *
ps_summary_event_get_info(const struct ps_summary_event *event);

/**
 * generic ps_summary client method status callback
 **/
typedef void (*ps_summary_method_status_cb_t) (struct ps_summary *object,
					       obus_handle_t handle,
					       enum obus_call_status status);
/**
 * @brief ps_summary method set_refresh_rate arguments presence structure.
 *
 * This structure contains a presence bit for each
 * of ps_summary method set_refresh_rate argument.
 * When a bit is set, the corresponding argument in
 * @ref ps_summary_set_refresh_rate_args_fields structure must be taken into account.
 **/
struct ps_summary_set_refresh_rate_args_fields {
	/** presence bit for argument refresh_rate */
	unsigned int refresh_rate:1;
};

/**
 * @brief ps_summary method set_refresh_rate arguments structure.
 *
 * This structure contains ps_summary method set_refresh_rate arguments values.
 **/
struct ps_summary_set_refresh_rate_args {
	/** arguments presence bit structure */
	struct ps_summary_set_refresh_rate_args_fields fields;
	/** Refresh rate in seconds */
	uint32_t refresh_rate;
};

/**
 * @brief initialize @ref ps_summary_set_refresh_rate_args structure.
 *
 * This function initialize @ref ps_summary_set_refresh_rate_args structure.
 * Each argument field has its presence bit cleared.
 *
 * @param[in]  args  pointer to allocated @ref ps_summary_set_refresh_rate_args structure.
 **/
void ps_summary_set_refresh_rate_args_init(struct
					   ps_summary_set_refresh_rate_args
					   *args);

/**
 * @brief check @ref ps_summary_set_refresh_rate_args structure contents is empty.
 *
 * This function check if each argument field has its presence bit cleared.
 *
 * @param[in]  args  @ref ps_summary_set_refresh_rate_args structure.
 *
 * @retval     1     Each argument field has its presence bit cleared.
 * @retval     0     One argument field (or more) has its presence bit set.
 **/
int ps_summary_set_refresh_rate_args_is_empty(const struct
					      ps_summary_set_refresh_rate_args
					      *args);

/**
 * @brief call method 'set_refresh_rate'.
 *
 * This function call method 'set_refresh_rate' on a ps_summary object
 *
 * Set the refresh rate
 *
 * @param[in]   client  obus client context.
 * @param[in]   object  ps_summary object.
 * @param[in]   args    call arguments.
 * @param[in]   cb      call status callback.
 * @param[out]  handle  call handle.
 *
 * @retval      0      Call request has been sent to server.
 * @retval   -EINVAL   Invalid function arguments.
 * @retval   -EPERM    Client is not connected.
 * @retval   -EPERM    Object is not registered.
 * @retval   -EPERM    Method is not ENABLED.
 * @retval   -ENOMEM   Memory error.
 **/
int ps_summary_call_set_refresh_rate(struct obus_client *client,
				     struct ps_summary *object,
				     const struct
				     ps_summary_set_refresh_rate_args *args,
				     ps_summary_method_status_cb_t cb,
				     uint16_t *handle);
/**
 * @brief ps_summary method set_mode arguments presence structure.
 *
 * This structure contains a presence bit for each
 * of ps_summary method set_mode argument.
 * When a bit is set, the corresponding argument in
 * @ref ps_summary_set_mode_args_fields structure must be taken into account.
 **/
struct ps_summary_set_mode_args_fields {
	/** presence bit for argument mode */
	unsigned int mode:1;
};

/**
 * @brief ps_summary method set_mode arguments structure.
 *
 * This structure contains ps_summary method set_mode arguments values.
 **/
struct ps_summary_set_mode_args {
	/** arguments presence bit structure */
	struct ps_summary_set_mode_args_fields fields;
	/** Mode */
	enum ps_summary_mode mode;
};

/**
 * @brief initialize @ref ps_summary_set_mode_args structure.
 *
 * This function initialize @ref ps_summary_set_mode_args structure.
 * Each argument field has its presence bit cleared.
 *
 * @param[in]  args  pointer to allocated @ref ps_summary_set_mode_args structure.
 **/
void ps_summary_set_mode_args_init(struct ps_summary_set_mode_args *args);

/**
 * @brief check @ref ps_summary_set_mode_args structure contents is empty.
 *
 * This function check if each argument field has its presence bit cleared.
 *
 * @param[in]  args  @ref ps_summary_set_mode_args structure.
 *
 * @retval     1     Each argument field has its presence bit cleared.
 * @retval     0     One argument field (or more) has its presence bit set.
 **/
int ps_summary_set_mode_args_is_empty(const struct ps_summary_set_mode_args
				      *args);

/**
 * @brief call method 'set_mode'.
 *
 * This function call method 'set_mode' on a ps_summary object
 *
 * Set the mode for pcpu
 *
 * @param[in]   client  obus client context.
 * @param[in]   object  ps_summary object.
 * @param[in]   args    call arguments.
 * @param[in]   cb      call status callback.
 * @param[out]  handle  call handle.
 *
 * @retval      0      Call request has been sent to server.
 * @retval   -EINVAL   Invalid function arguments.
 * @retval   -EPERM    Client is not connected.
 * @retval   -EPERM    Object is not registered.
 * @retval   -EPERM    Method is not ENABLED.
 * @retval   -ENOMEM   Memory error.
 **/
int ps_summary_call_set_mode(struct obus_client *client,
			     struct ps_summary *object,
			     const struct ps_summary_set_mode_args *args,
			     ps_summary_method_status_cb_t cb,
			     uint16_t *handle);

/* ps_summary object provider api */

/**

 * @struct ps_summary_provider

 * @brief callbacks for events on ps_summary objects
 */

struct ps_summary_provider {
	/** for internal use only */
	struct obus_provider *priv;
	/** called on a ps_summary object apparition */
	void (*add) (struct ps_summary *object,
		     struct ps_bus_event *bus_event, void *user_data);
	/** called on a ps_summary object removal */
	void (*remove) (struct ps_summary *object,
			struct ps_bus_event *bus_event, void *user_data);
	/** called on ps_summary object events */
	void (*event) (struct ps_summary *object,
		       struct ps_summary_event *event,
		       struct ps_bus_event *bus_event, void *user_data);
};

/**
 * @brief subscribe to events concerning ps_summary objects.
 *
 * @param[in] client bus client.
 * @param[in] provider callback set for reacting on ps_summary events.
 * @param[in] user_data data passed to callbacks on events.
 *
 * @retval 0 success.
 **/
int ps_summary_subscribe(struct obus_client *client,
			 struct ps_summary_provider *provider, void *user_data);

/**
 * @brief unsubscribe to events concerning ps_summary objects.
 *
 * @param[in] client bus client.
 * @param[in] provider passed to ps_summary_subscribe.
 *
 * @retval 0 success.
 **/ int ps_summary_unsubscribe(struct obus_client *client,
				struct ps_summary_provider *provider);

OBUS_END_DECLS

#endif /*_PS_SUMMARY_H_*/
