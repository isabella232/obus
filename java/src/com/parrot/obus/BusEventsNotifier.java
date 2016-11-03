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

import java.util.LinkedList;
import java.util.List;

/**
 * Notifier of bus events.
 * 
 * @param <TYPE> the generic type
 */
public class BusEventsNotifier<TYPE extends Enum<TYPE>> {

	/** Event is notified before being committed  */
	public static final boolean PRE_COMMIT = false;
	/** Event is notified after being committed  */
	public static final boolean POST_COMMIT = true;

	/**
	 * Base class of a event listener that can be registered to this notifier.
	 * 
	 * @param <TYPE> the bus event type
	 */
	public abstract static class Listener<TYPE extends Enum<TYPE>> {
		/** True if the listener want to be notified after the event has been process. */
        private final boolean mPostProcessing;

        /**
		 * Instantiates a new listener.
		 * 
		 * @param postProcessing true if the listener want to be notified after the event has been process
		 */
		public Listener(boolean postProcessing) {
			mPostProcessing = postProcessing;
		}

		/**
		 * Called when an bus event has been received.
		 * 
		 * @param eventType the type of the received bus event
		 */
		public abstract void onEvent(TYPE eventType);
	}

	/** List of registered listeners. */
    private final List<Listener<TYPE>> mListeners;

    /**
	 * Instantiates a new bus events notifier.
	 */
	public BusEventsNotifier() {
		mListeners = new LinkedList<Listener<TYPE>>();
	}

	/**
	 * Register new event listener.
	 * 
	 * @param listener the listener to register
	 */
	public void registerEventListener(Listener<TYPE> listener) {
        synchronized (mListeners) {
            mListeners.add(listener);
        }
    }

	/**
	 * Unregister a previously registered event listener
	 * 
	 * @param listener the listener to unregister register
	 */
	public void unregisterEventListener(Listener<TYPE> listener) {
        synchronized (mListeners) {
            mListeners.remove(listener);
        }
    }

	/**
	 * Notify listeners of an event.
	 * 
	 * @param event the event
	 * @param postProcessing true if the event has been process, false else.
	 */
	@SuppressWarnings("unchecked")
	public void notifyEvent(Enum<?> event, boolean postProcessing) {
        Listener<TYPE>[] listeners;
        synchronized (mListeners) {
            listeners = mListeners.toArray(new Listener[mListeners.size()]);
        }
        for (Listener listener : listeners) {
            if ((listener.mPostProcessing == postProcessing)) {
				listener.onEvent((TYPE)event);
			}
		}
	}
}