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

package com.parrot.obus.mock;

import java.io.PrintWriter;
import java.lang.reflect.Array;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;

import junit.framework.Assert;
import android.annotation.SuppressLint;

import com.parrot.obus.BusClient;
import com.parrot.obus.BusEventsNotifier;
import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.IBusObject;
import com.parrot.obus.MethodCallAck;
import com.parrot.obus.ObusAddress;

/**
 * A mock instance of a Bus client.
 * 
 * This class can be used to test code using Obus without any bus.
 * 
 * @param <BUS_EVENT_TYPE> Bus event type for this bus client
 */
public class MockBusClient<BUS_EVENT_TYPE extends Enum<BUS_EVENT_TYPE>> extends BusClient {

	/** true if the bus is started. */
	private boolean mStarted;

	/** Map of objects on this bus, by Handle. */
	private final Map<Integer, MockBusObject> mObjects;

	/** Bus event to send for the CONNECTED event */
	private final BUS_EVENT_TYPE mConnectedEvent;

	/** Bus event to send for the DISCONNECTED event */
	private final BUS_EVENT_TYPE mDisconnectedEvent;

	/** Queue of expected method call. */
	private final Queue<ExpectedCall> mExpectedCallQueue;

	/**
	 * Instantiates a new mock bus client.
	 * 
	 * @param connectedEvent Bus event to send for the CONNECTED event
	 * @param disconnectedEvent Bus event to send for the DISCONNECTED event
	 */
	@SuppressLint("UseSparseArrays") // it's test code...
	public MockBusClient(BUS_EVENT_TYPE connectedEvent, BUS_EVENT_TYPE disconnectedEvent, BusEventsNotifier<?> busEventNotifier) {
		mConnectedEvent = connectedEvent;
		mDisconnectedEvent = disconnectedEvent;
		mObjects = new HashMap<Integer, MockBusObject>();
		mExpectedCallQueue = new LinkedList<ExpectedCall>();
		setBusEventNotifier(busEventNotifier);
	}

	/**
	 * Start the bus.
	 * 
	 * Send the CONNECTED bus event with all objects already in the bus
	 * 
	 * @param addr not used
	 */
	@Override
	public void start(ObusAddress addr) {
		mStarted = true;
		sendBusEvent(mConnectedEvent, mObjects.values(), null, null);
	}

	/**
	 * Stop the bus
	 * 
	 * Send the DISCONNECTED bus event with all objects of the bus removed
	 */
	@Override
	public void stop() {
		mStarted = false;
		sendBusEvent(mDisconnectedEvent, null, null, mObjects.values());
	}

	/**
	 * Check if the bus is connected
	 * 
	 * Mock bus is connected as soon as the bus is started
	 */
	@Override
	public boolean isConnected() {
		return mStarted;
	}

	@Override
	public IBusObject getObject(int handle) {
		return mObjects.get(handle);
	}

	/**
	 * Send bus event.
	 * 
	 * @param busEventType the bus event type
	 * @param addedObject list of objects added to the bus in this bus event, null if none
	 * @param events list of object evnets in this bus event, or null if there is none
	 * @param removedObjects list of objects removed to the bus in this bus event, null if none
	 */
	public void sendBusEvent(BUS_EVENT_TYPE busEventType, Collection<MockBusObject> addedObject,
			Collection<MockBusObjectEvent> events, Collection<MockBusObject> removedObjects) {
		// Notify the bus event before dispatching it
		notifyBusEvent(busEventType, false);

		// Dispatch registrations
		if (addedObject != null) {
			for (MockBusObject object : addedObject) {
				// register the object to the bus
				sendAddObject(object, busEventType);
			}
		}

		if (events != null) {
			// Dispatch events before committing them
			for (MockBusObjectEvent event : events) {
				notifyObjectEvent(event, busEventType, false);
			}

			// Commit all events before
			for (MockBusObjectEvent event : events) {
				event.commit();
			}

			// Dispatch events after committing it
			for (MockBusObjectEvent evt : events) {
				notifyObjectEvent(evt, busEventType, true);
			}
		}

		if (removedObjects != null) {
			// Dispatch unregistrations
			for (MockBusObject object : removedObjects) {
				sendRemoveObject(object, busEventType);
			}
		}

		// Notify the bus event after all events have been committed
		notifyBusEvent(busEventType, true);

	}

	/**
	 * Send add object event
	 * 
	 * @param object the object to add
	 */
	public void sendAddObject(MockBusObject object) {
		sendAddObject(object, null);
	}

	/**
	 * Send remove object event.
	 * 
	 * @param object the object
	 */
	public void sendRemoveObject(MockBusObject object) {
		sendRemoveObject(object, null);
	}

