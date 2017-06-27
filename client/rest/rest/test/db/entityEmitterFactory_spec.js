import { expect } from 'chai';
import EventEmitter from 'events';
import entityEmitterFactory from '../../src/db/entityEmitterFactory';

describe('entity emitter factory', () => {
	it('registers block listener', () => {
		// Act:
		const queries = [];
		entityEmitterFactory.createEntityEmitter(query => {
			queries.push(query);
			return Promise.resolve(new EventEmitter());
		})
		.then(() => {
			// Assert:
			expect(queries.length).to.equal(1);
			expect(queries[0]).to.deep.equal({ ns: 'catapult.blocks', op: 'i' });
		});
	});

	function createEntityEmitter(opEmitters) {
		return entityEmitterFactory.createEntityEmitter(query => {
			const opEmitter = new EventEmitter();
			opEmitters[query.ns] = opEmitter;
			return Promise.resolve(opEmitter);
		});
	}

	function assertNoEvents(emitter, eventName) {
		emitter.once(eventName, () => {
			// Assert: fail the test if the event was raised
			expect(true, `${eventName} event was unexpected`).to.equal(false);
		});
	}

	it('maps op event to block event', () => {
		// Arrange:
		const opEmitters = {};
		return createEntityEmitter(opEmitters)
			.then(entityEmitter => {
				assertNoEvents(entityEmitter, 'error');

				return new Promise(resolve => {
					// - set up a block listener
					entityEmitter.once('block', block => {
						// Assert: the op event was translated to a block event
						expect(block).to.deep.equal({ height: 7 });
						resolve();
					});

					// Act: raise an op event
					opEmitters['catapult.blocks'].emit('op', { ns: 'catapult.blocks', op: 'i', o: { height: 7 } });
				});
			});
	});

	it('forwards op error event', () => {
		// Arrange:
		const opEmitters = {};
		return createEntityEmitter(opEmitters)
			.then(entityEmitter => {
				assertNoEvents(entityEmitter, 'block');

				return new Promise(resolve => {
					// - set up an error listener
					entityEmitter.once('error', err => {
						// Assert: the op error event was forwarded
						expect(err.message).to.equal('op error');
						resolve();
					});

					// Act: raise an op error event
					opEmitters['catapult.blocks'].emit('error', new Error('op error'));
				});
			});
	});
});
