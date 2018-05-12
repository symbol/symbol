/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

// eslint no-new workaround
Vue.create = options => new Vue(options);

Vue.component('subscription-group-item', {
	template: '#subscription-group-item-template',
	props: ['group'],
	methods: {
		remove(subscription) {
			this.$emit('remove', subscription.index);
		}
	}
});

Vue.component('subscription-groups-control', {
	template: '#subscription-groups-control-template',
	props: ['groups'],
	methods: {
		remove(index) {
			this.$emit('remove', index);
		}
	}
});

Vue.component('text-or-select-control', {
	template: '#text-or-select-control-template',
	props: ['value', 'options'],
	methods: {
		updateValue(value) {
			this.$emit('input', value);
		}
	}
});

Vue.component('subscription-selection-control', {
	template: '#subscription-selection-control-template',
	data: () => ({
		allChannels: [
			'block',
			'confirmedAdded', 'unconfirmedAdded', 'unconfirmedRemoved', 'status',
			'partialAdded', 'partialRemoved', 'cosignature'
		],
		wellKnownAddresses: [
			{ name: 'host4', value: 'SAAAIBC7AM65HOFDLYGFUT46H44TROZ7MUWCW6MZ' },
			{ name: 'host5', value: 'SAAA57DREOPYKUFX4OG7IQXKITMBWKD6KXTVBBQP' },
			{ name: 'host6', value: 'SAAA467G4ZDNOEGLNXLGWUAXZKC6VAES74J7N34D' },
			{ name: 'host7', value: 'SAAAMZYSPE5TRAVH7I3VSF7ZD542EVDLB7JT7Z4K' },

			{ name: 'BloodyRookie', value: 'SAAAZY5C3L6ONXRAPH2WYAPC3FKYFIPBBPFMLAS4' },
			{ name: 'Jaguar0625', value: 'SDALCF7EXRBX2FH4ZJUKYA334MZJVB2TOAE5OKYG' },
			{ name: 'gimre', value: 'SAAA66EEZKK3HGBRV57E6TOK335NK22BF2KGOEDS' }
		],

		address: 'SAAAIBC7AM65HOFDLYGFUT46H44TROZ7MUWCW6MZ', // host4
		selectedChannels: ['block'],

		subscriptions: []
	}),
	computed: {
		channelInfos() {
			return this.allChannels.map(channel => ({
				name: channel, active: this.isSelected(channel), disabled: !this.canSubscribe(channel)
			}));
		},
		subscriptionGroups() {
			const subscriptionGroups = {};

			this.subscriptions.forEach((subscription, index) => {
				// move all that are not address-dependent into 'global' group
				const group = subscription.channel === subscription.fullChannelPath ? 'global' : subscription.address;
				if (!(group in subscriptionGroups))
					subscriptionGroups[group] = { name: group, subscriptions: [] };

				subscriptionGroups[group].subscriptions.push({ channel: subscription.channel, index });
			});

			return subscriptionGroups;
		}
	},
	methods: {
		fullChannelPath(channel) {
			return 'block' === channel ? 'block' : `${channel}/${this.address}`;
		},
		isSelected(channel) {
			return this.selectedChannels.includes(channel);
		},
		canSubscribe(channel) {
			const fullChannelPath = this.fullChannelPath(channel);
			return this.subscriptions.every(subscription => fullChannelPath !== subscription.fullChannelPath);
		},
		subscribe() {
			this.selectedChannels.forEach(channel => {
				if (!this.canSubscribe(channel))
					throw Error(`cannot subscribe to selected channel ${channel}`);

				const fullChannelPath = this.fullChannelPath(channel);
				this.subscriptions.push({ fullChannelPath, address: this.address, channel });
				this.$emit('message', 'subscribe', fullChannelPath);
			});

			// subscriptions have been made to all selected channels, so they can be unselected
			this.selectedChannels.splice(0, this.selectedChannels.length);
		},
		unsubscribe(index) {
			const subscription = this.subscriptions[index];
			this.subscriptions.splice(index, 1);
			this.$emit('message', 'unsubscribe', subscription.fullChannelPath);
		}
	}
});

