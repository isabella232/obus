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

import java.nio.ByteBuffer;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.parrot.obus.ObusAddress;
import com.parrot.obus.internal.Protocol.Decoder;
import com.parrot.obus.internal.Protocol.RxPacket;
import com.parrot.obus.internal.Protocol.RxRawPacket;
import com.parrot.obus.internal.Protocol.TxPacket;

/**
 * Obus Transport for Inet client sockets.
 */
public class Transport {
	private static final Core.Logger _log = Core.getLogger("obus");

	public interface PacketHandler {
		public void onConnected();
		public void onDisconnected();
		public void recvPacket(RxPacket packet);
	}

	private static final int MSG_RX_CONNECTED = 0;
	private static final int MSG_RX_DISCONNECTED = 1;
	private static final int MSG_RX_PACKET = 2;
	private static final int MSG_TX_STOP = 3;
	private static final int MSG_TX_PACKET = 4;

	private Bus bus;
	private PacketHandler packetHandler;
	private boolean running;
	private Thread readThread;
	private Thread writeThread;
	private Handler readHandler;
	private Handler writeHandler;
	private ObusAddress socketAddress;
	private ObusSocket socket;

	/**
	 *
	 */
	public void start(Bus bus, PacketHandler packetHandler, ObusAddress addr) {
		/* Save parameters */
		this.bus = bus;
		this.packetHandler = packetHandler;
		this.socketAddress = addr;

		/* Handler to process Rx message in main thread context */
		this.readHandler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				if (Transport.this.packetHandler == null) {
					_log.warning("Rx packet lost");
					return;
				}
				switch (msg.what) {
				case MSG_RX_CONNECTED:
					Transport.this.packetHandler.onConnected();
					break;
				case MSG_RX_DISCONNECTED:
					Transport.this.packetHandler.onDisconnected();
					break;
				case MSG_RX_PACKET:
					/* Finish decoding in main thread (to synchronize with bus) */
					RxRawPacket rawPacket = (RxRawPacket)msg.obj;
					RxPacket packet = rawPacket.decode(Transport.this.bus);
					if (packet != null) {
						Transport.this.packetHandler.recvPacket(packet);
					}
					break;
				}
			}
		};

		/* Thread for Rx */
		this.readThread = new Thread(new Runnable() {
			@Override
			public void run() {
				_log.info("Reader thread: start");
				Transport.this.reader();
				_log.info("Reader thread: exit");
			}
		});

		/* Thread for Tx */
		this.writeThread = new Thread(new Runnable() {
			@Override
			public void run() {
				_log.info("Writer thread: start");
				Transport.this.writer();
				_log.info("Writer thread: exit");
			}
		});

		/* Start everything ! */
		this.running = true;
		this.readThread.start();
		this.writeThread.start();

		/* Make sure the write handler has been created by writer thread */
		synchronized (this) {
			while (this.writeHandler == null) {
				try {
					this.wait();
				} catch (InterruptedException e) {
				}
			}
		}
	}

	/**
	 *
	 */
	public void stop() {
		/* Ask everyone to stop */
		synchronized (this) {
			this.running = false;
			this.writeHandler.obtainMessage(MSG_TX_STOP).sendToTarget();
			if (this.socket != null) {
				this.socket.shutdown();
			}
		}

		/* Wait everyone */
		try {
			this.readThread.join();
			this.writeThread.join();
		} catch (InterruptedException e) {
		}

		/* Cleanup */
		this.bus = null;
		this.packetHandler = null;
		this.socketAddress = null;
		this.socket = null;
		this.readThread = null;
		this.writeThread = null;
		this.readHandler = null;
		this.writeHandler = null;
	}

	/**
	 *
	 */
	public void writePacket(TxPacket packet) {
		this.writeHandler.obtainMessage(MSG_TX_PACKET, packet).sendToTarget();
	}

	/**
	 *
	 */
	private void reader() {
		ByteBuffer buf = ByteBuffer.allocate(1024);
		while (true) {
			/* Open a socket */
			synchronized (this) {
				if (!this.running) {
					break;
				}
				this.socket = new ObusSocket();
			}

			/* Connect to address */
			if (this.socket.connect(this.socketAddress) && this.running) {
				/* Notify connection */
				this.readHandler.obtainMessage(MSG_RX_CONNECTED).sendToTarget();
				/* Read loop */
				Decoder decoder = new Decoder();
				while (true) {
					/* Need to clear buffer before reading */
					buf.clear();
					int count = this.socket.read(buf);
					if (count <= 0) {
						/* socket disconnected */
						break;
					}

					/* Setup buffer */
					buf.limit(count);
					buf.rewind();

					/* Decode data into raw packets and notify them */
					while (buf.hasRemaining()) {
						RxRawPacket rawPacket = decoder.decode(buf);
						if (rawPacket != null) {
							this.readHandler.obtainMessage(MSG_RX_PACKET, rawPacket).sendToTarget();
						}
					}
				}

				_log.info("socket disconnected");
				/* notify disconnection */
				this.readHandler.obtainMessage(MSG_RX_DISCONNECTED).sendToTarget();
			}

			/* Cleanup socket */
			synchronized (this) {
				if (this.socket != null) {
					this.socket.close();
					this.socket = null;
				}
			}

			/* Retry again in 1s */
			if (this.running) {
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
				}
			}
		}
	}

	/**
	 *
	 */
	private void writer() {
		/* Prepare a message loop and a handler */
		Looper.prepare();
		Handler handler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				switch (msg.what) {
				case MSG_TX_PACKET:
					/* Get packet to be sent */
					TxPacket packet = (TxPacket)msg.obj;
				
					/* Get socket channel, keep a local ref to avoid the
					    object to be destroyed. An AsynchronousCloseException
					    may occur though */
					ObusSocket socket = null;
					synchronized (Transport.this) {
						socket = Transport.this.socket;
					}

					/* Get buffer and rewind it before write */
					if (socket != null) {
						ByteBuffer buf = packet.getByteBuffer();
						buf.rewind();
						socket.write(buf);
					} else {
						_log.warning("Tx packet lost");
					}
					break;

				case MSG_TX_STOP:
					Looper.myLooper().quit();
					break;
				}
			}
		};

		/* Setup the write handler and wakeup start */
		synchronized (this) {
			this.writeHandler = handler;
			this.notify();
		}

		/* Go ! */
		Looper.loop();
	}
}
