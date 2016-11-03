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

import java.util.EnumSet;
import java.util.LinkedList;
import java.util.List;

import com.parrot.obus.IBusObject;
import com.parrot.obus.IBusObjectEvent;
import com.parrot.obus.internal.Descriptor.ObjectDesc;

/**
 * Notifier of events on a bus object.
 * 
 * @param <OBJECT> the object type for this notifier
 * @param <EVENT> the event type for this notifier
 * @param <BUS_EVENT_TYPE> the bus event type for this notifier
 */
public class BusObjectEventsNotifier<OBJECT extends IBusObject, EVENT extends IBusObjectEvent, BUS_EVENT_TYPE extends Enum<BUS_EVENT_TYPE>> {

	/** Event is notified before being committed */
	public static final boolean PRE_COMMIT = false;
	/** Event is notified after being committed */
	public static final boolean POST_COMMIT = true;

	/**
	 * Base class of a event listener that can be registered to this notifier.
	 * 
	 * @param <OBJECT> the object type for this listener
	 * @param <EVENT> the event type for this listener
	 * @param <BUS_EVENT_TYPE> the bus event type for this notifier
	 */
	public abstract static class Listener<OBJECT extends IBusObject, EVENT extends IBusObjectEvent, BUS_EVENT_TYPE extends Enum<BUS_EVENT_TYPE>> {

		/** Set of events this listener wants to receive, or null to receive all events */
		private final EnumSet<?> mEventSet;
		/** Object this listener wants to receive event from, or null to receive events from all objects */
		private final OBJECT mObject;
		/** True if the listener want to be notified after the event has been process. */
		private final boolean mPostProcessing;

		/**
		 * Instantiates a new listener of a set of events
		 * 
		 * @param eventSet set of events this listener want to receive
		 * @param postProcessing true if the listener want to be notified after the event has been process
		 */
		public Listener(EnumSet<?> eventSet, boolean postProcessing) {
			mEventSet = eventSet;
			mPostProcessing = postProcessing;
            mObject = null;
		}

		/**
		 * Instantiates a new listener of all events of a specific object.
		 * 
		 * @param object the object to listen events of
		 * @param postProcessing true if the listener want to be notified after the event has been process
		 */
		public Listener(OBJECT object, boolean postProcessing) {
			mObject = object;
			mPostProcessing = postProcessing;
            mEventSet = null;
		}

		/**
		 * Instantiates a new listener of a set of events of a specific object[.
		 * 
		 * @param object the object to listen events of
		 * @param eventSet set of events this listener want to receive
		 * @param postProcessing true if the listener want to be notified after the event has been process
		 */
		public Listener(OBJECT object, EnumSet<?> eventSet, boolean postProcessing) {
			mObject = object;
			mEventSet = eventSet;
			mPostProcessing = postProcessing;
		}

		/**
		 * Called when an event has been received for an object.
		 * 
		 * @param object the object for this event
		 * @param event the received event
		 * @param busEventType the type of the bus event this event is part of, null if not part of a bus event
		 */
		public abstract void onEvent(OBJECT object, EVENT event, BUS_EVENT_TYPE busEventType);
	}

	/** List of registered listeners. */
	private final List<Listener<OBJECT, EVENT, BUS_EVENT_TYPE>> mListeners;

	/**
	 * Instantiates a new bus events notifier.
	 * 
	 * @param busClient the bus client instance
	 * @param objectdesc the objectdesc for objects of this notifier
	 */
	public BusObjectEventsNotifier(BusClient busClient, ObjectDesc objectdesc) {
		mListeners = new LinkedList<Listener<OBJECT, EVENT, BUS_EVENT_TYPE>>();
		// register ourself with the bus client
		busClient.setObjectNotifier(objectdesc, this);
	}

	/**
	 * Register new event listener.
	 * 
	 * @param listener the listener to register
	 */
	public void registerEventListener(Listener<OBJECT, EVENT, BUS_EVENT_TYPE> listener) {
		synchronized (mListeners) {
			mListeners.add(listener);
		}
	}

	/**
	 * Unregister a previously registered event listener
	 * 
	 * @param listener the listener to unregister register
	 */
	public void unregisterEventListener(Listener<OBJECT, EVENT, BUS_EVENT_TYPE> listener) {
		synchronized (mListeners) {
			mListeners.remove(listener);
		}
	}

	/**
	 * Notify listeners of an event.
	 * 
	 * @param object the object concerned by this event
	 * @param event the event
	 * @param busEventType the bus event type
	 * @param postProcessing true if the event has been process, false else.
	 */
	@SuppressWarnings("unchecked")
	public void notifyEvent(IBusObject object, IBusObjectEvent event, Enum<?> busEventType, boolean postProcessing) {
        Listener<OBJECT, EVENT, BUS_EVENT_TYPE>[] listeners;
        synchronized (mListeners) {
            listeners = mListeners.toArray(new Listener[mListeners.size()]);
        }
        for (Listener<OBJECT, EVENT, BUS_EVENT_TYPE> listener : listeners) {
            if ((listener.mPostProcessing == postProcessing)
                    && ((listener.mEventSet == null) || listener.mEventSet.contains(event.getType()))
                    && ((listener.mObject == null) || listener.mObject.equals(object))) {
                listener.onEvent((OBJECT) object, (EVENT) event, (BUS_EVENT_TYPE) busEventType);
            }
        }
	}
}