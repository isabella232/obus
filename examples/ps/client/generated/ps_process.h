/**
 * @file ps_process.h
 *
 * @brief obus ps_process object client api
 *
 * @author obusgen 1.0.3 generated file, do not modify it.
 */
#ifndef _PS_PROCESS_H_
#define _PS_PROCESS_H_

#include "libobus.h"

OBUS_BEGIN_DECLS

/**
 * @brief ps_process object uid
 **/
#define PS_PROCESS_UID 1

/**
 * @brief ps_process state enumeration.
 *
 * Process state
 **/
enum ps_process_state {
	/** Unknown */
	PS_PROCESS_STATE_UNKNOWN = -1,
	/** Running */
	PS_PROCESS_STATE_RUNNING = 0,
	/** Sleeping */
	PS_PROCESS_STATE_SLEEPING = 1,
	/** Stopped */
	PS_PROCESS_STATE_STOPPED = 2,
	/** Zombie */
	PS_PROCESS_STATE_ZOMBIE = 3,
};

/**
 * @brief check if value is one of enum ps_process_state values.
 *
 * @param[in]  value  value to be checked.
 *
 * @retval 1 value is one of ps_process_state values.
 * @retval 0 value is not one of ps_process_state values.
 **/
int ps_process_state_is_valid(int32_t value);

/**
 * @brief get ps_process_state string value.
 *
 * @param[in]  value  state value to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *ps_process_state_str(enum ps_process_state value);

/**
 * @brief ps_process event type enumeration.
 *
 * This enumeration describes all kind of ps_process events.
 **/
enum ps_process_event_type {
	/** Process info updated */
	PS_PROCESS_EVENT_UPDATED = 0,
	/** for internal use only*/
	PS_PROCESS_EVENT_COUNT,
};

/**
 * @brief get ps_process_event_type string value.
 *
 * @param[in]  type  event type to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *ps_process_event_type_str(enum ps_process_event_type type);

/**
 * @brief ps_process object structure
 *
 * This opaque structure represent an ps_process object.
 **/
struct ps_process;

/**
 * @brief ps_process object event structure
 *
 * This opaque structure represent an ps_process object event.
 **/
struct ps_process_event;

/**
 * @brief ps bus event structure
 *
 * This opaque structure represent an ps bus event.
 **/
struct ps_bus_event;

/**
 * @brief ps_process object info fields structure.
 *
 * This structure contains a presence bit for each fields
 * (property or method state) in ps_process object.
 * When a bit is set, the corresponding field in
 * @ref ps_process_info structure must be taken into account.
 **/
struct ps_process_info_fields {
	/** pid field presence bit */
	unsigned int pid:1;
	/** ppid field presence bit */
	unsigned int ppid:1;
	/** name field presence bit */
	unsigned int name:1;
	/** exe field presence bit */
	unsigned int exe:1;
	/** pcpu field presence bit */
	unsigned int pcpu:1;
	/** state field presence bit */
	unsigned int state:1;
};

/**
 * @brief ps_process object info structure.
 *
 * This structure represent ps_process object contents.
 **/
struct ps_process_info {
	/** fields presence bit structure */
	struct ps_process_info_fields fields;
	/** Process id */
	uint32_t pid;
	/** Parrent process id */
	uint32_t ppid;
	/** Process name */
	const char *name;
	/** Excutable name */
	const char *exe;
	/** Cpu usage in percent */
	float pcpu;
	/** State */
	enum ps_process_state state;
};

/**
 * @brief read current ps_process object fields values.
 *
 * This function is used to read current object fields values.
 *
 * @param[in]  object  ps_process object.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  object is NULL or not an ps_process object.
 *
 * @note: object info pointer returned never changed during object life cycle
 * so that user may keep a reference on this pointer until object destruction.
 * this is not the case for info pointers members.
 **/

const struct ps_process_info *
ps_process_get_info(const struct ps_process *object);

/**
 * @brief log ps_process object.
 *
 * This function log object and its current fields values.
 *
 * @param[in]  object  ps_process object.
 * @param[in]  level   obus log level.
 **/
void ps_process_log(const struct ps_process *object, enum obus_log_level level);

/**
 * @brief set ps_process object user data pointer.
 *
 * This function store a user data pointer in a ps_process object.
 * This pointer is never used by libobus and can be retrieved using
 * @ref ps_process_get_user_data function.
 *
 * @param[in]  object      ps_process object.
 * @param[in]  user_data   user data pointer.
 *
 * @retval  0        success.
 * @retval  -EINVAL  object is NULL.
 **/
int ps_process_set_user_data(struct ps_process *object, void *user_data);

