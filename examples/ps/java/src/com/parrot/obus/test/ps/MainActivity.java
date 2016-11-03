/**
 * @author Yves-Marie Morgan
 *
 * Copyright (C) 2013 Parrot S.A.
 */

package com.parrot.obus.test.ps;

import java.net.InetSocketAddress;
import java.util.Comparator;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.parrot.obus.BusClient;
import com.parrot.obus.BusEventsNotifier;
import com.parrot.obus.BusObjectEventsNotifier;
import com.parrot.obus.BusObjectsRegistry;
import com.parrot.obus.BusObjectsRegistry.IKeyExtractor;
import com.parrot.obus.ObusAddress;
import com.parrot.obus.test.psbus.IPsBusEvent;
import com.parrot.obus.test.psbus.IPsBusEvent.Type;
import com.parrot.obus.test.psbus.IPsProcess;
import com.parrot.obus.test.psbus.IPsSummary;
import com.parrot.obus.test.psbus.IPsSummary.IEvent;
import com.parrot.obus.test.psbus.impl.PsBus;
import com.parrot.obus.test.psbus.impl.PsProcess;
import com.parrot.obus.test.psbus.impl.PsSummary;

/**
 * Example activity that display the process list
 */
public class MainActivity extends Activity {

	private static final String TAG = "obusapp";

	private TextView mBusStateView;
	private TextView mSummaryView;
	private ListView mProcessListView;
	private ArrayAdapter<MyListItem> mProcessListAdapter;
	private Map<IPsProcess, MyListItem> mProcessMap;
	private BusConnection mBusConnection;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Log.i(TAG, "onCreate");
		super.onCreate(savedInstanceState);

		/* Setup controls */
		setContentView(R.layout.activity_main);
		mBusStateView = (TextView) findViewById(R.id.textViewState);
		mSummaryView = (TextView) findViewById(R.id.textViewSummary);
		mProcessListView = (ListView) findViewById(R.id.listView1);

		/* Initial state : disconnected */
		mBusStateView.setText("Disconnected");
		mSummaryView.setText("");

		mProcessListAdapter = new ArrayAdapter<MyListItem>(this, android.R.layout.simple_list_item_activated_1);
		mProcessListView.setAdapter(mProcessListAdapter);
		mProcessMap = new HashMap<IPsProcess, MyListItem>();

