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

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.parrot.obus.BusClient;
import com.parrot.obus.BusEventsNotifier;
import com.parrot.obus.IBusMethodCall;
import com.parrot.obus.IBusObject;
import com.parrot.obus.MethodCallAck;
import com.parrot.obus.ObusAddress;
import com.parrot.obus.internal.Descriptor.BusDesc;
import com.parrot.obus.internal.Protocol.RxPacket;
import com.parrot.obus.internal.Protocol.RxPacketAck;
import com.parrot.obus.internal.Protocol.RxPacketAdd;
import com.parrot.obus.internal.Protocol.RxPacketBusEvent;
import com.parrot.obus.internal.Protocol.RxPacketConnResp;
import com.parrot.obus.internal.Protocol.RxPacketEvent;
import com.parrot.obus.internal.Protocol.RxPacketRemove;
import com.parrot.obus.internal.Protocol.TxPacket;
import com.parrot.obus.internal.Protocol.TxPacketCall;
import com.parrot.obus.internal.Protocol.TxPacketConnReq;
import com.parrot.obus.internal.Transport.PacketHandler;

/**
 * Obus client interface.
 */
public class Client extends BusClient {
	private static final Core.Logger _log = Core.getLogger("obus");

	/** Connection state */
	private static enum State {
		IDLE, CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED, REFUSED
	}

	/**
	 *
	 */
	private final String name;   /**< Client name */
	private final Bus bus;       /**< Client bus */
	private State state;         /**< Client connection status */
	private Transport transport; /**< Transport layer */
	private int handleCall;      /**< Next call handle */
	private final Map<Integer, ObusCall> pendingCalls; /**< Pending method calls */

	/**
	 * Constructor.
	 * @param name : name of this client. It will be used in logging in server.
	 * @param busDesc : bus descriptor for this client.
	 * @param busEventNotifier
	 */
	public Client(String name, BusDesc busDesc, BusEventsNotifier<?> busEventNotifier) {
		this.name = name;
		this.bus = new Bus(busDesc);
		this.state = State.IDLE;
		this.transport = null;
		this.handleCall = 1;
		this.pendingCalls = new HashMap<Integer, ObusCall>();
		this.setBusEventNotifier(busEventNotifier);
	}

	/**
	 * Start the client.
	 * @param addr : address to connect to.
	 */
	@Override
	public void start(ObusAddress addr) {
		if (this.state != State.IDLE) {
			throw new IllegalStateException();
		}
		this.state = State.CONNECTING;
		this.transport = new Transport();
		this.transport.start(this.bus, this.packetHandler, addr);
	}

	/**
	 * Stop the client.
	 */
	@Override
	public void stop() {
		if (this.state != State.IDLE) {
			this.transport.stop();
			this.disconnect();
			this.state = State.IDLE;
		}
	}

	/**
	 * Determine if the client is connected (in obus way, not only socket).
	 * @return true if socket is connected AND protocol is established.
	 */
	@Override
	public boolean isConnected() {
		return (this.state == State.CONNECTED);
	}

	/**
	 * Call bus method.
	 * 
	 * @param methodCall the method call instance to execute
	 * @param callback the callback to call when the method has been sent to the server
	 */
	@Override
	public void callBusMethod(IBusMethodCall methodCall, IBusMethodCall.AckCb callback) {
		callBusMethod((ObusCall)methodCall, callback);
	}

	/**
	 * Get an object by its Handle.
	 * 
	 * @param handle the handle of the requested object
	 * @return The object with the requested handle or null if there not found
	 */
	@Override
	public IBusObject getObject(int handle) {
		return this.bus.findObject(handle);
	}


	/**
	 *
	 */
	private int callBusMethod(ObusCall call, IBusMethodCall.AckCb callback) {
		/* Setup handle for call */
		this.handleCall++;
		if (this.handleCall >= 65536) {
			this.handleCall = 1;
		}
		call.handle = this.handleCall;
		this.pendingCalls.put(call.handle, call);

		/* put the CB into the call object */
		call.callback = callback;

		if (this.mLogCalls) {
			_log.info("Calling " + call.toString());
		}

		/* Create packet and send it */
		TxPacket packet = new TxPacketCall(call);
		this.transport.writePacket(packet);
		return call.handle;
	}

