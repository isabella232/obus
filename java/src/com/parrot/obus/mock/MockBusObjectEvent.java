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

import com.parrot.obus.IBusObject;
import com.parrot.obus.IBusObjectEvent;

/**
 * A mock event on a mock bus object.
 */
public abstract class MockBusObjectEvent implements IBusObjectEvent {

	/** Mock object concerned by this event. */
	private MockBusObject mObject;

	/**
	 * Instantiates a new mock bus object event.
	 * 
	 * @param object the concerned by this event
	 */
	protected MockBusObjectEvent(MockBusObject object) {
		mObject = object;
	}
	
	/* (non-Javadoc)
	 * @see com.parrot.obus.IBusObjectEvent#getObject()
	 */
	@Override
	public IBusObject getObject() {
		return mObject;
	}

	/**
	 * Commit this event.
	 */
	public void commit() {
		mObject.commitEvent(this);
	}
}
