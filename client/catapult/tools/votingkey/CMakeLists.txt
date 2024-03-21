cmake_minimum_required(VERSION 3.23)

catapult_define_tool(votingkey)
target_link_libraries(catapult.tools.votingkey catapult.finalization)
