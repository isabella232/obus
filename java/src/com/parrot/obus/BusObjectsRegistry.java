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

import com.parrot.obus.internal.Descriptor.ObjectDesc;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * Registry of bus objects.
 *
 * This registry store bus objects identified a key. Keys are extracted from the bus object using the {@link IKeyExtractor} injected
 * in the constructor.
 *
 * If the key extractor is null, the object is considered as a singleton and only the first created object is stored in the
 * registry.
 *
 * @param <OBJECT> the bus object class for this registry
 * @param <BUS_EVENT_TYPE> the bus event type for this registry
 * @param <KEY> the generic type
 */
public class BusObjectsRegistry<OBJECT extends IBusObject, BUS_EVENT_TYPE extends Enum<BUS_EVENT_TYPE>, KEY> {

	/**
	 * Observer of the registry notified when an object is added/removed to/from the registry.
	 *
	 * @param <OBJECT> the object type
	 * @param <BUS_EVENT_TYPE> the bus event type for this registry
	 */
	public interface IObserver<OBJECT, BUS_EVENT_TYPE> {

		/**
		 * Called when an object has been created by the remote server and has been added to the local registry.
		 *
		 * @param object the new object
		 * @param busEventType the type of the bus event this add is part of, null if not part of a bus event
		 */
		public void onObjectAdded(OBJECT object, BUS_EVENT_TYPE busEventType);

		/**
		 * Called when an object has been deleted by the remote server and has been removed from the local registry.
		 *
		 * @param object the removed object
		 * @param busEventType the type of the bus event this remove is part of, null if not part of a bus event
		 */
		public void onObjectRemoved(OBJECT object, BUS_EVENT_TYPE busEventType);
	}

	/**
	 * Interface allowing to extract the key that the registry will use to store a bus object
	 *
	 * Each time a new remove object is created, the bus add this object to a registry, identified by a key. Method
	 * {@link BusObjectsRegistry#get(Object)} allow to retrieve this object using its key.
	 *
	 * Key must be unique in the store. If 2 objects have the same key, the 2nd object is not added to the registry
	 *
	 * @param <OBJECT> the bus object class for this extractor
	 * @param <KEY> the class of the key
	 */
	public interface IKeyExtractor<OBJECT extends IBusObject, KEY> {

		/**
		 * Implementation should return the key computed for the object.
		 *
		 * @param object the object to compute the key of.
		 * @return the computed key.
		 */
		public KEY getKey(OBJECT object);

	}

	/**
	 * Interface defining a filter function to select items returned by {@link BusObjectsRegistry#getAll(IFilter)}.
	 *
	 * @param <OBJECT> the bus object class for this filter
	 */
	public interface IFilter<OBJECT extends IBusObject> {

		/**
		 * Check if an item match the filter.
		 *
		 * @param object the object to test
		 * @return true, if the object match, false else
		 */
		public boolean checkMatch(OBJECT object);
	}

	/** The bus client instance owning this registry */
	private final BusClient mBusClient;
	/** Map of objects by keys. */
	private final Map<KEY, OBJECT> mStore;
	/** List of registry observers. */
	private final List<IObserver<OBJECT, BUS_EVENT_TYPE>> mObservers;
	/** Key extractor that generate the key from the object data. Can be null for singleton object */
	private final IKeyExtractor<OBJECT, KEY> mKeyExtractor;
	/** Object descriptor of OBJECT class. */
	private ObjectDesc mObjectdesc;

	/**
	 * Instantiates a new bus objects registry.
	 *
	 * @param busClient the bus client instance
	 * @param objectdesc the objectdesc of objects managed by this registry
	 * @param keyExtractor object extracting the key from the object. Can be null for singleton object.
	 */
	public BusObjectsRegistry(BusClient busClient, ObjectDesc objectdesc, IKeyExtractor<OBJECT, KEY> keyExtractor) {
		mBusClient = busClient;
		mStore = new HashMap<KEY, OBJECT>();
		mObservers = new LinkedList<IObserver<OBJECT, BUS_EVENT_TYPE>>();
		mKeyExtractor = keyExtractor;
		mObjectdesc = objectdesc;
		// register ourself with the bus client
		busClient.setObjectRegistry(objectdesc, this);
	}

	/**
	 * Register a registry observer. This observer is notified when an object is added/removed to/from the registry.
	 *
	 * @param observer the observer to register
	 */
	public void registerObserver(IObserver<OBJECT, BUS_EVENT_TYPE> observer) {
		synchronized (mObservers) {
			mObservers.add(observer);
		}
	}

