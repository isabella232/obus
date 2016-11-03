/******************************************************************************
* @file ps_summary.h
*
* @brief obus ps_summary object server api
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
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
	/* PCPU of a process can never go above 100 event if SMP */
	PS_SUMMARY_MODE_SOLARIS = 0,
	/* PCPU can go up to ncpus *100 */
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
	/* Process summary info updated */
	PS_SUMMARY_EVENT_UPDATED = 0,
	/* Refresh rate has been changed */
	PS_SUMMARY_EVENT_REFRESH_RATE_CHANGED,
	/* Mode has been changed */
	PS_SUMMARY_EVENT_MODE_CHANGED,
	/* for internal use only */
	PS_SUMMARY_EVENT_COUNT,
};

/**
 * @brief get ps_summary_event_type string value.
 *
 * @param[in]  value  event type to be converted into string.
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
 * @ps_summary_info structure must be taken into account.
 **/
struct ps_summary_info_fields {
	unsigned int pcpus:1;
	unsigned int task_total:1;
	unsigned int task_running:1;
	unsigned int task_sleeping:1;
	unsigned int task_stopped:1;
	unsigned int task_zombie:1;
	unsigned int refresh_rate:1;
	unsigned int mode:1;
	unsigned int method_set_refresh_rate:1;
	unsigned int method_set_mode:1;
};

/**
 * @brief ps_summary object info structure.
 *
 * This structure represent ps_summary object contents.
 **/
struct ps_summary_info {
	/* fields presence bit structure */
	struct ps_summary_info_fields fields;
	/* Cpu usage in percent for each cpu */
	const uint32_t *pcpus;
	/* size of pcpus array */
	uint32_t n_pcpus;
	/* Total number of tasks (processes) */
	uint32_t task_total;
	/* Total number of running tasks */
	uint32_t task_running;
	/* Total number of sleeping tasks */
	uint32_t task_sleeping;
	/* Total number of stopped tasks */
	uint32_t task_stopped;
	/* Total number of zombie tasks */
	uint32_t task_zombie;
	/* Refresh rate in seconds */
	uint32_t refresh_rate;
	/* Mode for PCPU */
	enum ps_summary_mode mode;
	/* method set_refresh_rate state */
	enum obus_method_state method_set_refresh_rate;
	/* method set_mode state */
	enum obus_method_state method_set_mode;
};

/**
 * @brief ps_summary method set_refresh_rate arguments presence structure.
 *
 * This structure contains a presence bit for each
 * of ps_summary method set_refresh_rate argument.
 * When a bit is set, the corresponding argument in
 * @ps_summary_set_refresh_rate_args_fields structure must be taken into account.
 **/
struct ps_summary_set_refresh_rate_args_fields {
	unsigned int refresh_rate:1;
};

/**
 * @brief ps_summary method set_refresh_rate arguments structure.
 *
 * This structure contains ps_summary method set_refresh_rate arguments values.
 **/
struct ps_summary_set_refresh_rate_args {
	/* arguments presence bit structure */
	struct ps_summary_set_refresh_rate_args_fields fields;
	/* Refresh rate in seconds */
	uint32_t refresh_rate;
};
/**
 * @brief ps_summary method set_mode arguments presence structure.
 *
 * This structure contains a presence bit for each
 * of ps_summary method set_mode argument.
 * When a bit is set, the corresponding argument in
 * @ps_summary_set_mode_args_fields structure must be taken into account.
 **/
struct ps_summary_set_mode_args_fields {
	unsigned int mode:1;
};

/**
 * @brief ps_summary method set_mode arguments structure.
 *
 * This structure contains ps_summary method set_mode arguments values.
 **/
struct ps_summary_set_mode_args {
	/* arguments presence bit structure */
	struct ps_summary_set_mode_args_fields fields;
	/* Mode */
	enum ps_summary_mode mode;
};

/**
 * @brief ps_summary method handlers structure.
 *
 * This structure contains pointer to ps_summary methods implementations.
 * Server must implement theses callback and fill up
 * this structure before creating a ps_summary object.
 **/
