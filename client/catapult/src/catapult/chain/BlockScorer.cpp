#include "BlockScorer.h"
#include "catapult/model/Block.h"
#include "catapult/model/ImportanceHeight.h"

namespace catapult { namespace chain {

	namespace {
		constexpr uint64_t Two_To_54 = 18014398509481984;

		const double Two_To_256 = std::pow(2, 256);

		constexpr utils::TimeSpan TimeBetweenBlocks(const model::Block& parent, const model::Block& block) {
			return utils::TimeSpan::FromDifference(block.Timestamp, parent.Timestamp);
		}
	}

	uint64_t CalculateHit(const Hash256& generationHash) {
		// 1. v1 = generation-hash
		BlockTarget value;
		boost::multiprecision::import_bits(value, generationHash.cbegin(), generationHash.cend());

		// 2. temp = double(v1) / 2^256
		auto temp = value.convert_to<double>() / Two_To_256;

		// 3. temp = abs(log(temp))
		temp = std::abs(std::log(temp));
		if (std::isinf(temp))
			return std::numeric_limits<uint64_t>::max();

		// 4. r = temp * 2^54
		return static_cast<uint64_t>(temp * Two_To_54);
	}

	uint64_t CalculateScore(const model::Block& parentBlock, const model::Block& currentBlock) {
		if (currentBlock.Timestamp <= parentBlock.Timestamp)
			return 0u;

		// r = difficulty(1) - (t(1) - t(0)) / MS_In_S
		auto timeDiff = TimeBetweenBlocks(parentBlock, currentBlock);
		return currentBlock.Difficulty.unwrap() - timeDiff.seconds();
	}

	namespace {
		BlockTarget GetMultiplier(uint64_t timeDiff, const model::BlockChainConfiguration& config) {
			auto targetTime = config.BlockGenerationTargetTime.seconds();
			double smoother = 1.0;
			if (0 != config.BlockTimeSmoothingFactor) {
				double factor = config.BlockTimeSmoothingFactor / 1000.0;
				smoother = std::min(std::exp(factor * static_cast<int64_t>(timeDiff - targetTime) / targetTime), 100.0);
			}

			BlockTarget target(static_cast<uint64_t>(Two_To_54 * smoother));
			target <<= 10;
			return target;
		}
	}

	BlockTarget CalculateTarget(
			const utils::TimeSpan& timeSpan,
			Difficulty difficulty,
			Importance signerImportance,
			const model::BlockChainConfiguration& config) {
		BlockTarget target = timeSpan.seconds();
		target *= signerImportance.unwrap();
		target *= GetMultiplier(timeSpan.seconds(), config);
		target /= difficulty.unwrap();
		return target;
	}

	BlockTarget CalculateTarget(
			const model::Block& parentBlock,
			const model::Block& currentBlock,
			Importance signerImportance,
			const model::BlockChainConfiguration& config) {
		if (currentBlock.Timestamp <= parentBlock.Timestamp)
			return BlockTarget(0);

		auto timeDiff = TimeBetweenBlocks(parentBlock, currentBlock);
		return CalculateTarget(
				timeDiff,
				currentBlock.Difficulty,
				signerImportance,
				config);
	}

	BlockHitPredicate::BlockHitPredicate(
			const model::BlockChainConfiguration& config,
			const ImportanceLookupFunc& importanceLookup)
			: m_config(config)
			, m_importanceLookup(importanceLookup)
	{}

	bool BlockHitPredicate::operator()(const model::Block& parentBlock, const model::Block& block, const Hash256& generationHash) const {
		auto importance = m_importanceLookup(block.Signer, block.Height);
		auto hit = CalculateHit(generationHash);
		auto target = CalculateTarget(parentBlock, block, importance, m_config);
		return hit < target;
	}

	bool BlockHitPredicate::operator()(const BlockHitContext& context) const {
		auto importance = m_importanceLookup(context.Signer, context.Height);
		auto hit = CalculateHit(context.GenerationHash);
		auto target = CalculateTarget(context.ElapsedTime, context.Difficulty, importance, m_config);
		return hit < target;
	}
}}