	/**
	 * Dispatch content of a bus event
	 */
	private void dispatchBusEvent(ObusBusEvent busEvt) {
		if (!busEvt.dispatched) {

			if (this.mLogEvents) {
				_log.info("dispatching busevent: " + busEvt.toString());
			}

			busEvt.dispatched = true;

			Enum<?> busEventType = busEvt.getType();

			/* Notify the bus event before dispatching it */
			notifyBusEvent(busEvt.getType(), false);

			/* First add all registered objects to the bus */
			for (ObusObject obj: busEvt.registerList) {
				/* register the object to the bus */
				registerObject(obj, busEvt);
			}

			/* then dispatch objects to the corresponding registry */
			for (ObusObject obj: busEvt.registerList) {
				addObjectToRegistry(obj, busEventType);
			}
			
			/* Dispatch events before committing them */
			for (ObusEvent evt: busEvt.eventList) {
				notifyObjectEvent(evt, busEventType, false);
			}

			/* Commit all events*/
			for (ObusEvent evt: busEvt.eventList) {
				if (this.mLogEvents) {
					_log.info("event: " + evt.toString());
				}
				evt.commit();
			}

			/* Dispatch events after committing them*/
			for (ObusEvent evt: busEvt.eventList) {
				notifyObjectEvent(evt, busEventType, true);
			}

			/* First dispatch removed objects to the corresponding registry */
			for (ObusObject obj: busEvt.unregisterList) {
				removeObjectFromRegistry(obj, busEventType);
			}

			/* then remove objects from the bus */
			for (ObusObject obj: busEvt.unregisterList) {
				unregisterObject(obj, busEvt);
			}

			/* Notify the bus event after all events have been committed */
			notifyBusEvent(busEvt.getType(), true);

			if (this.mLogEvents) {
				_log.info("dispatched busevent: " + busEvt.toString());
			}

		}
	}

	/**
	 * Register an object.
	 * 
	 * @param obj the obj
	 * @param busEvt the bus evt
	 */
	private void registerObject(ObusObject obj, ObusBusEvent busEvt) {
		try {
			if (this.mLogObjects) {
				_log.info("add object:" + obj.toString());
			}

			/* Register object in bus */
			this.bus.registerObject(obj);

			/* add object to the client registry */
			Enum<?> busEventType;
			if (busEvt != null) {
				busEventType = busEvt.getType();
			} else {
				busEventType = null;
			}
		} catch (IllegalArgumentException e) {
			_log.error(e.toString());
		}
	}

	/**
	 * Unregister an object.
	 * 
	 * @param obj the obj
	 * @param busEvt the bus evt
	 */
	private void unregisterObject(ObusObject obj, ObusBusEvent busEvt) {
		try {
			if (this.mLogObjects) {
				_log.info("remove object:" + obj.toString());
			}

			/* remove the object from the client registry */
			Enum<?> busEventType;
			if (busEvt != null) {
				busEventType = busEvt.getType();
			} else {
				busEventType = null;
			}

			/* Unregister object from bus */
			this.removePendingCalls(obj);
			this.bus.unregisterObject(obj);
		} catch (IllegalArgumentException e) {
			_log.error(e.toString());
		}
	}


	/**
	 *
	 */
	private void disconnect() {
		State oldState = this.state;
		this.state = State.DISCONNECTING;
		_log.info("disconnect socket client " + this.name);

		/* Notify client of connection lost */
		if (oldState == State.CONNECTED) {
			ObusBusEvent busEvt = ObusBusEvent.create(this.bus, Core.BUSEVT_DISCONNECTED_UID);
			if (busEvt != null) {
				busEvt.unregisterList.addAll(this.bus.getAllObjects());
				this.dispatchBusEvent(busEvt);
			} else {
				_log.error("Unable to create event BUSEVT_DISCONNECTED_UID event");
			}
		}

		/* Reconnection always enabled */
		this.state = State.CONNECTING;
	}

	/**
	 *
	 */
	private void removePendingCalls(ObusObject obj) {
		Iterator<ObusCall> it = this.pendingCalls.values().iterator();
		while (it.hasNext()) {
			ObusCall call = it.next();
			if (call.obj == obj) {
				_log.warning("Object " + obj.uid + ":" + obj.handle +
						" removed with pending call handle=" + call.handle);
				/* Send a fake ack to user */
				call.ack = MethodCallAck.ABORTED;

				if (call.callback != null) {
					call.callback.onAck(call.ack);
				}
				it.remove();
			}
		}
	}

	/**
	 *
	 */
	private void sendConnectionRequest() {
		/* Create a TxPacketConnReq packet and send it */
		TxPacket packet = new TxPacketConnReq(this.name,
				this.bus.getDesc().name, this.bus.getDesc().crc);
		this.transport.writePacket(packet);
	}

