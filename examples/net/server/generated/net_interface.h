/******************************************************************************
* @file net_interface.h
*
* @brief obus net_interface object server api
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
#ifndef _NET_INTERFACE_H_
#define _NET_INTERFACE_H_

#include "libobus.h"

OBUS_BEGIN_DECLS

/**
 * @brief net_interface object uid
 **/
#define NET_INTERFACE_UID 1

/**
 * @brief net_interface state enumeration.
 *
 * interface state
 **/
enum net_interface_state {
	/* interface is activated */
	NET_INTERFACE_STATE_UP = 1,
	/* interface is not activated */
	NET_INTERFACE_STATE_DOWN = -3,
};

/**
 * @brief check if value is one of enum net_interface_state values.
 *
 * @param[in]  value  value to be checked.
 *
 * @retval 1 value is one of net_interface_state values.
 * @retval 0 value is not one of net_interface_state values.
 **/
int net_interface_state_is_valid(int32_t value);

/**
 * @brief get net_interface_state string value.
 *
 * @param[in]  value  state value to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *net_interface_state_str(enum net_interface_state value);

/**
 * @brief net_interface event type enumeration.
 *
 * This enumeration describes all kind of net_interface events.
 **/
enum net_interface_event_type {
	/* interface is activated */
	NET_INTERFACE_EVENT_UP = 0,
	/* interface is deactivated */
	NET_INTERFACE_EVENT_DOWN,
	/* interface is configured */
	NET_INTERFACE_EVENT_CONFIGURED,
	/* interface sent and received bytes updated */
	NET_INTERFACE_EVENT_TRAFFIC,
	/* interface method up failed */
	NET_INTERFACE_EVENT_UP_FAILED,
	/* interface method down failed */
	NET_INTERFACE_EVENT_DOWN_FAILED,
	/* for internal use only */
	NET_INTERFACE_EVENT_COUNT,
};

/**
 * @brief get net_interface_event_type string value.
 *
 * @param[in]  value  event type to be converted into string.
 *
 * @retval non NULL constant string value.
 **/
const char *net_interface_event_type_str(enum net_interface_event_type type);

/**
 * @brief net_interface object structure
 *
 * This opaque structure represent an net_interface object.
 **/
struct net_interface;

/**
 * @brief net bus event structure
 *
 * This opaque structure represent an net bus event.
 **/
struct net_bus_event;

/**
 * @brief net_interface object info fields structure.
 *
 * This structure contains a presence bit for each fields
 * (property or method state) in net_interface object.
 * When a bit is set, the corresponding field in
 * @net_interface_info structure must be taken into account.
 **/
struct net_interface_info_fields {
	unsigned int name:1;
	unsigned int state:1;
	unsigned int hw_addr:1;
	unsigned int ip_addr:1;
	unsigned int broadcast:1;
	unsigned int netmask:1;
	unsigned int bytes:1;
	unsigned int method_up:1;
	unsigned int method_down:1;
};

/**
 * @brief net_interface object info structure.
 *
 * This structure represent net_interface object contents.
 **/
struct net_interface_info {
	/* fields presence bit structure */
	struct net_interface_info_fields fields;
	/* interface name (ex: 'eth0') */
	const char *name;
	/* current interface state */
	enum net_interface_state state;
	/* interface hardware address */
	const char *hw_addr;
	/* interface ip address */
	const char *ip_addr;
	/* interface broadcast address */
	const char *broadcast;
	/* interface netmask */
	const char *netmask;
	/* number of bytes sent and received */
	const uint64_t *bytes;
	/* size of bytes array */
	uint32_t n_bytes;
	/* method up state */
	enum obus_method_state method_up;
	/* method down state */
	enum obus_method_state method_down;
};

/**
 * @brief net_interface method up arguments presence structure.
 *
 * This structure contains a presence bit for each
 * of net_interface method up argument.
 * When a bit is set, the corresponding argument in
 * @net_interface_up_args_fields structure must be taken into account.
 **/
struct net_interface_up_args_fields {
	unsigned int ip_addr:1;
	unsigned int netmask:1;
};

/**
 * @brief net_interface method up arguments structure.
 *
 * This structure contains net_interface method up arguments values.
 **/
struct net_interface_up_args {
	/* arguments presence bit structure */
	struct net_interface_up_args_fields fields;
	/* optional ip address to be used by interface */
	const char *ip_addr;
	/* optional netmask to be used by interface */
	const char *netmask;
};

/**
 * @brief net_interface method handlers structure.
 *
 * This structure contains pointer to net_interface methods implementations.
 * Server must implement theses callback and fill up
 * this structure before creating a net_interface object.
 **/
struct net_interface_method_handlers {
	/**
	 * @brief net_interface method up handler.
	 *
	 * activate network interface
	 *
	 * @param[in]  object  net_interface object.
	 * @param[in]  handle  client call sequence id.
	 * @param[in]  args    method up call arguments.
	 **/
	void (*method_up) (struct net_interface *object, obus_handle_t handle,
			   const struct net_interface_up_args *args);

