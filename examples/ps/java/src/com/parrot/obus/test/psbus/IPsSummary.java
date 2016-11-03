/******************************************************************************
* @file IPsSummary.java
*
* @brief IPsSummary
*
* @author obusgen 1.0.3 generated file, do not modify it.
******************************************************************************/
package com.parrot.obus.test.psbus;

import com.parrot.obus.MethodState;
import com.parrot.obus.IBusObject;
import com.parrot.obus.BusClient;
import com.parrot.obus.IBusObjectEvent;
import com.parrot.obus.IBusMethodCall;

/** Global system information */
@SuppressWarnings("javadoc")
public interface IPsSummary extends IBusObject {
	/** PCPU mode */
	public enum Mode {
		/** PCPU of a process can never go above 100 event if SMP */
		SOLARIS,
		/** PCPU can go up to ncpus * 100 */
		IRIX;
	}

	/** Event */
	public interface IEvent extends IBusObjectEvent {
		/** Event type */
		public static enum Type {
			/** Process summary info updated */
			UPDATED,
			/** Refresh rate has been changed */
			REFRESH_RATE_CHANGED,
			/** Mode has been changed */
			MODE_CHANGED
		}

		/** Event type getter */
		@Override
		public Type getType();

		/** Property checkers */
		public boolean hasPcpus();
		public boolean hasTaskTotal();
		public boolean hasTaskRunning();
		public boolean hasTaskSleeping();
		public boolean hasTaskStopped();
		public boolean hasTaskZombie();
		public boolean hasRefreshRate();
		public boolean hasMode();

		/** Method state checkers */
		public boolean hasMethodStateSetRefreshRate();
		public boolean hasMethodStateSetMode();

		/** Property getters */
		public int[] getPcpus();
		public int getTaskTotal();
		public int getTaskRunning();
		public int getTaskSleeping();
		public int getTaskStopped();
		public int getTaskZombie();
		public int getRefreshRate();
		public Mode getMode();

		/** Method state getters */
		public MethodState getMethodStateSetRefreshRate();
		public MethodState getMethodStateSetMode();

	}

	/** Property getters */
	public int[] getPcpus();
	public int getTaskTotal();
	public int getTaskRunning();
	public int getTaskSleeping();
	public int getTaskStopped();
	public int getTaskZombie();
	public int getRefreshRate();
	public Mode getMode();

	/** Method state getters */
	public MethodState getMethodStateSetRefreshRate();
	public MethodState getMethodStateSetMode();

	/** Set the refresh rate */
	public void setRefreshRate(BusClient client, int refreshRate, IBusMethodCall.AckCb callback);

	/** Set the mode for PCPU */
	public void setMode(BusClient client, Mode mode, IBusMethodCall.AckCb callback);

}
