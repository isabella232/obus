/******************************************************************************
* @file IPsProcess.java
*
* @brief IPsProcess
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus;

import com.parrot.obus.MethodState;
import com.parrot.obus.IBusObject;
import com.parrot.obus.BusClient;
import com.parrot.obus.IBusObjectEvent;
import com.parrot.obus.IBusMethodCall;

/** Process object */
@SuppressWarnings("javadoc")
public interface IPsProcess extends IBusObject {
	/** Process state */
	public enum State {
		/** Unknown */
		UNKNOWN,
		/** Running */
		RUNNING,
		/** Sleeping */
		SLEEPING,
		/** Stopped */
		STOPPED,
		/** Zombie */
		ZOMBIE;
	}

	/** Event */
	public interface IEvent extends IBusObjectEvent {
		/** Event type */
		public static enum Type {
			/** Process info updated */
			UPDATED
		}

		/** Event type getter */
		@Override
		public Type getType();

		/** Property checkers */
		public boolean hasPid();
		public boolean hasPpid();
		public boolean hasName();
		public boolean hasExe();
		public boolean hasPcpu();
		public boolean hasState();

		/** Method state checkers */

		/** Property getters */
		public int getPid();
		public int getPpid();
		public String getName();
		public String getExe();
		public int getPcpu();
		public State getState();

		/** Method state getters */

	}

	/** Property getters */
	public int getPid();
	public int getPpid();
	public String getName();
	public String getExe();
	public int getPcpu();
	public State getState();

	/** Method state getters */

}