	/**
	 *
	 */
	private void recvConnectionResponse(RxPacketConnResp packet) {
		_log.info("server connection response: " + packet.status +
				" (" + packet.objects.size() + " objects)");

		/* Check connection state */
		if (this.state != State.CONNECTING) {
			_log.warning("ignoring connection response in " +
					this.state + " state");
			return;
		}

		/* Handle connection refused */
		if (packet.status != Protocol.ConnRespStatus.ACCEPTED) {
			this.state = State.REFUSED;
			ObusBusEvent busEvt = ObusBusEvent.create(this.bus, Core.BUSEVT_CONNECTION_REFUSED_UID);
			if (busEvt != null) {
				this.dispatchBusEvent(busEvt);
			} else {
				_log.error("Unable to create event BUSEVT_CONNECTION_REFUSED_UID event");
			}
			return;
		}

		/* Connection succeed */
		this.state = State.CONNECTED;
		ObusBusEvent busEvt = ObusBusEvent.create(this.bus, Core.BUSEVT_CONNECTED_UID);
		if (busEvt != null) {
			busEvt.registerList.addAll(packet.objects);
			this.dispatchBusEvent(busEvt);
		} else {
			_log.error("Unable to create event BUSEVT_CONNECTED_UID event");
		}
	}

	/**
	 * Handle a Add packet. This type of packet contains a single object to register
	 */
	private void recvAdd(RxPacketAdd packet) {
		if (this.mLogObjects) {
			_log.info("add: " + packet.obj.toString());
		}
		registerObject(packet.obj, null); // no associated bus event
		addObjectToRegistry(packet.obj, null);  // no associated bus event
	}

	/**
	 * handle a Remove packet. This type of packet contains a single object to unregister
	 */
	private void recvRemove(RxPacketRemove packet) {
		if (this.mLogObjects) {
			_log.info("remove: " + packet.obj.toString());
		}
		unregisterObject(packet.obj, null); // no associated bus event
		removeObjectFromRegistry(packet.obj, null); // no associated bus event
	}

	/**
	 * Recv bus event.
	 * 
	 * @param packet the packet
	 */
	private void recvBusEvent(RxPacketBusEvent packet) {
		this.dispatchBusEvent(packet.busEvt);
	}

	/**
	 *
	 */
	private void recvEvent(RxPacketEvent packet) {
		ObusEvent evt = packet.evt;
		/* Notify the event before committing it */
		this.notifyObjectEvent(evt, null, false); // no associated bus event

		if (this.mLogEvents) {
			_log.info("event: " + evt.toString());
		}
		/* Commit event  */
		packet.evt.commit();

		/* Notify the event after committing it */
		this.notifyObjectEvent(evt, null, true); // no associated bus event
	}

	/**
	 *
	 */
	private void recvAck(RxPacketAck packet) {
		/* Retrieve pending call */
		ObusCall call =  this.pendingCalls.get(packet.handle);
		if (call == null) {
			_log.warning("Received ack for not pending call handle=" +
					packet.handle);
		} else {
			/* Call object callback ack */
			call.ack = packet.ack;

			if (this.mLogCalls) {
				_log.info("Call Ack " + call.toString());
			}

			if (call.callback != null) {
				call.callback.onAck(call.ack);
			}
			this.pendingCalls.remove(call.handle);
		}
	}

	/**
	 *
	 */
	private final PacketHandler packetHandler = new PacketHandler() {

		@Override
		public void onConnected() {
			_log.info("client " + Client.this.name + " socket connected");
			/* Send bus connection request */
			Client.this.sendConnectionRequest();
		}

		@Override
		public void onDisconnected() {
			_log.info("client " + Client.this.name + " socket disconnected");
			Client.this.disconnect();
		}

		@Override
		public void recvPacket(RxPacket packet) {
			switch (packet.header.packetType) {
			case CONRESP:
				Client.this.recvConnectionResponse((RxPacketConnResp)packet);
				break;
			case ADD:
				Client.this.recvAdd((RxPacketAdd)packet);
				break;
			case REMOVE:
				Client.this.recvRemove((RxPacketRemove)packet);
				break;
			case BUS_EVENT:
				Client.this.recvBusEvent((RxPacketBusEvent)packet);
				break;
			case EVENT:
				Client.this.recvEvent((RxPacketEvent)packet);
				break;
			case ACK:
				Client.this.recvAck((RxPacketAck)packet);
				break;
			default:
				_log.warning("Client: unhandled packet type: " +
						packet.header.packetType);
			}
		}
	};

	/* (non-Javadoc)
	 * @see com.parrot.obus.BusClient#dump(java.io.PrintWriter)
	 */
	@Override
	public void dump(PrintWriter writer) {
		writer.write("Bus: " + this.state + "\n");
		for (Object obj : this.bus.getAllObjects()) {
			writer.write(obj.toString() + "\n");
		}
	}
}
