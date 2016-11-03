/******************************************************************************
* @file PsProcess.java
*
* @brief PsProcess
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus.impl;

import com.parrot.obus.internal.Core.ObusCallCreator;
import com.parrot.obus.internal.Core.DecodeError;
import com.parrot.obus.internal.Core.ObusEventCreator;
import com.parrot.obus.internal.Core.ObusObjectCreator;
import com.parrot.obus.internal.Descriptor;
import com.parrot.obus.internal.Descriptor.EventDesc;
import com.parrot.obus.internal.Descriptor.EventUpdateDesc;
import com.parrot.obus.internal.Descriptor.FieldDesc;
import com.parrot.obus.internal.Descriptor.FieldDriverEnum;
import com.parrot.obus.internal.Descriptor.FieldRole;
import com.parrot.obus.internal.Descriptor.FieldType;
import com.parrot.obus.internal.Descriptor.MethodDesc;
import com.parrot.obus.internal.Descriptor.ObjectDesc;
import com.parrot.obus.internal.Descriptor.StructDesc;
import com.parrot.obus.MethodState;
import com.parrot.obus.BusClient;
import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.internal.ObusCall;
import com.parrot.obus.internal.ObusEvent;
import com.parrot.obus.internal.ObusObject;
import com.parrot.obus.internal.ObusStruct;
import com.parrot.obus.test.psbus.IPsProcess;

/** Object */
@SuppressWarnings("javadoc")
public final class PsProcess extends ObusObject implements IPsProcess {
	/** Event */
	public static final class Event extends ObusEvent implements IPsProcess.IEvent {
		/** */
		private Event(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {
			super(evtDesc, obj, struct);
		}

		/** */
		@Override
		public Type getType() {
			return (Type)this.desc.evtType;
		}

		/** Property checkers */
		@Override
		public boolean hasPid() {
			return this.struct.hasField(PsProcess.fieldDescPid);
		}
		@Override
		public boolean hasPpid() {
			return this.struct.hasField(PsProcess.fieldDescPpid);
		}
		@Override
		public boolean hasName() {
			return this.struct.hasField(PsProcess.fieldDescName);
		}
		@Override
		public boolean hasExe() {
			return this.struct.hasField(PsProcess.fieldDescExe);
		}
		@Override
		public boolean hasPcpu() {
			return this.struct.hasField(PsProcess.fieldDescPcpu);
		}
		@Override
		public boolean hasState() {
			return this.struct.hasField(PsProcess.fieldDescState);
		}

		/** Method state checkers */

		/** Property getters */
		@Override
		public int getPid() {
			return this.struct.getFieldInt(PsProcess.fieldDescPid);
		}
		@Override
		public int getPpid() {
			return this.struct.getFieldInt(PsProcess.fieldDescPpid);
		}
		@Override
		public String getName() {
			return this.struct.getFieldString(PsProcess.fieldDescName);
		}
		@Override
		public String getExe() {
			return this.struct.getFieldString(PsProcess.fieldDescExe);
		}
		@Override
		public int getPcpu() {
			return this.struct.getFieldInt(PsProcess.fieldDescPcpu);
		}
		@Override
		public State getState() {
			return (State)this.struct.getFieldEnum(PsProcess.fieldDescState);
		}

		/** Method state getters */

		/** Event creator */
		private static final ObusEventCreator<Event> creator = new ObusEventCreator<Event>(){
			@Override
			public Event create(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {
				return new Event(evtDesc, obj, struct);
			}
		};
	}

		public static final FieldDriverEnum<State> StateDriver = new FieldDriverEnum<State>(State.class) {
		@Override
		public State init() {
			return State.UNKNOWN;
		}
		@Override
		public int toInt(State val) {
			switch (val) {
				case UNKNOWN: return -1;
				case RUNNING: return 0;
				case SLEEPING: return 1;
				case STOPPED: return 2;
				case ZOMBIE: return 3;
				default: return 0;
			}
		}
		@Override
		public State fromInt(int val) throws DecodeError {
			switch (val) {
				case -1: return State.UNKNOWN;
				case 0: return State.RUNNING;
				case 1: return State.SLEEPING;
				case 2: return State.STOPPED;
				case 3: return State.ZOMBIE;
			default: throw new DecodeError("Invalid process State: " + val);
			}
		}
	};

	/** */
	private PsProcess(ObjectDesc objDesc, ObusStruct struct) {
		super(objDesc, struct);
	}

	/** Property getters */
	@Override
	public int getPid() {
		return this.struct.getFieldInt(PsProcess.fieldDescPid);
	}
	@Override
	public int getPpid() {
		return this.struct.getFieldInt(PsProcess.fieldDescPpid);
	}
	@Override
	public String getName() {
		return this.struct.getFieldString(PsProcess.fieldDescName);
	}
	@Override
	public String getExe() {
		return this.struct.getFieldString(PsProcess.fieldDescExe);
	}
	@Override
	public int getPcpu() {
		return this.struct.getFieldInt(PsProcess.fieldDescPcpu);
	}
	@Override
	public State getState() {
		return (State)this.struct.getFieldEnum(PsProcess.fieldDescState);
	}

	/** Method state getters */

	/** Method call */

	/** Property descriptors */
	private static final FieldDesc fieldDescPid = new FieldDesc(
			"pid",
			1,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescPpid = new FieldDesc(
			"ppid",
			2,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescName = new FieldDesc(
			"name",
			3,
			FieldType.OBUS_FIELD_STRING,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescExe = new FieldDesc(
			"exe",
			4,
			FieldType.OBUS_FIELD_STRING,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescPcpu = new FieldDesc(
			"pcpu",
			5,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescState = new FieldDesc(
			"state",
			6,
			FieldType.OBUS_FIELD_ENUM,
			FieldRole.OBUS_PROPERTY,
			StateDriver);

	/** Method state descriptors */

	/** Structure descriptor */
	private static final StructDesc structDesc = new StructDesc();
	static {
		PsProcess.structDesc.addField(PsProcess.fieldDescPid);
		PsProcess.structDesc.addField(PsProcess.fieldDescPpid);
		PsProcess.structDesc.addField(PsProcess.fieldDescName);
		PsProcess.structDesc.addField(PsProcess.fieldDescExe);
		PsProcess.structDesc.addField(PsProcess.fieldDescPcpu);
		PsProcess.structDesc.addField(PsProcess.fieldDescState);
	}

	/** Event descriptors */
	private static final EventDesc eventDescUpdated = new EventDesc(
			"updated",
			1,
			Event.Type.UPDATED,
			PsProcess.structDesc,
			Event.creator);
	static {
		PsProcess.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsProcess.fieldDescPpid, 0));
		PsProcess.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsProcess.fieldDescName, 0));
		PsProcess.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsProcess.fieldDescExe, 0));
		PsProcess.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsProcess.fieldDescPcpu, 0));
		PsProcess.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsProcess.fieldDescState, 0));
	}

	/** Object creator */
	private static final ObusObjectCreator<PsProcess> creator = new ObusObjectCreator<PsProcess>() {
		@Override
		public PsProcess create(ObjectDesc objDesc, ObusStruct struct) {
			return new PsProcess(objDesc, struct);
		}
	};

	/** Object descriptor */
	public static final ObjectDesc objectDesc = new ObjectDesc(
			"process",
			1,
			PsProcess.structDesc,
			PsProcess.creator);
	static {
		PsProcess.objectDesc.addEvent(PsProcess.eventDescUpdated);
	}
}
