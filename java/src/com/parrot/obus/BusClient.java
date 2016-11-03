/******************************************************************************
 * libobus-java - obus client java binding library.
 *
 * Copyright (c) 2013 Parrot S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PARROT COMPANY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

package com.parrot.obus;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;

import com.parrot.obus.internal.Descriptor.BusDesc;
import com.parrot.obus.internal.Descriptor.ObjectDesc;

/**
 * Base class for a bus client.
 * 
 * This class is abstract and the concrete implementation for OBus must be created with the factory function
 * {@link #create(String, BusDesc, BusEventsNotifier)}
 */
public abstract class BusClient {

	/**
	 * Factory function that create the obus bus client.
	 * 
	 * @param clientName the name of the client
	 * @param busDesc the bus descriptor of the bus
	 * @param busEventNotifier the bus event notifier for this bus
	 * @return the new bus client
	 */
	public static final BusClient create(String clientName, BusDesc busDesc, BusEventsNotifier<?> busEventNotifier) {
		return new com.parrot.obus.internal.Client(clientName, busDesc, busEventNotifier);
	}

	/** Map of objects registries, by object descriptor. */
	private final Map<ObjectDesc, BusObjectsRegistry<? extends IBusObject, ? extends Enum<?>, ?>> mObjRegistries;

	/** Map of objects notifiers, by object descriptor. */
	private final Map<ObjectDesc, BusObjectEventsNotifier<? extends IBusObject, ? extends IBusObjectEvent, ? extends Enum<?>>> mObjNotifiers;

	/** The bus event notifier. */
	private BusEventsNotifier<? extends Enum<?>> mBusEventNotifier;

	// Log settings. Default is false
	/** true to log object register/unregister */
	protected boolean mLogObjects = false;
	/** true to log events and bus events */
	protected boolean mLogEvents = false;
	/** true to log methods call and ack */
	protected boolean mLogCalls = false;

	/**
	 * protected constructor as this class is abstract.
	 */
	protected BusClient() {
		mObjRegistries = new HashMap<ObjectDesc, BusObjectsRegistry<? extends IBusObject, ? extends Enum<?>, ?>>();
		mObjNotifiers = new HashMap<ObjectDesc, BusObjectEventsNotifier<? extends IBusObject, ? extends IBusObjectEvent, ? extends Enum<?>>>();
	}

	/**
	 * Start the bus.
	 * 
	 * @param addr the bus server address
	 */
	abstract public void start(ObusAddress addr);

	/**
	 * Stop the bus
	 */
	abstract public void stop();

	/**
	 * Checks if is the bus is connected.
	 * 
	 * @return true, if the bus is connected
	 */
	abstract public boolean isConnected();

	/**
	 * Sets the bus event notifier.
	 * 
	 * @param busEventNotifier the bus event notifier to set
	 */
	protected void setBusEventNotifier(BusEventsNotifier<? extends Enum<?>> busEventNotifier) {
		if (mBusEventNotifier != null) {
			throw new IllegalArgumentException("bus event notifier  already set");
		}
		mBusEventNotifier = busEventNotifier;
	}

	/**
	 * Enable bus logging
	 * 
	 * @param logObjects true to log object register/unregister
	 * @param logEvents true to log events and bus events
	 * @param logCalls true to log methods call and ack
	 */
	public void setLogging(boolean logObjects, boolean logEvents, boolean logCalls) {
		mLogObjects = logObjects;
		mLogEvents = logEvents;
		mLogCalls = logCalls;
	}

	/**
	 * Dump the bus objects
	 * @param writer writer to write the dump to
	 */
	abstract public void dump(PrintWriter writer);

	/**
	 * Sets the object registry for a specific type of bus objects
	 * 
	 * @param objDesc the object descriptor of the bus object type to set the registry for
	 * @param registry the registry to set for this class of bus objects
	 */
	/* package */void setObjectRegistry(ObjectDesc objDesc, BusObjectsRegistry<? extends IBusObject, ? extends Enum<?>, ?> registry) {
		if (mObjRegistries.containsKey(objDesc)) {
			throw new IllegalArgumentException("registry for object uid=" + objDesc.uid + " already set");
		}
		mObjRegistries.put(objDesc, registry);
	}

	/**
	 * Sets the object events notifier for a specific type of bus objects
	 * 
	 * @param objDesc the object descriptor of the bus object type to set the notifier for
	 * @param notifier the notifier to set for this class of bus objects
	 */
	/* package */void setObjectNotifier(ObjectDesc objDesc,
			BusObjectEventsNotifier<? extends IBusObject, ? extends IBusObjectEvent, ? extends Enum<?>> notifier) {
		if (mObjNotifiers.containsKey(objDesc)) {
			throw new IllegalArgumentException("notifier for object uid=" + objDesc.uid + " already set");
		}
		mObjNotifiers.put(objDesc, notifier);
	}

	/**
	 * Call bus method.
	 * 
	 * @param methodCall the method call instance to execute
	 * @param callback the callback to call when the method has been sent to the server
	 */
	abstract public void callBusMethod(IBusMethodCall methodCall, IBusMethodCall.AckCb callback);

	/**
	 * Get an object by its Handle.
	 * 
	 * @param handle the handle of the requested object
	 * @return The object with the requested handle or null if there not found
	 */
	abstract public IBusObject getObject(int handle);

	/**
	 * Adds a new object to registry.
	 * 
	 * @param object the object to add
	 * @param busEventType the bus event this add is part of, null if this event is not part of a bus event
	 */
	protected void addObjectToRegistry(IBusObject object, Enum<?> busEventType) {
		BusObjectsRegistry<? extends IBusObject, ? extends Enum<?>, ?> registry = mObjRegistries.get(object.getDescriptor());
		if (registry != null) {
			registry.add(object, busEventType);
		}
	}

	/**
	 * Remove an object from registry.
	 * 
	 * @param object the object to remote
	 * @param busEventType the bus event this add is part of, null if this event is not part of a bus event
	 */
	protected void removeObjectFromRegistry(IBusObject object, Enum<?> busEventType) {
		BusObjectsRegistry<? extends IBusObject, ? extends Enum<?>, ?> registry = mObjRegistries.get(object.getDescriptor());
		if (registry != null) {
			registry.remove(object, busEventType);
		}
	}

	/**
	 * Notify of bus event.
	 * 
	 * @param busEventType the type of the bus event
	 * @param isCommitted true if the bus event has been committed
	 */
	protected void notifyBusEvent(Enum<?> busEventType, boolean isCommitted) {
		mBusEventNotifier.notifyEvent(busEventType, isCommitted);
	}

	/**
	 * Notify an event on an object.
	 * 
	 * @param event the event that is notified
	 * @param busEventType the bus event this event is part of, null if this event is not part of a bus event
	 * @param isCommitted true if the event has been committed
	 */
	protected void notifyObjectEvent(IBusObjectEvent event, Enum<?> busEventType, boolean isCommitted) {
		IBusObject object = event.getObject();
		BusObjectEventsNotifier<? extends IBusObject, ? extends IBusObjectEvent, ? extends Enum<?>> notifier = mObjNotifiers
				.get(object.getDescriptor());
		if (notifier != null) {
			notifier.notifyEvent(object, event, busEventType, isCommitted);
		}
	}
}
