/******************************************************************************
* @file PsSummary.java
*
* @brief PsSummary
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
import com.parrot.obus.test.psbus.IPsSummary;

/** Object */
@SuppressWarnings("javadoc")
public final class PsSummary extends ObusObject implements IPsSummary {
	/** Event */
	public static final class Event extends ObusEvent implements IPsSummary.IEvent {
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
		public boolean hasPcpus() {
			return this.struct.hasField(PsSummary.fieldDescPcpus);
		}
		@Override
		public boolean hasTaskTotal() {
			return this.struct.hasField(PsSummary.fieldDescTaskTotal);
		}
		@Override
		public boolean hasTaskRunning() {
			return this.struct.hasField(PsSummary.fieldDescTaskRunning);
		}
		@Override
		public boolean hasTaskSleeping() {
			return this.struct.hasField(PsSummary.fieldDescTaskSleeping);
		}
		@Override
		public boolean hasTaskStopped() {
			return this.struct.hasField(PsSummary.fieldDescTaskStopped);
		}
		@Override
		public boolean hasTaskZombie() {
			return this.struct.hasField(PsSummary.fieldDescTaskZombie);
		}
		@Override
		public boolean hasRefreshRate() {
			return this.struct.hasField(PsSummary.fieldDescRefreshRate);
		}
		@Override
		public boolean hasMode() {
			return this.struct.hasField(PsSummary.fieldDescMode);
		}

		/** Method state checkers */
		@Override
		public boolean hasMethodStateSetRefreshRate() {
			return this.struct.hasField(PsSummary.fieldDescMethodStateSetRefreshRate);
		}
		@Override
		public boolean hasMethodStateSetMode() {
			return this.struct.hasField(PsSummary.fieldDescMethodStateSetMode);
		}

		/** Property getters */
		@Override
		public int[] getPcpus() {
			return this.struct.getFieldIntArray(PsSummary.fieldDescPcpus);
		}
		@Override
		public int getTaskTotal() {
			return this.struct.getFieldInt(PsSummary.fieldDescTaskTotal);
		}
		@Override
		public int getTaskRunning() {
			return this.struct.getFieldInt(PsSummary.fieldDescTaskRunning);
		}
		@Override
		public int getTaskSleeping() {
			return this.struct.getFieldInt(PsSummary.fieldDescTaskSleeping);
		}
		@Override
		public int getTaskStopped() {
			return this.struct.getFieldInt(PsSummary.fieldDescTaskStopped);
		}
		@Override
		public int getTaskZombie() {
			return this.struct.getFieldInt(PsSummary.fieldDescTaskZombie);
		}
		@Override
		public int getRefreshRate() {
			return this.struct.getFieldInt(PsSummary.fieldDescRefreshRate);
		}
		@Override
		public Mode getMode() {
			return (Mode)this.struct.getFieldEnum(PsSummary.fieldDescMode);
		}

		/** Method state getters */
		@Override
		public MethodState getMethodStateSetRefreshRate() {
			return (MethodState)this.struct.getFieldEnum(PsSummary.fieldDescMethodStateSetRefreshRate);
		}
		@Override
		public MethodState getMethodStateSetMode() {
			return (MethodState)this.struct.getFieldEnum(PsSummary.fieldDescMethodStateSetMode);
		}

		/** Event creator */
		private static final ObusEventCreator<Event> creator = new ObusEventCreator<Event>(){
			@Override
			public Event create(EventDesc evtDesc, ObusObject obj, ObusStruct struct) {
				return new Event(evtDesc, obj, struct);
			}
		};
	}

