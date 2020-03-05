/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "CertificateLocator.h"
#include "catapult/io/RawFile.h"
#include "catapult/exceptions.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

namespace catapult { namespace test {

	namespace {
		void SaveToFile(const std::string& certDirectory, const std::string& filename, const std::string& buffer) {
			io::RawFile dataFile((boost::filesystem::path(certDirectory) / filename).generic_string(), io::OpenMode::Read_Write);
			dataFile.write({ reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size() });
		}

		void SavePemCertificate(const PemCertificate& pemCertificate, const std::string& certificateDirectory) {
			SaveToFile(certificateDirectory, "ca.pubkey.pem", pemCertificate.caPublicKeyString());
			SaveToFile(certificateDirectory, "node.key.pem", pemCertificate.nodePrivateKeyString());
			SaveToFile(certificateDirectory, "node.full.crt.pem", pemCertificate.certificateChainString());
		}
	}

	std::string GetDefaultCertificateDirectory() {
		auto certificateDirectory = "./cert";
		if (!boost::filesystem::exists(certificateDirectory))
			GenerateCertificateDirectory(certificateDirectory);

		return certificateDirectory;
	}

	void GenerateCertificateDirectory(const std::string& certificateDirectory) {
		GenerateCertificateDirectory(certificateDirectory, PemCertificate());
	}

	void GenerateCertificateDirectory(const std::string& certificateDirectory, const crypto::KeyPair& nodeKeyPair) {
		GenerateCertificateDirectory(certificateDirectory, PemCertificate(nodeKeyPair));
	}

	void GenerateCertificateDirectory(const std::string& certificateDirectory, const PemCertificate& pemCertificate) {
		CATAPULT_LOG(info) << "generating new certificate directory: " << certificateDirectory;
		boost::filesystem::create_directories(certificateDirectory);

		SavePemCertificate(pemCertificate, certificateDirectory);
	}
}}
