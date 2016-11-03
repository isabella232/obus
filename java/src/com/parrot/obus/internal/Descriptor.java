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

import java.util.Iterator;
import java.util.NoSuchElementException;

import android.util.SparseArray;

import com.parrot.obus.MethodState;
import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Core.ObusBusEventCreator;
import com.parrot.obus.internal.Core.ObusCallCreator;
import com.parrot.obus.internal.Core.ObusEventCreator;
import com.parrot.obus.internal.Core.ObusObjectCreator;

/**
 * Obus descriptors.
 */
public class Descriptor {

	/**
	 * Descriptor map. This maps uid to descriptor.
	 */
	public static class DescMap<T> extends SparseArray<T> {
		/**
		 * Get values to be used in a foreach loop.
		 * @return iterable object.
		 */
		public synchronized Iterable<T> values() {
			/* Only create Iterable when needed */
			if (this.valuesIterable == null) {
				this.valuesIterable = new ValuesIterable();
			}
			return this.valuesIterable;
		}

		/** Iterable for values */
		private ValuesIterable valuesIterable;
		private class ValuesIterable implements Iterable<T> {
			@Override
			public Iterator<T> iterator() {
				return new Iterator<T>() {
					/** Current index in iteration */
					private int idx = 0;
					/** Check if there is a next element by looking at size. */
					@Override
					public boolean hasNext() {
						return (this.idx < DescMap.this.size());
					}
					/** Get next element of iteration. */
					@Override
					public T next() {
						if (this.idx >= DescMap.this.size()) {
							throw new NoSuchElementException();
						}
						return DescMap.this.valueAt(this.idx++);
					}
					/** Not possible to remove elements during iteration */
					@Override
					public void remove() {
						throw new UnsupportedOperationException();
					}
				};
			}
		}
	}

	/**
	 * Field type (values part of protocol).
	 */
	public static class FieldType
	{
		public static final int OBUS_FIELD_U8 = 0;
		public static final int OBUS_FIELD_I8 = 1;
		public static final int OBUS_FIELD_U16 = 2;
		public static final int OBUS_FIELD_I16 = 3;
		public static final int OBUS_FIELD_U32 = 4;
		public static final int OBUS_FIELD_I32 = 5;
		public static final int OBUS_FIELD_U64 = 6;
		public static final int OBUS_FIELD_I64 = 7;
		public static final int OBUS_FIELD_ENUM = 8;
		public static final int OBUS_FIELD_STRING = 9;
		public static final int OBUS_FIELD_BOOL = 10;
		public static final int OBUS_FIELD_F32 = 11;
		public static final int OBUS_FIELD_F64 = 12;
		public static final int OBUS_FIELD_ARRAY = (1<<7);
		public static final int OBUS_FIELD_MASK = 0x7f;
	}

	/**
	 * Field role.
	 */
	public static enum FieldRole
	{
		OBUS_PROPERTY,  /**< Property */
		OBUS_METHOD,    /**< Method state */
		OBUS_ARGUMENT,  /**< Argument */
	}

	/**
	 * Field descriptor.
	 */
	public static class FieldDesc {

		public final String name;    /**< Field name */
		public final int uid;        /**< Field uid */
		public final int type;       /**< Field type */
		public final FieldRole role; /**< Field role */
		private int idx;             /**< Index in parent struct */

		/** Field driver for enum fields */
		public final FieldDriverEnum<?> driver;

		/**
		 * Create a new field descriptor.
		 * @param name : field name.
		 * @param uid : field uid.
		 * @param type : field type.
		 * @param role : field role.
		 * @param driver : field driver for enum fields.
		 */
		public FieldDesc(String name, int uid, int type, FieldRole role,
				FieldDriverEnum<?> driver) {
			this.name = name;
			this.uid = uid;
			this.type = type;
			this.role = role;
			this.driver = driver;
			this.idx = -1;
		}

		/** */
		public int getIdx() {
			return this.idx;
		}

		/**
		 * get Field Role string
		 * @return
		 */
		public String getRoleStr() {
			switch(this.role) {
			default:
			case OBUS_PROPERTY:
				return "property";
			case OBUS_METHOD:
				return "method";
			case OBUS_ARGUMENT:
				return "argument";
			}
		}