	/**
	 * @brief net_interface method down handler.
	 *
	 * deactivate network interface
	 *
	 * @param[in]  object  net_interface object.
	 * @param[in]  handle  client call sequence id.
	 * @param[in]  unused  unused argument.
	 **/
	void (*method_down) (struct net_interface *object,
			     obus_handle_t handle, void *unused);
};

/**
 * @brief check @net_interface_method_handlers structure is valid.
 *
 * @param[in]  handlers  net_interface methods handlers.
 *
 * @retval     1         all methods have non NULL handler.
 * @retval     0         one method (or more) has a NULL handler.
 **/
int net_interface_method_handlers_is_valid(const struct
					   net_interface_method_handlers
					   *handlers);

/**
 * @brief check @net_interface_up_args structure is complete (all arguments are present).
 *
 * @param[in]  args net_interface up method arguments.
 *
 * @retval     1         all methods arguments are present.
 * @retval     0         one method (or more) argument is missing.
 **/
int net_interface_up_args_is_complete(const struct net_interface_up_args *args);

/**
 * @brief initialize @net_interface_info structure.
 *
 * This function initialize @net_interface_info structure.
 * Each field has its presence bit cleared.
 *
 * @param[in]  info  pointer to allocated @net_interface_info structure.
 **/
void net_interface_info_init(struct net_interface_info *info);

/**
 * @brief check @net_interface_info structure contents is empty.
 *
 * This function check if each field has its presence bit cleared
 *
 * @param[in]  info  @net_interface_info structure.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int net_interface_info_is_empty(const struct net_interface_info *info);

/**
 * @brief set @net_interface_info methods state.
 *
 * This function set all net_interface methods state to given argument state
 *
 * @param[in]  info   @net_interface_info structure.
 * @param[in]  state  new methods state.
 **/
void net_interface_info_set_methods_state(struct net_interface_info *info,
					  enum obus_method_state state);

/**
 * @brief create a net_interface object.
 *
 * This function allocate net_interface object and initialize each
 * object field with the given @ref info field value only if @ref info field
 * presence bit is set (see @ref net_interface_info_fields).
 * This function also set a unique handle for this object.
 * Registering or unregistering object in bus do not alter handle value
 *
 * @param[in]  server    net bus server.
 * @param[in]  info      net_interface object initial values (may be NULL).
 * @param[in]  handlers  net_interface method handlers.
 *
 * @retval  object  success.
 * @retval  NULL    failure.
 *
 * @note: object does not keep any reference to @ref info so that user shall
 * allocate it on the stack.
 *
 * @note: if @ref info param is NULL, net_interface object is initialized with
 * default values.
 *
 * @note: object is not yet registered on bus,
 * call one of theses functions below to register it:
 * @ref net_interface_register
 * @ref net_bus_event_register_interface
 **/
struct net_interface *
net_interface_new(struct obus_server *srv,
		  const struct net_interface_info *info,
		  const struct net_interface_method_handlers *handlers);

/**
 * @brief destroy a net_interface object.
 *
 * This function release net_interface object memory.
 * Only non registered objects can be destroyed.
 *
 * @param[in]  object  net_interface object to be destroyed.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is registered.
 * @retval  -EINVAL invalid @ref object.
 **/
int net_interface_destroy(struct net_interface *object);

/**
 * @brief register a net_interface object.
 *
 * This function register a net_interface object in net bus.
 *
 * @param[in]  server  net bus server.
 * @param[in]  object  net_interface object to be registered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is already registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int net_interface_register(struct obus_server *server,
			   struct net_interface *object);

/**
 * @brief unregister a net_interface object.
 *
 * This function unregister a net_interface object in net bus.
 *
 * @param[in]  server  net bus server.
 * @param[in]  object  net_interface object to be unregistered.
 *
 * @retval  0       success.
 * @retval  -EPERM  @ref object is not registered.
 * @retval  -EINVAL invalid parameters.
 * @retval  < 0     other errors.
 **/
int net_interface_unregister(struct obus_server *server,
			     struct net_interface *object);

/**
 * @brief check if is a net_interface object registered.
 *
 * This function check whether a net_interface object is registered or not.
 *
 * @param[in]  object  net_interface object to checked.
 *
 * @retval  0  object is not registered.
 * @retval  1  object is registered.
 **/
int net_interface_is_registered(const struct net_interface *object);

/**
 * @brief read current net_interface object fields values.
 *
 * This function is used to read current object fields values.
 *
 * @param[in]  object  net_interface object.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  object is NULL or not an net_interface object.
 *
 * @note: object info pointer returned never changed during object life cycle
 * so that user may keep a reference on this pointer until object destruction.
 * this is not the case for info pointers members.
 **/

const struct net_interface_info *
net_interface_get_info(const struct net_interface *object);

/**
 * @brief log net_interface object.
 *
 * This function log object and its current fields values.
 *
 * @param[in]  object  net_interface object.
 * @param[in]  level   obus log level.
 **/
