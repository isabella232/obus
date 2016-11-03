/**
 * @file ps_process.h
 *
 * @brief obus ps_process object server api
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
 * @brief initialize @ref ps_process_info structure.
 *
 * This function initialize @ref ps_process_info structure.
 * Each field has its presence bit cleared.
 *
 * @param[in]  info  pointer to allocated @ref ps_process_info structure.
 **/
void ps_process_info_init(struct ps_process_info *info);

/**
 * @brief check @ref ps_process_info structure contents is empty.
 *
 * This function check if each field has its presence bit cleared
 *
 * @param[in]  info  @ref ps_process_info structure.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int ps_process_info_is_empty(const struct ps_process_info *info);

/**
 * @brief create a ps_process object.
 *
 * This function allocate ps_process object and initialize each
 * object field with the given @ref info field value only if @ref info field
 * presence bit is set (see @ref ps_process_info_fields).
 * This function also set a unique handle for this object.
 * Registering or unregistering object in bus do not alter handle value
 *
 * @param[in]  server    ps bus server.
 * @param[in]  info      ps_process object initial values (may be NULL).
 *
 * @retval  object  success.
 * @retval  NULL    failure.
 *
 * @note: object does not keep any reference to @ref info so that user shall
 * allocate it on the stack.
 *
 * @note: if @ref info param is NULL, ps_process object is initialized with
 * default values.
 *
 * @note: object is not yet registered on bus,
 * call one of theses functions below to register it:
 * @ref ps_process_register
 * @ref ps_bus_event_register_process
 **/
struct ps_process *
ps_process_new(struct obus_server *srv, const struct ps_process_info *info);

/**
 * @brief destroy a ps_process object.
 *
 * This function release ps_process object memory.
 * Only non registered objects can be destroyed.
 *
 * @param[in]  object  ps_process object to be destroyed.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is registered.
 * @retval  -EINVAL invalid @ref object.
 **/
int ps_process_destroy(struct ps_process *object);

/**
 * @brief register a ps_process object.
 *
 * This function register a ps_process object in ps bus.
 *
 * @param[in]  server  ps bus server.
 * @param[in]  object  ps_process object to be registered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is already registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int ps_process_register(struct obus_server *server, struct ps_process *object);

/**
 * @brief unregister a ps_process object.
 *
 * This function unregister a ps_process object in ps bus.
 *
 * @param[in]  server  ps bus server.
 * @param[in]  object  ps_process object to be unregistered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is not registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int ps_process_unregister(struct obus_server *server,
			  struct ps_process *object);

/**
 * @brief check if is a ps_process object registered.
 *
 * This function check whether a ps_process object is registered or not.
 *
 * @param[in]  object  ps_process object to checked.
 *
 * @retval  0  object is not registered.
 * @retval  1  object is registered.
 **/
int ps_process_is_registered(const struct ps_process *object);

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
 * @param[in]  server  ps bus server
 * @param[in]  handle  ps_process object handle.
 *
 * @retval  object  ps_process object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a ps_process.
 **/
struct ps_process *
ps_process_from_handle(struct obus_server *server, obus_handle_t handle);

/**
 * @brief get next registered ps_process object in bus.
 *
 * This function retrieve the next registered ps_process object in bus.
 *
 * @param[in]  server    ps bus server
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
ps_process_next(struct obus_server *server, struct ps_process *previous);

/**
 * @brief send a ps_process object event.
 *
 * This function send an event on a ps_process object.
 *
 * @param[in]  server    ps bus server.
 * @param[in]  object    ps_process object.
 * @param[in]  type      ps_process event type.
 * @param[in]  info      associated ps_process content to be updated.
 *
 * @retval  0          event sent and object content updated.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is not registered in bus.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int ps_process_send_event(struct obus_server *server, struct ps_process *object,
			  enum ps_process_event_type type,
			  const struct ps_process_info *info);

/**
 * @brief send a ps_process object event through a ps bus event.
 *
 * This function create a ps_process object event and attach it
 * to an existing ps bus event. Created ps_process object event
 * will be sent within corresponding bus event.
 * Unlike @ps_process_send_event, object content will not be updated
 * when this function returns but when
 * @ps_process_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_process object.
 * @param[in]  type       ps_process event type.
 * @param[in]  info       associated ps_process content to be updated.
 *
 * @retval  0          event created and associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -ENOMEM    memory error.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int ps_bus_event_add_process_event(struct ps_bus_event *bus_event,
				   struct ps_process *object,
				   enum ps_process_event_type type,
				   const struct ps_process_info *info);

/**
 * @brief register a ps_process object through a ps bus event.
 *
 * This function set a ps_process object to be registered when
 * associated ps bus event will be sent. Unlike @ps_process_register,
 * object will not be registered when this function returns but when
 * @ps_process_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_process object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int ps_bus_event_register_process(struct ps_bus_event *bus_event,
				  struct ps_process *object);

/**
 * @brief unregister a ps_process object through a ps bus event.
 *
 * This function set ps_process object to be unregistered when
 * associated ps bus event will be sent. Unlike @ps_process_register,
 * object will not be unregistered when this function returns but when
 * @ps_process_bus_event_send will be invoked.
 * Object is then automatically destroyed on
 * @ps_process_bus_event_destroy call.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_process object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int ps_bus_event_unregister_process(struct ps_bus_event *bus_event,
				    struct ps_process *object);

OBUS_END_DECLS

#endif /*_PS_PROCESS_H_*/
