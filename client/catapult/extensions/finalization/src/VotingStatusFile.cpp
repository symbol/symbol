/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "VotingStatusFile.h"
#include "catapult/io/PodIoUtils.h"
#include <filesystem>

namespace catapult {
namespace finalization {

    VotingStatusFile::VotingStatusFile(const std::string& filename)
        : m_filename(filename)
    {
    }

    chain::VotingStatus VotingStatusFile::load() const
    {
        chain::VotingStatus status;
        if (!std::filesystem::is_regular_file(m_filename)) {
            // if file doesn't exist, start at the first post-nemesis epoch
            status.Round = { FinalizationEpoch(2), FinalizationPoint(1) };
        } else {
            auto rawFile = open(io::OpenMode::Read_Only);
            status.Round.Epoch = FinalizationEpoch(io::Read32(rawFile));
            status.Round.Point = FinalizationPoint(io::Read32(rawFile));
            status.HasSentPrevote = !!io::Read8(rawFile);
            status.HasSentPrecommit = !!io::Read8(rawFile);
        }

        return status;
    }

    void VotingStatusFile::save(const chain::VotingStatus& status)
    {
        auto rawFile = open(io::OpenMode::Read_Write);
        io::Write32(rawFile, status.Round.Epoch.unwrap());
        io::Write32(rawFile, status.Round.Point.unwrap());
        io::Write8(rawFile, status.HasSentPrevote ? 1 : 0);
        io::Write8(rawFile, status.HasSentPrecommit ? 1 : 0);
    }

    io::RawFile VotingStatusFile::open(io::OpenMode mode) const
    {
        return io::RawFile(m_filename, mode);
    }
}
}
