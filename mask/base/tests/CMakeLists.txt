add_executable(query_test Query_test.cc)
#target_link_libraries(query_test mask boost_unit_test_framework)
target_link_libraries(query_test mask)

add_executable(stringutil_test StringUtil_test.cc)
target_link_libraries(stringutil_test mask)

add_executable(process_test Process_test.cc)
target_link_libraries(process_test mask)

add_executable(rapidstream_test RapidStream_test.cc)
target_link_libraries(rapidstream_test mask)

add_executable(tcpserver_test TcpServer_test.cc)
target_link_libraries(tcpserver_test mask)

add_executable(worker_test Worker_test.cc)
target_link_libraries(worker_test mask)

add_executable(url_test Url_test.cc)
target_link_libraries(url_test mask)