		/** */
		private void setIdx(int idx) {
			this.idx = idx;
		}
	}

	/**
	 * Struct descriptor.
	 */
	public static class StructDesc {

		/** Fields descriptors */
		private final DescMap<FieldDesc> fieldsDesc = new DescMap<FieldDesc>();

		/**
		 * Add a new field descriptor.
		 * @param fieldDesc : field descriptor.
		 */
		public void addField(FieldDesc fieldDesc) {
			fieldDesc.setIdx(this.fieldsDesc.size());
			this.fieldsDesc.put(fieldDesc.uid, fieldDesc);
		}

		/** */
		public DescMap<FieldDesc> getFieldsDesc() {
			return this.fieldsDesc;
		}
	}

	/**
	 * Method descriptor.
	 */
	public static class MethodDesc {
		public final String name; /**< Method name */
		public final int uid;     /**< Method uid */

		/** Method struct descriptor */
		public final StructDesc structDesc;

		/** Creator of specific ObusCall objects. */
		public final ObusCallCreator<?> creator;

		/**
		 * Create a new method descriptor.
		 * @param name : method name.
		 * @param uid : method uid.
		 * @param structDesc : method structure descriptor.
		 * @param creator : creator of specific ObusCall objects.
		 */
		public MethodDesc(String name, int uid, StructDesc structDesc,
				ObusCallCreator<?> creator) {
			this.name = name;
			this.uid = uid;
			this.structDesc = structDesc;
			this.creator = creator;
		}
	}

	/**
	 * Event update descriptor.
	 */
	public static class EventUpdateDesc {
		public final FieldDesc field;	/**< update field desc */
		public final int flags;			/**< update flags */

		/**
		 * Create a new event update descriptor.
		 * @param field : object field.
		 * @param flags : update flags.
		 */
		public EventUpdateDesc(FieldDesc field, int flags) {
			this.field = field;
			this.flags = flags;
		}
	}

	/**
	 * Event descriptor.
	 */
	public static class EventDesc {
		public final String name;     /**< Event name */
		public final int uid;         /**< Event uid */
		public final Enum<?> evtType; /**< Event type */

		/** Event struct descriptor */
		public final StructDesc structDesc;

		/** Creator of specific ObusEvent objects. */
		public final ObusEventCreator<?> creator;

		/** Event update descriptor array */
		private final DescMap<EventUpdateDesc> updatesDesc = new DescMap<EventUpdateDesc>();

		/**
		* Add a new update descriptor.
		* @param fieldDesc : field descriptor.
		*/
		public void addEventUpdateDesc(EventUpdateDesc desc) {
			this.updatesDesc.put(desc.field.uid, desc);
		}

		public boolean hasUpdateField(FieldDesc desc) {
			return this.updatesDesc.get(desc.uid) != null;
		}

		public DescMap<EventUpdateDesc> getUpdateFieldsDesc() {
			return this.updatesDesc;
		}

		/**
		 * Create a new event descriptor.
		 * @param name : event name.
		 * @param uid : event uid.
		 * @param evtType : event type.
		 * @param structDesc : event structure descriptor.
		 * @param creator : creator of specific ObusEvent objects.
		 */
		public EventDesc(String name, int uid, Enum<?> evtType,
				StructDesc structDesc, ObusEventCreator<?> creator) {
			this.name = name;
			this.uid = uid;
			this.evtType = evtType;
			this.structDesc = structDesc;
			this.creator = creator;
		}
	}

	/**
	 * Object descriptor.
	 */
	public static class ObjectDesc {
		public final String name; /**< Object type name */
		public final int uid;     /**< Object type uid */

		/**< Object struct descriptor */
		public final StructDesc structDesc;

		/** Creator of specific ObusObject objects */
		public final ObusObjectCreator<?> creator;

		/** Object events descriptors */
		public final DescMap<EventDesc> eventsDesc = new DescMap<EventDesc>();

		/** Object methods descriptors */
		public final DescMap<MethodDesc> methodsDesc = new DescMap<MethodDesc>();

