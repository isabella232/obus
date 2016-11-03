/******************************************************************************
* @file MockPsProcess.java
*
* @brief MockPsProcess
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus.mock;

import com.parrot.obus.test.psbus.IPsProcess;
import com.parrot.obus.test.psbus.impl.PsProcess;
import com.parrot.obus.MethodState;
import com.parrot.obus.internal.Descriptor.ObjectDesc;
import com.parrot.obus.BusClient;
import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.mock.MockBusObject;
import com.parrot.obus.mock.MockBusObjectEvent;
import com.parrot.obus.mock.MockMethodCall;

/** Mock Object */
@SuppressWarnings("javadoc")
public class MockPsProcess extends MockBusObject implements IPsProcess {
	/** Event */
	public static final class Event extends MockBusObjectEvent implements IPsProcess.IEvent {

		private Type type;
		private int pid;
		private int ppid;
		private String name;
		private String exe;
		private int pcpu;
		private State state;


		private boolean hasPid;
		private boolean hasPpid;
		private boolean hasName;
		private boolean hasExe;
		private boolean hasPcpu;
		private boolean hasState;


		/** event constructor */
		public Event(MockPsProcess object, Type type) {
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
		public boolean hasPid() {
			return this.hasPid;
		}
		@Override
		public boolean hasPpid() {
			return this.hasPpid;
		}
		@Override
		public boolean hasName() {
			return this.hasName;
		}
		@Override
		public boolean hasExe() {
			return this.hasExe;
		}
		@Override
		public boolean hasPcpu() {
			return this.hasPcpu;
		}
		@Override
		public boolean hasState() {
			return this.hasState;
		}

		/** Method state checkers */

		/** Property getters */
		@Override
		public int getPid() {
			return this.pid;
		}
		@Override
		public int getPpid() {
			return this.ppid;
		}
		@Override
		public String getName() {
			return this.name;
		}
		@Override
		public String getExe() {
			return this.exe;
		}
		@Override
		public int getPcpu() {
			return this.pcpu;
		}
		@Override
		public State getState() {
			return this.state;
		}

		/** Method state getters */

		/** Property setters */
		public void setPid(int pid) {
			this.pid = pid;
			this.hasPid = true;
		}
		public void setPpid(int ppid) {
			this.ppid = ppid;
			this.hasPpid = true;
		}
		public void setName(String name) {
			this.name = name;
			this.hasName = true;
		}
		public void setExe(String exe) {
			this.exe = exe;
			this.hasExe = true;
		}
		public void setPcpu(int pcpu) {
			this.pcpu = pcpu;
			this.hasPcpu = true;
		}
		public void setState(State state) {
			this.state = state;
			this.hasState = true;
		}

		/** Method state setters */

	}

	/** Methods enum */
	public enum Methods {
		
	}

	private int pid;
	private int ppid;
	private String name;
	private String exe;
	private int pcpu;
	private State state;


	/** ObjDesc getters */
	@Override
	public ObjectDesc getDescriptor() {
		return PsProcess.objectDesc;
	}
	/** Property getters */
	@Override
	public int getPid() {
		return this.pid;
	}
	@Override
	public int getPpid() {
		return this.ppid;
	}
	@Override
	public String getName() {
		return this.name;
	}
	@Override
	public String getExe() {
		return this.exe;
	}
	@Override
	public int getPcpu() {
		return this.pcpu;
	}
	@Override
	public State getState() {
		return this.state;
	}

	/** Method state getters */

	/** Property setters */
	public void setPid(int pid) {
		this.pid = pid;
	}
	public void setPpid(int ppid) {
		this.ppid = ppid;
	}
	public void setName(String name) {
		this.name = name;
	}
	public void setExe(String exe) {
		this.exe = exe;
	}
	public void setPcpu(int pcpu) {
		this.pcpu = pcpu;
	}
	public void setState(State state) {
		this.state = state;
	}

	/** Method state setters */

	/** Method call */

	/** commit event */
	@Override
	public void commitEvent(MockBusObjectEvent event) {
		MockPsProcess.Event evt = (MockPsProcess.Event)event;
		if (evt.hasPid) {
			this.pid = evt.getPid();
		}
		if (evt.hasPpid) {
			this.ppid = evt.getPpid();
		}
		if (evt.hasName) {
			this.name = evt.getName();
		}
		if (evt.hasExe) {
			this.exe = evt.getExe();
		}
		if (evt.hasPcpu) {
			this.pcpu = evt.getPcpu();
		}
		if (evt.hasState) {
			this.state = evt.getState();
		}
	}
}
