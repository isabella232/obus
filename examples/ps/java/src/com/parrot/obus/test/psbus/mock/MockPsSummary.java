/******************************************************************************
* @file MockPsSummary.java
*
* @brief MockPsSummary
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus.mock;

import com.parrot.obus.test.psbus.IPsSummary;
import com.parrot.obus.test.psbus.impl.PsSummary;
import com.parrot.obus.MethodState;
import com.parrot.obus.internal.Descriptor.ObjectDesc;
import com.parrot.obus.BusClient;
import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.mock.MockBusObject;
import com.parrot.obus.mock.MockBusObjectEvent;
import com.parrot.obus.mock.MockMethodCall;

/** Mock Object */
@SuppressWarnings("javadoc")
public class MockPsSummary extends MockBusObject implements IPsSummary {
	/** Event */
	public static final class Event extends MockBusObjectEvent implements IPsSummary.IEvent {

		private Type type;
		private int[] pcpus;
		private int taskTotal;
		private int taskRunning;
		private int taskSleeping;
		private int taskStopped;
		private int taskZombie;
		private int refreshRate;
		private Mode mode;

		private MethodState methodStateSetRefreshRate;
		private MethodState methodStateSetMode;

		private boolean hasPcpus;
		private boolean hasTaskTotal;
		private boolean hasTaskRunning;
		private boolean hasTaskSleeping;
		private boolean hasTaskStopped;
		private boolean hasTaskZombie;
		private boolean hasRefreshRate;
		private boolean hasMode;

		private boolean hasMethodStateSetRefreshRate;
		private boolean hasMethodStateSetMode;

		/** event constructor */
		public Event(MockPsSummary object, Type type) {
			super(object);
			this.type = type;
		}

		/** Event type getter */
		@Override
		public Type getType() {
			return this.type;
		}

		/** Property checkers */
		@Override
		public boolean hasPcpus() {
			return this.hasPcpus;
		}
		@Override
		public boolean hasTaskTotal() {
			return this.hasTaskTotal;
		}
		@Override
		public boolean hasTaskRunning() {
			return this.hasTaskRunning;
		}
		@Override
		public boolean hasTaskSleeping() {
			return this.hasTaskSleeping;
		}
		@Override
		public boolean hasTaskStopped() {
			return this.hasTaskStopped;
		}
		@Override
		public boolean hasTaskZombie() {
			return this.hasTaskZombie;
		}
		@Override
		public boolean hasRefreshRate() {
			return this.hasRefreshRate;
		}
		@Override
		public boolean hasMode() {
			return this.hasMode;
		}

		/** Method state checkers */
		@Override
		public boolean hasMethodStateSetRefreshRate() {
			return this.hasMethodStateSetRefreshRate;
		}
		@Override
		public boolean hasMethodStateSetMode() {
			return this.hasMethodStateSetMode;
		}

		/** Property getters */
		@Override
		public int[] getPcpus() {
			return this.pcpus;
		}
		@Override
		public int getTaskTotal() {
			return this.taskTotal;
		}
		@Override
		public int getTaskRunning() {
			return this.taskRunning;
		}
		@Override
		public int getTaskSleeping() {
			return this.taskSleeping;
		}
		@Override
		public int getTaskStopped() {
			return this.taskStopped;
		}
		@Override
		public int getTaskZombie() {
			return this.taskZombie;
		}
		@Override
		public int getRefreshRate() {
			return this.refreshRate;
		}
		@Override
		public Mode getMode() {
			return this.mode;
		}

		/** Method state getters */
		@Override
		public MethodState getMethodStateSetRefreshRate() {
			return this.methodStateSetRefreshRate;
		}
		@Override
		public MethodState getMethodStateSetMode() {
			return this.methodStateSetMode;
		}

		/** Property setters */
		public void setPcpus(int[] pcpus) {
			this.pcpus = pcpus;
			this.hasPcpus = true;
		}
		public void setTaskTotal(int taskTotal) {
			this.taskTotal = taskTotal;
			this.hasTaskTotal = true;
		}
		public void setTaskRunning(int taskRunning) {
			this.taskRunning = taskRunning;
			this.hasTaskRunning = true;
		}
		public void setTaskSleeping(int taskSleeping) {
			this.taskSleeping = taskSleeping;
			this.hasTaskSleeping = true;
		}
		public void setTaskStopped(int taskStopped) {
			this.taskStopped = taskStopped;
			this.hasTaskStopped = true;
		}
		public void setTaskZombie(int taskZombie) {
			this.taskZombie = taskZombie;
			this.hasTaskZombie = true;
		}
		public void setRefreshRate(int refreshRate) {
			this.refreshRate = refreshRate;
			this.hasRefreshRate = true;
		}
		public void setMode(Mode mode) {
			this.mode = mode;
			this.hasMode = true;
		}

