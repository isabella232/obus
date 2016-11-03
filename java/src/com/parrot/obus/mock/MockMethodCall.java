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

import java.util.Arrays;
import java.util.List;

import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.IBusObject;

/**
 * Represent a Mock method call on a mock object.
 * 
 * @param <E> the method enum
 */
public class MockMethodCall<E extends Enum<E>> implements IBusMethodCall {

	/** The method enum value for this call. */
	private final E mMethod;
	
	/** The object on which the call is made */
	private final IBusObject mObject;

	/** List of params of this call. */
	private final List<Object> mParams;

	/**
	 * Instantiates a new mock method call.
	 * 
	 * @param method the method enum
	 * @param object the on which the call is made
	 * @param params the call parameters
	 */
	public MockMethodCall(E method, IBusObject object, Object ... params) {
		mMethod = method;
		mObject = object;
		mParams = Arrays.asList(params);
	}

	/**
	 * Gets the method.
	 * 
	 * @return the method
	 */
	public E getMethod() {
		return mMethod;
	}

	/**
	 * Gets the object.
	 * 
	 * @return the object
	 */
	public IBusObject getObject() {
		return mObject;
	}
	
	/**
	 * Gets the params.
	 * 
	 * @return the params
	 */
	public List<Object> getParams() {
		return mParams;
	}
}
