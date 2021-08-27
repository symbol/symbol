import "finalization/finalization_round.cats"

# binary layout for finalized block header
struct FinalizedBlockHeader
	# finalization round
	round = FinalizationRound

	# finalization height
	height = Height

	# finalization hash
	hash = Hash256

