cmake_minimum_required(VERSION 3.23)

catapult_define_tool(network)
target_link_libraries(catapult.tools.network catapult.nodediscovery)
