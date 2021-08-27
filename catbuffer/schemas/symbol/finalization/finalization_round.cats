import "types.cats"

# binary layout for finalization round
struct FinalizationRound
	# finalization epoch
	epoch = FinalizationEpoch

	# finalization point
	point = FinalizationPoint