void net_interface_log(const struct net_interface *object,
		       enum obus_log_level level);

/**
 * @brief set net_interface object user data pointer.
 *
 * This function store a user data pointer in a net_interface object.
 * This pointer is never used by libobus and can be retrieved using
 * @ref net_interface_get_user_data function.
 *
 * @param[in]  object      net_interface object.
 * @param[in]  user_data   user data pointer.
 *
 * @retval  0        success.
 * @retval  -EINVAL  object is NULL.
 **/
int net_interface_set_user_data(struct net_interface *object, void *user_data);

/**
 * @brief get net_interface object user data pointer.
 *
 * This function retrieve user data pointer stored in a net_interface object
 * by a previous call to @ref net_interface_set_user_data function
 *
 * @param[in]  object  net_interface object.
 *
 * @retval  user_data  user data pointer.
 **/
void *net_interface_get_user_data(const struct net_interface *object);

/**
 * @brief get registered net_interface object obus handle.
 *
 * This function retrieve net_interface object obus handle.
 * obus handle is an unsigned 16 bits integer.
 * object handle is generated during object creation.
 * object handle can be used to reference an object into another one.
 *
 * @param[in]  object  net_interface object.
 *
 * @retval  handle               registered object obus handle.
 * @retval  OBUS_INVALID_HANDLE  if object is not registered.
 **/
obus_handle_t net_interface_get_handle(const struct net_interface *object);

/**
 * @brief get net_interface object from obus handle.
 *
 * This function retrieve net_interface object given its obus handle.
 *
 * @param[in]  server  net bus server
 * @param[in]  handle  net_interface object handle.
 *
 * @retval  object  net_interface object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a net_interface.
 **/
struct net_interface *
net_interface_from_handle(struct obus_server *server, obus_handle_t handle);

/**
 * @brief get next registered net_interface object in bus.
 *
 * This function retrieve the next registered net_interface object in bus.
 *
 * @param[in]  server    net bus server
 * @param[in]  previous  previous net_interface object in list (may be NULL).
 *
 * @retval  object  next net_interface object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    no more net_interface objects in bus.
 *
 * @note: if @ref previous is NULL, then the first
 * registered net_interface object is returned.
 **/
struct net_interface *
net_interface_next(struct obus_server *server, struct net_interface *previous);

/**
 * @brief send a net_interface object event.
 *
 * This function send an event on a net_interface object.
 *
 * @param[in]  server    net bus server.
 * @param[in]  object    net_interface object.
 * @param[in]  type      net_interface event type.
 * @param[in]  info      associated net_interface content to be updated.
 *
 * @retval  0          event sent and object content updated.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is not registered in bus.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int net_interface_send_event(struct obus_server *server,
			     struct net_interface *object,
			     enum net_interface_event_type type,
			     const struct net_interface_info *info);

/**
 * @brief send a net_interface object event through a net bus event.
 *
 * This function create a net_interface object event and attach it
 * to an existing net bus event. Created net_interface object event
 * will be sent within corresponding bus event.
 * Unlike @net_interface_send_event, object content will not be updated
 * when this function returns but when
 * @net_interface_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  net bus event.
 * @param[in]  object     net_interface object.
 * @param[in]  type       net_interface event type.
 * @param[in]  info       associated net_interface content to be updated.
 *
 * @retval  0          event created and associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -ENOMEM    memory error.
 *
 * @note: Partial info members copy is done inside function.
 * No reference to info members is kept.
 **/
int net_bus_event_add_interface_event(struct net_bus_event *bus_event,
				      struct net_interface *object,
				      enum net_interface_event_type type,
				      const struct net_interface_info *info);

/**
 * @brief register a net_interface object through a net bus event.
 *
 * This function set a net_interface object to be registered when
 * associated net bus event will be sent. Unlike @net_interface_register,
 * object will not be registered when this function returns but when
 * @net_interface_bus_event_send will be invoked.
 *
 * @param[in]  bus_event  net bus event.
 * @param[in]  object     net_interface object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int net_bus_event_register_interface(struct net_bus_event *bus_event,
				     struct net_interface *object);

/**
 * @brief unregister a net_interface object through a net bus event.
 *
 * This function set net_interface object to be unregistered when
 * associated net bus event will be sent. Unlike @net_interface_register,
 * object will not be unregistered when this function returns but when
 * @net_interface_bus_event_send will be invoked.
 * Object is then automatically destroyed on
 * @net_interface_bus_event_destroy call.
 *
 * @param[in]  bus_event  net bus event.
 * @param[in]  object     net_interface object to be registered.
 *
 * @retval  0          object registration request associated to bus event.
 * @retval  -EINVAL    invalid parameters.
 * @retval  -EPERM     object is already attached to an existing bus event.
 *
 **/
int net_bus_event_unregister_interface(struct net_bus_event *bus_event,
				       struct net_interface *object);

OBUS_END_DECLS

#endif /*_NET_INTERFACE_H_*/