	/**
	 * Send object event.
	 * 
	 * @param event the event
	 */
	public void sendEvent(MockBusObjectEvent event) {
		sendEvent(event, null);
	}

	/**
	 * Send add object event
	 * 
	 * @param object the added object
	 * @param busEventType the bus event type
	 */
	private void sendAddObject(MockBusObject object, BUS_EVENT_TYPE busEventType) {
		// add the object to the local store
		mObjects.put(object.getHandle(), object);
		if (mStarted) {
			// add the object to the client registry
			addObjectToRegistry(object, busEventType);
		}
	}

	/**
	 * Send remove object event.
	 * 
	 * @param object the removed object
	 * @param busEventType the bus event type
	 */
	private void sendRemoveObject(MockBusObject object, BUS_EVENT_TYPE busEventType) {
		// remove the object to the client registry
		removeObjectFromRegistry(object, busEventType);
		if (mStarted) {
			// add the object to the local store
			mObjects.remove(object.getHandle());
		}
	}

	/**
	 * Send a object event.
	 * 
	 * @param event the event
	 * @param busEventType the bus event type
	 */
	private void sendEvent(MockBusObjectEvent event, BUS_EVENT_TYPE busEventType) {
		// Notify the event before committing it
		notifyObjectEvent(event, busEventType, false); // no associated bus event
		// Commit event
		event.commit();
		// Notify the event after committing it
		notifyObjectEvent(event, busEventType, true); // no associated bus event
	}


	/**
	 * Add an expected method call to the queue of expected call.
	 * 
	 * @param call the expected method call
	 */
	public void expectCall(MockMethodCall<?> call) {
		expectCall(call, MethodCallAck.ACKED);
	}

	/**
	 * Add an expected method call to the queue of expected call.
	 * 
	 * @param call the expected method call
	 * @param callAck the call ack for this call
	 */
	public void expectCall(MockMethodCall<?> call, MethodCallAck callAck) {
		mExpectedCallQueue.add(new ExpectedCall(call, callAck));
	}

	/**
	 * Ensure that no method call are left in the queue.
	 */
	public void assertNoCallPending() {
		Assert.assertTrue(mExpectedCallQueue.isEmpty());
	}

	/**
	 * store expected MockMethodCall and MethodCallAck result
	 */
	private class ExpectedCall {
		/** expected method call. */
		private final MockMethodCall<?> mMockMethodCall;
		/** result of this call. */
		private final MethodCallAck mReturnCallAck;

		/**
		 * Instantiates a new expected call.
		 * 
		 * @param mockMethodCall expected method call
		 * @param callAck result of this call
		 */
		public ExpectedCall(MockMethodCall<?> mockMethodCall, MethodCallAck callAck) {
			mMockMethodCall = mockMethodCall;
			mReturnCallAck = callAck;
		}
	}

	/* (non-Javadoc)
	 * @see com.parrot.obus.BusClient#callBusMethod(com.parrot.obus.IBusMethodCall, com.parrot.obus.IBusMethodCall.AckCb)
	 */
	@Override
	public void callBusMethod(IBusMethodCall methodCall, IBusMethodCall.AckCb callback) {
		MockMethodCall<?> call = (MockMethodCall<?>) methodCall;
		// check if call is expected
		ExpectedCall expectedCall = mExpectedCallQueue.poll();
		Assert.assertNotNull("No expected call!", expectedCall);
		Assert.assertEquals(expectedCall.mMockMethodCall.getMethod(), call.getMethod());
		Assert.assertEquals(expectedCall.mMockMethodCall.getObject(), call.getObject());
		// check params array
		List<Object> expectedParams = expectedCall.mMockMethodCall.getParams();
		List<Object> params = call.getParams();
		Assert.assertEquals(expectedParams.size(), params.size());
		for (int i = 0; i < expectedParams.size(); i++) {
			Object expectedParam = expectedParams.get(i);
			Object param = params.get(i);
			// if expected param is an array
			if (expectedParam != null && expectedParam.getClass().isArray()) {
				// check that actual param is also an array
				Assert.assertTrue(param.getClass().isArray());
				// check arrays length
				Assert.assertEquals(Array.getLength(expectedParam), Array.getLength(param));
				// check each element
				for (int j = 0; j < Array.getLength(expectedParam); j++) {
					Assert.assertEquals(Array.get(expectedParam, j), Array.get(param, j));
				}
			} else {
				Assert.assertEquals(expectedParam, param);
			}
		}
		// call ack callback
		if (callback != null) {
			callback.onAck(expectedCall.mReturnCallAck);
		}
	}

	/* (non-Javadoc)
	 * @see com.parrot.obus.BusClient#dump(java.io.PrintWriter)
	 */
	@Override
	public void dump(PrintWriter writer) {
	}
}