/**
 * @brief get ps_process object user data pointer.
 *
 * This function retrieve user data pointer stored in a ps_process object
 * by a previous call to @ref ps_process_set_user_data function
 *
 * @param[in]  object  ps_process object.
 *
 * @retval  user_data  user data pointer.
 **/
void *ps_process_get_user_data(const struct ps_process *object);

/**
 * @brief get registered ps_process object obus handle.
 *
 * This function retrieve ps_process object obus handle.
 * obus handle is an unsigned 16 bits integer.
 * object handle is generated during object creation.
 * object handle can be used to reference an object into another one.
 *
 * @param[in]  object  ps_process object.
 *
 * @retval  handle               registered object obus handle.
 * @retval  OBUS_INVALID_HANDLE  if object is not registered.
 **/
obus_handle_t ps_process_get_handle(const struct ps_process *object);

/**
 * @brief get ps_process object from obus handle.
 *
 * This function retrieve ps_process object given its obus handle.
 *
 * @param[in]  client  ps bus client
 * @param[in]  handle  ps_process object handle.
 *
 * @retval  object  ps_process object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a ps_process.
 **/
struct ps_process *
ps_process_from_handle(struct obus_client *client, obus_handle_t handle);

/**
 * @brief get next registered ps_process object in bus.
 *
 * This function retrieve the next registered ps_process object in bus.
 *
 * @param[in]  client    ps bus client
 * @param[in]  previous  previous ps_process object in list (may be NULL).
 *
 * @retval  object  next ps_process object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    no more ps_process objects in bus.
 *
 * @note: if @p previous is NULL, then the first
 * registered ps_process object is returned.
 **/
struct ps_process *
ps_process_next(struct obus_client *client, struct ps_process *previous);

/**
 * @brief get ps_process event type.
 *
 * This function is used to retrieved ps_process event type.
 *
 * @param[in]  event  ps_process event.
 *
 * @retval     type   ps_process event type.
 **/
enum ps_process_event_type
ps_process_event_get_type(const struct ps_process_event *event);

/**
 * @brief log ps_process event.
 *
 * This function log ps_process event and its associated fields values.
 *
 * @param[in]  event   ps_process event.
 * @param[in]  level   obus log level.
 **/
void ps_process_event_log(const struct ps_process_event *event,
			  enum obus_log_level level);

/**
 * @brief check ps_process event contents is empty.
 *
 * This function check if each event field has its presence bit cleared.
 *
 * @param[in]  event   ps_process event.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int ps_process_event_is_empty(const struct ps_process_event *event);

/**
 * @brief commit ps_process event contents in object.
 *
 * This function copy ps_process event contents in object.
 *
 * @param[in]  event   ps_process event.
 *
 * @retval     0     Commit succeed.
 * @retval     <0    Commit failed.
 *
 * @note: if not call by client, event commit is done internally
 * on client provider event callback return.
 **/
int ps_process_event_commit(struct ps_process_event *event);

/**
 * @brief read ps_process event associated fields values.
 *
 * This function is used to read event fields values.
 *
 * @param[in]  event   ps_process event.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  event is NULL or not an ps_process object event.
 **/
const struct ps_process_info *
ps_process_event_get_info(const struct ps_process_event *event);

/**
 * generic ps_process client method status callback
 **/
typedef void (*ps_process_method_status_cb_t) (struct ps_process *object,
					       obus_handle_t handle,
					       enum obus_call_status status);

/* ps_process object provider api */

/**

 * @struct ps_process_provider

 * @brief callbacks for events on ps_process objects
 */

struct ps_process_provider {
	/** for internal use only */
	struct obus_provider *priv;
	/** called on a ps_process object apparition */
	void (*add) (struct ps_process *object,
		     struct ps_bus_event *bus_event, void *user_data);
	/** called on a ps_process object removal */
	void (*remove) (struct ps_process *object,
			struct ps_bus_event *bus_event, void *user_data);
	/** called on ps_process object events */
	void (*event) (struct ps_process *object,
		       struct ps_process_event *event,
		       struct ps_bus_event *bus_event, void *user_data);
};

/**
 * @brief subscribe to events concerning ps_process objects.
 *
 * @param[in] client bus client.
 * @param[in] provider callback set for reacting on ps_process events.
 * @param[in] user_data data passed to callbacks on events.
 *
 * @retval 0 success.
 **/
int ps_process_subscribe(struct obus_client *client,
			 struct ps_process_provider *provider, void *user_data);

/**
 * @brief unsubscribe to events concerning ps_process objects.
 *
 * @param[in] client bus client.
 * @param[in] provider passed to ps_process_subscribe.
 *
 * @retval 0 success.
 **/ int ps_process_unsubscribe(struct obus_client *client,
				struct ps_process_provider *provider);

OBUS_END_DECLS

#endif /*_PS_PROCESS_H_*/
