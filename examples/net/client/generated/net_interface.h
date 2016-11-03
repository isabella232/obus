/******************************************************************************
* @file net_interface.h
*
* @brief obus net_interface object client api
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
 * @brief net_interface object event structure
 *
 * This opaque structure represent an net_interface object event.
 **/
struct net_interface_event;

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
 * @param[in]  client  net bus client
 * @param[in]  handle  net_interface object handle.
 *
 * @retval  object  net_interface object.
 * @retval  NULL    invalid parameters.
 * @retval  NULL    corresponding handle object is not a net_interface.
 **/
struct net_interface *
net_interface_from_handle(struct obus_client *client, obus_handle_t handle);

/**
 * @brief get next registered net_interface object in bus.
 *
 * This function retrieve the next registered net_interface object in bus.
 *
 * @param[in]  client    net bus client
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
net_interface_next(struct obus_client *client, struct net_interface *previous);

/**
 * @brief get net_interface event type.
 *
 * This function is used to retrieved net_interface event type.
 *
 * @param[in]  event  net_interface event.
 *
 * @retval     type   net_interface event type.
 **/
enum net_interface_event_type
net_interface_event_get_type(const struct net_interface_event *event);

/**
 * @brief log net_interface event.
 *
 * This function log net_interface event and its associated fields values.
 *
 * @param[in]  event   net_interface event.
 * @param[in]  level   obus log level.
 **/
void net_interface_event_log(const struct net_interface_event *event,
			     enum obus_log_level level);

/**
 * @brief check net_interface event contents is empty.
 *
 * This function check if each event field has its presence bit cleared.
 *
 * @param[in]  event   net_interface event.
 *
 * @retval     1     Each field has its presence bit cleared.
 * @retval     0     One field (or more) has its presence bit set.
 **/
int net_interface_event_is_empty(const struct net_interface_event *event);

/**
 * @brief commit net_interface event contents in object.
 *
 * This function copy net_interface event contents in object.
 *
 * @param[in]  event   net_interface event.
 *
 * @retval     0     Commit succeed.
 * @retval     <0    Commit failed.
 *
 * @note: if not call by client, event commit is done internally
 * on client provider event callback return.
 **/
int net_interface_event_commit(struct net_interface_event *event);

/**
 * @brief read net_interface event associated fields values.
 *
 * This function is used to read event fields values.
 *
 * @param[in]  event   net_interface event.
 *
 * @retval  info  pointer to a constant object fields values.
 * @retval  NULL  event is NULL or not an net_interface object event.
 **/
const struct net_interface_info *
net_interface_event_get_info(const struct net_interface_event *event);

/**
 * generic net_interface client method status callback
 **/
typedef void (*net_interface_method_status_cb_t) (struct net_interface *object,
						  obus_handle_t handle,
						  enum obus_call_status status);
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
 * @brief initialize @net_interface_up_args structure.
 *
 * This function initialize @net_interface_up_args structure.
 * Each argument field has its presence bit cleared.
 *
 * @param[in]  args  pointer to allocated @net_interface_up_args structure.
 **/
void net_interface_up_args_init(struct net_interface_up_args *args);

/**
 * @brief check @net_interface_up_args structure contents is empty.
 *
 * This function check if each argument field has its presence bit cleared.
 *
 * @param[in]  arguments  @net_interface_up_args structure.
 *
 * @retval     1     Each argument field has its presence bit cleared.
 * @retval     0     One argument field (or more) has its presence bit set.
 **/
int net_interface_up_args_is_empty(const struct net_interface_up_args *args);

/**
 * @brief call method 'up'.
 *
 * This function call method 'up' on a net_interface object
 *
 * Activate network interface
 *
 * @param[in]   client  obus client context.
 * @param[in]   object  net_interface object.
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
int net_interface_call_up(struct obus_client *client,
			  struct net_interface *object,
			  const struct net_interface_up_args *args,
			  net_interface_method_status_cb_t cb,
			  uint16_t *handle);

/**
 * @brief call method 'down'.
 *
 * This function call method 'down' on a net_interface object
 *
 * Deactivate network interface
 *
 * @param[in]   client  obus client context.
 * @param[in]   object  net_interface object.
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
int net_interface_call_down(struct obus_client *client,
			    struct net_interface *object,
			    net_interface_method_status_cb_t cb,
			    uint16_t *handle);

/**
 * net_interface object provider api
 */

struct net_interface_provider {
	struct obus_provider *priv;
	void (*add) (struct net_interface *object,
		     struct net_bus_event *bus_event, void *user_data);
	void (*remove) (struct net_interface *object,
			struct net_bus_event *bus_event, void *user_data);
	void (*event) (struct net_interface *object,
		       struct net_interface_event *event,
		       struct net_bus_event *bus_event, void *user_data);
};

int net_interface_subscribe(struct obus_client *client,
			    struct net_interface_provider *provider,
			    void *user_data);
int net_interface_unsubscribe(struct obus_client *client,
			      struct net_interface_provider *provider);

OBUS_END_DECLS

#endif /*_NET_INTERFACE_H_*/
