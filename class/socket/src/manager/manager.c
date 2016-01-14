#include <thread/thread/thread.h>
#include <thread/rwlock.h>
#include <socket_manager.h>
#include <data/data/linked_list.h>

typedef struct private_socket_manager_t private_socket_manager_t;

/**
 * Private data of an socket_manager_t object.
 */
struct private_socket_manager_t {

	/**
	 * Public socket_manager_t interface.
	 */
	socket_manager_t public;

	/**
	 * List of registered socket constructors
	 */
	linked_list_t *sockets;

	/**
	 * Instantiated socket implementation
	 */
	socket_t *socket;

	/**
	 * The constructor used to create the current socket
	 */
	socket_constructor_t create;

	/**
	 * Lock for sockets list
	 */
	rwlock_t *lock;
};

