import unittest
from binascii import unhexlify
from collections import namedtuple

from cryptography.exceptions import InvalidTag

from symbolchain.Cipher import AesCbcCipher, AesGcmCipher
from symbolchain.CryptoTypes import SharedKey256

from .test.TestUtils import TestUtils

CipherTestCaseDescriptor = namedtuple('CipherTestCaseDescriptor', ['shared_key', 'iv', 'tag', 'cipher_text', 'clear_text'])


# region BasicCipherTest

class BasicCipherTest:
	# pylint: disable=no-member

	def _assert_can_encrypt(self, cipher_class, test_case):
		# Arrange:
		cipher = cipher_class(test_case.shared_key)

		# Act:
		result_text = cipher.encrypt(test_case.clear_text, test_case.iv)

		# Assert:
		self.assertEqual(test_case.cipher_text + test_case.tag, result_text)

	def _assert_can_decrypt(self, cipher_class, test_case):
		# Arrange:
		cipher = cipher_class(test_case.shared_key)

		# Act:
		result_text = cipher.decrypt(test_case.cipher_text + test_case.tag, test_case.iv)

		# Assert:
		self.assertEqual(test_case.clear_text, result_text)

	def test_valid_descriptors(self):
		for i, test_descriptor in enumerate(self.get_test_descriptors()):
			with self.subTest(f'encrypt_{i}'):
				self._assert_can_encrypt(self.CIPHER, test_descriptor)
			with self.subTest(f'decrypt_{i}'):
				self._assert_can_decrypt(self.CIPHER, test_descriptor)

# endregion


# region AesCbcCipher

class AesCbcCipherTest(BasicCipherTest, unittest.TestCase):
	CIPHER = AesCbcCipher

	# test vectors taken from wycheproof project:
	# https://github.com/google/wycheproof/blob/master/testvectors/aes_cbc_pkcs5_test.json
	def get_test_descriptors(self):  # pylint: disable=no-self-use
		return [
			# tcId: 123
			CipherTestCaseDescriptor(
				SharedKey256('7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97'),
				unhexlify('EB38EF61717E1324AE064E86F1C3E797'),
				b'',
				unhexlify('E7C166554D1BB32792C981FA674CC4D8'),
				unhexlify('')
			),
			# tcId: 127
			CipherTestCaseDescriptor(
				SharedKey256('E754076CEAB3FDAF4F9BCAB7D4F0DF0CBBAFBC87731B8F9B7CD2166472E8EEBC'),
				unhexlify('014D2E13DFBCB969BA3BB91442D52ECA'),
				b'',
				unhexlify('42C0B89A706ED2606CD94F9CB361FA51'),
				unhexlify('40')
			),
			# tcId: 125
			CipherTestCaseDescriptor(
				SharedKey256('96E1E4896FB2CD05F133A6A100BC5609A7AC3CA6D81721E922DADD69AD07A892'),
				unhexlify('E70D83A77A2CE722AC214C00837ACEDF'),
				b'',
				unhexlify('A615A39FF8F59F82CF72ED13E1B01E32459700561BE112412961365C7A0B58AA7A16D68C065E77EBE504999051476BD7'),
				unhexlify('91A17E4DFCC3166A1ADD26FF0E7C12056E8A654F28A6DE24F4BA739CEB5B5B18')
			),
		]

	def test_cannot_decrypt_with_with_wrong_iv(self):
		# Arrange:
		test_case = self.get_test_descriptors()[0]
		cipher = self.CIPHER(test_case.shared_key)

		# Act + Assert:
		with self.assertRaises(ValueError):
			cipher.decrypt(test_case.cipher_text, TestUtils.randbytes(len(test_case.iv)))

# endregion


# region AesGcmCipher

class AesGcmCipherTest(BasicCipherTest, unittest.TestCase):
	CIPHER = AesGcmCipher

	def get_test_descriptors(self):  # pylint: disable=no-self-use
		# test vectors taken from wycheproof project:
		# https://github.com/google/wycheproof/blob/master/testvectors/aes_gcm_test.json
		return [
			# tcId: 75
			CipherTestCaseDescriptor(
				SharedKey256('80BA3192C803CE965EA371D5FF073CF0F43B6A2AB576B208426E11409C09B9B0'),
				unhexlify('4DA5BF8DFD5852C1EA12379D'),
				unhexlify('4771A7C404A472966CEA8F73C8BFE17A'),
				b'',
				b''
			),
			# tcId: 76
			CipherTestCaseDescriptor(
				SharedKey256('CC56B680552EB75008F5484B4CB803FA5063EBD6EAB91F6AB6AEF4916A766273'),
				unhexlify('99E23EC48985BCCDEEAB60F1'),
				unhexlify('633C1E9703EF744FFFFB40EDF9D14355'),
				unhexlify('06'),
				unhexlify('2A')
			),
			# tcId: 87
			CipherTestCaseDescriptor(
				SharedKey256('D7ADDD3889FADF8C893EEE14BA2B7EA5BF56B449904869615BD05D5F114CF377'),
				unhexlify('8A3AD26B28CD13BA6504E260'),
				unhexlify('5E63374B519E6C3608321943D790CF9A'),
				unhexlify(
					'53CC8C920A85D1ACCB88636D08BBE4869BFDD96F437B2EC944512173A9C0FE7A'
					'47F8434133989BA77DDA561B7E3701B9A83C3BA7660C666BA59FEF96598EB621'
					'544C63806D509AC47697412F9564EB0A2E1F72F6599F5666AF34CFFCA06573FF'
					'B4F47B02F59F21C64363DAECB977B4415F19FDDA3C9AAE5066A57B669FFAA257'),
				unhexlify(
					'C877A76BF595560772167C6E3BCC705305DB9C6FCBEB90F4FEA85116038BC53C'
					'3FA5B4B4EA0DE5CC534FBE1CF9AE44824C6C2C0A5C885BD8C3CDC906F1267573'
					'7E434B983E1E231A52A275DB5FB1A0CAC6A07B3B7DCB19482A5D3B06A9317A54'
					'826CEA6B36FCE452FA9B5475E2AAF25499499D8A8932A19EB987C903BD8502FE')
			)
		]

	def test_cannot_decrypt_with_with_wrong_iv(self):
		# Arrange:
		test_case = self.get_test_descriptors()[0]
		cipher = self.CIPHER(test_case.shared_key)

		# Act + Assert:
		with self.assertRaises(InvalidTag):
			cipher.decrypt(test_case.cipher_text + test_case.tag, TestUtils.randbytes(len(test_case.iv)))

	def test_cannot_decrypt_with_with_wrong_tag(self):
		# Arrange:
		test_case = self.get_test_descriptors()[0]
		cipher = self.CIPHER(test_case.shared_key)

		# Act + Assert:
		with self.assertRaises(InvalidTag):
			cipher.decrypt(test_case.cipher_text + TestUtils.randbytes(len(test_case.tag)), test_case.iv)

# endregion