	/** Call class */
	public static final class CallSetRefreshRate extends ObusCall {
		/** */
		protected CallSetRefreshRate(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {
			super(mtdDesc, obj, struct);
		}

		/** Argument getters */
		public int getRefreshRate() {
			return this.struct.getFieldInt(CallSetRefreshRate.fieldDescRefreshRate);
		}

		/** Argument setters */
		public void setRefreshRate(int refreshRate) {
			this.struct.setFieldInt(CallSetRefreshRate.fieldDescRefreshRate, refreshRate);
		}

		/** Argument descriptors */
		private static final FieldDesc fieldDescRefreshRate = new FieldDesc(
				"refresh_rate",
				1,
				FieldType.OBUS_FIELD_U32,
				FieldRole.OBUS_ARGUMENT,
				null);

		/** Structure descriptor */
		private static final StructDesc structDesc = new StructDesc();
		static {
			CallSetRefreshRate.structDesc.addField(CallSetRefreshRate.fieldDescRefreshRate);
		}

		/** Call creator */
		private static final ObusCallCreator<CallSetRefreshRate> creator = new ObusCallCreator<CallSetRefreshRate>() {
			@Override
			public CallSetRefreshRate create(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {
				return new CallSetRefreshRate(mtdDesc, obj, struct);
			}
		};

		/** Method descriptor */
		private static final MethodDesc methodDesc = new MethodDesc(
				"set_refresh_rate",
				101,
				CallSetRefreshRate.structDesc,
				CallSetRefreshRate.creator);
	}

	/** Call class */
	public static final class CallSetMode extends ObusCall {
		/** */
		protected CallSetMode(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {
			super(mtdDesc, obj, struct);
		}

		/** Argument getters */
		public Mode getMode() {
			return (Mode)this.struct.getFieldEnum(CallSetMode.fieldDescMode);
		}

		/** Argument setters */
		public void setMode(Mode mode) {
			this.struct.setFieldEnum(CallSetMode.fieldDescMode, mode);
		}

		/** Argument descriptors */
		private static final FieldDesc fieldDescMode = new FieldDesc(
				"mode",
				1,
				FieldType.OBUS_FIELD_ENUM,
				FieldRole.OBUS_ARGUMENT,
				ModeDriver);

		/** Structure descriptor */
		private static final StructDesc structDesc = new StructDesc();
		static {
			CallSetMode.structDesc.addField(CallSetMode.fieldDescMode);
		}

		/** Call creator */
		private static final ObusCallCreator<CallSetMode> creator = new ObusCallCreator<CallSetMode>() {
			@Override
			public CallSetMode create(MethodDesc mtdDesc, ObusObject obj, ObusStruct struct) {
				return new CallSetMode(mtdDesc, obj, struct);
			}
		};

		/** Method descriptor */
		private static final MethodDesc methodDesc = new MethodDesc(
				"set_mode",
				102,
				CallSetMode.structDesc,
				CallSetMode.creator);
	}

		public static final FieldDriverEnum<Mode> ModeDriver = new FieldDriverEnum<Mode>(Mode.class) {
		@Override
		public Mode init() {
			return Mode.IRIX;
		}
		@Override
		public int toInt(Mode val) {
			switch (val) {
				case SOLARIS: return 0;
				case IRIX: return 1;
				default: return 0;
			}
		}
		@Override
		public Mode fromInt(int val) throws DecodeError {
			switch (val) {
				case 0: return Mode.SOLARIS;
				case 1: return Mode.IRIX;
			default: throw new DecodeError("Invalid summary Mode: " + val);
			}
		}
	};

	/** */
	private PsSummary(ObjectDesc objDesc, ObusStruct struct) {
		super(objDesc, struct);
	}

	/** Property getters */
	@Override
	public int[] getPcpus() {
		return this.struct.getFieldIntArray(PsSummary.fieldDescPcpus);
	}
	@Override
	public int getTaskTotal() {
		return this.struct.getFieldInt(PsSummary.fieldDescTaskTotal);
	}
	@Override
	public int getTaskRunning() {
		return this.struct.getFieldInt(PsSummary.fieldDescTaskRunning);
	}
	@Override
	public int getTaskSleeping() {
		return this.struct.getFieldInt(PsSummary.fieldDescTaskSleeping);
	}
	@Override
	public int getTaskStopped() {
		return this.struct.getFieldInt(PsSummary.fieldDescTaskStopped);
	}
	@Override
	public int getTaskZombie() {
		return this.struct.getFieldInt(PsSummary.fieldDescTaskZombie);
	}
	@Override
	public int getRefreshRate() {
		return this.struct.getFieldInt(PsSummary.fieldDescRefreshRate);
	}
	@Override
	public Mode getMode() {
		return (Mode)this.struct.getFieldEnum(PsSummary.fieldDescMode);
	}

	/** Method state getters */
	@Override
	public MethodState getMethodStateSetRefreshRate() {
		return (MethodState)this.struct.getFieldEnum(PsSummary.fieldDescMethodStateSetRefreshRate);
	}
	@Override
	public MethodState getMethodStateSetMode() {
		return (MethodState)this.struct.getFieldEnum(PsSummary.fieldDescMethodStateSetMode);
	}

	/** Method call */
	@Override
	public void setRefreshRate(BusClient client, int refreshRate, IBusMethodCall.AckCb callback) {
		CallSetRefreshRate call_ = (CallSetRefreshRate)ObusCall.create(CallSetRefreshRate.methodDesc, this);
		call_.setRefreshRate(refreshRate);
		client.callBusMethod(call_, callback);
	}
	@Override
	public void setMode(BusClient client, Mode mode, IBusMethodCall.AckCb callback) {
		CallSetMode call_ = (CallSetMode)ObusCall.create(CallSetMode.methodDesc, this);
		call_.setMode(mode);
		client.callBusMethod(call_, callback);
	}

	/** Property descriptors */
	private static final FieldDesc fieldDescPcpus = new FieldDesc(
			"pcpus",
			1,
			FieldType.OBUS_FIELD_U32 | FieldType.OBUS_FIELD_ARRAY,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescTaskTotal = new FieldDesc(
			"task_total",
			2,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescTaskRunning = new FieldDesc(
			"task_running",
			3,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescTaskSleeping = new FieldDesc(
			"task_sleeping",
			4,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescTaskStopped = new FieldDesc(
			"task_stopped",
			5,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescTaskZombie = new FieldDesc(
			"task_zombie",
			6,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescRefreshRate = new FieldDesc(
			"refresh_rate",
			7,
			FieldType.OBUS_FIELD_U32,
			FieldRole.OBUS_PROPERTY,
			null);
	private static final FieldDesc fieldDescMode = new FieldDesc(
			"mode",
			8,
			FieldType.OBUS_FIELD_ENUM,
			FieldRole.OBUS_PROPERTY,
			ModeDriver);

	/** Method state descriptors */
	private static final FieldDesc fieldDescMethodStateSetRefreshRate = new FieldDesc(
			"method_state_set_refresh_rate",
			101,
			FieldType.OBUS_FIELD_ENUM,
			FieldRole.OBUS_METHOD,
			Descriptor.FIELD_DRIVER_METHOD_STATE);
	private static final FieldDesc fieldDescMethodStateSetMode = new FieldDesc(
			"method_state_set_mode",
			102,
			FieldType.OBUS_FIELD_ENUM,
			FieldRole.OBUS_METHOD,
			Descriptor.FIELD_DRIVER_METHOD_STATE);

	/** Structure descriptor */
	private static final StructDesc structDesc = new StructDesc();
	static {
		PsSummary.structDesc.addField(PsSummary.fieldDescPcpus);
		PsSummary.structDesc.addField(PsSummary.fieldDescTaskTotal);
		PsSummary.structDesc.addField(PsSummary.fieldDescTaskRunning);
		PsSummary.structDesc.addField(PsSummary.fieldDescTaskSleeping);
		PsSummary.structDesc.addField(PsSummary.fieldDescTaskStopped);
		PsSummary.structDesc.addField(PsSummary.fieldDescTaskZombie);
		PsSummary.structDesc.addField(PsSummary.fieldDescRefreshRate);
		PsSummary.structDesc.addField(PsSummary.fieldDescMode);
		PsSummary.structDesc.addField(PsSummary.fieldDescMethodStateSetRefreshRate);
		PsSummary.structDesc.addField(PsSummary.fieldDescMethodStateSetMode);
	}

	/** Event descriptors */
	private static final EventDesc eventDescUpdated = new EventDesc(
			"updated",
			1,
			Event.Type.UPDATED,
			PsSummary.structDesc,
			Event.creator);
	static {
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescPcpus, 0));
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescTaskTotal, 0));
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescTaskRunning, 0));
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescTaskSleeping, 0));
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescTaskStopped, 0));
		PsSummary.eventDescUpdated.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescTaskZombie, 0));
	}
	private static final EventDesc eventDescRefreshRateChanged = new EventDesc(
			"refresh_rate_changed",
			2,
			Event.Type.REFRESH_RATE_CHANGED,
			PsSummary.structDesc,
			Event.creator);
	static {
		PsSummary.eventDescRefreshRateChanged.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescRefreshRate, 0));
	}
	private static final EventDesc eventDescModeChanged = new EventDesc(
			"mode_changed",
			3,
			Event.Type.MODE_CHANGED,
			PsSummary.structDesc,
			Event.creator);
	static {
		PsSummary.eventDescModeChanged.addEventUpdateDesc(new EventUpdateDesc(PsSummary.fieldDescMode, 0));
	}

	/** Object creator */
	private static final ObusObjectCreator<PsSummary> creator = new ObusObjectCreator<PsSummary>() {
		@Override
		public PsSummary create(ObjectDesc objDesc, ObusStruct struct) {
			return new PsSummary(objDesc, struct);
		}
	};

	/** Object descriptor */
	public static final ObjectDesc objectDesc = new ObjectDesc(
			"summary",
			2,
			PsSummary.structDesc,
			PsSummary.creator);
	static {
		PsSummary.objectDesc.addEvent(PsSummary.eventDescUpdated);
		PsSummary.objectDesc.addEvent(PsSummary.eventDescRefreshRateChanged);
		PsSummary.objectDesc.addEvent(PsSummary.eventDescModeChanged);
		PsSummary.objectDesc.addMethod(CallSetRefreshRate.methodDesc);
		PsSummary.objectDesc.addMethod(CallSetMode.methodDesc);
	}
}
