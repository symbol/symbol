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
const MerkleTree = require('../../src/routes/MerkelTree');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const fs = require('fs');
const fsPath = require('path');

describe('MerkleTree', () => {
	describe('merkle tree parse', () => {
		let merkleTree = new MerkleTree();
		it('getBitsFromMask', () => {
			let bits = merkleTree.getBitsFromMask(catapult.utils.convert.hexToUint8('0C46'));
			expect(bits.length).to.equal(5);
			expect(bits.join('')).to.equal('239AE');

			bits = merkleTree.getBitsFromMask(catapult.utils.convert.hexToUint8('8062'));
			expect(bits.length).to.equal(4);
			expect(bits.join('')).to.equal('79DE');

			bits = merkleTree.getBitsFromMask(catapult.utils.convert.hexToUint8('1002'));
			expect(bits.length).to.equal(2);
			expect(bits.join('')).to.equal('49');
		});

		it('getPathLength', () => {
			let pathLength = merkleTree.getPathLength(0);
			expect(pathLength).to.equal(0);

			pathLength = merkleTree.getPathLength(3);
			expect(pathLength).to.equal(2);

			pathLength = merkleTree.getPathLength(9);
			expect(pathLength).to.equal(5);
		});

		it('isBranch', () => {
			expect(merkleTree.isBranch(0)).to.equal(true);
			expect(merkleTree.isBranch(1)).to.equal(false);
		});

		it('isLeaf', () => {
			expect(merkleTree.isLeaf(255)).to.equal(true);
			expect(merkleTree.isLeaf(1)).to.equal(false);
		});

		it('nibbleAt', () => {
			const path = catapult.utils.convert.hexToUint8('B7BB382C56');
			expect(merkleTree.nibbleAt(path, 0)).to.equal(11);
			expect(merkleTree.nibbleAt(path, 5)).to.equal(8);
			expect(merkleTree.nibbleAt(path, 10)).to.equal(0);
		});

		it('encodePath', () => {
			let path = catapult.utils.convert.hexToUint8('B7BB382C56');
			expect(catapult.utils.convert.uint8ToHex(merkleTree.encodePath(path, 10, false))).to.equal('00B7BB382C56');
			path = catapult.utils.convert.hexToUint8('3DC610D300');
			expect(catapult.utils.convert.uint8ToHex(merkleTree.encodePath(path, 9, false))).to.equal('13DC610D30');
			path = catapult.utils.convert.hexToUint8('02396A7E61AE1B1C4C66BD6850C2951C77044CFA5BD0');
			expect(catapult.utils.convert.uint8ToHex(merkleTree.encodePath(path, 43, true)))
				.to.equal('302396A7E61AE1B1C4C66BD6850C2951C77044CFA5BD');
		});

		it('parseBranch', () => {
			const branch = '8080DA9B4AF63BE985715EA635AF98E3CF3B0A22F9A2BE1C7DD40B79948AA63E'
                + '36586E5D2E9D0C089C1C64BC0D42A11ADBD1CD6CDB4B7C294062F55113525A64AE3C';
			merkleTree = new MerkleTree();
			const { tree } = merkleTree;
			merkleTree.parseBranch(catapult.utils.convert.hexToUint8(branch), new Uint8Array(), 0);
			expect(tree.length).to.equal(1);
			expect(tree[0].type).to.equal(0);
			expect(tree[0].linkMask).to.equal('8080');
			expect(tree[0].links.length).to.equal(2);
			expect(tree[0].links[0].bit).to.equal('7');
			expect(tree[0].links[0].link).to.equal('DA9B4AF63BE985715EA635AF98E3CF3B0A22F9A2BE1C7DD40B79948AA63E3658');
			expect(tree[0].links[1].bit).to.equal('F');
			expect(tree[0].links[1].link).to.equal('6E5D2E9D0C089C1C64BC0D42A11ADBD1CD6CDB4B7C294062F55113525A64AE3C');
		});

		it('getBranchHash', () => {
			const encodedPath = '00';
			const links = [
				{
					bit: '6',
					link: '65DA42890A39521AF4F06A5C7A96AC14C3B70611C671FF7E00B401A0C7110ECB'
				},
				{
					bit: 'C',
					link: 'D0E3CD07177472CC42C7DBB645C15854D06F150E11B7D3C73AB05A53AFB88C07'
				},
				{
					bit: 'D',
					link: '1CB226DA02F7748A47D40798316418341991E4AF2D83E1EDDAECA55F1D00DD59'
				}
			];
			merkleTree = new MerkleTree();
			const branchHash = merkleTree.getBranchHash(encodedPath, links);
			expect(branchHash).to.equal('EA1E622F7655215A1A3D4C4FA112155BD8AE3AAF2E2C62899763A45D7E91C89C');
		});

		it('getLeafHash', () => {
			const encodedPath = '20B0D86F48CC652CA98B5A463EDF4849AC6D6CB431EDE4DFBA0698A26D4106D7';
			const value = 'EFB918C83096F8013736864B6914B95F8BB1892D448AED557BD852D18B261EE1';
			merkleTree = new MerkleTree();
			const leafHash = merkleTree.getLeafHash(encodedPath, value);
			expect(leafHash).to.equal('65DA42890A39521AF4F06A5C7A96AC14C3B70611C671FF7E00B401A0C7110ECB');
		});

		it('parseLeaf', () => {
			const leaf = '0C3EAFF635F01BB3B474F0AF1BE99FBDA85EEFB209CC7BD158D3540DE3A3F2D1';
			merkleTree = new MerkleTree();
			const { tree } = merkleTree;
			merkleTree.parseLeaf(catapult.utils.convert.hexToUint8(leaf),
				catapult.utils.convert.hexToUint8('04A7F2A487B42EA89323C4408F82415223ACFEC7DFA7924EFC31A70778AB17A0'), 63);
			expect(tree.length).to.equal(1);
			expect(tree[0].type).to.equal(255);
			expect(tree[0].linkMask).to.equal(undefined);
			expect(tree[0].path).to.equal('04A7F2A487B42EA89323C4408F82415223ACFEC7DFA7924EFC31A70778AB17A0');
			expect(tree[0].value).to.equal('0C3EAFF635F01BB3B474F0AF1BE99FBDA85EEFB209CC7BD158D3540DE3A3F2D1');
		});

		it('parseRaw', () => {
			const raw = '00008080DA9B4AF63BE985715EA635AF98E3CF3B0A22F9A2BE1C7DD40B79948AA63E36586E5D2E9D0C089C'
                + '1C64BC0D42A11ADBD1CD6CDB4B7C294062F55113525A64AE3CFF3F04A7F2A487B42EA89323C4408F82415223ACFEC7D'
                + 'FA7924EFC31A70778AB17A00C3EAFF635F01BB3B474F0AF1BE99FBDA85EEFB209CC7BD158D3540DE3A3F2D1';
			merkleTree = new MerkleTree();
			const { tree } = merkleTree;
			merkleTree.parseMerkleTreeFromRaw(catapult.utils.convert.hexToUint8(raw));
			expect(tree.length).to.equal(2);
			expect(tree[0].type).to.equal(0);
			expect(tree[0].linkMask).to.equal('8080');
			expect(tree[0].links.length).to.equal(2);
			expect(tree[0].links[0].bit).to.equal('7');
			expect(tree[0].links[0].link).to.equal('DA9B4AF63BE985715EA635AF98E3CF3B0A22F9A2BE1C7DD40B79948AA63E3658');
			expect(tree[0].links[1].bit).to.equal('F');
			expect(tree[0].links[1].link).to.equal('6E5D2E9D0C089C1C64BC0D42A11ADBD1CD6CDB4B7C294062F55113525A64AE3C');

			expect(tree[1].type).to.equal(255);
			expect(tree[1].linkMask).to.equal(undefined);
			expect(tree[1].path).to.equal('04A7F2A487B42EA89323C4408F82415223ACFEC7DFA7924EFC31A70778AB17A0');
			expect(tree[1].value).to.equal('0C3EAFF635F01BB3B474F0AF1BE99FBDA85EEFB209CC7BD158D3540DE3A3F2D1');
		});

		it('parseRaw -  multiple branches', () => {
			const raw = '0000FFFF5EC7D52423FC5C8BC0F8F15FC79C16193A852B32232008D15B3CEDDD97FC1E9F62C6805349E0'
                + 'C741E92D4693E98AA6D5CDE12ADE901E1B6E19FA5C55DA8B8628841F3802703D6B7B233542DFDBBC11F4DE72BCDE'
                + '62BEDBD64D2D6BA220D232462422162DFE1D04C9E9272ECC9F2B8EB37BF09A6C12C2DA0FE588D045E6DDDFBE74CE'
                + '2D046699B19A822AB233005596CCEF93989652B33ED7E909ADDDB33AD02886498B57680DBE4517257D5CF544D052'
                + '67D1EC516035740AF3515346E80CEA4F7D195DAD3163B925B21C3AF60CC264300E4C339E6BCEF40CB7B39C214D4A'
                + 'CE01666CEAC091FD1AEF8F14F6ABD861C32B07CB107B3EABDE088AF5E82D35D5F9437A08A4A30D0CA54C1308EA0E'
                + '04E3203F20E5A741D8F37EAB21ED33C3DF23F9D7DCC385A12DF001EFA572DDE4E2F201C3DAC8F9FB33ABF05A9B6E'
                + '07B46EB77B8BB762D14A56BEB17DF8DFA68B3A832AB364FC2885044727A09124C53D2EDD980B408AEE9E053F9D61'
                + '8EA792B1BE7736668CFFD23FDA4DA3233403754F188CAAC88B5C5F699AA780DF6CD737059361E4B8E552C0A0C395'
                + '973E4558425D60FF2B5B4C142D7A873258A04E7676A57110201600C50E4EF7A2D5F82C02E74B196924D5A5F7AC91'
                + 'DDD27FB71DFE0C7DDED1FB639C1E6DEC2F207A334A0F5CD6F7834B1C20112E11042D501793B9EF93451C53FAB12C'
                + '5D4A851349C2B288674C75994A4900006BEFDEC85B49EE58F3E2F16575386D82967D27D19657E4C60F17FAAD7B3B'
                + '531DC58E51A04403B2067D2FC9F7CB114C8D42B01540134630B686698F18AE8A88BC961DF808CC6F52274BBFB930'
                + '2E3720AA89DF5EC1743CE6B0406E57075B1C162D4DE824B940727BB626A9469322B91D5B3BB66A73711765A8C5C5'
                + '8BF986EE9DE27B28F27055712854EF6FC0007C89D5EF913F65B2DE33B9644FEE6EB551BA19FACEA2AA54E994728A'
                + '940664D2070D562A2C1A1717EEB895BE612BEBD3B43D2D61B88C599CD2E36DAC9EF6F95A628B8B12A82B0920D257'
                + '1A2AAEA67EA87638BE1102915FED8D7A23CF23488FE3DC8CA646DBBA67924CB543A85728E2A8353CB829FC81971A'
                + 'F7AD00F3AD44EF258BB4D93DD245BC9B53F9AB3E9C23DF688A0E076F7EC8D813BAB9C15503E53A8DF1C2BC68AEBD'
                + 'DD29A136C5CC09367FD87F9C56B77A105AD006E6D43B72E552471BB4B13576A8A7448A6548BB5C03E9AAA0BE7591'
                + 'A193EA95B6BDD4B78132DB94CC70389A62ADD291E5130CC09AFC3E5B5A58A655A14FFF3EDD0A830D85825627C194'
                + 'FD126705843470B23E2B6A223A317BC385CE912E25E7158DBBF548B8B76BA4CD15E895DBA9429E9A4D6C1226AB9D'
                + '53E91D911671A7';

			merkleTree = new MerkleTree();
			const { tree } = merkleTree;
			merkleTree.parseMerkleTreeFromRaw(catapult.utils.convert.hexToUint8(raw));
			expect(tree.length).to.equal(3);
			expect(tree[0].type).to.equal(0);
			expect(tree[0].linkMask).to.equal('FFFF');
			expect(tree[0].links.length).to.equal(16);
			expect(tree[0].links[0].bit).to.equal('0');
			expect(tree[0].links[0].link).to.equal('5EC7D52423FC5C8BC0F8F15FC79C16193A852B32232008D15B3CEDDD97FC1E9F');
			expect(tree[0].links[1].bit).to.equal('1');
			expect(tree[0].links[1].link).to.equal('62C6805349E0C741E92D4693E98AA6D5CDE12ADE901E1B6E19FA5C55DA8B8628');
			expect(tree[0].links[2].bit).to.equal('2');
			expect(tree[0].links[2].link).to.equal('841F3802703D6B7B233542DFDBBC11F4DE72BCDE62BEDBD64D2D6BA220D23246');
			expect(tree[0].links[3].bit).to.equal('3');
			expect(tree[0].links[3].link).to.equal('2422162DFE1D04C9E9272ECC9F2B8EB37BF09A6C12C2DA0FE588D045E6DDDFBE');
			expect(tree[0].links[4].bit).to.equal('4');
			expect(tree[0].links[4].link).to.equal('74CE2D046699B19A822AB233005596CCEF93989652B33ED7E909ADDDB33AD028');
			expect(tree[0].links[5].bit).to.equal('5');
			expect(tree[0].links[5].link).to.equal('86498B57680DBE4517257D5CF544D05267D1EC516035740AF3515346E80CEA4F');
			expect(tree[0].links[6].bit).to.equal('6');
			expect(tree[0].links[6].link).to.equal('7D195DAD3163B925B21C3AF60CC264300E4C339E6BCEF40CB7B39C214D4ACE01');
			expect(tree[0].links[7].bit).to.equal('7');
			expect(tree[0].links[7].link).to.equal('666CEAC091FD1AEF8F14F6ABD861C32B07CB107B3EABDE088AF5E82D35D5F943');
			expect(tree[0].links[8].bit).to.equal('8');
			expect(tree[0].links[8].link).to.equal('7A08A4A30D0CA54C1308EA0E04E3203F20E5A741D8F37EAB21ED33C3DF23F9D7');
			expect(tree[0].links[9].bit).to.equal('9');
			expect(tree[0].links[9].link).to.equal('DCC385A12DF001EFA572DDE4E2F201C3DAC8F9FB33ABF05A9B6E07B46EB77B8B');
			expect(tree[0].links[10].bit).to.equal('A');
			expect(tree[0].links[10].link).to.equal('B762D14A56BEB17DF8DFA68B3A832AB364FC2885044727A09124C53D2EDD980B');
			expect(tree[0].links[11].bit).to.equal('B');
			expect(tree[0].links[11].link).to.equal('408AEE9E053F9D618EA792B1BE7736668CFFD23FDA4DA3233403754F188CAAC8');
			expect(tree[0].links[12].bit).to.equal('C');
			expect(tree[0].links[12].link).to.equal('8B5C5F699AA780DF6CD737059361E4B8E552C0A0C395973E4558425D60FF2B5B');
			expect(tree[0].links[13].bit).to.equal('D');
			expect(tree[0].links[13].link).to.equal('4C142D7A873258A04E7676A57110201600C50E4EF7A2D5F82C02E74B196924D5');
			expect(tree[0].links[14].bit).to.equal('E');
			expect(tree[0].links[14].link).to.equal('A5F7AC91DDD27FB71DFE0C7DDED1FB639C1E6DEC2F207A334A0F5CD6F7834B1C');
			expect(tree[0].links[15].bit).to.equal('F');
			expect(tree[0].links[15].link).to.equal('20112E11042D501793B9EF93451C53FAB12C5D4A851349C2B288674C75994A49');

			expect(tree[1].type).to.equal(0);
			expect(tree[1].linkMask).to.equal('EF6B');
			expect(tree[1].links.length).to.equal(12);
			expect(tree[1].links[0].bit).to.equal('0');
			expect(tree[1].links[0].link).to.equal('DEC85B49EE58F3E2F16575386D82967D27D19657E4C60F17FAAD7B3B531DC58E');
			expect(tree[1].links[1].bit).to.equal('1');
			expect(tree[1].links[1].link).to.equal('51A04403B2067D2FC9F7CB114C8D42B01540134630B686698F18AE8A88BC961D');
			expect(tree[1].links[2].bit).to.equal('3');
			expect(tree[1].links[2].link).to.equal('F808CC6F52274BBFB9302E3720AA89DF5EC1743CE6B0406E57075B1C162D4DE8');
			expect(tree[1].links[3].bit).to.equal('5');
			expect(tree[1].links[3].link).to.equal('24B940727BB626A9469322B91D5B3BB66A73711765A8C5C58BF986EE9DE27B28');
			expect(tree[1].links[4].bit).to.equal('6');
			expect(tree[1].links[4].link).to.equal('F27055712854EF6FC0007C89D5EF913F65B2DE33B9644FEE6EB551BA19FACEA2');
			expect(tree[1].links[5].bit).to.equal('8');
			expect(tree[1].links[5].link).to.equal('AA54E994728A940664D2070D562A2C1A1717EEB895BE612BEBD3B43D2D61B88C');
			expect(tree[1].links[6].bit).to.equal('9');
			expect(tree[1].links[6].link).to.equal('599CD2E36DAC9EF6F95A628B8B12A82B0920D2571A2AAEA67EA87638BE110291');
			expect(tree[1].links[7].bit).to.equal('A');
			expect(tree[1].links[7].link).to.equal('5FED8D7A23CF23488FE3DC8CA646DBBA67924CB543A85728E2A8353CB829FC81');
			expect(tree[1].links[8].bit).to.equal('B');
			expect(tree[1].links[8].link).to.equal('971AF7AD00F3AD44EF258BB4D93DD245BC9B53F9AB3E9C23DF688A0E076F7EC8');
			expect(tree[1].links[9].bit).to.equal('D');
			expect(tree[1].links[9].link).to.equal('D813BAB9C15503E53A8DF1C2BC68AEBDDD29A136C5CC09367FD87F9C56B77A10');
			expect(tree[1].links[10].bit).to.equal('E');
			expect(tree[1].links[10].link).to.equal('5AD006E6D43B72E552471BB4B13576A8A7448A6548BB5C03E9AAA0BE7591A193');
			expect(tree[1].links[11].bit).to.equal('F');
			expect(tree[1].links[11].link).to.equal('EA95B6BDD4B78132DB94CC70389A62ADD291E5130CC09AFC3E5B5A58A655A14F');

			expect(tree[2].type).to.equal(255);
			expect(tree[2].linkMask).to.equal(undefined);
			expect(tree[2].path).to.equal('DD0A830D85825627C194FD126705843470B23E2B6A223A317BC385CE912E25');
			expect(tree[2].value).to.equal('E7158DBBF548B8B76BA4CD15E895DBA9429E9A4D6C1226AB9D53E91D911671A7');
		});

		it('parseRaw -  non zero nibbles', () => {
			const raw = '000AB7BB382C56806217BB7F07DD6E5ED7ED72441AC98FDDE6666476BC2DFD4C038BDE0A0FDF650309BBCB7268B70'
                + 'EE39982B7BBC9ED4AA138A20C529B57A8483A7F4140CF2DD707C403B450D28E74BB0D0286E32BE4EAE0871DDDA0F666CA7C06'
                + 'A948AB408D7D3759E0E1D7D8FAD9309D6468CD4D3D7CAE1CEB723F332C16935D3C1DC46120CE532A00093DC610D3001002625'
                + 'F41387807712CDF81178B3311FD269F2D539E4FF8E119C1D9900083E0B8577D6F47C31E28BDADCD84330066B6771B8A666268'
                + 'A0209A37E47525BEA3DFF3E9FF2B02396A7E61AE1B1C4C66BD6850C2951C77044CFA5BD04AAFD429BA7128AAA203D7F148DEE'
                + '82F2D1068C95AB5AF53B27B9C336327E8AA';

			merkleTree = new MerkleTree();
			const { tree } = merkleTree;
			merkleTree.parseMerkleTreeFromRaw(catapult.utils.convert.hexToUint8(raw));
			expect(tree.length).to.equal(3);
			expect(tree[0].type).to.equal(0);
			expect(tree[0].linkMask).to.equal('6280');
			expect(tree[0].path).to.equal('B7BB382C56');
			expect(tree[0].links.length).to.equal(4);
			expect(tree[0].links[0].bit).to.equal('7');
			expect(tree[0].links[0].link).to.equal('17BB7F07DD6E5ED7ED72441AC98FDDE6666476BC2DFD4C038BDE0A0FDF650309');
			expect(tree[0].links[1].bit).to.equal('9');
			expect(tree[0].links[1].link).to.equal('BBCB7268B70EE39982B7BBC9ED4AA138A20C529B57A8483A7F4140CF2DD707C4');
			expect(tree[0].links[2].bit).to.equal('D');
			expect(tree[0].links[2].link).to.equal('03B450D28E74BB0D0286E32BE4EAE0871DDDA0F666CA7C06A948AB408D7D3759');
			expect(tree[0].links[3].bit).to.equal('E');
			expect(tree[0].links[3].link).to.equal('E0E1D7D8FAD9309D6468CD4D3D7CAE1CEB723F332C16935D3C1DC46120CE532A');

			expect(tree[1].type).to.equal(0);
			expect(tree[1].linkMask).to.equal('0210');
			expect(tree[1].path).to.equal('3DC610D300');
			expect(tree[1].links.length).to.equal(2);
			expect(tree[1].links[0].bit).to.equal('4');
			expect(tree[1].links[0].link).to.equal('625F41387807712CDF81178B3311FD269F2D539E4FF8E119C1D9900083E0B857');
			expect(tree[1].links[1].bit).to.equal('9');
			expect(tree[1].links[1].link).to.equal('7D6F47C31E28BDADCD84330066B6771B8A666268A0209A37E47525BEA3DFF3E9');

			expect(tree[2].type).to.equal(255);
			expect(tree[2].linkMask).to.equal(undefined);
			expect(tree[2].path).to.equal('02396A7E61AE1B1C4C66BD6850C2951C77044CFA5BD0');
			expect(tree[2].value).to.equal('4AAFD429BA7128AAA203D7F148DEE82F2D1068C95AB5AF53B27B9C336327E8AA');
		});

		describe('merkle tree parse from testnet examples', () => {
			const treePath = fsPath.resolve(__dirname, '../merkle/trees.json');
			const trees = JSON.parse(fs.readFileSync(treePath, 'UTF-8').trim());
			trees.forEach(tree => {
				const fieldName = Object.keys(tree).filter(n => 'raw' !== n)[0];
				it(`${fieldName} ${tree[fieldName]}`, () => {
					const testnetTree = new MerkleTree();
					testnetTree.parseMerkleTreeFromRaw(catapult.utils.convert.hexToUint8(tree.raw));
					expect(testnetTree.tree.length).to.be.gt(0);
				});
			});
		});
	});
});