struct ps_summary_method_handlers {
	/**
	 * @brief ps_summary method set_refresh_rate handler.
	 *
	 * Set the refresh rate
	 *
	 * @param[in]  object  ps_summary object.
	 * @param[in]  handle  client call sequence id.
	 * @param[in]  args    method set_refresh_rate call arguments.
	 **/
	void (*method_set_refresh_rate) (struct ps_summary *object,
					 obus_handle_t handle,
					 const struct
					 ps_summary_set_refresh_rate_args *
					 args);

	/**
	 * @brief ps_summary method set_mode handler.
	 *
	 * Set the mode for PCPU
	 *
	 * @param[in]  object  ps_summary object.
	 * @param[in]  handle  client call sequence id.
	 * @param[in]  args    method set_mode call arguments.
	 **/
	void (*method_set_mode) (struct ps_summary *object,
				 obus_handle_t handle,
				 const struct ps_summary_set_mode_args *args);
};

/**
 * @brief check @ps_summary_method_handlers structure is valid.
 *
 * @param[in]  handlers  ps_summary methods handlers.
 *
 * @retval     1         all methods have non NULL handler.
 * @retval     0         one method (or more) has a NULL handler.
 **/
int ps_summary_method_handlers_is_valid(const struct ps_summary_method_handlers
					*handlers);

/**
 * @brief check @ps_summary_set_refresh_rate_args structure is complete (all arguments are present).
 *
 * @param[in]  args ps_summary set_refresh_rate method arguments.
 *
 * @retval     1         all methods arguments are present.
 * @retval     0         one method (or more) argument is missing.
 **/
int ps_summary_set_refresh_rate_args_is_complete(const struct
						 ps_summary_set_refresh_rate_args
						 *args);

/**
 * @brief check @ps_summary_set_mode_args structure is complete (all arguments are present).
 *
 * @param[in]  args ps_summary set_mode method arguments.
 *
 * @retval     1         all methods arguments are present.
 * @retval     0         one method (or more) argument is missing.
 **/
int ps_summary_set_mode_args_is_complete(const struct ps_summary_set_mode_args
					 *args);

/**
 * @brief initialize @ps_summary_info structure.
 *
 * This function initialize @ps_summary_info structure.
 * Each field has its presence bit cleared.
 *
 * @param[in]  info  pointer to allocated @ps_summary_info structure.
 **/
void ps_summary_info_init(struct ps_summary_info *info);

/**
 * @brief check @ps_summary_info structure contents is empty.
 *
 * This function check if each field has its presence bit cleared
 *
 * @param[in]  info  @ps_summary_info structure.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int ps_summary_info_is_empty(const struct ps_summary_info *info);

/**
 * @brief set @ps_summary_info methods state.
 *
 * This function set all ps_summary methods state to given argument state
 *
 * @param[in]  info   @ps_summary_info structure.
 * @param[in]  state  new methods state.
 **/
void ps_summary_info_set_methods_state(struct ps_summary_info *info,
				       enum obus_method_state state);

/**
 * @brief create a ps_summary object.
 *
 * This function allocate ps_summary object and initialize each
 * object field with the given @ref info field value only if @ref info field
 * presence bit is set (see @ref ps_summary_info_fields).
 * This function also set a unique handle for this object.
 * Registering or unregistering object in bus do not alter handle value
 *
 * @param[in]  server    ps bus server.
 * @param[in]  info      ps_summary object initial values (may be NULL).
 * @param[in]  handlers  ps_summary method handlers.
 *
 * @retval  object  success.
 * @retval  NULL    failure.
 *
 * @note: object does not keep any reference to @ref info so that user shall
 * allocate it on the stack.
 *
 * @note: if @ref info param is NULL, ps_summary object is initialized with
 * default values.
 *
 * @note: object is not yet registered on bus,
 * call one of theses functions below to register it:
 * @ref ps_summary_register
 * @ref ps_bus_event_register_summary
 **/
struct ps_summary *
ps_summary_new(struct obus_server *srv, const struct ps_summary_info *info,
	       const struct ps_summary_method_handlers *handlers);

/**
 * @brief destroy a ps_summary object.
 *
 * This function release ps_summary object memory.
 * Only non registered objects can be destroyed.
 *
 * @param[in]  object  ps_summary object to be destroyed.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is registered.
 * @retval  -EINVAL invalid @ref object.
 **/
