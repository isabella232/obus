/******************************************************************************
* @file IPsBusEvent.java
*
* @brief IPsBusEvent
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus;

@SuppressWarnings("javadoc")
/** Bus Event interface */
public interface IPsBusEvent {
	/** Event type */
	public static enum Type {
		/** ps bus connected */
		CONNECTED,
		/** ps bus disconnected */
		DISCONNECTED,
		/** ps bus connection refused */
		CONNECTION_REFUSED,
		/** System processes global update */
		UPDATED
	}

	/** Event type getter */
	public Type getType();

}