		/** Method state setters */
		public void setMethodStateSetRefreshRate(MethodState methodStateSetRefreshRate) {
			this.methodStateSetRefreshRate = methodStateSetRefreshRate;
			this.hasMethodStateSetRefreshRate = true;
		}
		public void setMethodStateSetMode(MethodState methodStateSetMode) {
			this.methodStateSetMode = methodStateSetMode;
			this.hasMethodStateSetMode = true;
		}

	}

	/** Methods enum */
	public enum Methods {
		/** Set the refresh rate */
		SET_REFRESH_RATE,
		/** Set the mode for PCPU */
		SET_MODE
	}

	private int[] pcpus;
	private int taskTotal;
	private int taskRunning;
	private int taskSleeping;
	private int taskStopped;
	private int taskZombie;
	private int refreshRate;
	private Mode mode;

	private MethodState methodStateSetRefreshRate;
	private MethodState methodStateSetMode;

	/** ObjDesc getters */
	@Override
	public ObjectDesc getDescriptor() {
		return PsSummary.objectDesc;
	}
	/** Property getters */
	@Override
	public int[] getPcpus() {
		return this.pcpus;
	}
	@Override
	public int getTaskTotal() {
		return this.taskTotal;
	}
	@Override
	public int getTaskRunning() {
		return this.taskRunning;
	}
	@Override
	public int getTaskSleeping() {
		return this.taskSleeping;
	}
	@Override
	public int getTaskStopped() {
		return this.taskStopped;
	}
	@Override
	public int getTaskZombie() {
		return this.taskZombie;
	}
	@Override
	public int getRefreshRate() {
		return this.refreshRate;
	}
	@Override
	public Mode getMode() {
		return this.mode;
	}

	/** Method state getters */
	@Override
	public MethodState getMethodStateSetRefreshRate() {
		return this.methodStateSetRefreshRate;
	}
	@Override
	public MethodState getMethodStateSetMode() {
		return this.methodStateSetMode;
	}

	/** Property setters */
	public void setPcpus(int[] pcpus) {
		this.pcpus = pcpus;
	}
	public void setTaskTotal(int taskTotal) {
		this.taskTotal = taskTotal;
	}
	public void setTaskRunning(int taskRunning) {
		this.taskRunning = taskRunning;
	}
	public void setTaskSleeping(int taskSleeping) {
		this.taskSleeping = taskSleeping;
	}
	public void setTaskStopped(int taskStopped) {
		this.taskStopped = taskStopped;
	}
	public void setTaskZombie(int taskZombie) {
		this.taskZombie = taskZombie;
	}
	public void setRefreshRate(int refreshRate) {
		this.refreshRate = refreshRate;
	}
	public void setMode(Mode mode) {
		this.mode = mode;
	}

	/** Method state setters */
	public void setMethodStateSetRefreshRate(MethodState methodStateSetRefreshRate) {
		this.methodStateSetRefreshRate = methodStateSetRefreshRate;
	}
	public void setMethodStateSetMode(MethodState methodStateSetMode) {
		this.methodStateSetMode = methodStateSetMode;
	}

	/** Method call */
	@Override
	public void setRefreshRate(BusClient client, int refreshRate, IBusMethodCall.AckCb callback) {
		client.callBusMethod(new MockMethodCall<Methods>(Methods.SET_REFRESH_RATE, this, refreshRate), callback);
	}
	@Override
	public void setMode(BusClient client, Mode mode, IBusMethodCall.AckCb callback) {
		client.callBusMethod(new MockMethodCall<Methods>(Methods.SET_MODE, this, mode), callback);
	}

	/** commit event */
	@Override
	public void commitEvent(MockBusObjectEvent event) {
		MockPsSummary.Event evt = (MockPsSummary.Event)event;
		if (evt.hasPcpus) {
			this.pcpus = evt.getPcpus();
		}
		if (evt.hasTaskTotal) {
			this.taskTotal = evt.getTaskTotal();
		}
		if (evt.hasTaskRunning) {
			this.taskRunning = evt.getTaskRunning();
		}
		if (evt.hasTaskSleeping) {
			this.taskSleeping = evt.getTaskSleeping();
		}
		if (evt.hasTaskStopped) {
			this.taskStopped = evt.getTaskStopped();
		}
		if (evt.hasTaskZombie) {
			this.taskZombie = evt.getTaskZombie();
		}
		if (evt.hasRefreshRate) {
			this.refreshRate = evt.getRefreshRate();
		}
		if (evt.hasMode) {
			this.mode = evt.getMode();
		}
		if (evt.hasMethodStateSetRefreshRate) {
			this.methodStateSetRefreshRate = evt.getMethodStateSetRefreshRate();
		}
		if (evt.hasMethodStateSetMode) {
			this.methodStateSetMode = evt.getMethodStateSetMode();
		}
	}
}