		mProcessListView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
				Log.d(TAG, "onItemClick");
			}
		});

		// Create the bus connection
		mBusConnection = new BusConnection();

		// BusEvent listener, before committing the bus event
		mBusConnection.mBusEventNotifier.registerEventListener(new BusEventsNotifier.Listener<IPsBusEvent.Type>(BusEventsNotifier.PRE_COMMIT) {
			@Override
			public void onEvent(Type eventType) {
				switch (eventType) {
				case CONNECTED:
					mBusStateView.setText("Connected");
					break;
				case DISCONNECTED:
					mBusStateView.setText("Disconnected");
					break;
				case CONNECTION_REFUSED:
					mBusStateView.setText("Connection refused");
					break;
				default:
					//nothing to do
				}

				// stop updating UI before handling the bus event
				mProcessListAdapter.setNotifyOnChange(false);
			}
		});

		// BusEvent listener, before committing the bus event
		mBusConnection.mBusEventNotifier.registerEventListener(new BusEventsNotifier.Listener<IPsBusEvent.Type>(BusEventsNotifier.POST_COMMIT) {
			@Override
			public void onEvent(Type eventType) {
				// sort the list and update the UI
				mProcessListAdapter.sort(MyListItem.comparator);
				mProcessListAdapter.notifyDataSetChanged();
			}
		});

		// Process objects registry listener
		mBusConnection.mPsProcessRegistry.registerObserver(new BusObjectsRegistry.IObserver<IPsProcess, IPsBusEvent.Type>() {
			@Override
			public void onObjectAdded(IPsProcess process, Type busEventType) {
				MyListItem item = new MyListItem(process);
				mProcessMap.put(process, item);
				mProcessListAdapter.add(item);
				mProcessListAdapter.sort(MyListItem.comparator);
			}

			@Override
			public void onObjectRemoved(IPsProcess process, Type busEventType) {
				MyListItem item = mProcessMap.get(process);
				mProcessListAdapter.remove(item);
				mProcessListAdapter.sort(MyListItem.comparator);
			}
		});

		// Process objects event listener
		mBusConnection.mPsProcessNotifier.registerEventListener(
				new BusObjectEventsNotifier.Listener<IPsProcess, IPsProcess.IEvent, IPsBusEvent.Type>(
						EnumSet.of(IPsProcess.IEvent.Type.UPDATED), BusObjectEventsNotifier.POST_COMMIT) {
					@Override
					public void onEvent(IPsProcess process, IPsProcess.IEvent event, Type busEventType) {
						if (event.hasPcpu()) {
							mProcessListAdapter.sort(MyListItem.comparator);
						}

						// notify view data changed only if not inside a bus event
						if (busEventType == null)
							mProcessListAdapter.notifyDataSetChanged();
					}
				});


		// Summary objects registry listener
		mBusConnection.mPsSummaryRegistry.registerObserver(new BusObjectsRegistry.IObserver<IPsSummary, IPsBusEvent.Type>() {
			@Override
			public void onObjectAdded(IPsSummary summary, Type busEventType) {
				updateSummary(summary);
			}

			@Override
			public void onObjectRemoved(IPsSummary summary, Type busEventType) {
				updateSummary(null);
			}
		});

		// Summary objects event listener
		mBusConnection.mPsSummaryNotifier.registerEventListener(
				new BusObjectEventsNotifier.Listener<IPsSummary, IPsSummary.IEvent, IPsBusEvent.Type>(
						EnumSet.of(IPsSummary.IEvent.Type.UPDATED), BusObjectEventsNotifier.POST_COMMIT) {
					@Override
					public void onEvent(IPsSummary summary, IEvent event, Type busEventType) {
						updateSummary(summary);
					}

				});

		// Start the bus
		mBusConnection.start();
	}

	/** */
	@Override
	protected void onDestroy() {
		Log.i(TAG, "onDestroy");
		mBusConnection.stop();
		super.onDestroy();
	}

	/**
	 *
	 */
	private void updateSummary(IPsSummary summary) {
		if (summary == null) {
			mSummaryView.setText("");
		} else {
			StringBuilder sb = new StringBuilder();
			int[] pcpus = summary.getPcpus();
			for (int i = 0; i < pcpus.length; i++) {
				sb.append(" %cpu");
				sb.append(i + 1);
				sb.append("=");
				sb.append(pcpus[i]);
			}
			sb.append("\ntotal=");
			sb.append(summary.getTaskTotal());
			sb.append(" running=");
			sb.append(summary.getTaskRunning());
			sb.append(" sleeping=");
			sb.append(summary.getTaskSleeping());
			sb.append(" stopped=");
			sb.append(summary.getTaskStopped());
			sb.append(" zombie=");
			sb.append(summary.getTaskZombie());
			mSummaryView.setText(sb.toString());
		}
	}

	/**
	 *
	 */
	private static class MyListItem {
		private final IPsProcess process;

		public MyListItem(IPsProcess process) {
			this.process = process;
		}

		@Override
		public String toString() {
			String name = this.process.getExe();
			if (name.length() == 0) {
				name = this.process.getExe();
			}
			return "pid=" + this.process.getPid() + " %cpu=" + this.process.getPcpu() + " state=" + this.process.getState() + "\n"
			+ name;
		}

		public static final Comparator<MyListItem> comparator = new Comparator<MyListItem>() {
			@Override
			public int compare(MyListItem arg0, MyListItem arg1) {
				int pcpu0 = arg0.process.getPcpu();
				int pcpu1 = arg1.process.getPcpu();
				return (pcpu1 > pcpu0) ? +1 : (pcpu1 < pcpu0) ? -1 : 0;
			}
		};
	}

	/**
	 * Class handling bus instantiation and connection
	 */
	private class BusConnection {
		/** Bus client */
		private BusClient mBusClient;
		/** Bus event notifier */
		private BusEventsNotifier<IPsBusEvent.Type> mBusEventNotifier;
		/** PsProcess registry, key is the process pid */
		private BusObjectsRegistry<IPsProcess, IPsBusEvent.Type, Integer> mPsProcessRegistry;
		/** PsProcess notifier */
		private BusObjectEventsNotifier<IPsProcess, IPsProcess.IEvent, IPsBusEvent.Type> mPsProcessNotifier;
		/** PsSummary registry, as a singleton */
		private BusObjectsRegistry<IPsSummary, IPsBusEvent.Type, Void> mPsSummaryRegistry;
		/** PsSummary Notifier */
		private BusObjectEventsNotifier<IPsSummary, IPsSummary.IEvent, IPsBusEvent.Type> mPsSummaryNotifier;

		public BusConnection() {
			// Bus event notifier
			mBusEventNotifier = new BusEventsNotifier<IPsBusEvent.Type>();

			// Create obus client
			mBusClient = BusClient.create(getApplication().getApplicationInfo().packageName, PsBus.busDesc, mBusEventNotifier);

			// psProcess Key extractor
			IKeyExtractor<IPsProcess, Integer> psProcessKeyExtractor = new BusObjectsRegistry.IKeyExtractor<IPsProcess, Integer>() {
				@Override
				public Integer getKey(IPsProcess object) {
					return object.getPid();
				}
			};

			// Ps Process registry, key is the Pid
			mPsProcessRegistry = new BusObjectsRegistry<IPsProcess, IPsBusEvent.Type, Integer>(mBusClient, PsProcess.objectDesc, psProcessKeyExtractor);
			// Ps Process notifier
			mPsProcessNotifier = new BusObjectEventsNotifier<IPsProcess, IPsProcess.IEvent, IPsBusEvent.Type>(mBusClient, PsProcess.objectDesc);
			// Ps Summary registry, singleton (i.e. no key)
			mPsSummaryRegistry = new BusObjectsRegistry<IPsSummary, IPsBusEvent.Type, Void>(mBusClient, PsSummary.objectDesc, null);
			// Ps Summary notifier
			mPsSummaryNotifier = new BusObjectEventsNotifier<IPsSummary, IPsSummary.IEvent, IPsBusEvent.Type>(mBusClient, PsSummary.objectDesc);
		}

		public void start() {
			mBusClient.start(new ObusAddress(new InetSocketAddress("192.168.0.1", 58001)));
		}

		public void stop() {
			mBusClient.stop();
		}
	}
}