int ps_summary_destroy(struct ps_summary *object);

/**
 * @brief register a ps_summary object.
 *
 * This function register a ps_summary object in ps bus.
 *
 * @param[in]  server  ps bus server.
 * @param[in]  object  ps_summary object to be registered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is already registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int ps_summary_register(struct obus_server *server, struct ps_summary *object);

/**
 * @brief unregister a ps_summary object.
 *
 * This function unregister a ps_summary object in ps bus.
 *
 * @param[in]  server  ps bus server.
 * @param[in]  object  ps_summary object to be unregistered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is not registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int ps_summary_unregister(struct obus_server *server,
			  struct ps_summary *object);

/**
 * @brief check if is a ps_summary object registered.
 *
 * This function check whether a ps_summary object is registered or not.
 *
 * @param[in]  object  ps_summary object to checked.
 *
 * @retval  0  object is not registered.
 * @retval  1  object is registered.
 **/
int ps_summary_is_registered(const struct ps_summary *object);

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
 * @param[in]  server  ps bus server
 * @param[in]  handle  ps_summary object handle.
 *
 * @retval  object  ps_summary object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a ps_summary.
 **/
struct ps_summary *
ps_summary_from_handle(struct obus_server *server, obus_handle_t handle);

/**
 * @brief get next registered ps_summary object in bus.
 *
 * This function retrieve the next registered ps_summary object in bus.
 *
 * @param[in]  server    ps bus server
 * @param[in]  previous  previous ps_summary object in list (may be NULL).
 *
 * @retval  object  next ps_summary object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    no more ps_summary objects in bus.
 *
 * @note: if @ref previous is NULL, then the first
 * registered ps_summary object is returned.
 **/
struct ps_summary *
ps_summary_next(struct obus_server *server, struct ps_summary *previous);

/**
 * @brief send a ps_summary object event.
 *
 * This function send an event on a ps_summary object.
 *
 * @param[in]  server    ps bus server.
 * @param[in]  object    ps_summary object.
 * @param[in]  type      ps_summary event type.
 * @param[in]  info      associated ps_summary content to be updated.
 *
 * @retval  0          event sent and object content updated.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is not registered in bus.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int ps_summary_send_event(struct obus_server *server, struct ps_summary *object,
			  enum ps_summary_event_type type,
			  const struct ps_summary_info *info);

/**
 * @brief send a ps_summary object event through a ps bus event.
 *
 * This function create a ps_summary object event and attach it
 * to an existing ps bus event. Created ps_summary object event
 * will be sent within corresponding bus event.
 * Unlike @ps_summary_send_event, object content will not be updated
 * when this function returns but when
 * @ps_summary_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_summary object.
 * @param[in]  type       ps_summary event type.
 * @param[in]  info       associated ps_summary content to be updated.
 *
 * @retval  0          event created and associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -ENOMEM    memory error.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int ps_bus_event_add_summary_event(struct ps_bus_event *bus_event,
				   struct ps_summary *object,
				   enum ps_summary_event_type type,
				   const struct ps_summary_info *info);

/**
 * @brief register a ps_summary object through a ps bus event.
 *
 * This function set a ps_summary object to be registered when
 * associated ps bus event will be sent. Unlike @ps_summary_register,
 * object will not be registered when this function returns but when
 * @ps_summary_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_summary object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int ps_bus_event_register_summary(struct ps_bus_event *bus_event,
				  struct ps_summary *object);

/**
 * @brief unregister a ps_summary object through a ps bus event.
 *
 * This function set ps_summary object to be unregistered when
 * associated ps bus event will be sent. Unlike @ps_summary_register,
 * object will not be unregistered when this function returns but when
 * @ps_summary_bus_event_send will be invoked.
 * Object is then automatically destroyed on
 * @ps_summary_bus_event_destroy call.
 *
 * @param[in]  bus_event  ps bus event.
 * @param[in]  object     ps_summary object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int ps_bus_event_unregister_summary(struct ps_bus_event *bus_event,
				    struct ps_summary *object);

OBUS_END_DECLS

#endif /*_PS_SUMMARY_H_*/