		/**
		 * Create a new object descriptor.
		 * @param name : object type name.
		 * @param uid : object type uid.
		 * @param structDesc : object structure descriptor.
		 * @param creator : creator of specific ObusObject objects
		 */
		public ObjectDesc(String name, int uid, StructDesc structDesc,
				ObusObjectCreator<?> creator) {
			this.name = name;
			this.uid = uid;
			this.structDesc = structDesc;
			this.creator = creator;
		}

		/**
		 * Add a new event descriptor.
		 * @param evtDesc : event descriptor.
		 */
		public void addEvent(EventDesc evtDesc) {
			this.eventsDesc.put(evtDesc.uid, evtDesc);
		}

		/**
		 * Add a new method descriptor.
		 * @param mtdDesc : method descriptor.
		 */
		public void addMethod(MethodDesc mtdDesc) {
			this.methodsDesc.put(mtdDesc.uid, mtdDesc);
		}
	}

	/**
	 * Bus event descriptor.
	 */
	public static class BusEventDesc {
		public final String name;        /**< Bus event name */
		public final int uid;            /**< Bus event uid */
		public final Enum<?> busEvtType; /**< Bus event type */

		/** Creator of specific ObusBusEvent objects */
		public final ObusBusEventCreator<?> creator;

		public BusEventDesc(String name, int uid, Enum<?> busEvtType,
				ObusBusEventCreator<?> creator) {
			this.name = name;
			this.uid = uid;
			this.busEvtType = busEvtType;
			this.creator = creator;
		}
	}

	/**
	 * Bus descriptor.
	 */
	public static class BusDesc {
		public final String name; /**< Bus name */
		public final int crc;     /**< Bus crc32 */

		/** Bus objects descriptors */
		public final DescMap<ObjectDesc> objectsDesc = new DescMap<ObjectDesc>();

		/** Bus events descriptors */
		public final DescMap<BusEventDesc> eventsDesc = new DescMap<BusEventDesc>();

		/**
		 * Create a new bus descriptor.
		 * @param name : bus name.
		 * @param crc : bus crc.
		 */
		public BusDesc(String name, int crc) {
			this.name = name;
			this.crc = crc;
		}

		/**
		 * Add a new object descriptor.
		 * @param objDesc : object descriptor.
		 */
		public void addObject(ObjectDesc objDesc) {
			this.objectsDesc.put(objDesc.uid, objDesc);
		}

		/**
		 * Add a new bus event descriptor.
		 * @param busEvtDesc : bus event descriptor
		 */
		public void addEvent(BusEventDesc busEvtDesc) {
			this.eventsDesc.put(busEvtDesc.uid, busEvtDesc);
		}
	}

	/**
	 * Generic enum field driver.
	 */
	public static abstract class FieldDriverEnum<E extends Enum<E>> {
		/** Runtime class of the enum */
		public final Class<E> cls;

		/**
		 * Create a new enum field driver.
		 * @param cls : runtime class of the enum field.
		 */
		public FieldDriverEnum(Class<E> cls) {
			this.cls = cls;
		}

		/**
		 * Initialize the enum to a default value.
		 * @return default value for the enum field.
		 */
		public abstract E init();

		/**
		 * Convert from Enum to int.
		 * @param val : Enum value.
		 * @return int value.
		 */
		public abstract int toInt(E val);

		/**
		 * Convert from int to Enum.
		 * @param val : int value.
		 * @return Enum value.
		 * @throws DecodeError if the value is not valid.
		 */
		public abstract E fromInt(int val) throws DecodeError;
	}

	/** Method state field driver */
	public static final FieldDriverEnum<MethodState> FIELD_DRIVER_METHOD_STATE =
			new FieldDriverEnum<MethodState>(MethodState.class) {
		@Override
		public MethodState init() {
			return MethodState.NOT_SUPPORTED;
		}
		@Override
		public int toInt(MethodState val) {
			switch(val) {
				case ENABLED: return 1;
				case DISABLED: return 2;
				default: return 0;
			}
		}
		@Override
		public MethodState fromInt(int val) throws DecodeError {
			switch (val) {
				case 0: return MethodState.NOT_SUPPORTED;
				case 1: return MethodState.ENABLED;
				case 2: return MethodState.DISABLED;
				default: throw new DecodeError("Invalid method state: " + val);
			}
		}
	};
}
