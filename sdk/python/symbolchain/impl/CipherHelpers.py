import secrets

from symbolchain.Cipher import AesCbcCipher, AesGcmCipher

GCM_IV_SIZE = 12
CBC_IV_SIZE = 16
SALT_SIZE = 32


def _decode(tag_or_salt_size, iv_size, encoded_message):
	tag_or_salt = encoded_message[:tag_or_salt_size]
	initialization_vector = encoded_message[tag_or_salt_size:tag_or_salt_size + iv_size]
	encoded_message_data = encoded_message[tag_or_salt_size + iv_size:]

	return tag_or_salt, initialization_vector, encoded_message_data


def decode_aes_gcm(shared_key_class, key_pair, recipient_public_key, encoded_message):
	tag, initialization_vector, encoded_message_data = _decode(AesGcmCipher.TAG_SIZE, GCM_IV_SIZE, encoded_message)

	shared_key = shared_key_class.derive_shared_key(key_pair, recipient_public_key)
	cipher = AesGcmCipher(shared_key)

	return cipher.decrypt(encoded_message_data + tag, initialization_vector)


def decode_aes_cbc(shared_key_class, key_pair, recipient_public_key, encoded_message):
	salt, initialization_vector, encoded_message_data = _decode(SALT_SIZE, CBC_IV_SIZE, encoded_message)

	shared_key = shared_key_class.derive_shared_key_deprecated(key_pair, recipient_public_key, salt)
	cipher = AesCbcCipher(shared_key)

	return cipher.decrypt(encoded_message_data, initialization_vector)


def encode_aes_gcm(shared_key_class, key_pair, recipient_public_key, message):
	shared_key = shared_key_class.derive_shared_key(key_pair, recipient_public_key)
	cipher = AesGcmCipher(shared_key)

	initialization_vector = secrets.token_bytes(GCM_IV_SIZE)
	cipher_text = cipher.encrypt(message, initialization_vector)

	tag_start_offset = len(cipher_text) - AesGcmCipher.TAG_SIZE
	tag = cipher_text[tag_start_offset:]

	return tag, initialization_vector, cipher_text[:tag_start_offset]


def encode_aes_cbc(shared_key_class, key_pair, recipient_public_key, message):
	salt = secrets.token_bytes(SALT_SIZE)
	shared_key = shared_key_class.derive_shared_key_deprecated(key_pair, recipient_public_key, salt)
	cipher = AesCbcCipher(shared_key)

	initialization_vector = secrets.token_bytes(CBC_IV_SIZE)
	cipher_text = cipher.encrypt(message, initialization_vector)

	return salt, initialization_vector, cipher_text
