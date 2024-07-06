/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

import MockServer from './utils/MockServer.js';
import test from './utils/routeTestUtils.js';
import networkRoutes from '../../src/routes/networkRoutes.js';
import { expect } from 'chai';
import sinon from 'sinon';
import tmp from 'tmp';
import fs from 'fs';

describe('network routes', () => {
	describe('get', () => {
		describe('network', () => {
			it('can retrieve network info', () => {
				// Act:
				const services = { config: { network: { name: 'foo', description: 'bar' } } };
				return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, services, routeContext => {
					// - invoke route synchronously
					routeContext.routeInvoker();

					// Assert:
					expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
					expect(routeContext.responses.length, 'single response is expected').to.equal(1);
					expect(routeContext.redirects.length, 'no redirects are expected').to.equal(0);

					// - no type information because formatting is completely bypassed
					const response = routeContext.responses[0];
					expect(response).to.deep.equal({
						name: services.config.network.name,
						description: services.config.network.description
					});
				});
			});

			it('skips network info not explicitly included', () => {
				// Act:
				const services = { config: { network: { name: 'foo', description: 'bar', head: 'fuu' } } };
				return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, services, routeContext => {
					// - invoke route synchronously
					routeContext.routeInvoker();

					const response = routeContext.responses[0];
					expect(response).to.deep.equal({
						name: services.config.network.name,
						description: services.config.network.description
					});
				});
			});
		});

		describe('network properties', () => {
			const assertCanRetrieveNetworkProperties = (lines, additionalExpectedSections = {}) => {
				// Arrange:
				const tempNetworkFile = tmp.fileSync();
				fs.writeFileSync(tempNetworkFile.name, lines.join('\n'));

				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } } });

				// Act:
				const route = mockServer.getRoute('/network/properties').get();
				return mockServer.callRoute(route).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						network: { identifier: 'testnet' },
						chain: { enableVerifiableState: true },
						plugins: { aggregate: { maxTransactionsPerAggregate: '1\'000' } },
						forkHeights: {
							bar: '987',
							foo: '123'
						},
						...additionalExpectedSections
					});
				});
			};

			const networkPropertiesLines = [
				'[network]',
				'identifier = testnet',
				'[chain]',
				'enableVerifiableState = true',
				'[plugin:catapult.plugins.aggregate]',
				'maxTransactionsPerAggregate = 1\'000',
				'[fork_heights]',
				'foo = 123',
				'bar = 987'
			];

			it('exposes configuration from known sections', () => assertCanRetrieveNetworkProperties(networkPropertiesLines));

			it('can parse treasury reissuance transaction signatures', () => assertCanRetrieveNetworkProperties(
				[].concat(
					networkPropertiesLines,
					[
						'[treasury_reissuance_transaction_signatures]',
						'3A785A34EA7FAB8AD7ED1B95EC0C0C1CC4097104DD3A47AB06E138D59DC48D75'
						+ '300996EDEF0C24641EE5EFFD83A3EFE10CE4CA41DAAF642342E988A0A0EA7FB6 = true',
						'401ECCE607FF9710A00B677A487D36B9B9B3B0DC6DF59DA0A2BD77603E80B82B'
						+ 'D0A82FE949055C5BB7A00F83AF4FF1242965CBF62C9D083344FF294D157259B2 = false',
						'395C2B37C7AABBEC3C08BD42DAF52D93D1BF003FF6A731E54F63003383EF1CE0'
						+ '302871ADD90DF04638DC617ACF2F5BB759C3DDC060E55A554477543210976C75 = true'
					]
				),
				{
					treasuryReissuanceTransactionSignatures: [ // sorted and filtered
						'395C2B37C7AABBEC3C08BD42DAF52D93D1BF003FF6A731E54F63003383EF1CE0'
						+ '302871ADD90DF04638DC617ACF2F5BB759C3DDC060E55A554477543210976C75',
						'3A785A34EA7FAB8AD7ED1B95EC0C0C1CC4097104DD3A47AB06E138D59DC48D75'
						+ '300996EDEF0C24641EE5EFFD83A3EFE10CE4CA41DAAF642342E988A0A0EA7FB6'
					]
				}
			));

			it('can parse corrupt aggregate transaction hashes', () => assertCanRetrieveNetworkProperties(
				[].concat(
					networkPropertiesLines,
					[
						'[corrupt_aggregate_transaction_hashes]',
						'8F5D7161352C39A7F179917851E66A2A9ED7675DD568B91F8314ABCEA654F368'
						+ ' = 18A842F09E7D9B23417EF83F27D341473DCAB1EECD653915C46DB7040590A25C',
						'9628FEB5BA4BC3716EDE29D7417E653CD7ACC8352D59EF0E1A061E61EB6F0953'
						+ ' = 52E56843BE40C9AC79DF2FBE15A11F9AA447076174E0D611C2010756D49D550E',
						'654A14F8D65FD23D3E5DC16D3CC1CA0B1CBC5B856987B5379A30B99114188E16'
						+ ' = 0EE76D5B0D09BAE81CC370CC6F231167041E20F93CE94D5C64D5813D1A541221'
					]
				),
				{
					corruptAggregateTransactionHashes: [ // sorted
						'654A14F8D65FD23D3E5DC16D3CC1CA0B1CBC5B856987B5379A30B99114188E16'
						+ ' = 0EE76D5B0D09BAE81CC370CC6F231167041E20F93CE94D5C64D5813D1A541221',
						'8F5D7161352C39A7F179917851E66A2A9ED7675DD568B91F8314ABCEA654F368'
						+ ' = 18A842F09E7D9B23417EF83F27D341473DCAB1EECD653915C46DB7040590A25C',
						'9628FEB5BA4BC3716EDE29D7417E653CD7ACC8352D59EF0E1A061E61EB6F0953'
						+ ' = 52E56843BE40C9AC79DF2FBE15A11F9AA447076174E0D611C2010756D49D550E'
					]
				}
			));

			it('hides configuration from other sections', () => assertCanRetrieveNetworkProperties([].concat(
				networkPropertiesLines,
				[
					// following section is not explicitly allowed, so will not show up in output
					'[private]',
					'secretCode = 42'
				]
			)));

			it('fails when file does not exist', () => {
				// Arrange:
				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: 'fake.dat' } } });

				// Act:
				const route = mockServer.getRoute('/network/properties').get();
				return mockServer.callRoute(route).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
					expect(mockServer.send.firstCall.args[0].message).to.equal('there was an error reading the network properties file');
				});
			});
		});

		describe('network inflation', () => {
			const assertFailsWithoutInflationProperties = (routePath, req) => {
				// Arrange:
				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { inflationPropertyFilePath: 'fake.dat' } } });

				// Act:
				const route = mockServer.getRoute(routePath).get();
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
					expect(mockServer.send.firstCall.args[0].message).to.equal('there was an error reading the inflation properties file');
				});
			};

			const assertCanRetrieveInflationProperties = lines => {
				// Arrange:
				const tempInflationFile = tmp.fileSync();
				fs.writeFileSync(tempInflationFile.name, lines.join('\n'));

				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, {
					config: { apiNode: { inflationPropertyFilePath: tempInflationFile.name } }
				});

				// Act:
				const route = mockServer.getRoute('/network/inflation').get();
				return mockServer.callRoute(route).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal([
						{ startHeight: '2', rewardAmount: '0' },
						{ startHeight: '5760', rewardAmount: '191997042' },
						{ startHeight: '172799', rewardAmount: '183764522' },
						{ startHeight: '435299', rewardAmount: '175884998' },
						{ startHeight: '697799', rewardAmount: '168343336' },
						{ startHeight: '960299', rewardAmount: '161125048' },
						{ startHeight: '1222799', rewardAmount: '154216270' },
						{ startHeight: '1485299', rewardAmount: '147603728' }
					]);
				});
			};

			it('returns inflation inflection points map', () => assertCanRetrieveInflationProperties([
				'[inflation]',
				'starting-at-height-2 = 0',
				'starting-at-height-5760 = 191997042',
				'starting-at-height-172799 = 183764522',
				'starting-at-height-435299 = 175884998',
				'starting-at-height-697799 = 168343336',
				'starting-at-height-960299 = 161125048',
				'starting-at-height-1222799 = 154216270',
				'starting-at-height-1485299 = 147603728'
			]));

			it('returns inflation inflection points map in sorted height order', () => assertCanRetrieveInflationProperties([
				'[inflation]',
				'starting-at-height-5760 = 191997042',
				'starting-at-height-2 = 0',
				'starting-at-height-435299 = 175884998',
				'starting-at-height-172799 = 183764522',
				'starting-at-height-697799 = 168343336',
				'starting-at-height-1222799 = 154216270',
				'starting-at-height-960299 = 161125048',
				'starting-at-height-1485299 = 147603728'
			]));

			it('fails when file does not exist', () => assertFailsWithoutInflationProperties('/network/inflation', {}));

			describe('network inflation at height', () => {
				const assertCanRetrievePointAtHeight = (queryHeight, expectedPoint) => {
					// Arrange:
					const tempInflationFile = tmp.fileSync();
					fs.writeFileSync(tempInflationFile.name, [
						'[inflation]',
						'starting-at-height-2 = 0',
						'starting-at-height-5760 = 191997042',
						'starting-at-height-172799 = 183764522',
						'starting-at-height-435299 = 175884998',
						'starting-at-height-697799 = 168343336',
						'starting-at-height-960299 = 161125048',
						'starting-at-height-1222799 = 154216270',
						'starting-at-height-1485299 = 147603728'
					].join('\n'));

					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, {}, {
						config: { apiNode: { inflationPropertyFilePath: tempInflationFile.name } }
					});

					// Act:
					const req = { params: { height: queryHeight } };
					const route = mockServer.getRoute('/network/inflation/at/:height').get();
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedPoint);
					});
				};

				const testCases = [
					['1', { startHeight: 'N/A', rewardAmount: '0' }, 'height is less than 1st inflection point'],
					['2', { startHeight: '2', rewardAmount: '0' }, 'height is 1st inflection point'],
					['10000', { startHeight: '5760', rewardAmount: '191997042' }, 'height does not match start height'],
					['172799', { startHeight: '172799', rewardAmount: '183764522' }, 'height matches start height'],
					['1222798', { startHeight: '960299', rewardAmount: '161125048' }, 'height is one less than 2nd last inflection point'],
					['1222799', { startHeight: '1222799', rewardAmount: '154216270' }, 'height is 2nd last inflection point'],
					['1322799', { startHeight: '1222799', rewardAmount: '154216270' }, 'height is less than last inflection point'],
					['1485299', { startHeight: '1485299', rewardAmount: '147603728' }, 'height is last inflection point'],
					['2485299', { startHeight: '1485299', rewardAmount: '147603728' }, 'height is greater than last inflection point']
				];

				testCases.forEach(testCase => {
					it(`succeeds when ${testCase[2]}`, () => assertCanRetrievePointAtHeight(testCase[0], testCase[1]));
				});

				it('fails when height parameter is malformed', () => {
					// Arrange:
					const tempInflationFile = tmp.fileSync();
					fs.writeFileSync(tempInflationFile.name, [
						'[inflation]',
						'starting-at-height-2 = 0'
					].join('\n'));

					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, {}, {
						config: { apiNode: { inflationPropertyFilePath: tempInflationFile.name } }
					});

					// Act:
					const req = { params: { height: '10x000' } };
					const route = mockServer.getRoute('/network/inflation/at/:height').get();
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
						expect(mockServer.send.firstCall.args[0].message)
							.to.equal('there was an error reading the inflation properties file');
					});
				});

				it('fails when file does not exist', () => assertFailsWithoutInflationProperties('/network/inflation/at/:height', {
					params: { height: '10000' }
				}));
			});
		});

		describe('network fees transaction', () => {
			const tempNodeFile = tmp.fileSync();
			fs.writeFileSync(tempNodeFile.name, [
				'[node]',
				'minFeeMultiplier = 1\'234\'567'
			].join('\n'));

			const tempNetworkFile = tmp.fileSync();
			fs.writeFileSync(tempNetworkFile.name, [
				'[chain]',
				'defaultDynamicFeeMultiplier = 1\'000'
			].join('\n'));

			const runNetworkFeesTest = (testName, feeMultipliers, average, median, max, min) => {
				const services = {
					config: {
						numBlocksTransactionFeeStats: feeMultipliers.length,
						apiNode: {
							nodePropertyFilePath: tempNodeFile.name,
							networkPropertyFilePath: tempNetworkFile.name
						}
					}
				};

				const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves(feeMultipliers);
				const db = {
					latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
				};

				it(`${testName}: [${feeMultipliers}] average:${average}, median:${median}, max:${max}, min:${min}`, () => {
					// Arrange:
					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, db, services);
					const route = mockServer.getRoute('/network/fees/transaction').get();

					// Act
					return mockServer.callRoute(route, {}).then(() => {
						// Assert
						expect(dbLatestBlocksFeeMultiplierFake.calledOnce).to.equal(true);
						expect(dbLatestBlocksFeeMultiplierFake.firstCall.args[0]).to.equal(feeMultipliers.length);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							averageFeeMultiplier: average,
							medianFeeMultiplier: median,
							highestFeeMultiplier: max,
							lowestFeeMultiplier: min,
							minFeeMultiplier: 1234567
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			};

			describe('network fees are computed correctly for the following values', () => {
				runNetworkFeesTest('One block', [0], 1000, 1000, 0, 0);
				runNetworkFeesTest('All 0s', [0, 0, 0, 0, 0], 1000, 1000, 0, 0);
				runNetworkFeesTest('With 0s', [10, 0, 0, 0, 10], 604, 1000, 10, 0);
				runNetworkFeesTest('Big values', [999999999, 999999999, 999999999, 999999999], 999999999, 999999999, 999999999, 999999999);
				runNetworkFeesTest('Correct average', [1, 1, 1, 1], 1, 1, 1, 1);
				runNetworkFeesTest('Correct median', [90, 92, 93, 88, 95, 88, 97, 87, 98], 92, 92, 98, 87);
				runNetworkFeesTest('Correct median even number', [35, 27, 31, 32, 30, 40, 29, 43], 33, 31, 43, 27);
				runNetworkFeesTest('Correct decimals floor', [10, 11, 12, 13], 11, 11, 13, 10);
				runNetworkFeesTest('Correct decimals floor', [23, 29, 31, 27], 27, 28, 31, 23);
			});
		});

		describe('network effective rental fees', () => {
			it('can retrieve network properties needed for rental fees', () => {
				const tempNetworkFile = tmp.fileSync();
				fs.writeFileSync(tempNetworkFile.name, [
					'[chain]',
					'maxDifficultyBlocks = 5',
					'defaultDynamicFeeMultiplier = 1\'000',
					'[plugin:catapult.plugins.namespace]',
					'rootNamespaceRentalFeePerBlock = 1\'000',
					'childNamespaceRentalFee = 100',
					'[plugin:catapult.plugins.mosaic]',
					'mosaicRentalFee = 500'
				].join('\n'));

				const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves([0, 1, 2, 3, 4]);
				const db = {
					latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
				};

				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, db, { config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } } });

				const route = mockServer.getRoute('/network/fees/rental').get();
				return mockServer.callRoute(route).then(() => {
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						effectiveChildNamespaceRentalFee: '300',
						effectiveMosaicRentalFee: '1500',
						effectiveRootNamespaceRentalFeePerBlock: '3000'
					});
				});
			});

			it('fails when file does not exist', () => {
				const mockServer = new MockServer();
				networkRoutes.register(mockServer.server, {}, { config: { apiNode: { networkPropertyFilePath: 'fake.dat' } } });

				const route = mockServer.getRoute('/network/fees/rental').get();
				return mockServer.callRoute(route).then(() => {
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0].statusCode).to.equal(409);
					expect(mockServer.send.firstCall.args[0].message).to.equal('there was an error reading the network properties file');
				});
			});

			const runNetworkEffectiveRentalFeesTest = (
				testName,
				maxDifficultyBlocks,
				defaultDynamicFeeMultiplier,
				rootNamespaceRentalFeePerBlock,
				childNamespaceRentalFee,
				mosaicRentalFee,
				feeMultipliers,
				effectiveRootNamespaceRentalFeePerBlock,
				effectiveChildNamespaceRentalFee,
				effectiveMosaicRentalFee
			) => {
				it(`${testName}: [${[feeMultipliers]}]`, () => {
					// Arrange:
					const tempNetworkFile = tmp.fileSync();
					fs.writeFileSync(tempNetworkFile.name, [
						'[chain]',
						`maxDifficultyBlocks = ${maxDifficultyBlocks}`,
						`defaultDynamicFeeMultiplier = ${defaultDynamicFeeMultiplier}`,
						'[plugin:catapult.plugins.namespace]',
						`rootNamespaceRentalFeePerBlock = ${rootNamespaceRentalFeePerBlock}`,
						`childNamespaceRentalFee = ${childNamespaceRentalFee}`,
						'[plugin:catapult.plugins.mosaic]',
						`mosaicRentalFee = ${mosaicRentalFee}`
					].join('\n'));

					const dbLatestBlocksFeeMultiplierFake = sinon.fake.resolves(feeMultipliers);
					const db = {
						latestBlocksFeeMultiplier: dbLatestBlocksFeeMultiplierFake
					};

					const mockServer = new MockServer();
					networkRoutes.register(mockServer.server, db, {
						config: { apiNode: { networkPropertyFilePath: tempNetworkFile.name } }
					});

					// Act:
					const route = mockServer.getRoute('/network/fees/rental').get();
					return mockServer.callRoute(route).then(() => {
						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							effectiveChildNamespaceRentalFee,
							effectiveMosaicRentalFee,
							effectiveRootNamespaceRentalFeePerBlock
						});
					});
				});
			};

			describe('network effective rental fees are computed correctly for the following values', () => {
				runNetworkEffectiveRentalFeesTest(
					'Simple all 1', 1,
					0, 1, 1, 1, [1],
					'1', '1', '1'
				);
				runNetworkEffectiveRentalFeesTest(
					'No need for default dynamic fee multiplier', 3,
					0, 1, 1, 1, [1, 2, 3],
					'2', '2', '2'
				);
				runNetworkEffectiveRentalFeesTest(
					'Default dynamic fee multiplier applied', 3,
					5, 1, 1, 1, [5, 0, 5],
					'5', '5', '5'
				);
				runNetworkEffectiveRentalFeesTest(
					'Standard case', 5,
					0, 1, 2, 3, [10, 10, 25, 50, 50],
					'25', '50', '75'
				);
				runNetworkEffectiveRentalFeesTest(
					'Decimals', 6,
					0, 1, 1, 1, [10, 10, 20, 29, 50, 50],
					'24', '24', '24'
				);
				runNetworkEffectiveRentalFeesTest(
					'Random case', 12,
					0, 1, 55, 28, [25, 32, 77, 9, 1, 50, 11, 4, 89, 56],
					'28', '1540', '784'
				);
				runNetworkEffectiveRentalFeesTest(
					'Big numbers', 3,
					0, '4294967295', '67632967295', '9007199254740993', [25, 32, 77],
					'137438953440', '2164254953440', '288230376151711776'
				);
			});
		});
	});
});