	/**
	 * Unregister a previously registered observer.
	 *
	 * @param observer the observer to unregister
	 */
	public void unregisterObserver(IObserver<OBJECT, BUS_EVENT_TYPE> observer) {
		synchronized (mObservers) {
			mObservers.remove(observer);
		}
	}

	/**
	 * Gets the singleton object in a singleton store.
	 *
	 * This method is useful if the store has been instantiated with Void as Object type
	 *
	 * @return the object in this store, null if there are no object in the store
	 */
	public OBJECT get() {
		synchronized (mStore) {
			return mStore.get(null);
		}
	}

	/**
	 * Gets an registry object by it's key.
	 *
	 * @param key the key of the object to get
	 * @return the object with this key, null if there are no object with the requested key
	 */
	public OBJECT get(KEY key) {
		synchronized (mStore) {
			return mStore.get(key);
		}
	}

	/**
	 * Gets the all objects in the store.
	 *
	 * @return list containing all objects of the store
	 */
	public List<OBJECT> getAll() {
		synchronized (mStore) {
			return new ArrayList<OBJECT>(mStore.values());
		}
	}

	/**
	 * Gets the all objects in the store matching a filter.
	 *
	 * @param filter the filter to test
	 * @return list containing all objects of the store
	 */
	public List<OBJECT> getAll(IFilter<OBJECT> filter) {
		List<OBJECT> result = new ArrayList<OBJECT>();
		synchronized (mStore) {
			for (OBJECT object : mStore.values()) {
				if (filter.checkMatch(object)) {
					result.add(object);
				}
			}
			return result;
		}
	}


	/**
	 * Gets an object by its handle.
	 *
	 * @param handle the handle of the requested object
	 * @return the object with this handle, null if not found
	 */
	@SuppressWarnings("unchecked")
	public OBJECT getByHandle(int handle) {
		IBusObject obj = mBusClient.getObject(handle);
		// ensure its the correct object typoe
		if (obj != null && obj.getDescriptor().equals(mObjectdesc)) {
			return (OBJECT) mBusClient.getObject(handle);
		}
		return null;
	}

	/**
	 * Adds an object to the registry.
	 *
	 * Registry key for this object is extracted using the key extractor injected during Registry construction. If there is already
	 * an object with this key, the new object is not added to the registry. If the object has been added, all observers are
	 * notified of the added object.
	 *
	 * @param object the object to add
	 * @param busEventType the bus event type adding this object, null if not part of a bus event
	 */
	@SuppressWarnings("unchecked")
	public void add(IBusObject object, Enum<?> busEventType) {
		boolean changed = false;
		synchronized (mStore) {
			KEY key = null;
			if (mKeyExtractor != null) {
				key = mKeyExtractor.getKey((OBJECT) object);
			}
			if (!mStore.containsKey(key)) {
				mStore.put(key, (OBJECT) object);
				changed = true;
			}
		}
		if (changed) {
            IObserver<OBJECT, BUS_EVENT_TYPE>[] observers;
            synchronized (mObservers) {
                observers = mObservers.toArray(new IObserver[mObservers.size()]);
            }
            for (IObserver<OBJECT, BUS_EVENT_TYPE> observer : observers) {
                observer.onObjectAdded((OBJECT) object, (BUS_EVENT_TYPE) busEventType);
			}
        }
	}

	/**
	 * Removes an object to the registry.
	 *
	 * If the registry contains this object, it's removed from the registry and all observers are notified of the removed object
	 *
	 * @param object the object to remove
	 * @param busEventType the bus event type removing this object, null if not part of a bus event
	 */
	@SuppressWarnings("unchecked")
	public void remove(IBusObject object, Enum<?> busEventType) {
		boolean changed = false;
		synchronized (mStore) {
			KEY key = null;
			if (mKeyExtractor != null) {
				key = mKeyExtractor.getKey((OBJECT) object);
			}
			if (object.equals(mStore.get(key))) {
				mStore.remove(key);
				changed = true;
			}
		}
		if (changed) {
            IObserver<OBJECT, BUS_EVENT_TYPE>[] observers;
            synchronized (mObservers) {
                observers = mObservers.toArray(new IObserver[mObservers.size()]);
            }
            for (IObserver<OBJECT, BUS_EVENT_TYPE> observer : observers) {
                observer.onObjectRemoved((OBJECT) object, (BUS_EVENT_TYPE) busEventType);
            }
		}
	}
}