Vue.component('web-socket-connection-control', {
	template: '#web-socket-connection-control-template',
	data: () => ({
		wellKnownHosts: [
			{ name: 'host1', value: 'set-proper-hostname' },
			{ name: 'host2', value: 'set-proper-hostname' },
			{ name: 'host3', value: 'set-proper-hostname' },
			{ name: 'host4', value: 'set-proper-hostname' },
			{ name: 'Localhost', value: 'localhost' }
		],

		canConnect: true,

		host: 'set-proper-hostname',

		uid: '',
		wsConnection: undefined
	}),
	computed: {
		wsHost() {
			return `ws://${this.host}:3000/ws`;
		}
	},
	methods: {
		connect() {
			this.canConnect = false;

			this.wsConnection = new WebSocket(this.wsHost);
			this.wsConnection.onopen = () => {
				this.raiseStatus({ level: 'info', type: 'websocket', message: 'connected' });
			};
			this.wsConnection.onmessage = e => {
				const json = JSON.parse(e.data);
				if ('uid' in json) {
					this.uid = json.uid;
					this.raiseStatus({ level: 'info', type: 'websocket', message: `negotiated uid ${this.uid}` });
				} else {
					this.raiseStatus({ level: 'info', type: 'message', message: json });
				}
			};
			this.wsConnection.onerror = () => {
				this.raiseStatus({ level: 'danger', type: 'websocket', message: 'encountered error' });
			};
			this.wsConnection.onclose = e => {
				this.raiseStatus({ level: 'warning', type: 'websocket', message: `closed with code ${e.code}` });
				this.reset();
			};
		},
		disconnect() {
			this.wsConnection.close();
		},
		sendMessage(action, channel) {
			this.raiseStatus({ level: 'light', type: 'subscription', message: { action, channel } });
			this.wsConnection.send(JSON.stringify({ uid: this.uid, [action]: channel }));
		},
		reset() {
			this.uid = '';
			this.canConnect = true;
		},
		raiseStatus(data) {
			// augment with host
			data.host = this.wsHost;
			this.$emit('message', data);
		}
	}
});

Vue.component('status-messages-panel', {
	template: '#status-messages-panel-template',
	props: ['messages']
});

const formatTimestamp = date => {
	const formatUnit = (unit, len = 2) => date[`get${unit}`]().toString().padStart(len, '0');
	return `${formatUnit('Hours')}:${formatUnit('Minutes')}:${formatUnit('Seconds')}.${formatUnit('Milliseconds', 3)}`;
};

const formatUint64 = value => bigInt(value[1]).shiftLeft(32).add(value[0]).toString();

Vue.component('web-socket-control', {
	template: '#web-socket-control-template',
	props: ['name'],
	data: () => ({
		messages: []
	}),
	methods: {
		addMessage(data) {
			let formattedData = {};

			if ('subscription' === data.type) {
				const message = `sending ${data.message.action} message for channel ${data.message.channel.replace('/', ' / ')}`;
				formattedData = { message };
			} else if ('message' === data.type) {
				if ('block' in data.message) {
					const message = `block received with height ${formatUint64(data.message.block.height)}`;
					formattedData = { level: 'light', message };
				} else {
					const message = `received unknown data with key(s): ${Object.keys(data.message)}`;
					formattedData = { level: 'warning', message };
				}
			}

			const timestamp = formatTimestamp(new Date());
			this.messages.unshift(Object.assign({ timestamp }, data, formattedData));

			const maxMessages = 100;
			if (maxMessages < this.messages.length)
				this.messages.splice(maxMessages, this.messages.length - maxMessages);
		}
	}
});

Vue.create({
	el: '#app'
});
