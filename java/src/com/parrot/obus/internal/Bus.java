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

package com.parrot.obus.internal;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.parrot.obus.internal.Descriptor.BusDesc;
import com.parrot.obus.internal.Descriptor.ObjectDesc;

/**
 * Bus that manage all objects.
 */
public class Bus {
	private final BusDesc desc;  /**< Bus descriptor */
	private final Map<Integer, ObusObject> objects; /** Bus object list */

	/**
	 *
	 */
	public Bus(BusDesc busDesc) {
		this.desc = busDesc;
		this.objects = new HashMap<Integer, ObusObject>();
	}

	/**
	 *
	 */
	public BusDesc getDesc() {
		return this.desc;
	}

	/**
	 * Register an object on the bus.
	 * @param obj : object to register.
	 */
	public void registerObject(ObusObject obj) {
		if (obj.handle == Core.OBUS_INVALID_HANDLE) {
			throw new IllegalArgumentException(
					"object uid=%d" + obj.uid + " invalid handle");
		}
		if (this.objects.containsKey(obj.handle)) {
			throw new IllegalArgumentException(
					"object uid=" + obj.uid +
					" handle=" + obj.handle +
					" already registered");
		}
		/* Add in table */
		this.objects.put(obj.handle, obj);
	}

	/**
	 * Unregister an object on the bus.
	 * @param obj : object to unregister.
	 */
	public void unregisterObject(ObusObject obj) {
		if (!this.objects.containsKey(obj.handle)) {
			throw new IllegalArgumentException(
					"object uid=" + obj.uid +
					" handle=" + obj.handle +
					" not registered");
		}
		/* Remove from table */
		this.objects.remove(obj.handle);
	}

	/**
	 * Find an object by its handle.
	 * @param handle : handle of object to find.
	 * @return object with given handle or null if not found.
	 */
	public ObusObject findObject(int handle) {
		return this.objects.get(handle);
	}

	/**
	 * Get all objects on the bus.
	 * @return read-only collection of objects.
	 */
	public Collection<ObusObject> getAllObjects() {
		return Collections.unmodifiableCollection(this.objects.values());
	}

	/**
	 * Get all objects of a given type on the bus.
	 * @param objDesc : descriptor of objects to query.
	 * @return read-only collection of objects.
	 */
	public Collection<ObusObject> getObjects(ObjectDesc objDesc) {
		/* Create an array where to store matching objects */
		ArrayList<ObusObject> newObjects = new ArrayList<ObusObject>(this.objects.size());
		for (ObusObject obj: this.objects.values()) {
			if (obj.uid == objDesc.uid) {
				newObjects.add(obj);
			}
		}
		return Collections.unmodifiableCollection(newObjects);
	}
}
